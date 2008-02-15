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

  char *configFile;

  char *startFolder;

  GWEN_DB_NODE *data;
  GWEN_DB_NODE *dbTempConfig;
  GWEN_DB_NODE *dbProfiles;

  AB_PROVIDER_LIST *providers;

  void *user_data;

  AB_JOB_LIST2 *currentJobs;

  GWEN_CRYPT_TOKEN_LIST2 *cryptTokenList;

};


static int AB_Banking__BaseInit(void);
static int AB_Banking__BaseFini(void);


static void AB_Banking__GetConfigFileNameAndDataDir(AB_BANKING *ab,
                                                    const char *dname);


static AB_PROVIDER *AB_Banking_FindProvider(AB_BANKING *ab, const char *name);

static AB_IMEXPORTER *AB_Banking_FindImExporter(AB_BANKING *ab,
                                                const char *name);

static int AB_Banking__OpenFile(const char *s, int wr);
static int AB_Banking__CloseFile(int fd);


static AB_PROVIDER *AB_Banking__LoadProviderPlugin(AB_BANKING *ab,
                                                   const char *modname);
static AB_IMEXPORTER *AB_Banking__LoadImExporterPlugin(AB_BANKING *ab,
                                                       const char *modname);
static AB_BANKINFO_PLUGIN*
  AB_Banking__LoadBankInfoPlugin(AB_BANKING *ab,
                                 const char *modname);
static AB_BANKINFO_PLUGIN *AB_Banking__GetBankInfoPlugin(AB_BANKING *ab,
                                                         const char *country);


static int AB_Banking___LoadData(AB_BANKING *ab,
                                 const char *prefix,
                                 const char *name);
static int AB_Banking__LoadData(AB_BANKING *ab,
                                const char *prefix,
                                const char *name);
static int AB_Banking___SaveData(AB_BANKING *ab,
                                 const char *prefix,
                                 const char *name);
static int AB_Banking__SaveData(AB_BANKING *ab,
                                const char *prefix,
                                const char *name);
static int AB_Banking__LoadAppData(AB_BANKING *ab);
static int AB_Banking__LoadSharedData(AB_BANKING *ab, const char *name);
static int AB_Banking__SaveExternalData(AB_BANKING *ab);



static int AB_Banking__ReadImExporterProfiles(AB_BANKING *ab,
                                              const char *path,
                                              GWEN_DB_NODE *db);


static int AB_Banking_InitProvider(AB_BANKING *ab, AB_PROVIDER *pro);
static int AB_Banking_FiniProvider(AB_BANKING *ab, AB_PROVIDER *pro);

static int AB_Banking__ExecuteQueue(AB_BANKING *ab, AB_JOB_LIST2 *jl2,
                                    AB_IMEXPORTER_CONTEXT *ctx,
                                    uint32_t pid);


static AB_ACCOUNT *AB_Banking__GetAccount(AB_BANKING *ab,
                                          const char *accountId);

static int AB_Banking__GetDebuggerPath(AB_BANKING *ab,
                                       const char *backend,
                                       GWEN_BUFFER *pbuf);

static int AB_Banking__TransformIban(const char *iban, int len,
                                     char *newIban, int maxLen);


static uint64_t AB_Banking__char2uint64(const char *accountId);

static int AB_Banking__LoadOldProviderData(AB_BANKING *ab, const char *name);

static int AB_Banking__LoadData(AB_BANKING *ab,
                                const char *prefix,
                                const char *name);

static void AB_Banking_ActivateAllProviders(AB_BANKING*ab);


#endif /* AQBANKING_BANKING_P_H */
