/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQEBICS_CLIENT_PROVIDER_P_H
#define AQEBICS_CLIENT_PROVIDER_P_H

#include "provider_l.h"

#include <aqbanking/provider_be.h>



typedef struct EBC_PROVIDER EBC_PROVIDER;
struct EBC_PROVIDER {
  GWEN_DB_NODE *dbConfig;
  uint32_t lastJobId;
  uint32_t lastMediumId;

  int connectTimeout;
  int transferTimeout;
};

static void GWENHYWFAR_CB EBC_Provider_FreeData(void *bp, void *p);

static int EBC_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int EBC_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static GWEN_DIALOG *EBC_Provider_GetEditUserDialog(AB_PROVIDER *pro, AB_USER *u);
static GWEN_DIALOG *EBC_Provider_GetNewUserDialog(AB_PROVIDER *pro, int i);

static AB_ACCOUNT *EBC_Provider_CreateAccountObject(AB_PROVIDER *pro);
static AB_USER *EBC_Provider_CreateUserObject(AB_PROVIDER *pro);


int EBC_Provider__signMessage(AB_PROVIDER *pro, EB_MSG *msg, AB_USER *u);
int EBC_Provider__generateNonce(GWEN_BUFFER *buf);


/* p_tools.inc */
static int EBC_Provider__addKiTxt(AB_PROVIDER *pro,
				  const GWEN_CRYPT_TOKEN_KEYINFO *ki,
                                  GWEN_BUFFER *lbuf,
                                  int version);

static const char *EBC_Provider_TechnicalCodeToString(const char *s);
static const char *EBC_Provider_BankCodeToString(const char *s);

static int EBC_Provider_EuSign_A004(AB_PROVIDER *pro,
				    AB_USER *u,
				    const char *requestType,
				    const uint8_t *pMsg,
				    uint32_t lMsg,
				    GWEN_BUFFER *sbuf);
static int EBC_Provider_MkEuZipDoc_A004(AB_PROVIDER *pro,
					AB_USER *u,
					const char *requestType,
					const uint8_t *pMsg,
					uint32_t lMsg,
					GWEN_BUFFER *sbuf);

static int EBC_Provider_EuSign_A005(AB_PROVIDER *pro,
				    AB_USER *u,
				    const char *requestType,
				    const uint8_t *pMsg,
				    uint32_t lMsg,
				    GWEN_BUFFER *sbuf);
static int EBC_Provider_MkEuZipDoc_A005(AB_PROVIDER *pro,
					AB_USER *u,
					const char *requestType,
					const uint8_t *pMsg,
					uint32_t lMsg,
					GWEN_BUFFER *sbuf);

static int EBC_Provider_ExecContext_STA(AB_PROVIDER *pro,
					AB_IMEXPORTER_CONTEXT *ctx,
					AB_USER *u,
					AB_ACCOUNT *a,
					GWEN_HTTP_SESSION *sess,
					AB_JOBQUEUE *jq);

#if 0
static int EBC_Provider_ExecContext__IZV(AB_PROVIDER *pro,
					 AB_IMEXPORTER_CONTEXT *ctx,
					 AB_USER *u,
					 AB_ACCOUNT *a,
					 GWEN_HTTP_SESSION *sess,
					 EBC_CONTEXT *ectx);

static int EBC_Provider_ExecContext_IZV(AB_PROVIDER *pro,
					AB_IMEXPORTER_CONTEXT *ctx,
					AB_USER *u,
					AB_ACCOUNT *a,
					GWEN_HTTP_SESSION *sess,
					EBC_CONTEXT *ectx);


static int EBC_Provider_ExecContext(AB_PROVIDER *pro,
				    AB_IMEXPORTER_CONTEXT *ctx,
				    AB_USER *u,
				    AB_ACCOUNT *a,
				    GWEN_HTTP_SESSION *sess,
				    EBC_CONTEXT *ectx);


static int EBC_Provider_ExecAccountQueue(AB_PROVIDER *pro,
					 AB_IMEXPORTER_CONTEXT *ctx,
					 AB_USER *u,
					 GWEN_HTTP_SESSION *sess,
					 EBC_ACCOUNTQUEUE *aq);

static int EBC_Provider_ExecUserQueue(AB_PROVIDER *pro,
				      AB_IMEXPORTER_CONTEXT *ctx,
				      EBC_USERQUEUE *uq);

int EBC_Provider_ExecQueue(AB_PROVIDER *pro,
			   AB_IMEXPORTER_CONTEXT *ctx);
#endif


int EBC_Provider_SendCommands(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_IMEXPORTER_CONTEXT *ctx);
int EBC_Provider__SendUserQueue(AB_PROVIDER *pro, AB_USERQUEUE *uq, AB_IMEXPORTER_CONTEXT *ctx);
int EBC_Provider__SendAccountQueue(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNTQUEUE *aq,
                                   GWEN_HTTP_SESSION *sess, AB_IMEXPORTER_CONTEXT *ctx);
void EBC_Provider_SortTransactionsIntoJobQueues(AB_PROVIDER *pro, AB_ACCOUNTQUEUE *aq);



int EBC_Provider__CreateTransactionLimitsForAccount(AB_PROVIDER *pro, AB_ACCOUNT *acc, AB_TRANSACTION_LIMITS_LIST *tll);
int EBC_Provider_UpdateAccountSpec(AB_PROVIDER *pro, AB_ACCOUNT_SPEC *as, int doLock);





#endif
