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
#include "provider_l.h"
#include "account_l.h"
#include "imexporter_l.h"
#include "bankinfoplugin_l.h"
#include "user_l.h"

#include <gwenhywfar/plugin.h>


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
};


int AB_Banking_PluginSystemInit(void);
int AB_Banking_PluginSystemFini(void);

GWEN_CONFIGMGR *AB_Banking_GetConfigMgr(AB_BANKING *ab);


static int AB_Banking__GetConfigManager(AB_BANKING *ab, const char *dname);


static AB_IMEXPORTER *AB_Banking_FindImExporter(AB_BANKING *ab, const char *name);


static AB_BANKINFO_PLUGIN *AB_Banking__LoadBankInfoPlugin(AB_BANKING *ab, const char *modname);
static AB_BANKINFO_PLUGIN *AB_Banking__GetBankInfoPlugin(AB_BANKING *ab, const char *country);


static int AB_Banking__ReadImExporterProfiles(AB_BANKING *ab,
                                              const char *path,
					      GWEN_DB_NODE *db,
					      int isGlobal);

static AB_IMEXPORTER *AB_Banking__LoadImExporterPlugin(AB_BANKING *ab, const char *modname);


static int AB_Banking__TransformIban(const char *iban, int len, char *newIban, int maxLen);


static void AB_Banking__fillTransactionRemoteInfo(AB_TRANSACTION *t);
/* static void AB_Banking__fillTransactionRemoteSepaInfo(AB_BANKING *ab, AB_TRANSACTION *t); */


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


int AB_Banking_Update(AB_BANKING *ab, uint32_t lastVersion, uint32_t currentVersion);
static int AB_Banking_Update_5_99_2_0(AB_BANKING *ab, uint32_t lastVersion, uint32_t currentVersion);


static int AB_Banking__SendCommands(AB_BANKING *ab, AB_TRANSACTION_LIST2* commandList, AB_IMEXPORTER_CONTEXT *ctx, uint32_t pid);



#endif /* AQBANKING_BANKING_P_H */
