/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_OUTBOX_L_H
#define AH_OUTBOX_L_H

#define AH_OUTBOX_FLAGS_ENDDIALOG 0x00000001

#define AH_OUTBOX_FLAGS_DEFAULT \
  (\
  AH_OUTBOX_FLAGS_ENDDIALOG \
  )

#define AH_OUTBOX_EXECUTE_WCB_ID "AH_OUTBOX_EXECUTE_WCB_ID"


typedef struct AH_OUTBOX AH_OUTBOX;

typedef struct AH_OUTBOX__CBOX AH_OUTBOX__CBOX;


#include "hbci_l.h"
#include "job_l.h"

#include "aqhbci/banking/user.h"
#include "aqhbci/msglayer/dialog_l.h"
#include "aqhbci/joblayer/jobqueue_l.h"

#include <aqbanking/backendsupport/imexporter.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/gwentime.h>


GWEN_LIST_FUNCTION_DEFS(AH_OUTBOX__CBOX, AH_Outbox__CBox);


AH_OUTBOX *AH_Outbox_new(AB_PROVIDER *pro);
void AH_Outbox_free(AH_OUTBOX *ob);
void AH_Outbox_Attach(AH_OUTBOX *ob);

AB_IMEXPORTER_CONTEXT *AH_Outbox_GetImExContext(const AH_OUTBOX *outbox);


void AH_Outbox_AddJob(AH_OUTBOX *ob, AH_JOB *j);

/* makes all jobs commit their data */
void AH_Outbox_Commit(AH_OUTBOX *ob, int doLock);


/* makes all jobs process their data */
void AH_Outbox_Process(AH_OUTBOX *ob);

/* makes all jobs commit their system data (only calls
 * @ref AH_Job_DefaultCommitHandler which only commits system data
 * like account data, bank parameter data etc according to the flags in
 * @ref AH_HBCIClient).
 */
void AH_Outbox_CommitSystemData(AH_OUTBOX *ob, int doLock);


unsigned int AH_Outbox_CountTodoJobs(AH_OUTBOX *ob);
unsigned int AH_Outbox_CountFinishedJobs(AH_OUTBOX *ob);


int AH_Outbox_Execute(AH_OUTBOX *ob,
                      AB_IMEXPORTER_CONTEXT *ctx,
                      int withProgress, int nounmount, int doLock);


AH_JOB *AH_Outbox_FindTransferJob(AH_OUTBOX *ob,
                                  AB_USER *u,
                                  AB_ACCOUNT *a,
                                  const char *jobName);


AH_JOB_LIST *AH_Outbox_GetFinishedJobs(AH_OUTBOX *ob);


int AH_Outbox__CBox_SendAndRecvQueueNoTan(AH_OUTBOX__CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *jq);
int AH_Outbox__CBox_SendAndRecvQueue(AH_OUTBOX__CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *jq);



AH_OUTBOX *AH_OutboxCBox_GetOutbox(const AH_OUTBOX__CBOX *cbox);
AB_PROVIDER *AH_OutboxCBox_GetProvider(const AH_OUTBOX__CBOX *cbox);
AB_USER *AH_OutboxCBox_GetUser(const AH_OUTBOX__CBOX *cbox);


#endif /* AH_OUTBOX_L_H */

