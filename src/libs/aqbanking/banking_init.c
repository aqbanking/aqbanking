/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/* This file is included by banking.c */


#ifdef OS_WIN32
# include <windows.h>
#endif

static uint32_t ab_plugin_init_count=0;

static /*@null@*/GWEN_PLUGIN_MANAGER *ab_pluginManagerBankInfo=NULL;
static /*@null@*/GWEN_PLUGIN_MANAGER *ab_pluginManagerProvider=NULL;
static /*@null@*/GWEN_PLUGIN_MANAGER *ab_pluginManagerImExporter=NULL;
static /*@null@*/AB_BANKINFO_PLUGIN_LIST *ab_bankInfoPlugins=NULL;

static /*@null@*/AB_IMEXPORTER_LIST *ab_imexporters=NULL;
static /*@null@*/AB_PROVIDER_LIST *ab_providers=NULL;


#define AB_DBIO_FOLDER "dbio"



int AB_Banking_PluginSystemInit(void)
{
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
      GWEN_Logger_SetLevel(AQBANKING_LOGDOMAIN, GWEN_LoggerLevel_Notice);
    }

    s=getenv("AQBANKING_LOGLEVEL");
    if (s && *s) {
      GWEN_LOGGER_LEVEL ll;

      ll=GWEN_Logger_Name2Level(s);
      GWEN_Logger_SetLevel(AQBANKING_LOGDOMAIN, ll);
    }

    DBG_INFO(AQBANKING_LOGDOMAIN,
             "AqBanking v"
             AQBANKING_VERSION_FULL_STRING
             ": initialising");

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
    ab_providers=AB_Provider_List_new();
    ab_bankInfoPlugins=AB_BankInfoPlugin_List_new();
  }
  ab_plugin_init_count++;
  return 0;
}



int AB_Banking_PluginSystemFini(void)
{
  if (ab_plugin_init_count) {
    if (--ab_plugin_init_count==0) {
      AB_BankInfoPlugin_List_free(ab_bankInfoPlugins);
      ab_bankInfoPlugins=NULL;
      AB_Provider_List_free(ab_providers);
      ab_providers=NULL;
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



int AB_Banking_Init(AB_BANKING *ab)
{
  int rv;

  assert(ab);
  /* do basic initialisation for all AB_BANKING objects */

  rv=AB_Banking_PluginSystemInit();
  if (rv) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    AB_Banking_PluginSystemFini();
    return rv;
  }

  if (ab->initCount==0) {
    uint32_t currentVersion;
    GWEN_DB_NODE *db=NULL;

    currentVersion=
      (AQBANKING_VERSION_MAJOR<<24) |
      (AQBANKING_VERSION_MINOR<<16) |
      (AQBANKING_VERSION_PATCHLEVEL<<8) |
      AQBANKING_VERSION_BUILD;

    /* check for config manager (created by AB_Banking_Init) */
    if (ab->configMgr==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "No config manager. Maybe the gwenhywfar plugins are not installed correctly?");
      AB_Banking_PluginSystemFini();
      return GWEN_ERROR_GENERIC;
    }

    /* load main group, check version */
    rv=GWEN_ConfigMgr_GetGroup(ab->configMgr, AB_CFG_GROUP_MAIN, "config", &db);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not load main config group (%d)", rv);
      AB_Banking_PluginSystemFini();
      return rv;
    }
    ab->lastVersion=(uint32_t) GWEN_DB_GetIntValue(db, "lastVersion", 0, 0);
    GWEN_DB_Group_free(db);

    /* check whether we need to update */
    if ((ab->lastVersion>0) && (ab->lastVersion<currentVersion)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Updating AqBanking");
      rv=AB_Banking_Update(ab, ab->lastVersion, currentVersion);
      if (rv<0) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
        AB_Banking_PluginSystemFini();
        return rv;
      }
    }
  }
  ab->initCount++;

  return 0;
}



int AB_Banking_Fini(AB_BANKING *ab)
{
  int rv;

  /* deinit local stuff */
  if (ab->initCount<1) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "AqBanking object not initialized!");
    return GWEN_ERROR_INVALID;
  }

  if (--(ab->initCount)==0) {
    GWEN_DB_NODE *db=NULL;

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
      GWEN_ConfigMgr_UnlockGroup(ab->configMgr, AB_CFG_GROUP_MAIN, "config");
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

    /* clear all active crypt token */
    AB_Banking_ClearCryptTokenList(ab);
  } /* if (--(ab->initCount)==0) */

  /* deinit global stuff (keeps its own counter) */
  rv=AB_Banking_PluginSystemFini();
  if (rv) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
  }

  return 0;
}



#ifdef OS_WIN32
BOOL APIENTRY DllMain(HINSTANCE hInst,
                      DWORD reason,
                      LPVOID reserved)
{
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






