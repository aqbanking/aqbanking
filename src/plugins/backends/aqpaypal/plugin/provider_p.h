/***************************************************************************
    begin       : Sat May 08 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQPAYPAL_PROVIDER_P_H
#define AQPAYPAL_PROVIDER_P_H

#include "provider_l.h"

#include <aqbanking/ab_queue.h>



typedef struct APY_PROVIDER APY_PROVIDER;
struct APY_PROVIDER {
  AB_QUEUE *queue;
};

static void GWENHYWFAR_CB APY_Provider_FreeData(void *bp, void *p);


static int APY_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int APY_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int APY_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j);
static int APY_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j);
static int APY_Provider_ResetQueue(AB_PROVIDER *pro);
static int APY_Provider_Execute(AB_PROVIDER *pro, AB_IMEXPORTER_CONTEXT *ctx);
static int APY_Provider_ExtendUser(AB_PROVIDER *pro, AB_USER *u,
				   AB_PROVIDER_EXTEND_MODE em,
				   GWEN_DB_NODE *dbBackend);
static int APY_Provider_ExtendAccount(AB_PROVIDER *pro, AB_ACCOUNT *a,
				      AB_PROVIDER_EXTEND_MODE em,
				      GWEN_DB_NODE *dbBackend);


static int APY_Provider_ParseResponse(AB_PROVIDER *pro, const char *s, GWEN_DB_NODE *db);

static int APY_Provider_ExecGetTrans(AB_PROVIDER *pro,
				     AB_IMEXPORTER_ACCOUNTINFO *ai,
				     AB_USER *u,
				     AB_JOB *j);

static int APY_Provider_ExecJobQueue(AB_PROVIDER *pro,
				     AB_IMEXPORTER_ACCOUNTINFO *ai,
				     AB_USER *u,
				     AB_ACCOUNT *a,
				     AB_JOBQUEUE *jq);

static int APY_Provider_ExecAccountQueue(AB_PROVIDER *pro,
					 AB_IMEXPORTER_CONTEXT *ctx,
					 AB_USER *u,
					 AB_ACCOUNTQUEUE *aq);

static int APY_Provider_ExecUserQueue(AB_PROVIDER *pro,
				      AB_IMEXPORTER_CONTEXT *ctx,
				      AB_USERQUEUE *uq);

static int APY_Provider_UpdateTrans(AB_PROVIDER *pro,
				    AB_USER *u,
				    AB_TRANSACTION *t);


#endif


