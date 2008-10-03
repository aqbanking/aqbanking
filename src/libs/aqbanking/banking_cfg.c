/***************************************************************************
 begin       : Sat Sep 27 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/



int AB_Banking_LoadAppConfig(AB_BANKING *ab, GWEN_DB_NODE **pDb) {
  assert(ab);
  assert(ab->appName);
  if (ab->appName) {
    int rv;

    rv=GWEN_ConfigMgr_GetGroup(ab->configMgr, AB_CFG_GROUP_APPS, ab->appName, pDb);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not load app group [%s] (%d)",
		ab->appName, rv);
      return rv;
    }
    return 0;
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No application name");
    return GWEN_ERROR_GENERIC;
  }
}



int AB_Banking_SaveAppConfig(AB_BANKING *ab, GWEN_DB_NODE *db) {
  assert(ab);
  assert(ab->appName);
  if (ab->appName) {
    int rv;

    rv=GWEN_ConfigMgr_SetGroup(ab->configMgr, AB_CFG_GROUP_APPS, ab->appName, db);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not save app group [%s] (%d)",
		ab->appName, rv);
      return rv;
    }
    return 0;
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No application name");
    return GWEN_ERROR_GENERIC;
  }
}



int AB_Banking_LockAppConfig(AB_BANKING *ab) {
  assert(ab);
  assert(ab->appName);
  if (ab->appName) {
    int rv;

    rv=GWEN_ConfigMgr_LockGroup(ab->configMgr, AB_CFG_GROUP_APPS, ab->appName);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not lock app group [%s] (%d)",
		ab->appName, rv);
      return rv;
    }
    return 0;
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No application name");
    return GWEN_ERROR_GENERIC;
  }
}



int AB_Banking_UnlockAppConfig(AB_BANKING *ab) {
  assert(ab);
  assert(ab->appName);
  if (ab->appName) {
    int rv;

    rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr, AB_CFG_GROUP_APPS, ab->appName);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not unlock app group [%s] (%d)",
		ab->appName, rv);
      return rv;
    }
    return 0;
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No application name");
    return GWEN_ERROR_GENERIC;
  }
}



int AB_Banking_LoadSharedConfig(AB_BANKING *ab, const char *name, GWEN_DB_NODE **pDb) {
  assert(ab);
  assert(name);
  if (name) {
    int rv;

    rv=GWEN_ConfigMgr_GetGroup(ab->configMgr, AB_CFG_GROUP_SHARED, name, pDb);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not load shared group [%s] (%d)",
		name, rv);
      return rv;
    }
    return 0;
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Name of shared group missing");
    return GWEN_ERROR_GENERIC;
  }
}



int AB_Banking_SaveSharedConfig(AB_BANKING *ab, const char *name, GWEN_DB_NODE *db) {
  assert(ab);
  assert(name);
  if (name) {
    GWEN_DB_NODE *db=NULL;
    int rv;

    rv=GWEN_ConfigMgr_SetGroup(ab->configMgr, AB_CFG_GROUP_SHARED, name, db);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not save shared group [%s] (%d)",
		name, rv);
      return rv;
    }
    return 0;
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Name of shared group missing");
    return GWEN_ERROR_GENERIC;
  }
}



int AB_Banking_LockSharedConfig(AB_BANKING *ab, const char *name) {
  assert(ab);
  assert(name);
  if (name) {
    int rv;

    rv=GWEN_ConfigMgr_LockGroup(ab->configMgr, AB_CFG_GROUP_SHARED, name);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not lock shared group [%s] (%d)",
		name, rv);
      return rv;
    }
    return 0;
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Name of shared group missing");
    return GWEN_ERROR_GENERIC;
  }
}



int AB_Banking_UnlockSharedConfig(AB_BANKING *ab, const char *name) {
  assert(ab);
  assert(name);
  if (name) {
    int rv;

    rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr, AB_CFG_GROUP_SHARED, name);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not unlock shared group [%s] (%d)",
		name, rv);
      return rv;
    }
    return 0;
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Name of shared group missing");
    return GWEN_ERROR_GENERIC;
  }
}



int AB_Banking_LoadPluginConfig(AB_BANKING *ab,
				const char *pluginName,
				const char *name,
				GWEN_DB_NODE **pDb) {
  assert(ab);
  assert(pluginName);
  assert(name);
  if (pluginName && name) {
    int rv;

    rv=GWEN_ConfigMgr_GetGroup(ab->configMgr, pluginName, name, pDb);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not load plugin group [%s] (%d)",
		name, rv);
      return rv;
    }
    return 0;
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Name of plugin group missing");
    return GWEN_ERROR_GENERIC;
  }
}



int AB_Banking_SavePluginConfig(AB_BANKING *ab,
				const char *pluginName,
				const char *name,
				GWEN_DB_NODE *db) {
  assert(ab);
  assert(pluginName);
  assert(name);
  if (pluginName && name) {
    int rv;

    rv=GWEN_ConfigMgr_SetGroup(ab->configMgr, pluginName, name, db);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not save plugin group [%s] (%d)",
		name, rv);
      return rv;
    }
    return 0;
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Name of plugin group missing");
    return GWEN_ERROR_GENERIC;
  }
}



int AB_Banking_LockPluginConfig(AB_BANKING *ab,
				const char *pluginName,
				const char *name) {
  assert(ab);
  assert(pluginName);
  assert(name);
  if (pluginName && name) {
    int rv;

    rv=GWEN_ConfigMgr_LockGroup(ab->configMgr, pluginName, name);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not lock plugin group [%s] (%d)",
		name, rv);
      return rv;
    }
    return 0;
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Name of plugin group missing");
    return GWEN_ERROR_GENERIC;
  }
}



int AB_Banking_UnlockPluginConfig(AB_BANKING *ab,
				  const char *pluginName,
				  const char *name) {
  assert(ab);
  assert(pluginName);
  assert(name);
  if (pluginName && name) {
    int rv;

    rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr, pluginName, name);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not unlock plugin group [%s] (%d)",
		name, rv);
      return rv;
    }
    return 0;
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Name of plugin group missing");
    return GWEN_ERROR_GENERIC;
  }
}



int AB_Banking_LoadAllUsers(AB_BANKING *ab) {
  GWEN_STRINGLIST *sl;
  int rv;

  sl=GWEN_StringList_new();
  rv=GWEN_ConfigMgr_ListSubGroups(ab->configMgr, AB_CFG_GROUP_USERS, sl);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_StringList_free(sl);
    return rv;
  }
  if (GWEN_StringList_Count(sl)) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      const char *t;
      GWEN_DB_NODE *db=NULL;

      t=GWEN_StringListEntry_Data(se);
      assert(t);

      rv=GWEN_ConfigMgr_GetGroup(ab->configMgr, AB_CFG_GROUP_BACKENDS, t, &db);
      if (rv<0) {
	DBG_WARN(AQBANKING_LOGDOMAIN,
		 "Could not load user group [%s] (%d), ignoring",
		 t, rv);
      }
      else {
	AB_USER *u=NULL;
	uint32_t uid;

        assert(db);
	uid=GWEN_DB_GetIntValue(db, "uniqueId", 0, 0);
	if (uid)
	  u=AB_Banking_GetUser(ab, uid);
	if (u) {
	  /* user already exists, reload existing user */
	  const char *s;
	  AB_PROVIDER *pro;

	  s=AB_User_GetBackendName(u);
	  assert(s && *s);
	  pro=AB_Banking_GetProvider(ab, s);
	  if (!pro) {
	    DBG_WARN(AQBANKING_LOGDOMAIN,
		     "Provider \"%s\" not found, ignoring user [%s]",
		     s, AB_User_GetUserId(u));
	  }
	  else {
	    int rv;
	    GWEN_DB_NODE *dbP;

            /* reload user from DB */
	    AB_User_ReadDb(u, db);

            /* let provider also reload user data */
	    dbP=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT,
				 "data/backend");
	    rv=AB_Provider_ExtendUser(pro, u, AB_ProviderExtendMode_Reload, dbP);
	    if (rv<0) {
	      DBG_WARN(AQBANKING_LOGDOMAIN, "Could not extend user [%s] (%d)",
		       AB_User_GetUserId(u), rv);
	    }
	  }
	}
	else {
          /* user is new, load and add it */
	  u=AB_User_fromDb(ab, db);
	  if (u) {
	    const char *s;
	    AB_PROVIDER *pro;
  
	    s=AB_User_GetBackendName(u);
	    assert(s && *s);
	    pro=AB_Banking_GetProvider(ab, s);
	    if (!pro) {
	      DBG_WARN(AQBANKING_LOGDOMAIN,
		       "Provider \"%s\" not found, ignoring user [%s]",
		       s, AB_User_GetUserId(u));
	    }
	    else {
	      int rv;
	      GWEN_DB_NODE *dbP;
  
	      dbP=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT,
				   "data/backend");
	      rv=AB_Provider_ExtendUser(pro, u, AB_ProviderExtendMode_Extend, dbP);
	      if (rv) {
		DBG_INFO(AQBANKING_LOGDOMAIN, "here");
		AB_User_free(u);
	      }
	      else {
		DBG_DEBUG(AQBANKING_LOGDOMAIN, "Adding user");
		AB_User_List_Add(u, ab->users);
	      }
	    }
	  }
	}
        GWEN_DB_Group_free(db);
      }
      se=GWEN_StringListEntry_Next(se);
    }
  }
  GWEN_StringList_free(sl);

  return 0;
}



int AB_Banking_LoadAllAccounts(AB_BANKING *ab) {
  GWEN_STRINGLIST *sl;
  int rv;

  sl=GWEN_StringList_new();
  rv=GWEN_ConfigMgr_ListSubGroups(ab->configMgr, AB_CFG_GROUP_ACCOUNTS, sl);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_StringList_free(sl);
    return rv;
  }
  if (GWEN_StringList_Count(sl)) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      const char *t;
      GWEN_DB_NODE *db=NULL;

      t=GWEN_StringListEntry_Data(se);
      assert(t);

      rv=GWEN_ConfigMgr_GetGroup(ab->configMgr, AB_CFG_GROUP_BACKENDS, t, &db);
      if (rv<0) {
	DBG_WARN(AQBANKING_LOGDOMAIN,
		 "Could not load account group [%s] (%d), ignoring",
		 t, rv);
      }
      else {
	AB_ACCOUNT *a=NULL;
	uint32_t uid;

        assert(db);
	uid=GWEN_DB_GetIntValue(db, "uniqueId", 0, 0);
	if (uid)
	  a=AB_Banking_GetAccount(ab, uid);
	if (a) {
	  /* account already exists, reload existing account */
	  const char *s;
	  AB_PROVIDER *pro;

	  s=AB_Account_GetBackendName(a);
	  assert(s && *s);
	  pro=AB_Banking_GetProvider(ab, s);
	  if (!pro) {
	    DBG_WARN(AQBANKING_LOGDOMAIN,
		     "Provider \"%s\" not found, ignoring account [%s/%s]",
		     s,
		     AB_Account_GetBankCode(a),
		     AB_Account_GetAccountNumber(a));
	  }
	  else {
	    int rv;
	    GWEN_DB_NODE *dbP;

            /* reload account from DB */
	    AB_Account_ReadDb(a, db);

            /* let provider also reload account data */
	    dbP=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT,
				 "data/backend");
	    rv=AB_Provider_ExtendAccount(pro, a, AB_ProviderExtendMode_Reload, dbP);
	    if (rv<0) {
	      DBG_WARN(AQBANKING_LOGDOMAIN, "Could not extend account [%s/%s] (%d)",
		       AB_Account_GetBankCode(a), AB_Account_GetAccountNumber(a), rv);
	    }
	  }
	}
	else {
          /* account is new, load and add it */
	  a=AB_Account_fromDb(ab, db);
	  if (a) {
	    const char *s;
	    AB_PROVIDER *pro;
  
	    s=AB_Account_GetBackendName(a);
	    assert(s && *s);
	    pro=AB_Banking_GetProvider(ab, s);
	    if (!pro) {
	      DBG_WARN(AQBANKING_LOGDOMAIN,
		       "Provider \"%s\" not found, ignoring account [%s/%s]",
		       s, AB_Account_GetBankCode(a), AB_Account_GetAccountNumber(a));
	    }
	    else {
	      int rv;
	      GWEN_DB_NODE *dbP;
  
	      dbP=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT,
				   "data/backend");
	      rv=AB_Provider_ExtendAccount(pro, a, AB_ProviderExtendMode_Extend, dbP);
	      if (rv) {
		DBG_INFO(AQBANKING_LOGDOMAIN, "here");
		AB_Account_free(a);
	      }
	      else {
		DBG_DEBUG(AQBANKING_LOGDOMAIN, "Adding account");
		AB_Account_List_Add(a, ab->accounts);
	      }
	    }
	  }
	}
        GWEN_DB_Group_free(db);
      }
      se=GWEN_StringListEntry_Next(se);
    }
  }
  GWEN_StringList_free(sl);

  return 0;
}



int AB_Banking_LoadConfig(AB_BANKING *ab) {
  int rv;
  GWEN_DB_NODE *db=NULL;

  /* check for config manager (created by AB_Banking_Init) */
  if (ab->configMgr==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "No config manager (maybe the gwenhywfar plugins are not installed?");
    return GWEN_ERROR_GENERIC;
  }

  /* load main group, check version */
  rv=GWEN_ConfigMgr_GetGroup(ab->configMgr, AB_CFG_GROUP_MAIN, "config", &db);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not load main config group (%d)", rv);
    return rv;
  }
  ab->lastVersion=GWEN_DB_GetIntValue(db, "lastVersion", 0, 0);
  GWEN_DB_Group_free(db);

  /* init all providers */
  AB_Banking_ActivateAllProviders(ab);

  /* load all users */
  rv=AB_Banking_LoadAllUsers(ab);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* load all accounts */
  rv=AB_Banking_LoadAllAccounts(ab);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* update active providers if necessary */
  if (AB_Provider_List_GetCount(ab->providers) &&
      ab->lastVersion <
      ((AQBANKING_VERSION_MAJOR<<24) |
       (AQBANKING_VERSION_MINOR<<16) |
       (AQBANKING_VERSION_PATCHLEVEL<<8) |
       AQBANKING_VERSION_BUILD)) {
    AB_PROVIDER *pro;

    pro=AB_Provider_List_First(ab->providers);
    while(pro) {
      int rv;

      rv=AB_Provider_Update(pro, ab->lastVersion,
			    ((AQBANKING_VERSION_MAJOR<<24) |
			     (AQBANKING_VERSION_MINOR<<16) |
			     (AQBANKING_VERSION_PATCHLEVEL<<8) |
			     AQBANKING_VERSION_BUILD));
      if (rv) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Could not update provider \"%s\"",
		  AB_Provider_GetName(pro));
	return rv;
      }

      pro=AB_Provider_List_Next(pro);
    } /* while */
  }

  /* done */
  return 0;
}



int AB_Banking_UnloadConfig(AB_BANKING *ab) {
  AB_PROVIDER *pro;
  int rv;

  assert(ab);

  /* clear all active crypt token */
  AB_Banking_ClearCryptTokenList(ab, 0);

  AB_Account_List_Clear(ab->accounts);
  AB_User_List_Clear(ab->users);

  /* deinit all providers */
  pro=AB_Provider_List_First(ab->providers);
  while(pro) {
    while (AB_Provider_IsInit(pro)) {
      rv=AB_Banking_FiniProvider(ab, pro);
      if (rv) {
	DBG_WARN(AQBANKING_LOGDOMAIN,
		 "Error deinitializing backend \"%s\"",
		 AB_Provider_GetName(pro));
	break;
      }
    }
    pro=AB_Provider_List_Next(pro);
  } /* while */
  AB_Provider_List_Clear(ab->providers);

  return 0;
}



int AB_Banking_SaveConfig(AB_BANKING *ab) {
  GWEN_DB_NODE *db=NULL;
  int rv;

  assert(ab);

  /* check for config manager (created by AB_Banking_Init) */
  if (ab->configMgr==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "No config manager (maybe the gwenhywfar plugins are not installed?");
    return GWEN_ERROR_GENERIC;
  }

  /* lock group */
  rv=GWEN_ConfigMgr_LockGroup(ab->configMgr, AB_CFG_GROUP_MAIN, "config");
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to lock main config group (%d)", rv);
    return rv;
  }

  /* load group (is locked now) */
  rv=GWEN_ConfigMgr_GetGroup(ab->configMgr, AB_CFG_GROUP_MAIN, "config", &db);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not load main config group (%d)", rv);
    return rv;
  }

  /* modify group */
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "lastVersion",
		      (AQBANKING_VERSION_MAJOR<<24) |
		      (AQBANKING_VERSION_MINOR<<16) |
		      (AQBANKING_VERSION_PATCHLEVEL<<8) |
		      AQBANKING_VERSION_BUILD);

  /* save group (still locked) */
  rv=GWEN_ConfigMgr_SetGroup(ab->configMgr, AB_CFG_GROUP_MAIN, "config", db);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not save main config group (%d)", rv);
    GWEN_ConfigMgr_UnlockGroup(ab->configMgr, AB_CFG_GROUP_MAIN, "config");
    GWEN_DB_Group_free(db);
    return rv;
  }

  /* unlock group */
  rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr, AB_CFG_GROUP_MAIN, "config");
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not unlock main config group (%d)", rv);
    GWEN_DB_Group_free(db);
    return rv;
  }

  GWEN_DB_Group_free(db);

  return 0;
}



int AB_Banking_GetUserDataDir(const AB_BANKING *ab, GWEN_BUFFER *buf){
  if (ab->dataDir) {
    GWEN_Buffer_AppendString(buf, ab->dataDir);
    return 0;
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No data dir (not init?)");
    return GWEN_ERROR_GENERIC;
  }
}



int AB_Banking_GetSharedDataDir(const AB_BANKING *ab,
                                const char *name,
                                GWEN_BUFFER *buf){
  assert(ab);
  if (ab->dataDir) {
    GWEN_Buffer_AppendString(buf, ab->dataDir);
    GWEN_Buffer_AppendString(buf, DIRSEP "shared" DIRSEP);
    if (GWEN_Text_EscapeToBufferTolerant(name, buf)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Bad share name, aborting.");
      abort();
    }
    else {
      char *s;
  
      s=GWEN_Buffer_GetStart(buf);
      while(*s) {
	*s=tolower(*s);
	s++;
      }
    }
    return 0;
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No data dir (not init?)");
    return GWEN_ERROR_GENERIC;
  }
}



int AB_Banking_GetAppUserDataDir(const AB_BANKING *ab, GWEN_BUFFER *buf){
  int rv;

  assert(ab->appEscName);
  rv=AB_Banking_GetUserDataDir(ab, buf);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  GWEN_Buffer_AppendString(buf, DIRSEP "apps" DIRSEP);
  GWEN_Buffer_AppendString(buf, ab->appEscName);
  GWEN_Buffer_AppendString(buf, DIRSEP "data");

  return 0;
}



int AB_Banking_GetProviderUserDataDir(const AB_BANKING *ab,
				      const char *name,
				      GWEN_BUFFER *buf){
  int rv;

  rv=AB_Banking_GetUserDataDir(ab, buf);
  if (rv)
    return rv;
  GWEN_Buffer_AppendString(buf, DIRSEP "backends" DIRSEP);
  GWEN_Buffer_AppendString(buf, name);
  GWEN_Buffer_AppendString(buf, DIRSEP "data");
  return 0;
}




