/***************************************************************************
 begin       : Sat Jun 30 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/* This file is included by banking.c */



int AB_Banking_ReadNamedConfigGroup(AB_BANKING *ab,
                                     const char *groupName,
                                     const char *subGroupName,
                                     int doLock,
                                     int doUnlock,
                                     GWEN_DB_NODE **pDb) {
  GWEN_DB_NODE *db=NULL;
  int rv;

  assert(ab);

  /* check for config manager (created by AB_Banking_Init) */
  if (ab->configMgr==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "No config manager (maybe the gwenhywfar plugins are not installed?");
    return GWEN_ERROR_GENERIC;
  }


  /* lock group if requested */
  if (doLock) {
    rv=GWEN_ConfigMgr_LockGroup(ab->configMgr, groupName, subGroupName);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to lock config group (%d)", rv);
      return rv;
    }
  }

  /* load group */
  rv=GWEN_ConfigMgr_GetGroup(ab->configMgr, groupName, subGroupName, &db);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not load config group (%d)", rv);
    if (doLock)
      GWEN_ConfigMgr_UnlockGroup(ab->configMgr, groupName, subGroupName);
    return rv;
  }

  /* unlock group */
  if (doUnlock) {
    rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr, groupName, subGroupName);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to unlock config group (%d)", rv);
      GWEN_DB_Group_free(db);
      return rv;
    }
  }

  *pDb=db;
  return 0;
}



int AB_Banking_WriteNamedConfigGroup(AB_BANKING *ab,
                                      const char *groupName,
                                      const char *subGroupName,
                                      int doLock,
                                      int doUnlock,
                                      GWEN_DB_NODE *db) {
  int rv;

  assert(ab);
  assert(db);

  /* check for config manager (created by AB_Banking_Init) */
  if (ab->configMgr==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No config manager (maybe the gwenhywfar plugins are not installed?");
    return GWEN_ERROR_GENERIC;
  }


  /* lock group */
  if (doLock) {
    rv=GWEN_ConfigMgr_LockGroup(ab->configMgr, groupName, subGroupName);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to lock config group (%d)", rv);
      return rv;
    }
  }

  /* store group (is locked now) */
  rv=GWEN_ConfigMgr_SetGroup(ab->configMgr, groupName, subGroupName, db);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not load config group (%d)", rv);
    if (doLock)
      GWEN_ConfigMgr_UnlockGroup(ab->configMgr, groupName, subGroupName);
    return rv;
  }

  /* unlock group */
  if (doUnlock) {
    rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr, groupName, subGroupName);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to unlock config group (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int AB_Banking_ReadConfigGroup(AB_BANKING *ab,
                                const char *groupName,
                                uint32_t uniqueId,
                                int doLock,
                                int doUnlock,
                                GWEN_DB_NODE **pDb) {
  int rv;
  char idBuf[256];

  assert(ab);

  /* check for config manager (created by AB_Banking_Init) */
  if (ab->configMgr==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No config manager (maybe the gwenhywfar plugins are not installed?");
    return GWEN_ERROR_GENERIC;
  }


  /* make config manager id from given unique id */
  rv=GWEN_ConfigMgr_MkUniqueIdFromId(ab->configMgr, groupName, uniqueId, 0, idBuf, sizeof(idBuf)-1);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to create a unique id for config group (%d)", rv);
    return rv;
  }
  idBuf[sizeof(idBuf)-1]=0;

  rv=AB_Banking_ReadNamedConfigGroup(ab, groupName, idBuf, doLock, doUnlock, pDb);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return rv;
}



int AB_Banking_WriteConfigGroup(AB_BANKING *ab,
                                 const char *groupName,
                                 uint32_t uniqueId,
                                 int doLock,
                                 int doUnlock,
                                 GWEN_DB_NODE *db) {
  int rv;
  char idBuf[256];

  assert(ab);
  assert(db);

  /* check for config manager (created by AB_Banking_Init) */
  if (ab->configMgr==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No config manager (maybe the gwenhywfar plugins are not installed?");
    return GWEN_ERROR_GENERIC;
  }


  /* make config manager id from given unique id */
  rv=GWEN_ConfigMgr_MkUniqueIdFromId(ab->configMgr, groupName, uniqueId, 0, idBuf, sizeof(idBuf)-1);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to create a unique id for config group (%d)", rv);
    return rv;
  }
  idBuf[sizeof(idBuf)-1]=0;

  rv=AB_Banking_WriteNamedConfigGroup(ab, groupName, idBuf, doLock, doUnlock, db);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return rv;
}



int AB_Banking_DeleteConfigGroup(AB_BANKING *ab, const char *groupName, uint32_t uniqueId) {
  int rv;
  char idBuf[256];

  assert(ab);

  /* check for config manager (created by AB_Banking_Init) */
  if (ab->configMgr==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No config manager (maybe the gwenhywfar plugins are not installed?");
    return GWEN_ERROR_GENERIC;
  }

  /* make config manager id from given unique id */
  rv=GWEN_ConfigMgr_MkUniqueIdFromId(ab->configMgr, groupName, uniqueId, 0, idBuf, sizeof(idBuf)-1);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to create a unique id for config group (%d)", rv);
    return rv;
  }
  idBuf[sizeof(idBuf)-1]=0;

  /* unlock group */
  rv=GWEN_ConfigMgr_DeleteGroup(ab->configMgr, groupName, idBuf);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to delete config group (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Banking_UnlockConfigGroup(AB_BANKING *ab, const char *groupName, uint32_t uniqueId) {
  int rv;
  char idBuf[256];

  assert(ab);

  /* check for config manager (created by AB_Banking_Init) */
  if (ab->configMgr==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No config manager (maybe the gwenhywfar plugins are not installed?");
    return GWEN_ERROR_GENERIC;
  }

  /* make config manager id from given unique id */
  rv=GWEN_ConfigMgr_MkUniqueIdFromId(ab->configMgr, groupName, uniqueId, 0, idBuf, sizeof(idBuf)-1);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to create a unique id for config group (%d)", rv);
    return rv;
  }
  idBuf[sizeof(idBuf)-1]=0;

  /* unlock group */
  rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr, groupName, idBuf);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to unlock config group (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Banking_ReadConfigGroups(AB_BANKING *ab,
                                 const char *groupName,
                                 const char *uidField,
                                 const char *matchVar,
                                 const char *matchVal,
                                 GWEN_DB_NODE **pDb) {
  GWEN_STRINGLIST *sl;
  int rv;

  sl=GWEN_StringList_new();
  rv=GWEN_ConfigMgr_ListSubGroups(ab->configMgr, groupName, sl);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_StringList_free(sl);
    return rv;
  }
  if (GWEN_StringList_Count(sl)) {
    GWEN_DB_NODE *dbAll;
    GWEN_STRINGLISTENTRY *se;
    int ignoredGroups=0;

    dbAll=GWEN_DB_Group_new("all");

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      const char *t;
      GWEN_DB_NODE *db=NULL;

      t=GWEN_StringListEntry_Data(se);
      assert(t);

      /* lock before reading */
      rv=GWEN_ConfigMgr_LockGroup(ab->configMgr, groupName, t);
      if (rv<0) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to lock config group \"%s\" (%d), ignoring", t, rv);
        ignoredGroups++;
      }
      else {
        rv=GWEN_ConfigMgr_GetGroup(ab->configMgr, groupName, t, &db);
        if (rv<0) {
          DBG_WARN(AQBANKING_LOGDOMAIN, "Could not load group [%s] (%d), ignoring", t, rv);
          GWEN_ConfigMgr_UnlockGroup(ab->configMgr, groupName, t);
          ignoredGroups++;
        }
        else {
          int doAdd=1;

          /* unlock after reading */
          rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr, groupName, t);
          if (rv<0) {
            DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not unlock group [%s] (%d)", t, rv);
          }

          assert(db);
          GWEN_DB_GroupRename(db, t);
          if (doAdd && uidField && *uidField) {
            int v;
  
            v=GWEN_DB_GetIntValue(db, uidField, 0, 0);
            if (v==0)
              doAdd=0;
          }
  
          if (doAdd && matchVar && *matchVar) {
            const char *s;
  
            s=GWEN_DB_GetCharValue(db, matchVar, 0, NULL);
            if (s && *s) {
              if (strcasecmp(s, matchVal)!=0)
                doAdd=0;
            }
            else {
              if (matchVal && *matchVal)
                doAdd=0;
            }
          }
  
          if (doAdd)
            GWEN_DB_AddGroup(dbAll, db);
          else
            GWEN_DB_Group_free(db);
        } /* if getGroup ok */
      } /* if locking ok */
      se=GWEN_StringListEntry_Next(se);
    } /* while se */

    if (GWEN_DB_Groups_Count(dbAll)) {
      *pDb=dbAll;
      if (ignoredGroups)
        return GWEN_ERROR_PARTIAL;
      return 0;
    }
    else {
      DBG_WARN(AQBANKING_LOGDOMAIN, "No matching config groups found");
      GWEN_DB_Group_free(dbAll);
      return GWEN_ERROR_NOT_FOUND;
    }
  }
  GWEN_StringList_free(sl);

  DBG_INFO(AQBANKING_LOGDOMAIN, "No account specs found");
  return GWEN_ERROR_NOT_FOUND;
}










int AB_Banking_ReadAccountSpec(AB_BANKING *ab, uint32_t uniqueId, AB_ACCOUNT_SPEC **pAccountSpec) {
  AB_ACCOUNT_SPEC *accountSpec;
  GWEN_DB_NODE *db=NULL;
  int rv;

  assert(ab);

  rv=AB_Banking_ReadConfigGroup(ab, AB_CFG_GROUP_ACCOUNTSPECS, uniqueId, 1, 1, &db);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  accountSpec=AB_AccountSpec_new();
  AB_AccountSpec_ReadDb(accountSpec, db);
  AB_AccountSpec_SetUniqueId(accountSpec, uniqueId);

  GWEN_DB_Group_free(db);

  if (pAccountSpec)
    *pAccountSpec=accountSpec;
  else
    AB_AccountSpec_free(accountSpec);
  return 0;
}




int AB_Banking_WriteAccountSpec(AB_BANKING *ab, const AB_ACCOUNT_SPEC *accountSpec) {
  GWEN_DB_NODE *db=NULL;
  int rv;
  uint32_t uniqueId;

  assert(ab);

  uniqueId=AB_AccountSpec_GetUniqueId(accountSpec);

  /* write account spec to DB */
  db=GWEN_DB_Group_new("accountSpec");
  AB_AccountSpec_toDb(accountSpec, db);

  rv=AB_Banking_WriteConfigGroup(ab, AB_CFG_GROUP_ACCOUNTSPECS, uniqueId, 1, 1, db);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(db);
    return rv;
  }
  GWEN_DB_Group_free(db);

  return 0;
}



int AB_Banking_DeleteAccountSpec(AB_BANKING *ab, uint32_t uid) {
  int rv;

  rv=AB_Banking_DeleteConfigGroup(ab, AB_CFG_GROUP_ACCOUNTSPECS, uid);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}





int AB_Banking_Read_AccountConfig(AB_BANKING *ab, uint32_t uid, int doLock, int doUnlock, GWEN_DB_NODE **pDb) {
  int rv;

  rv=AB_Banking_ReadConfigGroup(ab, AB_CFG_GROUP_ACCOUNTS, uid, doLock, doUnlock, pDb);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return rv;
}



int AB_Banking_Write_AccountConfig(AB_BANKING *ab, uint32_t uid, int doLock, int doUnlock, GWEN_DB_NODE *db){
  int rv;

  rv=AB_Banking_WriteConfigGroup(ab, AB_CFG_GROUP_ACCOUNTS, uid, doLock, doUnlock, db);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Banking_Delete_AccountConfig(AB_BANKING *ab, uint32_t uid) {
  int rv;

  rv=AB_Banking_DeleteConfigGroup(ab, AB_CFG_GROUP_ACCOUNTS, uid);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Banking_Unlock_AccountConfig(AB_BANKING *ab, uint32_t uid) {
  int rv;

  rv=AB_Banking_UnlockConfigGroup(ab, AB_CFG_GROUP_ACCOUNTS, uid);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Banking_Read_UserConfig(AB_BANKING *ab, uint32_t uid, int doLock, int doUnlock, GWEN_DB_NODE **pDb) {
  int rv;

  rv=AB_Banking_ReadConfigGroup(ab, AB_CFG_GROUP_USERS, uid, doLock, doUnlock, pDb);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Banking_Write_UserConfig(AB_BANKING *ab, uint32_t uid, int doLock, int doUnlock, GWEN_DB_NODE *db){
  int rv;

  rv=AB_Banking_WriteConfigGroup(ab, AB_CFG_GROUP_USERS, uid, doLock, doUnlock, db);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Banking_Delete_UserConfig(AB_BANKING *ab, uint32_t uid) {
  int rv;

  rv=AB_Banking_DeleteConfigGroup(ab, AB_CFG_GROUP_USERS, uid);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Banking_Unlock_UserConfig(AB_BANKING *ab, uint32_t uid) {
  int rv;

  rv=AB_Banking_UnlockConfigGroup(ab, AB_CFG_GROUP_USERS, uid);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}








int AB_Banking_GetAccountSpecList(AB_BANKING *ab, AB_ACCOUNT_SPEC_LIST** pAccountSpecList) {
  GWEN_DB_NODE *dbAll=NULL;
  int rv;

  rv=AB_Banking_ReadConfigGroups(ab, AB_CFG_GROUP_ACCOUNTSPECS, "uniqueId", NULL, NULL, &dbAll);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  else {
    AB_ACCOUNT_SPEC_LIST *accountSpecList;
    GWEN_DB_NODE *db;

    accountSpecList=AB_AccountSpec_List_new();

    db=GWEN_DB_GetFirstGroup(dbAll);
    while(db) {
      AB_ACCOUNT_SPEC *a=NULL;

      assert(db);
      a=AB_AccountSpec_fromDb(db);
      if (a) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "Adding account spec");
        AB_AccountSpec_List_Add(a, accountSpecList);
      }

      db=GWEN_DB_GetNextGroup(db);
    }

    if (AB_AccountSpec_List_GetCount(accountSpecList)) {
      *pAccountSpecList=accountSpecList;
      return 0;
    }
    else {
      DBG_WARN(AQBANKING_LOGDOMAIN, "No valid account specs found");
      AB_AccountSpec_List_free(accountSpecList);
      return GWEN_ERROR_NOT_FOUND;
    }
  }

  DBG_INFO(AQBANKING_LOGDOMAIN, "No account specs found");
  return GWEN_ERROR_NOT_FOUND;
}



int AB_Banking_GetAccountSpecByUniqueId(AB_BANKING *ab, uint32_t uniqueAccountId, AB_ACCOUNT_SPEC** pAccountSpec) {
  int rv;

  rv=AB_Banking_ReadAccountSpec(ab, uniqueAccountId, pAccountSpec);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return rv;
}





int AB_Banking__SendCommands(AB_BANKING *ab, AB_TRANSACTION_LIST2* commandList, AB_IMEXPORTER_CONTEXT *ctx, uint32_t pid) {
  AB_ACCOUNTQUEUE_LIST *aql;
  AB_PROVIDERQUEUE_LIST *pql;
  AB_TRANSACTION_LIST2_ITERATOR *jit;
  AB_ACCOUNTQUEUE *aq;
  AB_PROVIDERQUEUE *pq;
  int rv;

  /* sort commands by account */

  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Info, I18N("Sorting commands by account"));
  aql=AB_AccountQueue_List_new();
  jit=AB_Transaction_List2_First(commandList);
  if (jit) {
    AB_TRANSACTION *t;

    t=AB_Transaction_List2Iterator_Data(jit);
    while(t) {
      uint32_t uid;

      uid=AB_Transaction_GetUniqueAccountId(t);
      if (uid==0) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "No unique account id given in transaction, aborting");
        AB_AccountQueue_List_free(aql);
        return GWEN_ERROR_BAD_DATA;
      }
      aq=AB_AccountQueue_List_GetByAccountId(aql, uid);
      if (aq==NULL) {
        aq=AB_AccountQueue_new();
        AB_AccountQueue_SetAccountId(aq, uid);
        AB_AccountQueue_List_Add(aq, aql);
      }
      AB_AccountQueue_AddTransaction(aq, t);

      t=AB_Transaction_List2Iterator_Next(jit);
    }
    AB_Transaction_List2Iterator_free(jit);
  } /* if (jit) */

  /* sort account queues by provider */
  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Info, I18N("Sorting commands by provider"));
  pql=AB_ProviderQueue_List_new();
  while( (AB_AccountQueue_List_First(aql)) ) {
    uint32_t uid;
    AB_ACCOUNT_SPEC *as=NULL;
    const char *s;

    uid=AB_AccountQueue_GetAccountId(aq);
    rv=AB_Banking_GetAccountSpecByUniqueId(ab, uid, &as);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to load account spec for account %lu (%d)", (unsigned long int)uid, rv);
      AB_ProviderQueue_List_free(pql);
      AB_AccountQueue_List_free(aql);
      return GWEN_ERROR_BAD_DATA;
    }
    AB_AccountQueue_SetAccountSpec(aq, as);

    s=AB_AccountSpec_GetBackendName(as);
    if (!(s && *s)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Account spec for account %lu has no backend setting", (unsigned long int)uid);
      AB_ProviderQueue_List_free(pql);
      AB_AccountQueue_List_free(aql);
      return GWEN_ERROR_BAD_DATA;
    }

    pq=AB_ProviderQueue_List_GetByProviderName(pql, s);
    if (pq==NULL) {
      pq=AB_ProviderQueue_new();
      AB_ProviderQueue_SetProviderName(pq, s);

      AB_ProviderQueue_List_Add(pq, pql);
    }

    AB_AccountQueue_List_Del(aq);
    AB_ProviderQueue_AddAccountQueue(pq, aq);
  }

  /* send to each backend */
  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Info, I18N("Send commands to providers"));
  pq=AB_ProviderQueue_List_First(pql);
  while(pq) {
    AB_PROVIDERQUEUE *pqNext;
    const char *providerName;

    pqNext=AB_ProviderQueue_List_Next(pq);
    AB_ProviderQueue_List_Del(pq);

    providerName=AB_ProviderQueue_GetProviderName(pq);
    if (providerName && *providerName) {
      AB_PROVIDER *pro;

      pro=AB_Banking_BeginUseProvider(ab, providerName);
      if (pro) {
        AB_IMEXPORTER_CONTEXT *localCtx;

	GWEN_Gui_ProgressLog2(pid, GWEN_LoggerLevel_Info, I18N("Send commands to provider \"%s\""), providerName);
        localCtx=AB_ImExporterContext_new();
        rv=AB_Provider_SendCommands(pro, pq, localCtx);
        if (rv<0) {
	  GWEN_Gui_ProgressLog2(pid, GWEN_LoggerLevel_Error, I18N("Error Sending commands to provider \"%s\":%d"), providerName, rv);
          DBG_INFO(AQBANKING_LOGDOMAIN, "Error sending commands to provider \"%s\" (%d)", AB_Provider_GetName(pro), rv);
        }
        AB_ImExporterContext_AddContext(ctx, localCtx);
        AB_Banking_EndUseProvider(ab, pro);
      }
      else {
	GWEN_Gui_ProgressLog2(pid, GWEN_LoggerLevel_Info, I18N("Provider \"%s\" is not available."), providerName);
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not start using provider \"%s\"", providerName);
      }
    }
    AB_ProviderQueue_free(pq);

    pq=pqNext;
  }

  /* done */
  AB_ProviderQueue_List_free(pql);
  AB_AccountQueue_List_free(aql);

  return 0;
}



int AB_Banking_SendCommands(AB_BANKING *ab, AB_TRANSACTION_LIST2* commandList, AB_IMEXPORTER_CONTEXT *ctx) {
  uint32_t pid;
  int rv;

  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS |
			     GWEN_GUI_PROGRESS_SHOW_PROGRESS |
			     GWEN_GUI_PROGRESS_SHOW_LOG |
			     GWEN_GUI_PROGRESS_ALWAYS_SHOW_LOG |
			     GWEN_GUI_PROGRESS_KEEP_OPEN |
			     GWEN_GUI_PROGRESS_SHOW_ABORT,
			     I18N("Executing Jobs"),
			     I18N("Now the jobs are send via their "
				  "backends to the credit institutes."),
			     0, /* no progress count */
			     0);
  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Notice, "AqBanking v"AQBANKING_VERSION_FULL_STRING);
  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Notice, I18N("Sending jobs to the bank(s)"));

  rv=AB_Banking__SendCommands(ab, commandList, ctx, pid);
  AB_Banking_ClearCryptTokenList(ab);
  if (rv) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
  }

  GWEN_Gui_ProgressEnd(pid);
  return rv;
}










