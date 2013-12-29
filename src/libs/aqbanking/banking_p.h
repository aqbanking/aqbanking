/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
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


#define AB_CFG_GROUP_USERS    "users"
#define AB_CFG_GROUP_ACCOUNTS "accounts"
#define AB_CFG_GROUP_MAIN     "aqbanking"
#define AB_CFG_GROUP_APPS     "apps"
#define AB_CFG_GROUP_SHARED   "shared"


#include "banking_l.h"
#include "provider_l.h"
#include "account_l.h"
#include "job_l.h"
#include "imexporter_l.h"
#include "pin_l.h"
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

  GWEN_STRINGLIST *activeProviders;

  char *startFolder;

  GWEN_DB_NODE *dbProfiles;

  AB_PROVIDER_LIST *providers;

  void *user_data;

  AB_JOB_LIST2 *currentJobs;

  GWEN_CRYPT_TOKEN_LIST2 *cryptTokenList;

  GWEN_CONFIGMGR *configMgr;
};


static int AB_Banking__GetConfigManager(AB_BANKING *ab, const char *dname);


static AB_PROVIDER *AB_Banking_FindProvider(AB_BANKING *ab, const char *name);

static AB_IMEXPORTER *AB_Banking_FindImExporter(AB_BANKING *ab,
                                                const char *name);


static AB_PROVIDER *AB_Banking__LoadProviderPlugin(AB_BANKING *ab,
                                                   const char *modname);
static AB_IMEXPORTER *AB_Banking__LoadImExporterPlugin(AB_BANKING *ab,
                                                       const char *modname);
static AB_BANKINFO_PLUGIN*
  AB_Banking__LoadBankInfoPlugin(AB_BANKING *ab,
                                 const char *modname);
static AB_BANKINFO_PLUGIN *AB_Banking__GetBankInfoPlugin(AB_BANKING *ab,
                                                         const char *country);


static int AB_Banking__ReadImExporterProfiles(AB_BANKING *ab,
                                              const char *path,
					      GWEN_DB_NODE *db,
					      int isGlobal);


static int AB_Banking_InitProvider(AB_BANKING *ab, AB_PROVIDER *pro);
static int AB_Banking_FiniProvider(AB_BANKING *ab, AB_PROVIDER *pro);

static int AB_Banking__ExecuteQueue(AB_BANKING *ab, AB_JOB_LIST2 *jl2,
                                    AB_IMEXPORTER_CONTEXT *ctx);


static AB_ACCOUNT *AB_Banking__GetAccount(AB_BANKING *ab,
					  const char *accountId);

static int AB_Banking__GetDebuggerPath(AB_BANKING *ab,
                                       const char *backend,
                                       GWEN_BUFFER *pbuf);

static int AB_Banking__TransformIban(const char *iban, int len,
                                     char *newIban, int maxLen);


static uint64_t AB_Banking__char2uint64(const char *accountId);

static void AB_Banking_ActivateAllProviders(AB_BANKING *ab);

static void AB_Banking__fillTransactionRemoteInfo(AB_TRANSACTION *t);
static void AB_Banking__fillTransactionRemoteSepaInfo(AB_BANKING *ab, AB_TRANSACTION *t);


static int AB_Banking_LoadAllUsers(AB_BANKING *ab);

static int AB_Banking_LoadAllAccounts(AB_BANKING *ab);

static int AB_Banking_LoadConfig(AB_BANKING *ab);
static int AB_Banking_UnloadConfig(AB_BANKING *ab);
static int AB_Banking_SaveConfig(AB_BANKING *ab);

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


#endif /* AQBANKING_BANKING_P_H */
