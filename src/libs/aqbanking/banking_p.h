/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQBANKING_BANKING_P_H
#define AQBANKING_BANKING_P_H

#define AB_BANKING_MAX_PIN_TRY 10

#define AB_BANKING_WCB_GENERIC "AB_BANKING_WCB_GENERIC"

#define AB_BANKING_REGKEY_PATHS       "Software\\AqBanking\\Paths"
#define AB_BANKING_REGKEY_DATADIR     "pkgdatadir"
#define AB_BANKING_REGKEY_BANKINFODIR "bankinfodir"
#define AB_BANKING_REGKEY_PROVIDERDIR "providerdir"
#define AB_BANKING_REGKEY_IMPORTERDIR "importerdir"
#define AB_BANKING_REGKEY_SYSCONFDIR  "sysconfdir"
#define AB_BANKING_REGKEY_WIZARDDIR   "wizarddir"

#define AB_BANKING_USERDATADIR ".banking"
#define AB_WIZARD_FOLDER "wizards"

/**
 * Name of the default configuration file within the users home folder.
 */
#define AB_BANKING_CONFIGFILE "settings.conf"
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
#include <gwenhywfar/waitcallback.h>


struct AB_BANKING {
  GWEN_INHERIT_ELEMENT(AB_BANKING)
  int isOpen;
  int isBasicsOpen;
  char *appName;
  char *appEscName;
  int appExtensions;
  GWEN_TYPE_UINT32 lastVersion;

  char *dataDir;

  AB_JOB_LIST *enqueuedJobs;
  AB_USER_LIST *users;
  AB_ACCOUNT_LIST *accounts;

  GWEN_STRINGLIST *activeProviders;

  char *configFile;

  char *startFolder;

  GWEN_DB_NODE *data;
  GWEN_DB_NODE *dbTempConfig;
  GWEN_DB_NODE *dbProfiles;

  AB_PROVIDER_LIST *providers;
  AB_IMEXPORTER_LIST *imexporters;
  AB_BANKINFO_PLUGIN_LIST *bankInfoPlugins;

  AB_BANKING_MESSAGEBOX_FN messageBoxFn;
  AB_BANKING_INPUTBOX_FN inputBoxFn;
  AB_BANKING_SHOWBOX_FN showBoxFn;
  AB_BANKING_HIDEBOX_FN hideBoxFn;
  AB_BANKING_PROGRESS_START_FN progressStartFn;
  AB_BANKING_PROGRESS_ADVANCE_FN progressAdvanceFn;
  AB_BANKING_PROGRESS_LOG_FN progressLogFn;
  AB_BANKING_PROGRESS_END_FN progressEndFn;
  AB_BANKING_PRINT_FN printFn;

  AB_BANKING_GETPIN_FN getPinFn;
  AB_BANKING_SETPINSTATUS_FN setPinStatusFn;
  AB_BANKING_GETTAN_FN getTanFn;
  AB_BANKING_SETTANSTATUS_FN setTanStatusFn;
  int pinCacheEnabled;
  int alwaysAskForCert;

  AB_PIN_LIST *pinList;

  GWEN_PLUGIN_MANAGER *pluginManagerBankInfo;
  GWEN_PLUGIN_MANAGER *pluginManagerProvider;
  GWEN_PLUGIN_MANAGER *pluginManagerImExporter;
  GWEN_PLUGIN_MANAGER *pluginManagerPkgdatadir;
  GWEN_PLUGIN_MANAGER *pluginManagerCryptToken;
  void *user_data;

  GWEN_WAITCALLBACK *waitCallback;

  int progressNestingLevel;
  GWEN_TYPE_UINT32 lastProgressId;

  AB_JOB_LIST2 *currentJobs;

};


static void AB_Banking__GetConfigFileNameAndDataDir(AB_BANKING *ab,
                                                    const char *dname);


static AB_PROVIDER *AB_Banking_FindProvider(AB_BANKING *ab, const char *name);

static AB_IMEXPORTER *AB_Banking_FindImExporter(AB_BANKING *ab,
                                                const char *name);

static void AB_Banking__AddJobDir(const AB_BANKING *ab,
                                  const char *as,
                                  GWEN_BUFFER *buf);
static void AB_Banking__AddJobPath(const AB_BANKING *ab,
                                   const char *as,
                                   GWEN_TYPE_UINT32 jid,
                                   GWEN_BUFFER *buf);

static int AB_Banking__OpenFile(const char *s, int wr);
static int AB_Banking__CloseFile(int fd);


static int AB_Banking__OpenJobAs(AB_BANKING *ab,
                                 GWEN_TYPE_UINT32 jid,
                                 const char *as,
                                 int wr);
static int AB_Banking__CloseJob(const AB_BANKING *ab, int fd);

static AB_JOB *AB_Banking__LoadJobFile(AB_BANKING *ab, const char *s);


#if 0 /* FIXME: This function is not used */
static AB_JOB *AB_Banking__LoadJobAs(AB_BANKING *ab,
                                     GWEN_TYPE_UINT32 jid,
                                     const char *as);
#endif
static int AB_Banking__SaveJobAs(AB_BANKING *ab,
                                 AB_JOB *j,
                                 const char *as);

static AB_JOB_LIST2 *AB_Banking__LoadJobsAs(AB_BANKING *ab, const char *as);

static int AB_Banking__UnlinkJobAs(AB_BANKING *ab,
                                   AB_JOB *j,
                                   const char *as);

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
                                    GWEN_TYPE_UINT32 pid);


static AB_ACCOUNT *AB_Banking__GetAccount(AB_BANKING *ab,
                                          const char *accountId);

static int AB_Banking__HashPin(AB_PIN *p);
static int AB_Banking__SaveBadPins(AB_BANKING *ab);
static int AB_Banking__CheckBadPin(AB_BANKING *ab, AB_PIN *p);

static int AB_Banking__GetPin(AB_BANKING *ab,
                              GWEN_TYPE_UINT32 flags,
                              const char *token,
                              const char *title,
                              const char *text,
                              char *buffer,
                              int minLen,
                              int maxLen);

static int AB_Banking__GetDebuggerPath(AB_BANKING *ab,
                                       const char *backend,
                                       GWEN_BUFFER *pbuf);

static int AB_Banking__TransformIban(const char *iban, int len,
                                     char *newIban, int maxLen);


static GWEN_TYPE_UINT64 AB_Banking__char2uint64(const char *accountId);

static int AB_Banking__LoadOldProviderData(AB_BANKING *ab, const char *name);

static int AB_Banking__LoadData(AB_BANKING *ab,
                                const char *prefix,
                                const char *name);

/** this function is only used as long as most major applications still
 * use the old queue execution functions */
static void AB_Banking__DistribContextAmongJobs(AB_IMEXPORTER_CONTEXT *ctx,
                                                AB_JOB_LIST2 *jl2);


static AB_IMEXPORTER_ACCOUNTINFO*
  AB_Banking__FindAccountInfo(AB_IMEXPORTER_CONTEXT *ctx,
                              const AB_ACCOUNT *a);


#endif /* AQBANKING_BANKING_P_H */
