/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: banking.c 1106 2007-01-09 21:14:59Z martin $
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/* This file is included by banking.c */


int AB_Banking_ExecutionProgress(AB_BANKING *ab, uint32_t pid) {
  if (!ab->currentJobs)
    return 0;
  else {
    AB_JOB_LIST2_ITERATOR *jit;
    uint32_t count=0;

    jit=AB_Job_List2_First(ab->currentJobs);
    if (jit) {
      AB_JOB *j;

      j=AB_Job_List2Iterator_Data(jit);
      while(j) {
        AB_JOB_STATUS jst;

        jst=AB_Job_GetStatus(j);
        if (jst==AB_Job_StatusFinished ||
            jst==AB_Job_StatusPending ||
            jst==AB_Job_StatusError)
          count++;
        j=AB_Job_List2Iterator_Next(jit);
      } /* while */
      AB_Job_List2Iterator_free(jit);
    }
    return GWEN_Gui_ProgressAdvance(pid, count);
  }
}



int AB_Banking__ExecuteQueue(AB_BANKING *ab,
                             AB_JOB_LIST2 *jl,
                             AB_IMEXPORTER_CONTEXT *ctx,
                             uint32_t pid){
  AB_PROVIDER *pro;
  int succ;

  assert(ab);
  pro=AB_Provider_List_First(ab->providers);
  succ=0;

  ab->currentJobs=jl;

  while(pro) {
    AB_JOB_LIST2_ITERATOR *jit;
    int jobs=0;
    int rv;

    jit=AB_Job_List2_First(jl);
    if (jit) {
      AB_JOB *j;

      j=AB_Job_List2Iterator_Data(jit);
      while(j) {
	AB_JOB_STATUS jst;

	jst=AB_Job_GetStatus(j);
	DBG_INFO(AQBANKING_LOGDOMAIN, "Checking job...");
	if (jst==AB_Job_StatusEnqueued ||
	    jst==AB_Job_StatusPending) {
	  AB_ACCOUNT *a;

	  a=AB_Job_GetAccount(j);
	  assert(a);
	  if (AB_Account_GetProvider(a)==pro) {
	    DBG_INFO(AQBANKING_LOGDOMAIN, "Same provider, adding job");
	    /* same provider, add job */
	    AB_Job_Log(j, GWEN_LoggerLevel_Info, "aqbanking",
		       "Adding job to backend");
	    rv=AB_Provider_AddJob(pro, j, pid);
	    if (rv) {
	      DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not add job (%d)", rv);
	      AB_Job_SetStatus(j, AB_Job_StatusError);
	      AB_Job_SetResultText(j, "Refused by backend");
	      AB_Job_Log(j, GWEN_LoggerLevel_Error, "aqbanking",
			 "Adding job: Refused by backend");
	    }
	    else {
	      jobs++;
	      if (AB_Job_GetStatus(j)!=AB_Job_StatusPending) {
		AB_Job_SetStatus(j, AB_Job_StatusSent);
	      }
	    }
	  }
	} /* if job enqueued */
	else {
          DBG_DEBUG(AQBANKING_LOGDOMAIN,
                    "Job %08x in queue with status \"%s\"",
                    AB_Job_GetJobId(j),
                    AB_Job_Status2Char(AB_Job_GetStatus(j)));
        }
        j=AB_Job_List2Iterator_Next(jit);
      } /* while */
      AB_Job_List2Iterator_free(jit);
    }

    if (jobs) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Letting backend \"%s\" work",
                 AB_Provider_GetName(pro));
      rv=AB_Provider_Execute(pro, ctx, pid);
      if (rv) {
        int lrv;

	if (rv==GWEN_ERROR_USER_ABORTED) {
          DBG_INFO(AQBANKING_LOGDOMAIN, "Aborted by user");
          ab->currentJobs=0;
          return rv;
        }
        DBG_NOTICE(AQBANKING_LOGDOMAIN, "Error executing backend's queue");
        lrv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
				GWEN_GUI_MSG_FLAGS_CONFIRM_B1 |
				GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL,
				I18N("Error"),
				I18N("Error executing backend's queue.\n"
				     "What shall we do?"),
				I18N("Continue"), I18N("Abort"), 0,
				pid);
	if (lrv!=1) {
	  DBG_INFO(AQBANKING_LOGDOMAIN, "Aborted by user");
	  ab->currentJobs=0;
	  return GWEN_ERROR_USER_ABORTED;
	}
      }
      else {
	rv=AB_Banking_ExecutionProgress(ab, pid);
	if (rv==GWEN_ERROR_USER_ABORTED) {
	  DBG_INFO(AQBANKING_LOGDOMAIN, "Aborted by user");
	  ab->currentJobs=0;
	  return rv;
	}
	succ++;
      }
    } /* if jobs in backend's queue */

    pro=AB_Provider_List_Next(pro);
  } /* while */
  ab->currentJobs=0;

  if (!succ) {
    DBG_WARN(AQBANKING_LOGDOMAIN, "Not a single job successfully executed");
    /* don't return an error here, because at least when retrieving the
     * list of allowed iTAN modes there will most definately be an error with
     * the only job in the queue
     */
    /*return GWEN_ERROR_GENERIC;*/
  }

  return 0;
}




int AB_Banking_ExecuteJobs(AB_BANKING *ab, AB_JOB_LIST2 *jl2,
			   AB_IMEXPORTER_CONTEXT *ctx,
			   uint32_t guiid){
  int rv;
  uint32_t pid;
  AB_JOB_LIST2_ITERATOR *jit;
  AB_PROVIDER *pro=0;

  assert(ab);

  DBG_DEBUG(AQBANKING_LOGDOMAIN, "Attaching to jobs, dequeing them");
  jit=AB_Job_List2_First(jl2);
  if (jit) {
    AB_JOB *j;

    j=AB_Job_List2Iterator_Data(jit);
    while(j) {
      AB_Job_SetStatus(j, AB_Job_StatusEnqueued);
      j=AB_Job_List2Iterator_Next(jit);
    } /* while */
    AB_Job_List2Iterator_free(jit);
  }

  /* clear temporarily accepted certificates */
  GWEN_DB_ClearGroup(ab->dbTempConfig, "certificates");

  /* execute jobs */
  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS |
			     GWEN_GUI_PROGRESS_SHOW_PROGRESS |
			     GWEN_GUI_PROGRESS_SHOW_LOG |
			     GWEN_GUI_PROGRESS_ALWAYS_SHOW_LOG |
			     GWEN_GUI_PROGRESS_KEEP_OPEN |
			     GWEN_GUI_PROGRESS_SHOW_ABORT,
			     I18N("Executing Jobs"),
			     I18N("Now the jobs are send via their "
				  "backends to the credit institutes."),
			     AB_Job_List2_GetSize(jl2),
			     guiid);
  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Notice,
		       I18N("Sending jobs to the bank(s)"));
  rv=AB_Banking__ExecuteQueue(ab, jl2, ctx, pid);
  AB_Banking_ClearCryptTokenList(ab, guiid);
  if (rv) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
  }

  /* clear temporarily accepted certificates again */
  GWEN_DB_ClearGroup(ab->dbTempConfig, "certificates");

  /* clear queue */
  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Notice,
		       I18N("Postprocessing jobs"));
  jit=AB_Job_List2_First(jl2);
  if (jit) {
    AB_JOB *j;

    j=AB_Job_List2Iterator_Data(jit);
    while(j) {
      switch(AB_Job_GetStatus(j)) {
      case AB_Job_StatusEnqueued:
	/* job still enqueued, so it has never been sent */
	AB_Job_SetStatus(j, AB_Job_StatusError);
	AB_Job_SetResultText(j, "Job has never been sent");
	AB_Job_Log(j, GWEN_LoggerLevel_Error, "aqbanking",
		   "Job has never been sent");
	break;
  
      case AB_Job_StatusPending:
	AB_Job_Log(j, GWEN_LoggerLevel_Notice, "aqbanking",
		   "Job is still pending");
	break;
  
      case AB_Job_StatusSent:
      case AB_Job_StatusFinished:
      case AB_Job_StatusError:
      default:
	AB_Job_Log(j, GWEN_LoggerLevel_Info, "aqbanking",
		   "Job finished");
	break;
      }

      j=AB_Job_List2Iterator_Next(jit);
    } /* while */
    AB_Job_List2Iterator_free(jit);
  }

  /* reset all provider queues, this makes sure no job remains in any queue */
  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Notice,
		       I18N("Resetting provider queues"));
  pro=AB_Provider_List_First(ab->providers);
  while(pro) {
    int lrv;

    lrv=AB_Provider_ResetQueue(pro);
    if (lrv) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Error resetting providers queue (%d)",
               lrv);
    }
    pro=AB_Provider_List_Next(pro);
  } /* while */

  GWEN_Gui_ProgressEnd(pid);

  return rv;
}



int AB_Banking_ActivateProvider(AB_BANKING *ab, const char *pname) {
  AB_PROVIDER *pro;

  if (GWEN_StringList_HasString(ab->activeProviders, pname)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Provider already active");
    return GWEN_ERROR_FOUND;
  }

  pro=AB_Banking_GetProvider(ab, pname);
  if (!pro) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not load backend \"%s\"", pname);
    return GWEN_ERROR_NOT_FOUND;
  }

  GWEN_StringList_AppendString(ab->activeProviders, pname, 0, 1);
  return 0;
}



int AB_Banking_DeactivateProvider(AB_BANKING *ab, const char *pname) {
  AB_ACCOUNT *a;
  AB_USER *u;
  AB_PROVIDER *pro;

  if (!GWEN_StringList_HasString(ab->activeProviders, pname)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Provider not active");
    return GWEN_ERROR_INVALID;
  }

  pro=AB_Banking_FindProvider(ab, pname);
  if (pro)
    AB_Banking_FiniProvider(ab, pro);

  GWEN_StringList_RemoveString(ab->activeProviders, pname);

  /* delete accounts which use this backend */
  a=AB_Account_List_First(ab->accounts);
  while(a) {
    AB_ACCOUNT *na;

    na=AB_Account_List_Next(a);
    pro=AB_Account_GetProvider(a);
    assert(pro);
    if (strcasecmp(AB_Provider_GetName(pro), pname)==0) {
      AB_Account_List_Del(a);
      AB_Account_free(a);
    }
    a=na;
  }

  /* delete users which use this backend */
  u=AB_User_List_First(ab->users);
  while(u) {
    AB_USER *nu;
    const char *s;

    nu=AB_User_List_Next(u);
    s=AB_User_GetBackendName(u);
    assert(s && *s);
    if (strcasecmp(s, pname)==0) {
      AB_User_List_Del(u);
      AB_User_free(u);
    }
    u=nu;
  }

  return 0;
}



int AB_Banking_IsProviderActive(AB_BANKING *ab, const char *backend){
  AB_PROVIDER *pro;

  pro=AB_Banking_FindProvider(ab, backend);
  if (!pro) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider \"%s\" not found", backend);
    return 0;
  }

  return AB_Provider_IsInit(pro);
}



const GWEN_STRINGLIST *AB_Banking_GetActiveProviders(const AB_BANKING *ab) {
  assert(ab);
  if (GWEN_StringList_Count(ab->activeProviders)==0)
    return 0;
  return ab->activeProviders;
}



AB_PROVIDER *AB_Banking__LoadProviderPlugin(AB_BANKING *ab,
                                            const char *modname){
  GWEN_PLUGIN *pl;

  pl=GWEN_PluginManager_GetPlugin(ab_pluginManagerProvider, modname);
  if (pl) {
    AB_PROVIDER *pro;

    pro=AB_Plugin_Provider_Factory(pl, ab);
    if (!pro) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Error in plugin [%s]: No provider created",
		modname);
      return NULL;
    }
    return pro;
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Plugin [%s] not found", modname);
    return NULL;
  }
}



int AB_Banking_FindDebugger(AB_BANKING *ab,
			    const char *backend,
			    const char *frontends,
			    GWEN_BUFFER *pbuf){
  GWEN_PLUGIN_DESCRIPTION_LIST2 *pl;
  char *s;
  char *p;

  pl=AB_Banking_GetDebuggerDescrs(ab, backend);
  if (!pl) {
    DBG_WARN(AQBANKING_LOGDOMAIN,
	     "No debuggers available for backend \"%s\"", backend);
    return -1;
  }

  if (frontends==0) {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *pit;
    GWEN_PLUGIN_DESCRIPTION *pd;
    const char *name;

    pit=GWEN_PluginDescription_List2_First(pl);
    assert(pit);
    pd=GWEN_PluginDescription_List2Iterator_Data(pit);
    while(pd) {
      name=GWEN_PluginDescription_GetName(pd);
      if (!name) {
	DBG_WARN(AQBANKING_LOGDOMAIN,
		 "Found a plugin description with no name");
      }
      else {
	int rv;

	rv=AB_Banking__GetDebuggerPath(ab, backend, pbuf);
	if (rv) {
	  DBG_INFO(AQBANKING_LOGDOMAIN, "here");
	  return rv;
	}
	GWEN_Buffer_AppendString(pbuf, DIRSEP);
	GWEN_Buffer_AppendString(pbuf, name);
	GWEN_PluginDescription_List2Iterator_free(pit);
	GWEN_PluginDescription_List2_freeAll(pl);
	return 0;
      }
      pd=GWEN_PluginDescription_List2Iterator_Next(pit);
    }
    GWEN_PluginDescription_List2Iterator_free(pit);
  } /* if no frontend list */

  /* check for every given frontend */
  s=strdup(frontends);

  p=s;
  while(*p) {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *pit;
    GWEN_PLUGIN_DESCRIPTION *pd;
    char *t;

    t=strchr(p, ';');
    if (t)
      *(t++)=0;

    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Trying frontend \"%s\"", p);

    pit=GWEN_PluginDescription_List2_First(pl);
    assert(pit);
    pd=GWEN_PluginDescription_List2Iterator_Data(pit);
    assert(pd);
    while(pd) {
      GWEN_XMLNODE *n;
      const char *fr;

      n=GWEN_PluginDescription_GetXmlNode(pd);
      assert(n);
      fr=GWEN_XMLNode_GetProperty(n, "frontend", "");
      if (-1!=GWEN_Text_ComparePattern(fr, p, 0)) {
	const char *name;

	name=GWEN_PluginDescription_GetName(pd);
	if (!name) {
	  DBG_WARN(AQBANKING_LOGDOMAIN,
		   "Found a plugin description with no name");
	}
	else {
	  int rv;

	  rv=AB_Banking__GetDebuggerPath(ab, backend, pbuf);
	  if (rv) {
	    DBG_INFO(AQBANKING_LOGDOMAIN, "here");
	    return rv;
	  }
	  GWEN_Buffer_AppendString(pbuf, DIRSEP);
	  GWEN_Buffer_AppendString(pbuf, name);
          free(s);
	  GWEN_PluginDescription_List2Iterator_free(pit);
	  GWEN_PluginDescription_List2_freeAll(pl);
	  return 0;
	}
      }
      pd=GWEN_PluginDescription_List2Iterator_Next(pit);
    } /* while pd */
    GWEN_PluginDescription_List2Iterator_free(pit);

    if (!t)
      break;
    p=t;
  } /* while */

  free(s);
  GWEN_PluginDescription_List2_freeAll(pl);
  DBG_ERROR(AQBANKING_LOGDOMAIN, "No matching debugger found");
  return -1;
}



int AB_Banking_FindWizard(AB_BANKING *ab,
                          const char *backend,
			  const char *frontends,
			  GWEN_BUFFER *pbuf){
  GWEN_PLUGIN_DESCRIPTION_LIST2 *pl;
  char *s;
  char *pfront;
  assert(ab);
  assert(pbuf);

  pl=AB_Banking_GetWizardDescrs(ab);
  if (!pl) {
    DBG_WARN(AQBANKING_LOGDOMAIN,
	     "No wizards available.");
    return -1;
  }

  if (frontends==0) {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *pit;
    GWEN_PLUGIN_DESCRIPTION *pd;
    const char *name;

    pit=GWEN_PluginDescription_List2_First(pl);
    assert(pit);
    pd=GWEN_PluginDescription_List2Iterator_Data(pit);
    while(pd) {
      name=GWEN_PluginDescription_GetName(pd);
      if (!name) {
	DBG_WARN(AQBANKING_LOGDOMAIN,
		 "Found a plugin description with no name.");
      }
      else {
	GWEN_STRINGLIST *sl;
	const char *wizard_folder;

	sl=GWEN_PathManager_GetPaths(AB_PM_LIBNAME, AB_PM_WIZARDDIR);
	/* Out of laziness we simply use the first path. */
	wizard_folder = GWEN_StringList_FirstString(sl);
        GWEN_Buffer_AppendString(pbuf, wizard_folder);
	GWEN_StringList_free(sl);

        GWEN_Buffer_AppendString(pbuf, DIRSEP);
        GWEN_Buffer_AppendString(pbuf, name);
	/* For windows, we need the exe extension as well */
	if (strlen(EXEEXT) > 0)
	  GWEN_Buffer_AppendString(pbuf, EXEEXT);
	GWEN_PluginDescription_List2Iterator_free(pit);
	GWEN_PluginDescription_List2_freeAll(pl);
	return 0;
      }
      pd=GWEN_PluginDescription_List2Iterator_Next(pit);
    }
    GWEN_PluginDescription_List2Iterator_free(pit);
  } /* if no frontend list */

  /* check for every given frontend */
  s=strdup(frontends);

  pfront=s;
  while(*pfront) {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *pit;
    GWEN_PLUGIN_DESCRIPTION *pd;
    char *t;

    t=strchr(pfront, ';');
    if (t)
      *(t++)=0;

    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Trying frontend \"%s\"", pfront);

    pit=GWEN_PluginDescription_List2_First(pl);
    assert(pit);
    pd=GWEN_PluginDescription_List2Iterator_Data(pit);
    assert(pd);
    while(pd) {
      GWEN_XMLNODE *n;
      const char *fr;

      n=GWEN_PluginDescription_GetXmlNode(pd);
      assert(n);
      fr=GWEN_XMLNode_GetProperty(n, "frontend", "");
      if (-1!=GWEN_Text_ComparePattern(fr, pfront, 0)) {
	const char *name;

	name=GWEN_PluginDescription_GetName(pd);
	if (!name) {
	  DBG_WARN(AQBANKING_LOGDOMAIN,
		   "Found a plugin description with no name");
	}
	else {
	  GWEN_STRINGLIST *sl;
	  const char *wizard_folder;

	  sl=GWEN_PathManager_GetPaths(AB_PM_LIBNAME, AB_PM_WIZARDDIR);
	  /* Out of laziness we simply use the first path. */
	  wizard_folder = GWEN_StringList_FirstString(sl);
	  GWEN_Buffer_AppendString(pbuf, wizard_folder);
	  GWEN_StringList_free(sl);

	  GWEN_Buffer_AppendString(pbuf, DIRSEP);
          GWEN_Buffer_AppendString(pbuf, name);
	  /* For windows, we need the exe extension as well */
	  if (strlen(EXEEXT) > 0)
	    GWEN_Buffer_AppendString(pbuf, EXEEXT);
          free(s);
	  GWEN_PluginDescription_List2Iterator_free(pit);
	  GWEN_PluginDescription_List2_freeAll(pl);
	  return 0;
	}
      }
      pd=GWEN_PluginDescription_List2Iterator_Next(pit);
    } /* while pd */
    GWEN_PluginDescription_List2Iterator_free(pit);

    if (!t)
      break;
    pfront=t;
  } /* while */

  free(s);
  GWEN_PluginDescription_List2_freeAll(pl);
  DBG_ERROR(AQBANKING_LOGDOMAIN, "No matching wizard found");
  return -1;
}



GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetProviderDescrs(AB_BANKING *ab){
  GWEN_PLUGIN_DESCRIPTION_LIST2 *l;
  GWEN_PLUGIN_MANAGER *pm;
 
  pm = GWEN_PluginManager_FindPluginManager("provider");
  if (!pm) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not find plugin manager for \"%s\"",
	      "provider");
    return 0;
  }

  l = GWEN_PluginManager_GetPluginDescrs(pm);
  if (l) {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *it;
    GWEN_PLUGIN_DESCRIPTION *pd;

    it=GWEN_PluginDescription_List2_First(l);
    assert(it);
    pd=GWEN_PluginDescription_List2Iterator_Data(it);
    assert(pd);
    while(pd) {
      if (GWEN_StringList_HasString(ab->activeProviders,
                                    GWEN_PluginDescription_GetName(pd)))
        GWEN_PluginDescription_SetIsActive(pd, 1);
      else
        GWEN_PluginDescription_SetIsActive(pd, 0);
      pd=GWEN_PluginDescription_List2Iterator_Next(it);
    }
    GWEN_PluginDescription_List2Iterator_free(it);
  }

  return l;
}



GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetWizardDescrs(AB_BANKING *ab){
  GWEN_PLUGIN_DESCRIPTION_LIST2 *wdl;

  GWEN_STRINGLIST *sl;
  const char *wizard_folder;

  sl=GWEN_PathManager_GetPaths(AB_PM_LIBNAME, AB_PM_WIZARDDIR);
  /* Out of laziness we simply use the first path. */
  wizard_folder = GWEN_StringList_FirstString(sl);

  wdl=GWEN_LoadPluginDescrs(wizard_folder);

  GWEN_StringList_free(sl);
  return wdl;
}



GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetDebuggerDescrs(AB_BANKING *ab,
                                                            const char *pn){
  GWEN_BUFFER *pbuf;
  GWEN_PLUGIN_DESCRIPTION_LIST2 *wdl;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(pbuf,
                           AQBANKING_PLUGINS
                           DIRSEP
			   AB_PROVIDER_DEBUGGER_FOLDER
                           DIRSEP);
  GWEN_Buffer_AppendString(pbuf, pn);

  wdl=GWEN_LoadPluginDescrs(GWEN_Buffer_GetStart(pbuf));

  GWEN_Buffer_free(pbuf);
  return wdl;
}



int AB_Banking__LoadOldProviderData(AB_BANKING *ab, const char *name){
  GWEN_BUFFER *pbuf;
  GWEN_DB_NODE *db;

  assert(ab);
  pbuf=GWEN_Buffer_new(0, 256, 0, 1);

  if (AB_Banking_GetUserDataDir(ab, pbuf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not get user data dir");
    GWEN_Buffer_free(pbuf);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_AppendString(pbuf, DIRSEP "backends" DIRSEP);
  GWEN_Buffer_AppendString(pbuf, name);
  GWEN_Buffer_AppendString(pbuf, DIRSEP "settings.conf");

  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
                      "banking/backends");
  assert(db);
  db=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, name);
  assert(db);
  DBG_INFO(AQBANKING_LOGDOMAIN,
           "Reading file \"%s\"", GWEN_Buffer_GetStart(pbuf));
  if (GWEN_DB_ReadFile(db, GWEN_Buffer_GetStart(pbuf),
                       GWEN_DB_FLAGS_DEFAULT |
                       GWEN_PATH_FLAGS_CREATE_GROUP |
		       GWEN_DB_FLAGS_LOCKFILE, 0, 2000)) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "Old config file \"%s\", not found",
	     GWEN_Buffer_GetStart(pbuf));
    GWEN_Buffer_free(pbuf);
    return 0;
  }
  GWEN_Buffer_free(pbuf);

  /* sucessfully read */
  return 0;
}



GWEN_DB_NODE *AB_Banking_GetProviderData(AB_BANKING *ab,
                                         AB_PROVIDER *pro) {
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbT;

  assert(ab);
  assert(pro);

  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
                      "banking/backends");
  assert(db);
  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       AB_Provider_GetEscapedName(pro));
  if (!dbT && ab->lastVersion < ((1<<24) | (8<<16) | (1<<8) | 3)) {
    int rv;

    /* load separate file for older releases */
    rv=AB_Banking__LoadOldProviderData(ab, AB_Provider_GetEscapedName(pro));
    if (rv) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Could not load settings for backend \"%s\"",
                AB_Provider_GetName(pro));
      abort();
    }
  }

  dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT,
                       AB_Provider_GetEscapedName(pro));
  assert(dbT);
  return dbT;
}



int AB_Banking_InitProvider(AB_BANKING *ab, AB_PROVIDER *pro) {
  return AB_Provider_Init(pro);
}



int AB_Banking_FiniProvider(AB_BANKING *ab, AB_PROVIDER *pro) {
  int rv;

  rv=AB_Provider_Fini(pro);
  if (rv) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Error deinit backend (%d)", rv);
  }
  return rv;
}



AB_PROVIDER *AB_Banking_FindProvider(AB_BANKING *ab, const char *name) {
  AB_PROVIDER *pro;

  assert(ab);
  assert(name);
  pro=AB_Provider_List_First(ab->providers);
  while(pro) {
    if (strcasecmp(AB_Provider_GetName(pro), name)==0)
      break;
    pro=AB_Provider_List_Next(pro);
  } /* while */

  return pro;
}



AB_PROVIDER *AB_Banking_GetProvider(AB_BANKING *ab, const char *name) {
  AB_PROVIDER *pro;

  assert(ab);
  assert(name);
  pro=AB_Banking_FindProvider(ab, name);
  if (pro)
    return pro;
  pro=AB_Banking__LoadProviderPlugin(ab, name);
  if (pro) {
    if (AB_Banking_InitProvider(ab, pro)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not init provider \"%s\"", name);
      AB_Provider_free(pro);
      return 0;
    }
    AB_Provider_List_Add(pro, ab->providers);
  }

  return pro;
}



int AB_Banking_GetCryptToken(AB_BANKING *ab,
			     const char *tname,
			     const char *cname,
			     GWEN_CRYPT_TOKEN **pCt) {
  GWEN_CRYPT_TOKEN *ct=NULL;
  GWEN_CRYPT_TOKEN_LIST2_ITERATOR *it;

  assert(ab);

  assert(pCt);
  assert(tname);
  assert(cname);

  it=GWEN_Crypt_Token_List2_First(ab->cryptTokenList);
  if (it) {
    ct=GWEN_Crypt_Token_List2Iterator_Data(it);
    assert(ct);
    while(ct) {
      const char *s1;
      const char *s2;

      s1=GWEN_Crypt_Token_GetTypeName(ct);
      s2=GWEN_Crypt_Token_GetTokenName(ct);
      assert(s1);
      assert(s2);
      if (strcasecmp(s1, tname)==0 &&
	  strcasecmp(s2, cname)==0)
	break;
      ct=GWEN_Crypt_Token_List2Iterator_Next(it);
    }
  }

  if (ct==NULL) {
    GWEN_PLUGIN_MANAGER *pm;
    GWEN_PLUGIN *pl;

    /* get crypt token */
    pm=GWEN_PluginManager_FindPluginManager(GWEN_CRYPT_TOKEN_PLUGIN_TYPENAME);
    if (pm==0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "CryptToken plugin manager not found");
      return GWEN_ERROR_INTERNAL;
    }
  
    pl=GWEN_PluginManager_GetPlugin(pm, tname);
    if (pl==0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Plugin \"%s\" not found", tname);
      return GWEN_ERROR_NOT_FOUND;
    }

    ct=GWEN_Crypt_Token_Plugin_CreateToken(pl, cname);
    if (ct==0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not create crypt token");
      return GWEN_ERROR_IO;
    }

    /* add to internal list */
    GWEN_Crypt_Token_List2_PushBack(ab->cryptTokenList, ct);
  }

  *pCt=ct;
  return 0;
}



void AB_Banking_ClearCryptTokenList(AB_BANKING *ab, uint32_t guiid) {
  GWEN_CRYPT_TOKEN_LIST2_ITERATOR *it;

  assert(ab);
  assert(ab->cryptTokenList);

  it=GWEN_Crypt_Token_List2_First(ab->cryptTokenList);
  if (it) {
    GWEN_CRYPT_TOKEN *ct;

    ct=GWEN_Crypt_Token_List2Iterator_Data(it);
    assert(ct);
    while(ct) {
      while(GWEN_Crypt_Token_IsOpen(ct)) {
	int rv;

	rv=GWEN_Crypt_Token_Close(ct, 0, guiid);
	if (rv) {
	  DBG_WARN(AQBANKING_LOGDOMAIN,
		   "Could not close crypt token [%s:%s], abandoning (%d)",
		   GWEN_Crypt_Token_GetTypeName(ct),
		   GWEN_Crypt_Token_GetTokenName(ct),
		   rv);
	  GWEN_Crypt_Token_Close(ct, 1, guiid);
	}
      }
      GWEN_Crypt_Token_free(ct);
      ct=GWEN_Crypt_Token_List2Iterator_Next(it);
    }
  }
  GWEN_Crypt_Token_List2_Clear(ab->cryptTokenList);
}



int AB_Banking_CheckCryptToken(AB_BANKING *ab,
			       GWEN_CRYPT_TOKEN_DEVICE devt,
			       GWEN_BUFFER *typeName,
			       GWEN_BUFFER *tokenName,
			       uint32_t guiid) {
  GWEN_PLUGIN_MANAGER *pm;
  int rv;

  /* get crypt token */
  pm=GWEN_PluginManager_FindPluginManager(GWEN_CRYPT_TOKEN_PLUGIN_TYPENAME);
  if (pm==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "CryptToken plugin manager not found");
    return GWEN_ERROR_NOT_FOUND;
  }

  /* try to determine the type and name */
  rv=GWEN_Crypt_Token_PluginManager_CheckToken(pm,
					       devt,
					       typeName,
					       tokenName,
					       guiid);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



void AB_Banking__fillTransactionFromAccount(AB_TRANSACTION *t, const AB_ACCOUNT *a) {
  const char *s;

  s=AB_Transaction_GetLocalName(t);
  if (!s)
    AB_Transaction_SetLocalName(t, AB_Account_GetOwnerName(a));
  s=AB_Transaction_GetLocalBankCode(t);
  if (!s)
    AB_Transaction_SetLocalBankCode(t, AB_Account_GetBankCode(a));
  s=AB_Transaction_GetLocalAccountNumber(t);
  if (!s)
    AB_Transaction_SetLocalAccountNumber(t, AB_Account_GetAccountNumber(a));
  s=AB_Transaction_GetLocalIban(t);
  if (!s)
    AB_Transaction_SetLocalIban(t, AB_Account_GetIBAN(a));
  s=AB_Transaction_GetLocalBic(t);
  if (!s)
    AB_Transaction_SetLocalBic(t, AB_Account_GetBIC(a));
}



int AB_Banking_FillGapsInImExporterContext(AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *iec) {
  AB_IMEXPORTER_ACCOUNTINFO *iea;
  int notFounds=0;

  assert(iec);
  iea=AB_ImExporterContext_GetFirstAccountInfo(iec);
  while(iea) {
    AB_ACCOUNT *a;

    a=AB_Banking_GetAccountByCodeAndNumber(ab,
					   AB_ImExporterAccountInfo_GetBankCode(iea),
					   AB_ImExporterAccountInfo_GetAccountNumber(iea));
    if (!a)
      a=AB_Banking_GetAccountByIban(ab, AB_ImExporterAccountInfo_GetIban(iea));
    if (a) {
      AB_TRANSACTION *t;

      AB_ImExporterAccountInfo_FillFromAccount(iea, a);

      /* fill transactions */
      t=AB_ImExporterAccountInfo_GetFirstTransaction(iea);
      while(t) {
	AB_Banking__fillTransactionFromAccount(t, a);
	t=AB_ImExporterAccountInfo_GetNextTransaction(iea);
      }

      /* fill standing orders */
      t=AB_ImExporterAccountInfo_GetFirstStandingOrder(iea);
      while(t) {
	AB_Banking__fillTransactionFromAccount(t, a);
	t=AB_ImExporterAccountInfo_GetNextStandingOrder(iea);
      }

      /* fill transfers */
      t=AB_ImExporterAccountInfo_GetFirstTransfer(iea);
      while(t) {
	AB_Banking__fillTransactionFromAccount(t, a);
	t=AB_ImExporterAccountInfo_GetNextTransfer(iea);
      }

      /* fill dated transfers */
      t=AB_ImExporterAccountInfo_GetFirstDatedTransfer(iea);
      while(t) {
	AB_Banking__fillTransactionFromAccount(t, a);
	t=AB_ImExporterAccountInfo_GetNextDatedTransfer(iea);
      }

      /* fill noted transactions */
      t=AB_ImExporterAccountInfo_GetFirstNotedTransaction(iea);
      while(t) {
	AB_Banking__fillTransactionFromAccount(t, a);
	t=AB_ImExporterAccountInfo_GetNextNotedTransaction(iea);
      }
    }
    else
      notFounds++;

    iea=AB_ImExporterContext_GetNextAccountInfo(iec);
  }

  return (notFounds==0)?0:1;
}





