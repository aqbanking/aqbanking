/***************************************************************************
 begin       : Sat Sep 29 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/* This file is included by banking.c */




int AB_Banking_UpdateConfList(AB_BANKING *ab, const char *groupName) {
  GWEN_DB_NODE *dbAll=NULL;
  int rv;

  /* read all config groups which have a variable called "uniqueId" */
  rv=AB_Banking_ReadConfigGroups(ab, groupName, "uniqueId", NULL, NULL, &dbAll);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  else {
    GWEN_DB_NODE *db;

    db=GWEN_DB_GetFirstGroup(dbAll);
    while(db) {
      uint32_t uid;
      const char *subGroupName;

      /* get groupName (uid assigned by previous versions of AqBanking) */
      subGroupName=GWEN_DB_GroupName(db);
      assert(subGroupName);

      uid=GWEN_DB_GetIntValue(db, "uniqueId", 0, 0);
      if (uid==0) {
	DBG_WARN(AQBANKING_LOGDOMAIN, "%s: Unique id is ZERO (%s), ignoring group", groupName, subGroupName);
      }
      else {
	char idBuf[256];

	/* create groupname derived from uid */
	rv=GWEN_ConfigMgr_MkUniqueIdFromId(ab->configMgr, groupName, uid, 0, idBuf, sizeof(idBuf)-1);
	if (rv<0) {
	  DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
	  GWEN_DB_Group_free(dbAll);
	  return rv;
	}
  
	/* Compare group names: if the name equals the unique id, all is fine.
	 * Otherwise we have to change the name */
	if (strcmp(subGroupName, idBuf)!=0) {
	  DBG_WARN(AQBANKING_LOGDOMAIN,
		   "%s: Groupname not derived from unique id (%s != %s), creating new group (%lu)",
		   groupName, subGroupName, idBuf, (unsigned long int)uid);
	  rv=AB_Banking_WriteConfigGroup(ab, groupName, uid, 1, 1, db);
	  if (rv<0) {
	    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
	    GWEN_DB_Group_free(dbAll);
	    return rv;
	  }
  
	  DBG_WARN(AQBANKING_LOGDOMAIN, "%s: Removing old group \"%s\" (%lu)", groupName, subGroupName, (unsigned long int)uid);
	  rv=GWEN_ConfigMgr_DeleteGroup(ab->configMgr, groupName, subGroupName);
	  if (rv<0) {
	    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
	    GWEN_DB_Group_free(dbAll);
	    return rv;
	  }
	}
      }

      db=GWEN_DB_GetNextGroup(db);
    } /* while(db) */
    GWEN_DB_Group_free(dbAll);
  } /* else */

  return 0;
}



int AB_Banking_UpdateUserList(AB_BANKING *ab) {
  int rv;

  rv=AB_Banking_UpdateConfList(ab, AB_CFG_GROUP_USERS);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Banking_UpdateAccountList(AB_BANKING *ab) {
  int rv;

  rv=AB_Banking_UpdateConfList(ab, AB_CFG_GROUP_ACCOUNTS);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Banking_Update(AB_BANKING *ab, uint32_t lastVersion, uint32_t currentVersion) {
  DBG_INFO(AQBANKING_LOGDOMAIN, "Updating from version %d.%d.%d.%d",
	   (lastVersion>>24) & 0xff,  (lastVersion>>16)  & 0xff, (lastVersion>>8)  & 0xff, lastVersion & 0xff);

  if (lastVersion<((5<<24) | (99<<16) | (2<<8) | 0)) {
    int rv;


    rv=AB_Banking_UpdateAccountList(ab);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=AB_Banking_UpdateUserList(ab);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=AB_Banking_Update_Account_SetUserId(ab);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=AB_Banking_Update_Account_SetBackendName(ab);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    /* create account specs */
    rv=AB_Banking_Update_Backend_InitDeinit(ab);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}




int AB_Banking_Update_Backend_InitDeinit(AB_BANKING *ab) {
  GWEN_PLUGIN_DESCRIPTION_LIST2 *descrs;
  GWEN_PLUGIN_MANAGER *pm;

  DBG_INFO(AQBANKING_LOGDOMAIN, "Updating to 5.99.2.0");

  pm=GWEN_PluginManager_FindPluginManager("provider");
  if (!pm) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not find plugin manager for \"%s\"",
	      "provider");
    return GWEN_ERROR_INTERNAL;
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

      pro=AB_Banking_BeginUseProvider(ab, pname);
      if (!pro) {
	DBG_WARN(AQBANKING_LOGDOMAIN,
		 "Could not load backend \"%s\", ignoring", pname);
      }
      else {
	int rv;

	DBG_NOTICE(AQBANKING_LOGDOMAIN, "Initializing backend \"%s\"", pname);
	rv=AB_Banking_EndUseProvider(ab, pro);
	if (rv<0) {
	  DBG_WARN(AQBANKING_LOGDOMAIN, "Error initializing backend \"%s\" (%d), ignoring", pname, rv);
	}
      }

      pd=GWEN_PluginDescription_List2Iterator_Next(it);
    } /* while */
    GWEN_PluginDescription_List2Iterator_free(it);
    GWEN_PluginDescription_List2_freeAll(descrs);
  }

  return 0;
}



int AB_Banking_Update_Account_SetUserId(AB_BANKING *ab) {
  GWEN_DB_NODE *dbAll=NULL;
  int rv;

  DBG_INFO(AQBANKING_LOGDOMAIN, "Set UserId in accounts");

  /* read all config groups which have a variable called "uniqueId" */
  rv=AB_Banking_ReadConfigGroups(ab, AB_CFG_GROUP_ACCOUNTS, "uniqueId", NULL, NULL, &dbAll);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  else {
    GWEN_DB_NODE *db;

    db=GWEN_DB_GetFirstGroup(dbAll);
    while(db) {
      uint32_t uid;
      const char *subGroupName;

      /* get groupName (uid assigned by previous versions of AqBanking) */
      subGroupName=GWEN_DB_GroupName(db);
      assert(subGroupName);

      uid=GWEN_DB_GetIntValue(db, "uniqueId", 0, 0);
      if (uid==0) {
	DBG_WARN(AQBANKING_LOGDOMAIN, "%s: Unique id is ZERO (%s), ignoring group", AB_CFG_GROUP_ACCOUNTS, subGroupName);
      }
      else {
	int d;

	/* create new var */
	d=GWEN_DB_GetIntValue(db, "user", 0, 0);
	GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "userId", d);
	/* delete old vars */
	GWEN_DB_DeleteVar(db, "user");
	GWEN_DB_DeleteVar(db, "selectedUser");

        /* write back */
        rv=AB_Banking_WriteConfigGroup(ab, AB_CFG_GROUP_ACCOUNTS, uid, 1, 1, db);
        if (rv<0) {
            DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
            GWEN_DB_Group_free(dbAll);
            return rv;
        }
      }

      db=GWEN_DB_GetNextGroup(db);
    } /* while(db) */
    GWEN_DB_Group_free(dbAll);
  } /* else */

  return 0;
}



int AB_Banking_Update_Account_SetBackendName(AB_BANKING *ab) {
  GWEN_DB_NODE *dbAll=NULL;
  int rv;

  DBG_INFO(AQBANKING_LOGDOMAIN, "Set BackendName in accounts");

  /* read all config groups which have a variable called "uniqueId" */
  rv=AB_Banking_ReadConfigGroups(ab, AB_CFG_GROUP_ACCOUNTS, "uniqueId", NULL, NULL, &dbAll);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  else {
    GWEN_DB_NODE *db;

    db=GWEN_DB_GetFirstGroup(dbAll);
    while(db) {
      uint32_t uid;
      const char *subGroupName;

      /* get groupName (uid assigned by previous versions of AqBanking) */
      subGroupName=GWEN_DB_GroupName(db);
      assert(subGroupName);

      uid=GWEN_DB_GetIntValue(db, "uniqueId", 0, 0);
      if (uid==0) {
	DBG_WARN(AQBANKING_LOGDOMAIN, "%s: Unique id is ZERO (%s), ignoring group", AB_CFG_GROUP_ACCOUNTS, subGroupName);
      }
      else {
        const char *s;

	/* create new var */
        s=GWEN_DB_GetCharValue(db, "provider", 0, NULL);
        GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "backendName", s);
	/* delete old var */
        GWEN_DB_DeleteVar(db, "provider");

        /* write back */
        rv=AB_Banking_WriteConfigGroup(ab, AB_CFG_GROUP_ACCOUNTS, uid, 1, 1, db);
        if (rv<0) {
            DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
            GWEN_DB_Group_free(dbAll);
            return rv;
        }
      }

      db=GWEN_DB_GetNextGroup(db);
    } /* while(db) */
    GWEN_DB_Group_free(dbAll);
  } /* else */

  return 0;
}




