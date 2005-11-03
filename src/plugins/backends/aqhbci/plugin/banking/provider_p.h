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

#ifndef AH_PROVIDER_P_H
#define AH_PROVIDER_P_H

#define AH_PROVIDER_NAME "AQHBCI"
#define AH_PROVIDER_DATADIR ".libaqhbci"

#define AH_HBCI_WCB_GENERIC "AH_HBCI_WCB_GENERIC"

#include <aqhbci/hbci.h>
#include <aqhbci/provider.h>
#include "job_l.h"
#include <aqhbci/job.h>
#include <aqhbci/outbox.h>
#include <aqhbci/jobplugin.h>



struct AH_PROVIDER {
  AH_HBCI *hbci;
  AB_JOB_LIST2 *bankingJobs;
  AH_OUTBOX *outbox;
  GWEN_DB_NODE *dbConfig;
  AH_JOBPLUGIN_LIST *jobPlugins;
  GWEN_DB_NODE *dbTempConfig;
};
void AH_Provider_FreeData(void *bp, void *p);


AH_JOB *AH_Provider__FindMyJob(AH_JOB_LIST *mjl, GWEN_TYPE_UINT32 jid);
AH_ACCOUNT *AH_Provider__FindMyAccount(AB_PROVIDER *pro, AB_ACCOUNT *a);


/**
 * factory function, this is used by AqBanking when loading a provider
 * plugin.
 */
AB_PROVIDER *aqhbci_factory(AB_BANKING *ab);


/** @name Overwritten Virtual Functions
 *
 */
/*@{*/
int AH_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
int AH_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
int AH_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j);
int AH_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j);
int AH_Provider_Execute(AB_PROVIDER *pro);
int AH_Provider_ResetQueue(AB_PROVIDER *pro);
AB_ACCOUNT_LIST2 *AH_Provider_GetAccountList(AB_PROVIDER *pro);
int AH_Provider_UpdateAccount(AB_PROVIDER *pro, AB_ACCOUNT *a);
int AH_Provider_AddAccount(AB_PROVIDER *pro, AB_ACCOUNT *a);
/*@}*/


int AH_Provider__FillAccount(AB_PROVIDER *pro,
                             AB_ACCOUNT *a,
                             AH_ACCOUNT *ma);

AH_JOB *AH_Provider__GetPluginJob(AH_PROVIDER *hp,
                                  AB_JOB_TYPE jt,
                                  AH_CUSTOMER *mcu,
                                  AH_ACCOUNT *ma);

AH_JOBPLUGIN *AH_Provider_FindJobPlugin(AH_PROVIDER *hp, const char *name);


AH_JOBPLUGIN *AH_Provider_LoadJobPlugin(AH_PROVIDER *hp,
                                        const char *path,
                                        const char *modname);

int AH_Provider_LoadJobPlugins(AH_PROVIDER *pro, const char *path);


int AH_Provider_LoadAllJobPlugins(AB_PROVIDER *pro);



#endif /* AH_PROVIDER_P_H */




