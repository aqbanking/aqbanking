/***************************************************************************
 begin       : Sat Sep 27 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/



int AB_Banking__GetConfigManager(AB_BANKING *ab, const char *dname) {
  GWEN_BUFFER *buf;
  char home[256];

  if (GWEN_Directory_GetHomeDirectory(home, sizeof(home))) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not determine home directory, aborting.");
    abort();
  }

  buf=GWEN_Buffer_new(0, 256, 0, 1);

  if (dname) {
    /* setup data dir */
    ab->dataDir=strdup(dname);

    /* determine config manager URL */
    GWEN_Buffer_AppendString(buf, "dir://");
    GWEN_Buffer_AppendString(buf, dname);
    GWEN_Buffer_AppendString(buf, DIRSEP);
    GWEN_Buffer_AppendString(buf, "settings");
  }
  else {
    const char *s;
    uint32_t pos;


    GWEN_Buffer_AppendString(buf, "dir://");
    pos=GWEN_Buffer_GetPos(buf);

    /* determine config directory */
    s=getenv("AQBANKING_HOME");
    if (s && !*s)
      s=0;
    if (s)
      GWEN_Buffer_AppendString(buf, s);
    else {
      /* use default */
      GWEN_Buffer_AppendString(buf, home);
      GWEN_Buffer_AppendString(buf, DIRSEP);
      GWEN_Buffer_AppendString(buf, AB_BANKING_USERDATADIR);
    }

    /* as we are at it: store default data dir */
    ab->dataDir=strdup(GWEN_Buffer_GetStart(buf)+pos);

    /* continue with settings folder */
    GWEN_Buffer_AppendString(buf, DIRSEP);
    GWEN_Buffer_AppendString(buf, "settings");

  }

  DBG_INFO(AQBANKING_LOGDOMAIN,
	   "Using data folder [%s]",
	   ab->dataDir);
  DBG_INFO(AQBANKING_LOGDOMAIN,
	   "Using ConfigManager [%s]",
	   GWEN_Buffer_GetStart(buf));

  ab->configMgr=GWEN_ConfigMgr_Factory(GWEN_Buffer_GetStart(buf));
  if (ab->configMgr==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not create ConfigMgr[%s]. "
	      "Maybe the gwenhywfar plugins are not installed?",
	      GWEN_Buffer_GetStart(buf));
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }

  /* done */
  GWEN_Buffer_free(buf);
  return 0;
}



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

    /* check for config manager (created by AB_Banking_Init) */
    if (ab->configMgr==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "No config manager. Maybe the gwenhywfar plugins are not installed correctly?");
      return GWEN_ERROR_GENERIC;
    }

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



