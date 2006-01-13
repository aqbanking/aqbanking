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

#define AH_PROVIDER_DATADIR ".libaqhbci"

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
static void AH_Provider_FreeData(void *bp, void *p);


static AH_JOB *AH_Provider__FindMyJob(AH_JOB_LIST *mjl, GWEN_TYPE_UINT32 jid);


/** @name Overwritten Virtual Functions
 *
 */
/*@{*/
static int AH_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int AH_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int AH_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j);
static int AH_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j);
static int AH_Provider_Execute(AB_PROVIDER *pro);
static int AH_Provider_ResetQueue(AB_PROVIDER *pro);
static int AH_Provider_ExtendUser(AB_PROVIDER *pro, AB_USER *u,
                                  AB_PROVIDER_EXTEND_MODE em);
static int AH_Provider_ExtendAccount(AB_PROVIDER *pro, AB_ACCOUNT *a,
                                     AB_PROVIDER_EXTEND_MODE em);
static int AH_Provider_Update(AB_PROVIDER *pro,
                              GWEN_TYPE_UINT32 lastVersion,
                              GWEN_TYPE_UINT32 currentVersion);

/*@}*/


static AH_JOB *AH_Provider__GetPluginJob(AH_PROVIDER *hp,
                                         AB_JOB_TYPE jt,
                                         AB_USER *mu,
                                         AB_ACCOUNT *ma);

static AH_JOBPLUGIN *AH_Provider_FindJobPlugin(AH_PROVIDER *hp,
                                               const char *name);


static AH_JOBPLUGIN *AH_Provider_LoadJobPlugin(AH_PROVIDER *hp,
                                               const char *path,
                                               const char *modname);

static int AH_Provider_LoadJobPlugins(AH_PROVIDER *pro, const char *path);


static int AH_Provider_LoadAllJobPlugins(AB_PROVIDER *pro);



#endif /* AH_PROVIDER_P_H */




