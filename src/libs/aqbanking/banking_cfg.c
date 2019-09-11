/***************************************************************************
 begin       : Sat Sep 27 2008
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/



int AB_Banking__GetConfigManager(AB_BANKING *ab, const char *dname)
{
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
    GWEN_Buffer_AppendString(buf, AB_BANKING_SETTINGS_DIR);
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
    GWEN_Buffer_AppendString(buf, AB_BANKING_SETTINGS_DIR);

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



int AB_Banking_LoadSharedConfig(AB_BANKING *ab, const char *name, GWEN_DB_NODE **pDb)
{
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



int AB_Banking_SaveSharedConfig(AB_BANKING *ab, const char *name, GWEN_DB_NODE *db)
{
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



int AB_Banking_LockSharedConfig(AB_BANKING *ab, const char *name)
{
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



int AB_Banking_UnlockSharedConfig(AB_BANKING *ab, const char *name)
{
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



int AB_Banking_GetUserDataDir(const AB_BANKING *ab, GWEN_BUFFER *buf)
{
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
                                GWEN_BUFFER *buf)
{
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
      while (*s) {
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



int AB_Banking_GetAppUserDataDir(const AB_BANKING *ab, GWEN_BUFFER *buf)
{
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
                                      GWEN_BUFFER *buf)
{
  int rv;

  rv=AB_Banking_GetUserDataDir(ab, buf);
  if (rv)
    return rv;
  GWEN_Buffer_AppendString(buf, DIRSEP "backends" DIRSEP);
  GWEN_Buffer_AppendString(buf, name);
  GWEN_Buffer_AppendString(buf, DIRSEP "data");
  return 0;
}



GWEN_STRINGLIST *AB_Banking_GetGlobalDataDirs(void)
{
  GWEN_STRINGLIST *sl;

  sl=GWEN_PathManager_GetPaths(AB_PM_LIBNAME, AB_PM_DATADIR);
  return sl;
}



GWEN_STRINGLIST *AB_Banking_GetGlobalSysconfDirs(void)
{
  GWEN_STRINGLIST *sl;

  sl=GWEN_PathManager_GetPaths(AB_PM_LIBNAME, AB_PM_SYSCONFDIR);
  return sl;
}



int AB_Banking_ReadNamedConfigGroup(const AB_BANKING *ab,
                                    const char *groupName,
                                    const char *subGroupName,
                                    int doLock,
                                    int doUnlock,
                                    GWEN_DB_NODE **pDb)
{
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
                                     GWEN_DB_NODE *db)
{
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



int AB_Banking_ReadConfigGroup(const AB_BANKING *ab,
                               const char *groupName,
                               uint32_t uniqueId,
                               int doLock,
                               int doUnlock,
                               GWEN_DB_NODE **pDb)
{
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



int AB_Banking_HasConfigGroup(const AB_BANKING *ab,
                              const char *groupName,
                              uint32_t uniqueId)
{
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

  rv=GWEN_ConfigMgr_HasGroup(ab->configMgr, groupName, idBuf);
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
                                GWEN_DB_NODE *db)
{
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



int AB_Banking_DeleteConfigGroup(AB_BANKING *ab, const char *groupName, uint32_t uniqueId)
{
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



int AB_Banking_UnlockConfigGroup(AB_BANKING *ab, const char *groupName, uint32_t uniqueId)
{
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



int AB_Banking_ReadConfigGroups(const AB_BANKING *ab,
                                const char *groupName,
                                const char *uidField,
                                const char *matchVar,
                                const char *matchVal,
                                GWEN_DB_NODE **pDb)
{
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
    while (se) {
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
      if (ignoredGroups) {
        GWEN_StringList_free(sl);
        return GWEN_ERROR_PARTIAL;
      }
      GWEN_StringList_free(sl);
      return 0;
    }
    else {
      DBG_WARN(AQBANKING_LOGDOMAIN, "No matching config groups found");
      GWEN_DB_Group_free(dbAll);
      GWEN_StringList_free(sl);
      return GWEN_ERROR_NOT_FOUND;
    }
  }
  GWEN_StringList_free(sl);

  DBG_INFO(AQBANKING_LOGDOMAIN, "No account specs found");
  return GWEN_ERROR_NOT_FOUND;
}





