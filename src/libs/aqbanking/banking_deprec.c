/***************************************************************************
 begin       : Tue Sep 30 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/* This file is included by banking.c */


int AB_Banking__ImportConfDir(AB_BANKING *ab,
			      const char *path,
			      const char *groupName) {
  GWEN_STRINGLIST *sl;
  GWEN_BUFFER *nbuf;
  int rv;
  uint32_t pos;

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(nbuf, path);

  sl=GWEN_StringList_new();
  rv=GWEN_Directory_GetDirEntries(GWEN_Buffer_GetStart(nbuf), sl, NULL);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No configurations for group [%s]", groupName);
  }
  else {
    GWEN_STRINGLISTENTRY *se;

    GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
    pos=GWEN_Buffer_GetPos(nbuf);

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      GWEN_DB_NODE *dbConfig=NULL;
      GWEN_BUFFER *gbuf;

      /* get group name */
      gbuf=GWEN_Buffer_new(0, 256, 0, 1);
      rv=GWEN_Text_UnescapeToBuffer(GWEN_StringListEntry_Data(se), gbuf);
      if (rv<0) {
	DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
	GWEN_Buffer_free(gbuf);
	GWEN_DB_Group_free(dbConfig);
	GWEN_StringList_free(sl);
	GWEN_Buffer_free(nbuf);
	return rv;
      }
      GWEN_Buffer_AppendString(nbuf, GWEN_StringListEntry_Data(se));
      GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
      GWEN_Buffer_AppendString(nbuf, "settings.conf");

      DBG_DEBUG(AQBANKING_LOGDOMAIN, "Reading file [%s]", GWEN_Buffer_GetStart(nbuf));

      if (!GWEN_Directory_GetPath(GWEN_Buffer_GetStart(nbuf),
				  GWEN_PATH_FLAGS_PATHMUSTEXIST |
				  GWEN_PATH_FLAGS_NAMEMUSTEXIST |
				  GWEN_PATH_FLAGS_VARIABLE)) {
	dbConfig=GWEN_DB_Group_new("shared");
	rv=GWEN_DB_ReadFile(dbConfig,
			    GWEN_Buffer_GetStart(nbuf),
			    GWEN_DB_FLAGS_DEFAULT |
			    GWEN_PATH_FLAGS_CREATE_GROUP |
			    GWEN_DB_FLAGS_ALLOW_EMPTY_STREAM);
	if (rv<0) {
	  DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
	  GWEN_DB_Group_free(dbConfig);
	  GWEN_Buffer_free(gbuf);
	  GWEN_StringList_free(sl);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}
  
	rv=GWEN_ConfigMgr_LockGroup(ab->configMgr,
				    groupName,
				    GWEN_Buffer_GetStart(gbuf));
	if (rv<0) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Unable to lock shared config [%s] (%d)",
		    GWEN_Buffer_GetStart(gbuf), rv);
	  GWEN_DB_Group_free(dbConfig);
	  GWEN_Buffer_free(gbuf);
	  GWEN_StringList_free(sl);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}
  
	rv=GWEN_ConfigMgr_SetGroup(ab->configMgr,
				   groupName,
				   GWEN_Buffer_GetStart(gbuf),
				   dbConfig);
  
	if (rv<0) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Unable to save shared config [%s] (%d)",
		    GWEN_Buffer_GetStart(gbuf), rv);
	  GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
				     groupName,
				     GWEN_Buffer_GetStart(gbuf));
	  GWEN_DB_Group_free(dbConfig);
	  GWEN_Buffer_free(gbuf);
	  GWEN_StringList_free(sl);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}
  
	/* unlock */
	rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
				      groupName,
				      GWEN_Buffer_GetStart(gbuf));
	if (rv<0) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Unable to unlock shared config [%s] (%d)",
		    GWEN_Buffer_GetStart(gbuf), rv);
	  GWEN_DB_Group_free(dbConfig);
	  GWEN_Buffer_free(gbuf);
	  GWEN_StringList_free(sl);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}
      } /* if settings.conf in folder */
      else {
	DBG_INFO(AQBANKING_LOGDOMAIN, "Path [%s] not found",
                 GWEN_Buffer_GetStart(nbuf));
      }

      GWEN_DB_Group_free(dbConfig);
      GWEN_Buffer_free(gbuf);
      GWEN_Buffer_Crop(nbuf, 0, pos);
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }
  GWEN_StringList_free(sl);
  GWEN_Buffer_free(nbuf);

  return 0;
}



int AB_Banking_ImportConf3(AB_BANKING *ab) {
  char home[256];
  GWEN_BUFFER *nbuf;
  uint32_t pos;
  GWEN_DB_NODE *dbSettings;
  GWEN_DB_NODE *db;
  int rv;
  uint32_t highestUid=0;
  uint32_t lastVersion;

  /* check for config manager (created by AB_Banking_Init) */
  if (ab->configMgr==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "No config manager. Maybe the gwenhywfar plugins are not installed correctly?");
    return GWEN_ERROR_GENERIC;
  }

  rv=GWEN_Directory_GetHomeDirectory(home, sizeof(home)-1);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(nbuf, home);
  GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
  GWEN_Buffer_AppendString(nbuf, ".aqbanking");
  GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
  pos=GWEN_Buffer_GetPos(nbuf);

  GWEN_Buffer_AppendString(nbuf, "settings.conf");

  dbSettings=GWEN_DB_Group_new("settings");
  rv=GWEN_DB_ReadFile(dbSettings,
		      GWEN_Buffer_GetStart(nbuf),
		      GWEN_DB_FLAGS_DEFAULT |
		      GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbSettings);
    GWEN_Buffer_free(nbuf);
    return rv;
  }

  /* now the settings.conf has been read, copy stuff */
  lastVersion=GWEN_DB_GetIntValue(dbSettings, "lastVersion", 0, 0);

  /* import backends */
  db=GWEN_DB_GetGroup(dbSettings,
		      GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		      "backends");
  if (db) {
    db=GWEN_DB_GetFirstGroup(db);
    while(db) {
      const char *groupName;

      groupName=GWEN_DB_GroupName(db);
      rv=AB_Banking_LockPluginConfig(ab,
				     AB_CFG_GROUP_BACKENDS,
				     groupName);
      if (rv<0) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Unable to lock plugin config [%s] (%d)",
		  groupName, rv);
	GWEN_DB_Group_free(dbSettings);
	GWEN_Buffer_free(nbuf);
	return rv;
      }

      rv=AB_Banking_SavePluginConfig(ab,
				     AB_CFG_GROUP_BACKENDS,
				     groupName,
				     db);
      if (rv<0) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Unable to save plugin config [%s] (%d)",
		  groupName, rv);
	AB_Banking_UnlockPluginConfig(ab,
				      AB_CFG_GROUP_BACKENDS,
				      groupName);
	GWEN_DB_Group_free(dbSettings);
	GWEN_Buffer_free(nbuf);
	return rv;
      }

      /* unlock */
      rv=AB_Banking_UnlockPluginConfig(ab,
				       AB_CFG_GROUP_BACKENDS,
				       groupName);
      if (rv<0) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Unable to unlock plugin config [%s] (%d)",
		  groupName, rv);
	GWEN_DB_Group_free(dbSettings);
	GWEN_Buffer_free(nbuf);
	return rv;
      }

      db=GWEN_DB_GetNextGroup(db);
    } /* while */
  } /* if backends */


  /* import users */
  db=GWEN_DB_GetGroup(dbSettings,
		      GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		      "users");
  if (db) {
    db=GWEN_DB_FindFirstGroup(db, "user");
    while(db) {
      uint32_t uid;

      uid=GWEN_DB_GetIntValue(db, "uniqueId", 0, 0);
      if (uid==0) {
	DBG_WARN(AQBANKING_LOGDOMAIN, "No unique id for user");
      }
      else {
	char groupName[32];

	if (uid>highestUid)
	  highestUid=uid;

	rv=GWEN_ConfigMgr_GetUniqueId(ab->configMgr,
				      AB_CFG_GROUP_USERS,
				      groupName, sizeof(groupName)-1);
	if (rv<0) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Unable to create a unique id for user [%08x] (%d)",
		    uid, rv);
	  GWEN_DB_Group_free(dbSettings);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}
	groupName[sizeof(groupName)-1]=0;

	rv=GWEN_ConfigMgr_LockGroup(ab->configMgr,
				    AB_CFG_GROUP_USERS,
				    groupName);
	if (rv<0) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Unable to lock user config [%08x] (%d)",
		    uid, rv);
	  GWEN_DB_Group_free(dbSettings);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}

	rv=GWEN_ConfigMgr_SetGroup(ab->configMgr,
				   AB_CFG_GROUP_USERS,
				   groupName,
				   db);

	if (rv<0) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Unable to save user config [%08x] (%d)",
		    uid, rv);
	  GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
				     AB_CFG_GROUP_USERS,
				     groupName);
	  GWEN_DB_Group_free(dbSettings);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}
  
	/* unlock */
	rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
				      AB_CFG_GROUP_USERS,
				      groupName);
	if (rv<0) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Unable to unlock user config [%08x] (%d)",
		    uid, rv);
	  GWEN_DB_Group_free(dbSettings);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}
      } /* if unique id */

      db=GWEN_DB_FindNextGroup(db, "user");
    } /* while */
  } /* if users */


  /* import accounts */
  db=GWEN_DB_GetGroup(dbSettings,
		      GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		      "accounts");
  if (db) {
    db=GWEN_DB_FindFirstGroup(db, "account");
    while(db) {
      uint32_t uid;

      uid=GWEN_DB_GetIntValue(db, "uniqueId", 0, 0);
      if (uid==0) {
	DBG_WARN(AQBANKING_LOGDOMAIN, "No unique id for account");
      }
      else {
	char groupName[32];

	if (uid>highestUid)
	  highestUid=uid;

	rv=GWEN_ConfigMgr_GetUniqueId(ab->configMgr,
				      AB_CFG_GROUP_ACCOUNTS,
				      groupName, sizeof(groupName)-1);
	if (rv<0) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Unable to create a unique id for account [%08x] (%d)",
		    uid, rv);
	  GWEN_DB_Group_free(dbSettings);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}
	groupName[sizeof(groupName)-1]=0;

	rv=GWEN_ConfigMgr_LockGroup(ab->configMgr,
				    AB_CFG_GROUP_ACCOUNTS,
				    groupName);
	if (rv<0) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Unable to lock account config [%08x] (%d)",
		    uid, rv);
	  GWEN_DB_Group_free(dbSettings);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}

	rv=GWEN_ConfigMgr_SetGroup(ab->configMgr,
				   AB_CFG_GROUP_ACCOUNTS,
				   groupName,
				   db);

	if (rv<0) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Unable to save account config [%08x] (%d)",
		    uid, rv);
	  GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
				     AB_CFG_GROUP_ACCOUNTS,
				     groupName);
	  GWEN_DB_Group_free(dbSettings);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}
  
	/* unlock */
	rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
				      AB_CFG_GROUP_ACCOUNTS,
				      groupName);
	if (rv<0) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Unable to unlock account config [%08x] (%d)",
		    uid, rv);
	  GWEN_DB_Group_free(dbSettings);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}
      } /* if unique id */

      db=GWEN_DB_FindNextGroup(db, "account");
    } /* while */
  } /* if accounts */


  /* save main configuration */
  rv=GWEN_ConfigMgr_LockGroup(ab->configMgr,
			      AB_CFG_GROUP_MAIN,
			      "config");
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to lock main config (%d)", rv);
    GWEN_DB_Group_free(dbSettings);
    GWEN_Buffer_free(nbuf);
    return rv;
  }

  GWEN_DB_ClearGroup(dbSettings, NULL);
  GWEN_DB_SetIntValue(dbSettings, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "lastVersion", lastVersion);
  rv=GWEN_ConfigMgr_SetGroup(ab->configMgr,
			     AB_CFG_GROUP_MAIN,
			     "config",
			     dbSettings);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to save main config (%d)", rv);
    GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
			       AB_CFG_GROUP_MAIN,
			       "config");
    GWEN_DB_Group_free(dbSettings);
    GWEN_Buffer_free(nbuf);
    return rv;
  }

  /* unlock */
  rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
				AB_CFG_GROUP_MAIN,
				"config");
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to unlock main config (%d)", rv);
    GWEN_DB_Group_free(dbSettings);
    GWEN_Buffer_free(nbuf);
    return rv;
  }
  GWEN_DB_Group_free(dbSettings);


  /* import shared configurations */
  GWEN_Buffer_Crop(nbuf, 0, pos);
  GWEN_Buffer_AppendString(nbuf, "shared");
  rv=AB_Banking__ImportConfDir(ab,
			       GWEN_Buffer_GetStart(nbuf),
			       AB_CFG_GROUP_SHARED);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(nbuf);
    return rv;
  }

  /* import app configurations */
  GWEN_Buffer_Crop(nbuf, 0, pos);
  GWEN_Buffer_AppendString(nbuf, "apps");
  rv=AB_Banking__ImportConfDir(ab,
			       GWEN_Buffer_GetStart(nbuf),
			       AB_CFG_GROUP_APPS);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(nbuf);
    return rv;
  }

  GWEN_Buffer_free(nbuf);

  if (highestUid) {
    rv=AB_Banking_SetUniqueId(ab, highestUid);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Unable to store highest unique id used (%d)",
		rv);
      return rv;
    }
  }

  return 0;
}



int AB_Banking_HasConf3(AB_BANKING *ab) {
  char home[256];
  GWEN_BUFFER *nbuf;
  //uint32_t pos;
  GWEN_DB_NODE *dbSettings;
  GWEN_DB_NODE *db;
  int rv;
  int backends=0;
  int users=0;
  int accounts=0;

  rv=GWEN_Directory_GetHomeDirectory(home, sizeof(home)-1);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(nbuf, home);
  GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
  GWEN_Buffer_AppendString(nbuf, ".aqbanking");
  GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
  //pos=GWEN_Buffer_GetPos(nbuf);

  GWEN_Buffer_AppendString(nbuf, "settings.conf");

  if (GWEN_Directory_GetPath(GWEN_Buffer_GetStart(nbuf),
			     GWEN_PATH_FLAGS_PATHMUSTEXIST |
			     GWEN_PATH_FLAGS_NAMEMUSTEXIST |
			     GWEN_PATH_FLAGS_VARIABLE)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No settings.conf");
    GWEN_Buffer_free(nbuf);
    return GWEN_ERROR_NOT_FOUND;
  }

  dbSettings=GWEN_DB_Group_new("settings");
  rv=GWEN_DB_ReadFile(dbSettings,
		      GWEN_Buffer_GetStart(nbuf),
		      GWEN_DB_FLAGS_DEFAULT |
		      GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbSettings);
    GWEN_Buffer_free(nbuf);
    return rv;
  }

  /* count backends */
  db=GWEN_DB_GetGroup(dbSettings,
		      GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		      "backends");
  if (db) {
    db=GWEN_DB_GetFirstGroup(db);
    while(db) {
      backends++;
      db=GWEN_DB_GetNextGroup(db);
    } /* while */
  } /* if backends */


  /* count users */
  db=GWEN_DB_GetGroup(dbSettings,
		      GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		      "users");
  if (db) {
    db=GWEN_DB_FindFirstGroup(db, "user");
    while(db) {
      uint32_t uid;

      uid=GWEN_DB_GetIntValue(db, "uniqueId", 0, 0);
      if (uid==0) {
	DBG_WARN(AQBANKING_LOGDOMAIN, "No unique id for user");
      }
      else {
	users++;
      } /* if unique id */

      db=GWEN_DB_FindNextGroup(db, "user");
    } /* while */
  } /* if users */


  /* count accounts */
  db=GWEN_DB_GetGroup(dbSettings,
		      GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		      "accounts");
  if (db) {
    db=GWEN_DB_FindFirstGroup(db, "account");
    while(db) {
      uint32_t uid;

      uid=GWEN_DB_GetIntValue(db, "uniqueId", 0, 0);
      if (uid==0) {
	DBG_WARN(AQBANKING_LOGDOMAIN, "No unique id for account");
      }
      else {
        accounts++;
      } /* if unique id */

      db=GWEN_DB_FindNextGroup(db, "account");
    } /* while */
  } /* if accounts */
  GWEN_DB_Group_free(dbSettings);
  GWEN_Buffer_free(nbuf);

  if (users)
    return 0;
  if (backends || accounts)
    return GWEN_ERROR_PARTIAL;

  return GWEN_ERROR_NO_DATA;
}




int AB_Banking_ImportConf2(AB_BANKING *ab) {
  char home[256];
  GWEN_BUFFER *nbuf;
  uint32_t pos;
  GWEN_DB_NODE *dbSettings;
  GWEN_DB_NODE *db;
  int rv;
  uint32_t highestUid=0;
  //uint32_t lastVersion;

  /* check for config manager (created by AB_Banking_Init) */
  if (ab->configMgr==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "No config manager. Maybe the gwenhywfar plugins are not installed correctly?");
    return GWEN_ERROR_GENERIC;
  }

  rv=GWEN_Directory_GetHomeDirectory(home, sizeof(home)-1);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(nbuf, home);
  GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
  GWEN_Buffer_AppendString(nbuf, ".banking");
  GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
  pos=GWEN_Buffer_GetPos(nbuf);

  GWEN_Buffer_AppendString(nbuf, "settings.conf");

  dbSettings=GWEN_DB_Group_new("settings");
  rv=GWEN_DB_ReadFile(dbSettings,
		      GWEN_Buffer_GetStart(nbuf),
		      GWEN_DB_FLAGS_DEFAULT |
		      GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbSettings);
    GWEN_Buffer_free(nbuf);
    return rv;
  }

  /* now the settings.conf has been read, copy stuff */
  //lastVersion=GWEN_DB_GetIntValue(dbSettings, "lastVersion", 0, 0);

  /* import backends */
  db=GWEN_DB_GetGroup(dbSettings,
		      GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		      "backends");
  if (db) {
    db=GWEN_DB_GetFirstGroup(db);
    while(db) {
      const char *groupName;

      groupName=GWEN_DB_GroupName(db);
      rv=AB_Banking_LockPluginConfig(ab,
				     AB_CFG_GROUP_BACKENDS,
				     groupName);
      if (rv<0) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Unable to lock plugin config [%s] (%d)",
		  groupName, rv);
	GWEN_DB_Group_free(dbSettings);
	GWEN_Buffer_free(nbuf);
	return rv;
      }

      rv=AB_Banking_SavePluginConfig(ab,
				     AB_CFG_GROUP_BACKENDS,
				     groupName,
				     db);
      if (rv<0) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Unable to save plugin config [%s] (%d)",
		  groupName, rv);
	AB_Banking_UnlockPluginConfig(ab,
				      AB_CFG_GROUP_BACKENDS,
				      groupName);
	GWEN_DB_Group_free(dbSettings);
	GWEN_Buffer_free(nbuf);
	return rv;
      }

      /* unlock */
      rv=AB_Banking_UnlockPluginConfig(ab,
				       AB_CFG_GROUP_BACKENDS,
				       groupName);
      if (rv<0) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Unable to unlock plugin config [%s] (%d)",
		  groupName, rv);
	GWEN_DB_Group_free(dbSettings);
	GWEN_Buffer_free(nbuf);
	return rv;
      }

      db=GWEN_DB_GetNextGroup(db);
    } /* while */
  } /* if backends */


  /* import users */
  db=GWEN_DB_GetGroup(dbSettings,
		      GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		      "users");
  if (db) {
    db=GWEN_DB_FindFirstGroup(db, "user");
    while(db) {
      uint32_t uid;

      uid=GWEN_DB_GetIntValue(db, "uniqueId", 0, 0);
      if (uid==0) {
	DBG_WARN(AQBANKING_LOGDOMAIN, "No unique id for user");
      }
      else {
	char groupName[32];

	if (uid>highestUid)
	  highestUid=uid;

	rv=GWEN_ConfigMgr_GetUniqueId(ab->configMgr,
				      AB_CFG_GROUP_USERS,
				      groupName, sizeof(groupName)-1);
	if (rv<0) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Unable to create a unique id for user [%08x] (%d)",
		    uid, rv);
	  GWEN_DB_Group_free(dbSettings);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}
	groupName[sizeof(groupName)-1]=0;

	rv=GWEN_ConfigMgr_LockGroup(ab->configMgr,
				    AB_CFG_GROUP_USERS,
				    groupName);
	if (rv<0) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Unable to lock user config [%08x] (%d)",
		    uid, rv);
	  GWEN_DB_Group_free(dbSettings);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}

	rv=GWEN_ConfigMgr_SetGroup(ab->configMgr,
				   AB_CFG_GROUP_USERS,
				   groupName,
				   db);

	if (rv<0) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Unable to save user config [%08x] (%d)",
		    uid, rv);
	  GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
				     AB_CFG_GROUP_USERS,
				     groupName);
	  GWEN_DB_Group_free(dbSettings);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}
  
	/* unlock */
	rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
				      AB_CFG_GROUP_USERS,
				      groupName);
	if (rv<0) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Unable to unlock user config [%08x] (%d)",
		    uid, rv);
	  GWEN_DB_Group_free(dbSettings);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}
      } /* if unique id */

      db=GWEN_DB_FindNextGroup(db, "user");
    } /* while */
  } /* if users */


  /* import accounts */
  db=GWEN_DB_GetGroup(dbSettings,
		      GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		      "accounts");
  if (db) {
    db=GWEN_DB_FindFirstGroup(db, "account");
    while(db) {
      uint32_t uid;

      uid=GWEN_DB_GetIntValue(db, "uniqueId", 0, 0);
      if (uid==0) {
	DBG_WARN(AQBANKING_LOGDOMAIN, "No unique id for account");
      }
      else {
	char groupName[32];

	if (uid>highestUid)
	  highestUid=uid;

	rv=GWEN_ConfigMgr_GetUniqueId(ab->configMgr,
				      AB_CFG_GROUP_ACCOUNTS,
				      groupName, sizeof(groupName)-1);
	if (rv<0) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Unable to create a unique id for account [%08x] (%d)",
		    uid, rv);
	  GWEN_DB_Group_free(dbSettings);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}
	groupName[sizeof(groupName)-1]=0;

	rv=GWEN_ConfigMgr_LockGroup(ab->configMgr,
				    AB_CFG_GROUP_ACCOUNTS,
				    groupName);
	if (rv<0) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Unable to lock account config [%08x] (%d)",
		    uid, rv);
	  GWEN_DB_Group_free(dbSettings);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}

	rv=GWEN_ConfigMgr_SetGroup(ab->configMgr,
				   AB_CFG_GROUP_ACCOUNTS,
				   groupName,
				   db);

	if (rv<0) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Unable to save account config [%08x] (%d)",
		    uid, rv);
	  GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
				     AB_CFG_GROUP_ACCOUNTS,
				     groupName);
	  GWEN_DB_Group_free(dbSettings);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}
  
	/* unlock */
	rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
				      AB_CFG_GROUP_ACCOUNTS,
				      groupName);
	if (rv<0) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Unable to unlock account config [%08x] (%d)",
		    uid, rv);
	  GWEN_DB_Group_free(dbSettings);
	  GWEN_Buffer_free(nbuf);
	  return rv;
	}
      } /* if unique id */

      db=GWEN_DB_FindNextGroup(db, "account");
    } /* while */
  } /* if accounts */
  GWEN_DB_Group_free(dbSettings);

  /* import shared configurations */
  GWEN_Buffer_Crop(nbuf, 0, pos);
  GWEN_Buffer_AppendString(nbuf, "shared");
  rv=AB_Banking__ImportConfDir(ab,
			       GWEN_Buffer_GetStart(nbuf),
			       AB_CFG_GROUP_SHARED);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(nbuf);
    return rv;
  }

  /* import app configurations */
  GWEN_Buffer_Crop(nbuf, 0, pos);
  GWEN_Buffer_AppendString(nbuf, "apps");
  rv=AB_Banking__ImportConfDir(ab,
			       GWEN_Buffer_GetStart(nbuf),
			       AB_CFG_GROUP_APPS);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(nbuf);
    return rv;
  }

  GWEN_Buffer_free(nbuf);

  if (highestUid) {
    rv=AB_Banking_SetUniqueId(ab, highestUid);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Unable to store highest unique id used (%d)",
		rv);
      return rv;
    }
  }

  return 0;
}



int AB_Banking_HasConf2(AB_BANKING *ab) {
  char home[256];
  GWEN_BUFFER *nbuf;
  //uint32_t pos;
  GWEN_DB_NODE *dbSettings;
  GWEN_DB_NODE *db;
  int rv;
  int backends=0;
  int users=0;
  int accounts=0;

  rv=GWEN_Directory_GetHomeDirectory(home, sizeof(home)-1);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(nbuf, home);
  GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
  GWEN_Buffer_AppendString(nbuf, ".banking");
  GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
  //pos=GWEN_Buffer_GetPos(nbuf);

  GWEN_Buffer_AppendString(nbuf, "settings.conf");

  if (GWEN_Directory_GetPath(GWEN_Buffer_GetStart(nbuf),
			     GWEN_PATH_FLAGS_PATHMUSTEXIST |
			     GWEN_PATH_FLAGS_NAMEMUSTEXIST |
			     GWEN_PATH_FLAGS_VARIABLE)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No settings.conf");
    GWEN_Buffer_free(nbuf);
    return GWEN_ERROR_NOT_FOUND;
  }

  dbSettings=GWEN_DB_Group_new("settings");
  rv=GWEN_DB_ReadFile(dbSettings,
		      GWEN_Buffer_GetStart(nbuf),
		      GWEN_DB_FLAGS_DEFAULT |
		      GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbSettings);
    GWEN_Buffer_free(nbuf);
    return rv;
  }

  /* count backends */
  db=GWEN_DB_GetGroup(dbSettings,
		      GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		      "backends");
  if (db) {
    db=GWEN_DB_GetFirstGroup(db);
    while(db) {
      backends++;
      db=GWEN_DB_GetNextGroup(db);
    } /* while */
  } /* if backends */


  /* count users */
  db=GWEN_DB_GetGroup(dbSettings,
		      GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		      "users");
  if (db) {
    db=GWEN_DB_FindFirstGroup(db, "user");
    while(db) {
      uint32_t uid;

      uid=GWEN_DB_GetIntValue(db, "uniqueId", 0, 0);
      if (uid==0) {
	DBG_WARN(AQBANKING_LOGDOMAIN, "No unique id for user");
      }
      else {
	users++;
      } /* if unique id */

      db=GWEN_DB_FindNextGroup(db, "user");
    } /* while */
  } /* if users */


  /* count accounts */
  db=GWEN_DB_GetGroup(dbSettings,
		      GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		      "accounts");
  if (db) {
    db=GWEN_DB_FindFirstGroup(db, "account");
    while(db) {
      uint32_t uid;

      uid=GWEN_DB_GetIntValue(db, "uniqueId", 0, 0);
      if (uid==0) {
	DBG_WARN(AQBANKING_LOGDOMAIN, "No unique id for account");
      }
      else {
        accounts++;
      } /* if unique id */

      db=GWEN_DB_FindNextGroup(db, "account");
    } /* while */
  } /* if accounts */
  GWEN_DB_Group_free(dbSettings);
  GWEN_Buffer_free(nbuf);

  if (users)
    return 0;
  if (backends || accounts)
    return GWEN_ERROR_PARTIAL;

  return GWEN_ERROR_NO_DATA;
}



