/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
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


#include <aqbanking/ab_userqueue.h>


typedef struct AH_PROVIDER AH_PROVIDER;
struct AH_PROVIDER {
  AH_HBCI *hbci;
  AB_JOB_LIST2 *bankingJobs;
  AH_OUTBOX *outbox;
  GWEN_DB_NODE *dbTempConfig;
};
static void GWENHYWFAR_CB AH_Provider_FreeData(void *bp, void *p);


static AH_JOB *AH_Provider__FindMyJob(AH_JOB_LIST *mjl, uint32_t jid);

static int AH_Provider__HashRmd160(const uint8_t *p, unsigned int l, uint8_t *buf);
static int AH_Provider__HashSha256(const uint8_t *p, unsigned int l, uint8_t *buf);


/** @name Overwritten Virtual Functions
 *
 */
/*@{*/
static int AH_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int AH_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int AH_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j);
static int AH_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j);
static int AH_Provider_Execute(AB_PROVIDER *pro,
			       AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Provider_ResetQueue(AB_PROVIDER *pro);
static int AH_Provider_ExtendUser(AB_PROVIDER *pro, AB_USER *u,
				  AB_PROVIDER_EXTEND_MODE em,
				  GWEN_DB_NODE *dbBackend);
static int AH_Provider_ExtendAccount(AB_PROVIDER *pro, AB_ACCOUNT *a,
				     AB_PROVIDER_EXTEND_MODE em,
				     GWEN_DB_NODE *dbBackend);
static int AH_Provider_Update(AB_PROVIDER *pro,
                              uint32_t lastVersion,
                              uint32_t currentVersion);

static GWEN_DIALOG *AH_Provider_GetNewUserDialog(AB_PROVIDER *pro, int i);
static GWEN_DIALOG *AH_Provider_GetNewCardUserDialog(AB_PROVIDER *pro);

static GWEN_DIALOG *AH_Provider_GetEditUserDialog(AB_PROVIDER *pro, AB_USER *u);
static GWEN_DIALOG *AH_Provider_GetUserTypeDialog(AB_PROVIDER *pro);

static GWEN_DIALOG *AH_Provider_GetEditAccountDialog(AB_PROVIDER *pro, AB_ACCOUNT *a);

static int AH_Provider_SendCommands(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_IMEXPORTER_CONTEXT *ctx);


static int AH_Provider_GetIniLetterTxt1(AB_PROVIDER *pro,
					AB_USER *u,
					int useBankKey,
					GWEN_BUFFER *lbuf,
					int nounmount);
static int AH_Provider_GetIniLetterHtml1(AB_PROVIDER *pro,
					 AB_USER *u,
					 int useBankKey,
					 GWEN_BUFFER *lbuf,
					 int nounmount);

static int AH_Provider_GetIniLetterTxt2(AB_PROVIDER *pro,
					AB_USER *u,
					int useBankKey,
					GWEN_BUFFER *lbuf,
					int nounmount);
static int AH_Provider_GetIniLetterHtml2(AB_PROVIDER *pro,
					 AB_USER *u,
					 int useBankKey,
					 GWEN_BUFFER *lbuf,
					 int nounmount);

static int AH_Provider__JobTypeToCommand(int jt);

static int AH_Provider__GetMultiHbciJob(AB_PROVIDER *pro, AB_JOB *j, AH_JOB **pHbciJob);
static int AH_Provider__CreateHbciJob(AB_PROVIDER *pro, AB_JOB *j, AH_JOB **pHbciJob);

static int AH_Provider__CreateHbciJob2(AB_PROVIDER *pro, AB_USER *mu, AB_ACCOUNT *ma, int cmd, AH_JOB **pHbciJob);
static int AH_Provider__GetMultiHbciJob2(AB_PROVIDER *pro, AH_OUTBOX *outbox, AB_USER *mu, AB_ACCOUNT *ma, int cmd, AH_JOB **pHbciJob);


static int AH_Provider__AddAccountSepaInfoJobsToOutBox(AB_PROVIDER *pro, AH_OUTBOX *ob);


static int AH_Provider__SortProviderQueueIntoUserQueueList(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_USERQUEUE_LIST *uql);
static void AH_Provider__FreeUsersAndAccountsFromUserQueueList(AB_PROVIDER *pro, AB_USERQUEUE_LIST *uql);
static int AH_Provider__AddCommandsToOutbox(AB_PROVIDER *pro, AB_USERQUEUE_LIST *uql, AB_IMEXPORTER_CONTEXT *ctx, AH_OUTBOX *outbox);
static int AH_Provider__AddCommandToOutbox(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *t, AH_OUTBOX *outbox);


/*@}*/

#endif /* AH_PROVIDER_P_H */




