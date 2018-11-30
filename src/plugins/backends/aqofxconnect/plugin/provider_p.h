/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AO_PROVIDER_P_H
#define AO_PROVIDER_P_H


#define AO_PROVIDER_CONNECT_TIMEOUT 15
#define AO_PROVIDER_SEND_TIMEOUT    10
#define AO_PROVIDER_RECV_TIMEOUT    60

#include <aqofxconnect/provider.h>

#include <aqbanking/transaction.h>

#include <gwenhywfar/buffer.h>



struct AO_PROVIDER {
  GWEN_DB_NODE *dbConfig;

  int connectTimeout;
  int sendTimeout;
  int recvTimeout;
  uint32_t lastJobId;
};

static GWEN_DIALOG *AO_Provider_GetEditUserDialog(AB_PROVIDER *pro, AB_USER *u);
static GWEN_DIALOG *AO_Provider_GetNewUserDialog(AB_PROVIDER *pro, int i);



/* ***************************************************************************************************************
 *
 *                                                provider.c
 *
 * ***************************************************************************************************************/


static void GWENHYWFAR_CB AO_Provider_FreeData(void *bp, void *p);
static int AO_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int AO_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static AB_ACCOUNT *AO_Provider_CreateAccountObject(AB_PROVIDER *pro);
static AB_USER *AO_Provider_CreateUserObject(AB_PROVIDER *pro);



/* ***************************************************************************************************************
 *
 *                                                provider_accspec.c
 *
 * ***************************************************************************************************************/


static int AO_Provider__CreateTransactionLimitsForAccount(AB_PROVIDER *pro, const AB_ACCOUNT *acc, AB_TRANSACTION_LIMITS_LIST *tll);
static int AO_Provider_AccountToAccountSpec(AB_PROVIDER *pro, const AB_ACCOUNT *acc, AB_ACCOUNT_SPEC *as);
static int AO_Provider_WriteAccountSpecForAccount(AB_PROVIDER *pro, const AB_ACCOUNT *acc);
static int AO_Provider_CreateInitialAccountSpecs(AB_PROVIDER *pro);



/* ***************************************************************************************************************
 *
 *                                                provider_cmd_accinfo.c
 *
 * ***************************************************************************************************************/


static int AO_Provider__AddAccountInfoReq(AB_PROVIDER *pro, AB_USER *u, GWEN_BUFFER *buf);



/* ***************************************************************************************************************
 *
 *                                                provider_cmd_stm.c
 *
 * ***************************************************************************************************************/


static int AO_Provider__AddBankStatementReq(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j, GWEN_BUFFER *buf);
static int AO_Provider__AddCreditCardStatementReq(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j, GWEN_BUFFER *buf);
static int AO_Provider__AddInvStatementReq(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j, GWEN_BUFFER *buf);
static int AO_Provider__AddStatementRequest(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j, GWEN_BUFFER *buf);
static int AO_Provider_RequestStatements(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j, AB_IMEXPORTER_CONTEXT *ictx);



/* ***************************************************************************************************************
 *
 *                                                provider_network.c
 *
 * ***************************************************************************************************************/


static int AO_Provider_CreateConnection(AB_PROVIDER *pro, AB_USER *u, GWEN_HTTP_SESSION **pSess);
static int AO_Provider_SendAndReceive(AB_PROVIDER *pro, AB_USER *u, const uint8_t *p, unsigned int plen, GWEN_BUFFER **pRbuf);



/* ***************************************************************************************************************
 *
 *                                                provider_request.c
 *
 * ***************************************************************************************************************/


static int AO_Provider__AddHeaders(AB_PROVIDER *pro, AB_USER *u, GWEN_BUFFER *buf);
static int AO_Provider__AddSignOn(AB_PROVIDER *pro, AB_USER *u, GWEN_BUFFER *buf);
static int AO_Provider__WrapRequest(AB_PROVIDER *pro, AB_USER *u, const char *mTypeName, const char *tTypeName, GWEN_BUFFER *buf);
static int AO_Provider__WrapMessage(AB_PROVIDER *pro, AB_USER *u, GWEN_BUFFER *buf);



/* ***************************************************************************************************************
 *
 *                                                provider_sendcmd.c
 *
 * ***************************************************************************************************************/


static AB_TRANSACTION *AO_Provider_FindJobById(AB_TRANSACTION_LIST2 *jl, uint32_t jid);
static int AO_Provider__AddJobToList2(AB_PROVIDER *pro, AB_TRANSACTION *j, AB_TRANSACTION_LIST2 *jobList);
static int AO_Provider__SendJobList(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION_LIST2 *jl, AB_IMEXPORTER_CONTEXT *ctx);
static void AO_Provider__FinishJobs(AB_PROVIDER *pro, AB_TRANSACTION_LIST2 *jobList, AB_IMEXPORTER_CONTEXT *ctx);
static int AO_Provider__SendAccountQueue(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNTQUEUE *aq, AB_IMEXPORTER_CONTEXT *ctx);
static int AO_Provider__SendUserQueue(AB_PROVIDER *pro, AB_USERQUEUE *uq, AB_IMEXPORTER_CONTEXT *ctx);
static int AO_Provider_SendCommands(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_IMEXPORTER_CONTEXT *ctx);
static void AO_Provider__AddOrModifyAccount(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *acc);
static int AO_Provider__ProcessImporterContext(AB_PROVIDER *pro, AB_USER *u, AB_IMEXPORTER_CONTEXT *ictx);



/* ***************************************************************************************************************
 *
 *                                                provider_update.c
 *
 * ***************************************************************************************************************/


static int AO_Provider_UpdatePreInit(AB_PROVIDER *pro, uint32_t lastVersion, uint32_t currentVersion);
static int AO_Provider_UpdatePostInit(AB_PROVIDER *pro, uint32_t lastVersion, uint32_t currentVersion);




#endif

