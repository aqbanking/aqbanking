/***************************************************************************
 begin       : Sat Jun 30 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/* This file is included by banking.c */



int AB_Banking6_ReadConfigGroup(AB_BANKING *ab,
                                const char *groupName,
                                uint32_t uniqueId,
                                int doLock,
                                int doUnlock
                                GWEN_DB_NODE **pDb) {
  GWEN_DB_NODE *db=NULL;
  int rv;
  char idBuf[256];

  assert(ab);

  /* check for config manager (created by AB_Banking_Init) */
  if (ab->configMgr==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "No config manager (maybe the gwenhywfar plugins are not installed?");
    return GWEN_ERROR_GENERIC;
  }


  /* make config manager id from given unique id */
  rv=GWEN_ConfigMgr_MkUniqueIdFromId(ab->configMgr, groupName, uniqueId, 0, idBuf, sizeof(idBuf)-1);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to create a unique id for config group (%d)", rv);
    return rv;
  }
  idBuf[sizeof[idBuf]-1]=0;


  /* lock group if requested */
  if (doLock) {
    rv=GWEN_ConfigMgr_LockGroup(ab->configMgr, groupName, idBuf);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to lock config group (%d)", rv);
      return rv;
    }
  }

  /* load group */
  rv=GWEN_ConfigMgr_GetGroup(ab->configMgr, groupName, idBuf, &db);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not load config group (%d)", rv);
    if (doLock)
      GWEN_ConfigMgr_UnlockGroup(ab->configMgr, groupName, idBuf);
    return rv;
  }

  /* unlock group */
  if (doUnlock) {
    rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr, groupName, idBuf);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to unlock accountspec config group (%d)", rv);
      GWEN_DB_Group_free(db);
      return rv;
    }
  }

  *pDb=db;
  return 0;
}



int AB_Banking6_WriteConfigGroup(AB_BANKING *ab,
                                 const char *groupName,
                                 uint32_t uniqueId,
                                 int doLock,
                                 int doUnlock,
                                 GWEN_DB_NODE *db) {
  GWEN_DB_NODE *db=NULL;
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
  idBuf[sizeof[idBuf]-1]=0;

  /* lock group */
  if (doLock) {
    rv=GWEN_ConfigMgr_LockGroup(ab->configMgr, groupName, idBuf);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to lock accountspec config group (%d)", rv);
      return rv;
    }
  }

  /* store group (is locked now) */
  rv=GWEN_ConfigMgr_SetGroup(ab->configMgr, groupName, idBuf, db);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not load config group (%d)", rv);
    if (doLock)
      GWEN_ConfigMgr_UnlockGroup(ab->configMgr, groupName, idBuf);
    return rv;
  }

  /* unlock group */
  if (doUnlock) {
    rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr, AB_CFG_GROUP_ACCOUNTSPECS, idBuf);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to unlock accountspec config group (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int AB_Banking6_ReadConfigGroups(AB_BANKING *ab,
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
      rv=GWEN_ConfigMgr_LockGroup(ab->configMgr, groupName, idBuf);
      if (rv<0) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to lock config group \"%s\" (%d), ignoring", t, rv);
        ignoredGroups++;
      }
      else {
        rv=GWEN_ConfigMgr_GetGroup(ab->configMgr, groupName, t, &db);
        if (rv<0) {
          DBG_WARN(AQBANKING_LOGDOMAIN, "Could not load group [%s] (%d), ignoring", t, rv);
          GWEN_ConfigMgr_UnlockGroup(ab->configMgr, groupName, idBuf);
          ignoredGroups++;
        }
        else {
          int doAdd=1;

          /* unlock after reading */
          rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr, groupName, idBuf);
          if (rv<0) {
            DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not unlock group [%s] (%d)", t, rv);
          }

          assert(db);
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
      *pDb=db;
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










int AB_Banking6_ReadAccountSpec(AB_BANKING *ab, uint32_t uniqueId, AB_ACCOUNT_SPEC **pAccountSpec) {
  AB_ACCOUNT_SPEC *accountSpec;
  GWEN_DB_NODE *db=NULL;
  int rv;

  assert(ab);

  rv=AB_Banking6_ReadConfigGroup(ab, AB_CFG_GROUP_ACCOUNTSPECS, uniqueId, 1, 1, &db);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  accountSpec=AB_AccountSpec_new();
  AB_AccountSpec_ReadDb(accountSpec, db);
  AB_AccountSpec_SetUniqueId(accountSpec, uniqueId);

  GWEN_DB_Group_free(db);

  *pAccountSpec=accountSpec;
  return 0;
}




int AB_Banking6_WriteAccountSpec(AB_BANKING *ab, const AB_ACCOUNT_SPEC *accountSpec) {
  GWEN_DB_NODE *db=NULL;
  int rv;

  assert(ab);

  /* write account spec to DB */
  db=GWEN_DB_Group_new("accountSpec");
  AB_AccountSpec_toDb(a, db);

  rv=AB_Banking6_WriteConfigGroup(ab, AB_CFG_GROUP_ACCOUNTSPECS, uniqueId, 1, 1, db);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(db);
    return rv;
  }
  GWEN_DB_Group_free(db);

  return 0;
}



int AB_Banking6_GetAccountSpecList(AB_BANKING *ab, AB_ACCOUNT_SPEC_LIST** pAccountSpecList) {
  GWEN_DB_NODE *dbAll=NULL;
  int rv;

  rv=AB_Banking6_ReadConfigGroups(ab, AB_CFG_GROUP_ACCOUNTSPECS, "uniqueId", NULL, NULL, &dbAll);
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

    if (AB_AccountSpec_List_Count(accountSpecList)) {
      pAccountSpecList=accountSpecList;
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



int AB_Banking6_LockAndRead_AccountConfig(AB_BANKING *ab, uint32_t uid, GWEN_DB_GROUP **pDb) {
  int rv;

  rv=AB_Banking6_ReadConfigGroup(ab, AB_CFG_GROUP_ACCOUNTS, uid, 1, 0, pDb);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Banking6_WriteAndUnlock_AccountConfig(AB_BANKING *ab, uint32_t uid, GWEN_DB_GROUP *db) {
  int rv;

  rv=AB_Banking6_WriteConfigGroup(ab, AB_CFG_GROUP_ACCOUNTS, uid, 0, 1, db);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}





