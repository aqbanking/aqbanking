/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_BANKING_P_H
#define AQBANKING_BANKING_P_H

#define AB_BANKING_MAX_PIN_TRY 10

#define AB_BANKING_REGKEY_PATHS       "Software\\AqBanking\\Paths"
#define AB_BANKING_REGKEY_DATADIR     "pkgdatadir"
#define AB_BANKING_REGKEY_BANKINFODIR "bankinfodir"
#define AB_BANKING_REGKEY_PROVIDERDIR "providerdir"
#define AB_BANKING_REGKEY_IMPORTERDIR "importerdir"
#define AB_BANKING_REGKEY_SYSCONFDIR  "sysconfdir"
#define AB_BANKING_REGKEY_WIZARDDIR   "wizarddir"
#define AB_BANKING_REGKEY_LOCALEDIR   "localedir"

#define AB_WIZARD_FOLDER "wizards"

#ifdef OS_WIN32
# define AB_BANKING_USERDATADIR "aqbanking"
#else
# define AB_BANKING_USERDATADIR ".aqbanking"
#endif

/**
 * Name of the default configuration file within the users home folder.
 */
#define AB_BANKING_CONFIGFILE     "settings.conf"
#define AB_BANKING_OLD_CONFIGFILE ".aqbanking.conf"

#define AB_BANKING_SETTINGS_DIR   "settings6" /* temporarily changed to settings6 for testing purposes */

#define AB_CFG_GROUP_MAIN         "aqbanking"
#define AB_CFG_GROUP_APPS         "apps"
#define AB_CFG_GROUP_SHARED       "shared"
#define AB_CFG_GROUP_ACCOUNTSPECS "accountspecs"
#define AB_CFG_GROUP_USERSPECS    "userspecs"



#include "banking_l.h"
#include "backendsupport/provider_l.h"
#include "backendsupport/imexporter_l.h"
#include "backendsupport/bankinfoplugin_l.h"

#include <gwenhywfar/plugin.h>
#include <gwenhywfar/syncio_memory.h>



struct AB_BANKING {
  GWEN_INHERIT_ELEMENT(AB_BANKING)
  int initCount;
  char *appName;
  char *appEscName;
  int appExtensions;
  uint32_t lastVersion;

  char *dataDir;

  char *startFolder;

  GWEN_DB_NODE *dbProfiles;

  GWEN_CRYPT_TOKEN_LIST2 *cryptTokenList;

  GWEN_CONFIGMGR *configMgr;

  GWEN_DB_NODE *dbRuntimeConfig;
};


/* static GWEN_CONFIGMGR *AB_Banking_GetConfigMgr(AB_BANKING *ab); */


static int AB_Banking__GetConfigManager(AB_BANKING *ab, const char *dname);


static AB_IMEXPORTER *AB_Banking_FindImExporter(AB_BANKING *ab, const char *name);


static AB_BANKINFO_PLUGIN *AB_Banking_CreateImBankInfoPlugin(AB_BANKING *ab, const char *modname);
static AB_BANKINFO_PLUGIN *AB_Banking_FindBankInfoPlugin(AB_BANKING *ab, const char *country);
static AB_BANKINFO_PLUGIN *AB_Banking_GetBankInfoPlugin(AB_BANKING *ab, const char *country);


static int AB_Banking__TransformIban(const char *iban, int len, char *newIban, int maxLen);




/* ========================================================================================================================
 *                                                banking_update.c
 * ========================================================================================================================
 */

/**
 * This functions changes the GWEN_ConfigMgr id of configuration groups to match the AqBanking-ID.
 *
 * GWEN_ConfigMgr has its own scheme to assign unique ids to configuration groups (see @ref GWEN_ConfigMgr_GetUniqueId).
 * However, since version 6 AqBanking needs to directly access configuration groups (e.g. when a backend wants to
 * use an AB_USER object it just loads it in time). Fo this to work there needs to be a way to derive the config manager
 * id to the id assigned by AqBanking.
 * For this we use @ref GWEN_ConfigMgr_MkUniqueIdFromId() to make the config manager derive a static unique id from the
 * given AqBanking-assigned id.
 * This function checks every config group and checks whether its id has been created by GWEN_ConfigMgr_MkUniqueIdFromId.
 * If it is not a new config group will be created with the new id and the old group is deleted.
 */
static int AB_Banking_UpdateConfList(AB_BANKING *ab, const char *groupName);
static int AB_Banking_UpdateUserList(AB_BANKING *ab);
static int AB_Banking_UpdateAccountList(AB_BANKING *ab);
static int AB_Banking_Update_Account_SetUserId(AB_BANKING *ab);
static int AB_Banking_Update_Account_SetBackendName(AB_BANKING *ab);
static int AB_Banking_Update_Backend_InitDeinit(AB_BANKING *ab);



int AB_Banking_Update(AB_BANKING *ab, uint32_t lastVersion, uint32_t currentVersion);


int AB_Banking_CopyOldSettingsFolderIfNeeded(AB_BANKING *ab);




/* ========================================================================================================================
 *                                                banking_cfg.c
 * ========================================================================================================================
 */

static int AB_Banking_ReadNamedConfigGroup(const AB_BANKING *ab,
                                           const char *groupName,
                                           const char *subGroupName,
                                           int doLock,
                                           int doUnlock,
                                           GWEN_DB_NODE **pDb);

static int AB_Banking_WriteNamedConfigGroup(AB_BANKING *ab,
                                            const char *groupName,
                                            const char *subGroupName,
                                            int doLock,
                                            int doUnlock,
                                            GWEN_DB_NODE *db);


static int AB_Banking_ReadConfigGroup(const AB_BANKING *ab,
                                      const char *groupName,
                                      uint32_t uniqueId,
                                      int doLock,
                                      int doUnlock,
                                      GWEN_DB_NODE **pDb);

static int AB_Banking_HasConfigGroup(const AB_BANKING *ab,
                                     const char *groupName,
                                     uint32_t uniqueId);


static int AB_Banking_WriteConfigGroup(AB_BANKING *ab,
                                       const char *groupName,
                                       uint32_t uniqueId,
                                       int doLock,
                                       int doUnlock,
                                       GWEN_DB_NODE *db);

static int AB_Banking_DeleteConfigGroup(AB_BANKING *ab, const char *groupName, uint32_t uniqueId);

static int AB_Banking_UnlockConfigGroup(AB_BANKING *ab, const char *groupName, uint32_t uniqueId);






static AB_IMEXPORTER *AB_Banking_GetImExporter(AB_BANKING *ab, const char *name);




#endif /* AQBANKING_BANKING_P_H */
