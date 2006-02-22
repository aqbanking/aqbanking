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

#include "provider_l.h"
#include "job_l.h"
#include "outbox_l.h"


typedef struct AH_PROVIDER AH_PROVIDER;
struct AH_PROVIDER {
  AH_HBCI *hbci;
  AB_JOB_LIST2 *bankingJobs;
  AH_OUTBOX *outbox;
  GWEN_DB_NODE *dbConfig;
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
static int AH_Provider_Execute(AB_PROVIDER *pro, AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Provider_ResetQueue(AB_PROVIDER *pro);
static int AH_Provider_ExtendUser(AB_PROVIDER *pro, AB_USER *u,
                                  AB_PROVIDER_EXTEND_MODE em);
static int AH_Provider_ExtendAccount(AB_PROVIDER *pro, AB_ACCOUNT *a,
                                     AB_PROVIDER_EXTEND_MODE em);
static int AH_Provider_Update(AB_PROVIDER *pro,
                              GWEN_TYPE_UINT32 lastVersion,
                              GWEN_TYPE_UINT32 currentVersion);

/*@}*/

#endif /* AH_PROVIDER_P_H */




