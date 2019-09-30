/***************************************************************************
    begin       : Sat May 08 2010
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQPAYPAL_PROVIDER_P_H
#define AQPAYPAL_PROVIDER_P_H

#include "provider_l.h"

#include <aqbanking/backendsupport/queue.h>
#include <aqbanking/backendsupport/account_p.h>



typedef struct APY_PROVIDER APY_PROVIDER;
struct APY_PROVIDER {
  int dummy;
};

static void GWENHYWFAR_CB APY_Provider_FreeData(void *bp, void *p);


static int APY_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int APY_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);

static AB_ACCOUNT *APY_Provider_CreateAccountObject(AB_PROVIDER *pro);
static AB_USER *APY_Provider_CreateUserObject(AB_PROVIDER *pro);


static int APY_Provider_UpdatePreInit(AB_PROVIDER *pro, uint32_t lastVersion, uint32_t currentVersion);
static int APY_Provider_UpdatePostInit(AB_PROVIDER *pro, uint32_t lastVersion, uint32_t currentVersion);



static GWEN_DIALOG *APY_Provider_GetNewUserDialog(AB_PROVIDER *pro, int i);
static GWEN_DIALOG *APY_Provider_GetEditUserDialog(AB_PROVIDER *pro, AB_USER *u);


static int APY_Provider_ParseResponse(AB_PROVIDER *pro, const char *s, GWEN_DB_NODE *db);

static int APY_Provider_ExecGetBal(AB_PROVIDER *pro,
                                   AB_IMEXPORTER_ACCOUNTINFO *ai,
                                   AB_USER *u,
                                   AB_TRANSACTION *j);

static int APY_Provider_ExecGetTrans(AB_PROVIDER *pro,
                                     AB_IMEXPORTER_ACCOUNTINFO *ai,
                                     AB_USER *u,
                                     AB_TRANSACTION *j);

static int APY_Provider_UpdateTrans(AB_PROVIDER *pro,
                                    AB_USER *u,
                                    AB_TRANSACTION *t);

int APY_Provider_UpdateAccountSpec(AB_PROVIDER *pro, AB_ACCOUNT_SPEC *as, int doLock);

/* from provider_sendcmd.c */
static int APY_Provider__AddJobToList2(AB_PROVIDER *pro, AB_TRANSACTION *j, AB_TRANSACTION_LIST2 *jobList);
static int APY_Provider__SendJobList(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION_LIST2 *jl,
                                     AB_IMEXPORTER_CONTEXT *ctx, AB_IMEXPORTER_ACCOUNTINFO *ai);
static int APY_Provider__SendAccountQueue(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNTQUEUE *aq,
                                          AB_IMEXPORTER_CONTEXT *ctx);
static int APY_Provider__SendUserQueue(AB_PROVIDER *pro, AB_USERQUEUE *uq, AB_IMEXPORTER_CONTEXT *ctx);
static int APY_Provider_SendCommands(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_IMEXPORTER_CONTEXT *ctx);


#endif


