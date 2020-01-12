/***************************************************************************
 begin       : Sat Sep 29 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/* This file is included by banking.c */


#include <gwenhywfar/directory.h>


#define AQBANKING_COPYFOLDER_MAX_DEPTH 10


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static int _copyFile(const char *sourceFile, const char *destFile);
static int _copyFolder(const char *sourceFolder, const char *destFolder, int depth);
static void _getOldStandardSourceFolder(GWEN_BUFFER *dbuf);
static void _getNewStandardSourceFolder(GWEN_BUFFER *dbuf);
static int _haveConfigAtFolder(const char *cfgFolder);


/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */





int AB_Banking_UpdateConfList(AB_BANKING *ab, const char *groupName)
{
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
    while (db) {
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



int AB_Banking_UpdateUserList(AB_BANKING *ab)
{
  int rv;

  rv=AB_Banking_UpdateConfList(ab, AB_CFG_GROUP_USERS);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Banking_UpdateAccountList(AB_BANKING *ab)
{
  int rv;

  rv=AB_Banking_UpdateConfList(ab, AB_CFG_GROUP_ACCOUNTS);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Banking_Update(AB_BANKING *ab, uint32_t lastVersion, uint32_t currentVersion)
{
  DBG_INFO(AQBANKING_LOGDOMAIN, "Updating from version %d.%d.%d.%d",
           (lastVersion>>24) & 0xff, (lastVersion>>16)  & 0xff, (lastVersion>>8)  & 0xff, lastVersion & 0xff);

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




int AB_Banking_Update_Backend_InitDeinit(AB_BANKING *ab)
{
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
    while (pd) {
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



int AB_Banking_Update_Account_SetUserId(AB_BANKING *ab)
{
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
    while (db) {
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



int AB_Banking_Update_Account_SetBackendName(AB_BANKING *ab)
{
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
    while (db) {
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

        /* create new var (only if not already set */
        s=GWEN_DB_GetCharValue(db, "backendName", 0, NULL);
        if (s==NULL) {
          s=GWEN_DB_GetCharValue(db, "provider", 0, NULL);
          if (!(s && *s)) {
            DBG_WARN(AQBANKING_LOGDOMAIN, "%s: Neither provider nor backendName set (%s), not modifying group",
                     AB_CFG_GROUP_ACCOUNTS, subGroupName);
          }
          else {
            GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "backendName", s);
            /* delete old var */
            GWEN_DB_DeleteVar(db, "provider");
          }
        }

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



int AB_Banking_CopyOldSettingsFolderIfNeeded(AB_BANKING *ab)
{
  GWEN_BUFFER *bufDest;
  int rv;

  bufDest=GWEN_Buffer_new(0, 256, 0, 1);
  _getNewStandardSourceFolder(bufDest);

  if (!_haveConfigAtFolder(GWEN_Buffer_GetStart(bufDest))) {
    GWEN_BUFFER *bufSource;

    DBG_ERROR(AQBANKING_LOGDOMAIN, "No current settings folder, trying to copy old one");
    bufSource=GWEN_Buffer_new(0, 256, 0, 1);
    _getOldStandardSourceFolder(bufSource);

    if (_haveConfigAtFolder(GWEN_Buffer_GetStart(bufSource))) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "There is an old settings folder, copying that");
      rv=_copyFolder(GWEN_Buffer_GetStart(bufSource), GWEN_Buffer_GetStart(bufDest), 0);
      if (rv<0) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Error copying old settings, please copy the folder\n"
		  "  %s\n"
		  " manually to\n"
		  "  %s\n"
		  "(Error code was: %d)",
		  GWEN_Buffer_GetStart(bufSource),
		  GWEN_Buffer_GetStart(bufDest),
		  rv);
	GWEN_Gui_ShowError("Error Copying old Settings",
			   "Error copying old settings, please copy the folder\n"
			   "  %s\n"
			   " manually to\n"
			   "  %s\n"
			   "(Error code was: %d)",
			   GWEN_Buffer_GetStart(bufSource),
			   GWEN_Buffer_GetStart(bufDest),
			   rv);
	GWEN_Buffer_free(bufDest);
	GWEN_Buffer_free(bufSource);
	return rv;
      }
    }
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "There is no old settings folder, need initial setup");
    }
    GWEN_Buffer_free(bufSource);
  }
  GWEN_Buffer_free(bufDest);
  return 0;
}





int _copyFile(const char *sourceFile, const char *destFile)
{
  int rv;
  GWEN_BUFFER *bufSource;

  bufSource=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_SyncIo_Helper_ReadFile(sourceFile, bufSource);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(bufSource);
    return rv;
  }

  rv=GWEN_SyncIo_Helper_WriteFile(destFile, (const uint8_t*) GWEN_Buffer_GetStart(bufSource), GWEN_Buffer_GetUsedBytes(bufSource));
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(bufSource);
    return rv;
  }

  GWEN_Buffer_free(bufSource);
  return 0;
}



int _copyFolder(const char *sourceFolder, const char *destFolder, int depth)
{
  int rv;
  GWEN_STRINGLIST *slSourceEntries;
  GWEN_STRINGLISTENTRY *se;
  GWEN_BUFFER *bufDest;
  uint32_t bufferPosDest;
  GWEN_BUFFER *bufSource;
  uint32_t bufferPosSource;

  if (depth>=AQBANKING_COPYFOLDER_MAX_DEPTH) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Recursion too deep, maybe some circular links? Aborting.");
    return GWEN_ERROR_INTERNAL;
  }

  /* get source files and folders */
  slSourceEntries=GWEN_StringList_new();

  rv=GWEN_Directory_GetFileEntriesWithType(sourceFolder, slSourceEntries, NULL);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_StringList_free(slSourceEntries);
    return rv;
  }

  GWEN_StringList_Sort(slSourceEntries, 1, GWEN_StringList_SortModeNoCase);

  /* create destination folder, if it does not exist */
  rv=GWEN_Directory_GetPath(destFolder, GWEN_PATH_FLAGS_CHECKROOT);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_StringList_free(slSourceEntries);
    return rv;
  }

  /* prepare dest buffer */
  bufDest=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(bufDest, destFolder);
  GWEN_Buffer_AppendString(bufDest, GWEN_DIR_SEPARATOR_S);
  bufferPosDest=GWEN_Buffer_GetPos(bufDest);

  /* prepare source buffer */
  bufSource=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(bufSource, sourceFolder);
  GWEN_Buffer_AppendString(bufSource, GWEN_DIR_SEPARATOR_S);
  bufferPosSource=GWEN_Buffer_GetPos(bufSource);

  /* copy files */
  se=GWEN_StringList_FirstEntry(slSourceEntries);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    if (s && *s=='f') {

      GWEN_Buffer_AppendString(bufSource, s+1);
      GWEN_Buffer_AppendString(bufDest, s+1);

      rv=_copyFile(GWEN_Buffer_GetStart(bufSource), GWEN_Buffer_GetStart(bufDest));
      if (rv<0) {
	DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
	GWEN_Buffer_free(bufSource);
	GWEN_Buffer_free(bufDest);
	GWEN_StringList_free(slSourceEntries);
	return rv;
      }

      GWEN_Buffer_Crop(bufSource, 0, bufferPosSource);
      GWEN_Buffer_Crop(bufDest, 0, bufferPosDest);
    }

    se=GWEN_StringListEntry_Next(se);
  }

  /* create folders recursively */
  se=GWEN_StringList_FirstEntry(slSourceEntries);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    if (s && *s=='d') {
      if (strcmp(s+1, ".")!=0 && strcmp(s+1, "..")!=0) {
	GWEN_Buffer_AppendString(bufSource, s+1);
	GWEN_Buffer_AppendString(bufDest, s+1);

	rv=_copyFolder(GWEN_Buffer_GetStart(bufSource), GWEN_Buffer_GetStart(bufDest), depth+1);
	if (rv<0) {
	  DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
	  GWEN_Buffer_free(bufSource);
	  GWEN_Buffer_free(bufDest);
	  GWEN_StringList_free(slSourceEntries);
	  return rv;
	}

	GWEN_Buffer_Crop(bufSource, 0, bufferPosSource);
	GWEN_Buffer_Crop(bufDest, 0, bufferPosDest);
      }
    }

    se=GWEN_StringListEntry_Next(se);
  }

  GWEN_Buffer_free(bufSource);
  GWEN_Buffer_free(bufDest);
  GWEN_StringList_free(slSourceEntries);
  return 0;
}



void _getOldStandardSourceFolder(GWEN_BUFFER *dbuf)
{
  char home[256];

  if (GWEN_Directory_GetHomeDirectory(home, sizeof(home))) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not determine home directory, aborting.");
    abort();
  }

  GWEN_Buffer_AppendString(dbuf, home);
  GWEN_Buffer_AppendString(dbuf, GWEN_DIR_SEPARATOR_S);
  GWEN_Buffer_AppendString(dbuf, AB_BANKING_USERDATADIR);
  GWEN_Buffer_AppendString(dbuf, GWEN_DIR_SEPARATOR_S);
  GWEN_Buffer_AppendString(dbuf, "settings");
}



void _getNewStandardSourceFolder(GWEN_BUFFER *dbuf)
{
  char home[256];

  if (GWEN_Directory_GetHomeDirectory(home, sizeof(home))) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not determine home directory, aborting.");
    abort();
  }

  GWEN_Buffer_AppendString(dbuf, home);
  GWEN_Buffer_AppendString(dbuf, GWEN_DIR_SEPARATOR_S);
  GWEN_Buffer_AppendString(dbuf, AB_BANKING_USERDATADIR);
  GWEN_Buffer_AppendString(dbuf, GWEN_DIR_SEPARATOR_S);
  GWEN_Buffer_AppendString(dbuf, AB_BANKING_SETTINGS_DIR);
}



int _haveConfigAtFolder(const char *cfgFolder)
{
  GWEN_BUFFER *dbuf;
  int rv;

  dbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(dbuf, cfgFolder);
  GWEN_Buffer_AppendString(dbuf, GWEN_DIR_SEPARATOR_S);
  GWEN_Buffer_AppendString(dbuf, AB_CFG_GROUP_USERS);

  rv=GWEN_Directory_GetPath(GWEN_Buffer_GetStart(dbuf), GWEN_PATH_FLAGS_NAMEMUSTEXIST | GWEN_PATH_FLAGS_CHECKROOT);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No AqBanking config folder found at [%s] (%d)", GWEN_Buffer_GetStart(dbuf), rv);
    GWEN_Buffer_free(dbuf);
    return 0;
  }
  DBG_ERROR(AQBANKING_LOGDOMAIN, "AqBanking config folder found at [%s]", GWEN_Buffer_GetStart(dbuf));
  GWEN_Buffer_free(dbuf);

  return 1;
}






