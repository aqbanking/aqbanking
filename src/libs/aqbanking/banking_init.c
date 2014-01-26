/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004-2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/* This file is included by banking.c */


#ifdef OS_WIN32
# include <windows.h>
#endif

static uint32_t ab_init_count=0;

static uint32_t ab_plugin_init_count=0;

static GWEN_PLUGIN_MANAGER *ab_pluginManagerBankInfo=NULL;
static GWEN_PLUGIN_MANAGER *ab_pluginManagerProvider=NULL;
static GWEN_PLUGIN_MANAGER *ab_pluginManagerImExporter=NULL;
static AB_IMEXPORTER_LIST *ab_imexporters=NULL;
static AB_BANKINFO_PLUGIN_LIST *ab_bankInfoPlugins=NULL;


#define AB_DBIO_FOLDER "dbio"



int AB_Banking_PluginSystemInit(void) {
  if (ab_plugin_init_count==0) {
    const char *s;
    GWEN_PLUGIN_MANAGER *pm;
    int rv;

    rv=GWEN_Init();
    if (rv) {
      DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, rv);
      return rv;
    }
    if (!GWEN_Logger_IsOpen(AQBANKING_LOGDOMAIN)) {
      GWEN_Logger_Open(AQBANKING_LOGDOMAIN,
		       "aqbanking", 0,
		       GWEN_LoggerType_Console,
		       GWEN_LoggerFacility_User);
    }

    s=getenv("AQBANKING_LOGLEVEL");
    if (s && *s) {
      GWEN_LOGGER_LEVEL ll;

      ll=GWEN_Logger_Name2Level(s);
      GWEN_Logger_SetLevel(AQBANKING_LOGDOMAIN, ll);
    }
    else
      GWEN_Logger_SetLevel(AQBANKING_LOGDOMAIN, GWEN_LoggerLevel_Notice);

    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "AqBanking v"
	     AQBANKING_VERSION_FULL_STRING
	     " (compiled at "
	     COMPILE_DATETIME
	     "): initialising");

    /* define locale paths */
    GWEN_PathManager_DefinePath(AB_PM_LIBNAME, AB_PM_LOCALEDIR);
    GWEN_PathManager_AddPathFromWinReg(AB_PM_LIBNAME,
				       AB_PM_LIBNAME,
				       AB_PM_LOCALEDIR,
				       AB_BANKING_REGKEY_PATHS,
				       AB_BANKING_REGKEY_LOCALEDIR);
#if defined(OS_WIN32) || defined(ENABLE_LOCAL_INSTALL)
    /* add folder relative to EXE */
    GWEN_PathManager_AddRelPath(AB_PM_LIBNAME,
				AB_PM_LIBNAME,
				AB_PM_LOCALEDIR,
				LOCALEDIR,
				GWEN_PathManager_RelModeExe);
#else
    /* add absolute folder */
    GWEN_PathManager_AddPath(AB_PM_LIBNAME,
			     AB_PM_LIBNAME,
			     AB_PM_LOCALEDIR,
			     LOCALEDIR);
#endif



    if (1) {
      GWEN_STRINGLIST *sl=GWEN_PathManager_GetPaths(AB_PM_LIBNAME,
						    AB_PM_LOCALEDIR);
      const char *localedir=GWEN_StringList_FirstString(sl);

      rv=GWEN_I18N_BindTextDomain_Dir(PACKAGE, localedir);
      if (rv) {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not bind textdomain (%d)", rv);
      }
      else {
	rv=GWEN_I18N_BindTextDomain_Codeset(PACKAGE, "UTF-8");
	if (rv) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not set codeset (%d)", rv);
	}
      }

      GWEN_StringList_free(sl);
    }


    /* define sysconf paths */
    GWEN_PathManager_DefinePath(AB_PM_LIBNAME, AB_PM_SYSCONFDIR);
    GWEN_PathManager_AddPathFromWinReg(AB_PM_LIBNAME,
				       AB_PM_LIBNAME,
				       AB_PM_SYSCONFDIR,
				       AB_BANKING_REGKEY_PATHS,
				       AB_BANKING_REGKEY_SYSCONFDIR);
#if defined(OS_WIN32) || defined(ENABLE_LOCAL_INSTALL)
    /* add folder relative to EXE */
    GWEN_PathManager_AddRelPath(AB_PM_LIBNAME,
				AB_PM_LIBNAME,
				AB_PM_SYSCONFDIR,
				AQBANKING_SYSCONF_DIR,
				GWEN_PathManager_RelModeExe);
#else
    /* add absolute folder */
    GWEN_PathManager_AddPath(AB_PM_LIBNAME,
			     AB_PM_LIBNAME,
			     AB_PM_SYSCONFDIR,
			     AQBANKING_SYSCONF_DIR);
#endif

    /* define data paths */
    GWEN_PathManager_DefinePath(AB_PM_LIBNAME, AB_PM_DATADIR);
    GWEN_PathManager_AddPathFromWinReg(AB_PM_LIBNAME,
				       AB_PM_LIBNAME,
				       AB_PM_DATADIR,
				       AB_BANKING_REGKEY_PATHS,
				       AB_BANKING_REGKEY_DATADIR);
#if defined(OS_WIN32) || defined(ENABLE_LOCAL_INSTALL)
    /* add folder relative to EXE */
    GWEN_PathManager_AddRelPath(AB_PM_LIBNAME,
				AB_PM_LIBNAME,
				AB_PM_DATADIR,
				AQBANKING_DATA_DIR,
				GWEN_PathManager_RelModeExe);
#else
    /* add absolute folder */
    GWEN_PathManager_AddPath(AB_PM_LIBNAME,
			     AB_PM_LIBNAME,
			     AB_PM_DATADIR,
			     AQBANKING_DATA_DIR);
#endif

    /* define wizard paths */
    GWEN_PathManager_DefinePath(AB_PM_LIBNAME, AB_PM_WIZARDDIR);
    GWEN_PathManager_AddPathFromWinReg(AB_PM_LIBNAME,
				       AB_PM_LIBNAME,
				       AB_PM_WIZARDDIR,
				       AB_BANKING_REGKEY_PATHS,
				       AB_BANKING_REGKEY_WIZARDDIR);
#if defined(OS_WIN32) || defined(ENABLE_LOCAL_INSTALL)
    /* add folder relative to EXE */
    GWEN_PathManager_AddRelPath(AB_PM_LIBNAME,
				AB_PM_LIBNAME,
				AB_PM_WIZARDDIR,
				AQBANKING_PLUGINS
				DIRSEP
				AB_WIZARD_FOLDER,
				GWEN_PathManager_RelModeExe);
#else
    /* add absolute folder */
    GWEN_PathManager_AddPath(AB_PM_LIBNAME,
			     AB_PM_LIBNAME,
			     AB_PM_WIZARDDIR,
			     AQBANKING_PLUGINS
			     DIRSEP
			     AB_WIZARD_FOLDER);
#endif

    /* create bankinfo plugin manager */
    DBG_INFO(AQBANKING_LOGDOMAIN, "Registering bankinfo plugin manager");
    pm=GWEN_PluginManager_new("bankinfo", AB_PM_LIBNAME);
    if (GWEN_PluginManager_Register(pm)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not register bankinfo plugin manager");
      return GWEN_ERROR_GENERIC;
    }

    GWEN_PluginManager_AddPathFromWinReg(pm,
					 AB_PM_LIBNAME,
					 AB_BANKING_REGKEY_PATHS,
					 AB_BANKING_REGKEY_BANKINFODIR);
#if defined(OS_WIN32) || defined(ENABLE_LOCAL_INSTALL)
    /* add folder relative to EXE */
    GWEN_PluginManager_AddRelPath(pm,
				  AB_PM_LIBNAME,
				  AQBANKING_PLUGINS
				  DIRSEP
				  AB_BANKINFO_PLUGIN_FOLDER,
				  GWEN_PathManager_RelModeExe);
#else
    /* add absolute folder */
    GWEN_PluginManager_AddPath(pm,
			       AB_PM_LIBNAME,
			       AQBANKING_PLUGINS
			       DIRSEP
			       AB_BANKINFO_PLUGIN_FOLDER);
#endif
    ab_pluginManagerBankInfo=pm;


    /* create provider plugin manager */
    DBG_INFO(AQBANKING_LOGDOMAIN, "Registering provider plugin manager");
    pm=GWEN_PluginManager_new("provider", AB_PM_LIBNAME);
    if (GWEN_PluginManager_Register(pm)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not register provider plugin manager");
      return GWEN_ERROR_GENERIC;
    }

    GWEN_PluginManager_AddPathFromWinReg(pm,
					 AB_PM_LIBNAME,
					 AB_BANKING_REGKEY_PATHS,
					 AB_BANKING_REGKEY_PROVIDERDIR);
#if defined(OS_WIN32) || defined(ENABLE_LOCAL_INSTALL)
    /* add folder relative to EXE */
    GWEN_PluginManager_AddRelPath(pm,
				  AB_PM_LIBNAME,
				  AQBANKING_PLUGINS
				  DIRSEP
				  AB_PROVIDER_FOLDER,
				  GWEN_PathManager_RelModeExe);
#else
    /* add absolute folder */
    GWEN_PluginManager_AddPath(pm,
			       AB_PM_LIBNAME,
			       AQBANKING_PLUGINS
			       DIRSEP
			       AB_PROVIDER_FOLDER);
#endif
    ab_pluginManagerProvider=pm;


    /* create imexporters plugin manager */
    DBG_INFO(AQBANKING_LOGDOMAIN, "Registering imexporters plugin manager");
    pm=GWEN_PluginManager_new("imexporter", AB_PM_LIBNAME);
    if (GWEN_PluginManager_Register(pm)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not register imexporters plugin manager");
      return GWEN_ERROR_GENERIC;
    }

    GWEN_PluginManager_AddPathFromWinReg(pm,
					 AB_PM_LIBNAME,
					 AB_BANKING_REGKEY_PATHS,
					 AB_BANKING_REGKEY_IMPORTERDIR);
#if defined(OS_WIN32) || defined(ENABLE_LOCAL_INSTALL)
    /* add folder relative to EXE */
    GWEN_PluginManager_AddRelPath(pm,
				  AB_PM_LIBNAME,
				  AQBANKING_PLUGINS
				  DIRSEP
				  AB_IMEXPORTER_FOLDER,
				  GWEN_PathManager_RelModeExe);
#else
    /* add absolute folder */
    GWEN_PluginManager_AddPath(pm,
			       AB_PM_LIBNAME,
			       AQBANKING_PLUGINS
			       DIRSEP
			       AB_IMEXPORTER_FOLDER);
#endif
    ab_pluginManagerImExporter=pm;


    /* insert DBIO plugin folder */
    pm=GWEN_PluginManager_FindPluginManager("dbio");
    if (pm) {
#if defined(OS_WIN32) || defined(ENABLE_LOCAL_INSTALL)
      /* insert folder relative to EXE */
      GWEN_PluginManager_InsertRelPath(pm,
                                       AB_PM_LIBNAME,
                                       AQBANKING_PLUGINS
                                       DIRSEP
                                       AB_DBIO_FOLDER,
                                       GWEN_PathManager_RelModeExe);
#else
      /* add absolute folder */
      GWEN_PluginManager_InsertPath(pm,
                                    AB_PM_LIBNAME,
                                    AQBANKING_PLUGINS
                                    DIRSEP
                                    AB_DBIO_FOLDER);
#endif
    }
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not find DBIO plugin manager, maybe GWEN_Init() was not called?");
      return GWEN_ERROR_GENERIC;
    }


    ab_imexporters=AB_ImExporter_List_new();
    ab_bankInfoPlugins=AB_BankInfoPlugin_List_new();
  }
  ab_plugin_init_count++;
  return 0;
}



int AB_Banking_PluginSystemFini(void) {
  if (ab_plugin_init_count) {
    if (--ab_plugin_init_count==0) {
      AB_BankInfoPlugin_List_free(ab_bankInfoPlugins);
      ab_bankInfoPlugins=NULL;
      AB_ImExporter_List_free(ab_imexporters);
      ab_imexporters=NULL;

      /* unregister and unload provider plugin manager */
      if (ab_pluginManagerProvider) {
	if (GWEN_PluginManager_Unregister(ab_pluginManagerProvider)) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Could not unregister provider plugin manager");
	}
	GWEN_PluginManager_free(ab_pluginManagerProvider);
	ab_pluginManagerProvider=NULL;
      }

      /* unregister and unload bankinfo plugin manager */
      if (ab_pluginManagerBankInfo) {
	if (GWEN_PluginManager_Unregister(ab_pluginManagerBankInfo)) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Could not unregister bankinfo plugin manager");
	}
	GWEN_PluginManager_free(ab_pluginManagerBankInfo);
	ab_pluginManagerBankInfo=NULL;
      }

      /* unregister and unload imexporters plugin manager */
      if (ab_pluginManagerImExporter) {
	if (GWEN_PluginManager_Unregister(ab_pluginManagerImExporter)) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Could not unregister imexporter plugin manager");
	}
	GWEN_PluginManager_free(ab_pluginManagerImExporter);
	ab_pluginManagerImExporter=NULL;
      }

      /* undefine our own paths */
      GWEN_PathManager_UndefinePath(AB_PM_LIBNAME, AB_PM_LOCALEDIR);
      GWEN_PathManager_UndefinePath(AB_PM_LIBNAME, AB_PM_DATADIR);
      GWEN_PathManager_UndefinePath(AB_PM_LIBNAME, AB_PM_SYSCONFDIR);
      GWEN_PathManager_UndefinePath(AB_PM_LIBNAME, AB_PM_WIZARDDIR);

      /* remove AqBanking additions to all pathmanagers */
      GWEN_PathManager_RemovePaths(AB_PM_LIBNAME);

      GWEN_Logger_Close(AQBANKING_LOGDOMAIN);
      GWEN_Fini();
    }
  }
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN,
	     "AB_Banking_PluginSystemFini() called too often!");
  }
  return 0;
}



int AB_Banking_Init(AB_BANKING *ab) {
  assert(ab);
  /* do basic initialisation for all AB_BANKING objects */
  if (ab_init_count==0) {
    int rv;

    rv=AB_Banking_PluginSystemInit();
    if (rv) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      AB_Banking_PluginSystemFini();
      return rv;
    }
  }
  ab_init_count++;

  if (ab_init_count==0) {
    /* nothing to do extra here right now */
  }
  ab->initCount++;

  return 0;
}



int AB_Banking_Fini(AB_BANKING *ab) {
  /* deinit local stuff */
  if (ab->initCount<1) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "AqBanking object not initialized!");
    return GWEN_ERROR_INVALID;
  }

  if (--(ab->initCount)==0) {
    /* nothing to do for banking objects here */
  }

  /* deinit global stuff */
  if (ab_init_count<1) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "AqBanking not initialized, internal error");
    return GWEN_ERROR_INVALID;
  }

  if (--(ab_init_count)==0) {
    int rv;

    rv=AB_Banking_PluginSystemFini();
    if (rv) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    }
  }

  return 0;
}



int AB_Banking_OnlineInit(AB_BANKING *ab) {

  assert(ab);

  if (ab->initCount==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Object not initialised");
    return GWEN_ERROR_INVALID;
  }

  if (ab->onlineInitCount==0) {
    int rv;

    rv=AB_Banking_LoadConfig(ab);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      AB_Banking_UnloadConfig(ab);
      return rv;
    }
  } /* if first init */

  ab->onlineInitCount++;

  return 0;
}



int AB_Banking_OnlineFini(AB_BANKING *ab) {
  int rv=0;
  assert(ab);

  if (ab->onlineInitCount<1) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Online part of AqBanking not initialized!");
    return GWEN_ERROR_INVALID;
  }

  if (ab->onlineInitCount==1) {
    rv=AB_Banking_SaveConfig(ab);
    AB_Banking_UnloadConfig(ab);
  }
  ab->onlineInitCount--;

  return rv;
}



void AB_Banking_ActivateAllProviders(AB_BANKING *ab){
  GWEN_PLUGIN_DESCRIPTION_LIST2 *descrs;
  GWEN_PLUGIN_MANAGER *pm;

  pm=GWEN_PluginManager_FindPluginManager("provider");
  if (!pm) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not find plugin manager for \"%s\"",
	      "provider");
    return;
  }

  descrs=GWEN_PluginManager_GetPluginDescrs(pm);
  if (descrs) {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *it;
    GWEN_PLUGIN_DESCRIPTION *pd;

    it=GWEN_PluginDescription_List2_First(descrs);
    assert(it);
    pd=GWEN_PluginDescription_List2Iterator_Data(it);
    assert(pd);
    while(pd) {
      const char *pname=GWEN_PluginDescription_GetName(pd);
      AB_PROVIDER *pro;

      pro=AB_Banking_GetProvider(ab, pname);
      if (!pro) {
	DBG_WARN(AQBANKING_LOGDOMAIN,
		 "Could not load backend \"%s\", ignoring", pname);
      }
      else {
	GWEN_StringList_AppendString(ab->activeProviders, pname, 0, 1);
      }

      pd=GWEN_PluginDescription_List2Iterator_Next(it);
    } /* while */
    GWEN_PluginDescription_List2Iterator_free(it);
    GWEN_PluginDescription_List2_freeAll(descrs);
  }
}



#ifdef OS_WIN32
BOOL APIENTRY DllMain(HINSTANCE hInst,
                      DWORD reason,
                      LPVOID reserved) {
  int err;

  switch (reason) {
  case DLL_PROCESS_ATTACH:
    err=GWEN_Init();
    if (err) {
      fprintf(stderr, "Could not initialize Gwenhywfar, aborting\n");
      return FALSE;
    }
    /* DEBUG */
    /*fprintf(stderr, "Gwenhywfar initialized.\n");*/
    break;

  case DLL_PROCESS_DETACH:
    err=GWEN_Fini();
    if (err) {
      fprintf(stderr, "Could not deinitialize Gwenhywfar\n");
    }
    /* DEBUG */
    /*fprintf(stderr, "Gwenhywfar deinitialized.\n");*/
    break;

  case DLL_THREAD_ATTACH:
    break;

  case DLL_THREAD_DETACH:
    break;
  }
  return TRUE;
}
#endif






