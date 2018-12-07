/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQEBICS_CLIENT_PROVIDER_P_H
#define AQEBICS_CLIENT_PROVIDER_P_H

#include "provider_l.h"

#include "context_l.h"
#include "queues_l.h"

#include <aqbanking/job_be.h>
#include <aqbanking/provider_be.h>



typedef struct EBC_PROVIDER EBC_PROVIDER;
struct EBC_PROVIDER {
  AB_JOB_LIST2 *bankingJobs;
  EBC_QUEUE *queue;

  GWEN_DB_NODE *dbConfig;
  uint32_t lastJobId;
  uint32_t lastMediumId;
  EBC_CONTEXT_LIST *contextList;

  int connectTimeout;
  int transferTimeout;
};

static void GWENHYWFAR_CB EBC_Provider_FreeData(void *bp, void *p);

static int EBC_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int EBC_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int EBC_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j);
static int EBC_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j);
static int EBC_Provider_Execute(AB_PROVIDER *pro, AB_IMEXPORTER_CONTEXT *ctx);
static int EBC_Provider_ResetQueue(AB_PROVIDER *pro);
static int EBC_Provider_ExtendUser(AB_PROVIDER *pro, AB_USER *u,
				   AB_PROVIDER_EXTEND_MODE em,
				   GWEN_DB_NODE *db);
static int EBC_Provider_ExtendAccount(AB_PROVIDER *pro, AB_ACCOUNT *a,
				      AB_PROVIDER_EXTEND_MODE em,
				      GWEN_DB_NODE *db);
static GWEN_DIALOG *EBC_Provider_GetEditUserDialog(AB_PROVIDER *pro, AB_USER *u);
static GWEN_DIALOG *EBC_Provider_GetNewUserDialog(AB_PROVIDER *pro, int i);


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
					EBC_CONTEXT *ectx);

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

AB_JOB *EBC_Provider_FindJobById(AB_JOB_LIST2 *jl, uint32_t jid);

int EBC_Provider_CountDoneJobs(AB_JOB_LIST2 *jl);


void EBC_Provider_SetJobListStatus(AB_JOB_LIST2 *jl, AB_JOB_STATUS js);






#endif
