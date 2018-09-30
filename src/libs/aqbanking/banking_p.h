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
# define AB_BANKING_USERDATADIR "aqbanking6"  /* temporarily changed to aqbanking6 for testing purposes */
#else
# define AB_BANKING_USERDATADIR ".aqbanking6"
#endif

/**
 * Name of the default configuration file within the users home folder.
 */
#define AB_BANKING_CONFIGFILE     "settings.conf"
#define AB_BANKING_OLD_CONFIGFILE ".aqbanking.conf"


#define AB_CFG_GROUP_MAIN         "aqbanking"
#define AB_CFG_GROUP_APPS         "apps"
#define AB_CFG_GROUP_SHARED       "shared"
#define AB_CFG_GROUP_ACCOUNTSPECS "accountspecs"
#define AB_CFG_GROUP_USERSPECS    "userspecs"



#include "banking_l.h"
#include "provider_l.h"
#include "account_l.h"
#include "job_l.h"
#include "imexporter_l.h"
#include "bankinfoplugin_l.h"
#include "user_l.h"

#include <gwenhywfar/plugin.h>


struct AB_BANKING {
  GWEN_INHERIT_ELEMENT(AB_BANKING)
  int initCount;
  int onlineInitCount;
  char *appName;
  char *appEscName;
  int appExtensions;
  uint32_t lastVersion;

  char *dataDir;

  AB_USER_LIST *users;
  AB_ACCOUNT_LIST *accounts;

  char *startFolder;

  GWEN_DB_NODE *dbProfiles;

  void *user_data;

  AB_JOB_LIST2 *currentJobs;

  GWEN_CRYPT_TOKEN_LIST2 *cryptTokenList;

  GWEN_CONFIGMGR *configMgr;
};


static int AB_Banking__GetConfigManager(AB_BANKING *ab, const char *dname);


static AB_IMEXPORTER *AB_Banking_FindImExporter(AB_BANKING *ab,
                                                const char *name);


static AB_BANKINFO_PLUGIN*
  AB_Banking__LoadBankInfoPlugin(AB_BANKING *ab,
                                 const char *modname);
static AB_BANKINFO_PLUGIN *AB_Banking__GetBankInfoPlugin(AB_BANKING *ab,
                                                         const char *country);


static int AB_Banking__ReadImExporterProfiles(AB_BANKING *ab,
                                              const char *path,
					      GWEN_DB_NODE *db,
					      int isGlobal);

static AB_IMEXPORTER *AB_Banking__LoadImExporterPlugin(AB_BANKING *ab, const char *modname);


static int AB_Banking__TransformIban(const char *iban, int len,
                                     char *newIban, int maxLen);


static void AB_Banking__fillTransactionRemoteInfo(AB_TRANSACTION *t);
/* static void AB_Banking__fillTransactionRemoteSepaInfo(AB_BANKING *ab, AB_TRANSACTION *t); */


/* only for import of older configurations */
static int AB_Banking_SetUniqueId(AB_BANKING *ab, uint32_t uid);

static int AB_Banking__ImportConfDir(AB_BANKING *ab,
				     const char *path,
				     const char *groupName);

/**
 * Write the settings of the user while in exclusive use. This is also called
 * internally by @ref AB_Banking_EndExclUseUser.
 * Please note that the user must be locked with @ref AB_Banking_BeginExclUseUser.
 */
static int AB_Banking_SaveUser(AB_BANKING *ab, AB_USER *u);



int AB_Banking6_Update(AB_BANKING *ab, uint32_t lastVersion, uint32_t currentVersion);
static int AB_Banking6_Update_5_99_1_0(AB_BANKING *ab, uint32_t lastVersion, uint32_t currentVersion);



#endif /* AQBANKING_BANKING_P_H */
