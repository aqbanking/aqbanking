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

#ifndef AH_OUTBOX_P_H
#define AH_OUTBOX_P_H


/* private flags */
//#define AH_OUTBOX_FLAGS_FINISHED  0x00010000
#define AH_OUTBOX_FLAGS_LAST      0x00020000

#define AH_OUTBOX_TIME_DISTANCE 750


typedef enum {
  AH_Outbox_SendResultSent=0,
  AH_Outbox_SendResultError,
  AH_Outbox_SendResultNoSend,
  AH_Outbox_SendResultFinished,
  AH_Outbox_SendResultAborted
} AH_OUTBOX_SENDRESULT;


#include "jobqueue_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include "outbox_l.h"


typedef struct AH_OUTBOX__CBOX AH_OUTBOX__CBOX;
GWEN_LIST_FUNCTION_DEFS(AH_OUTBOX__CBOX, AH_Outbox__CBox);


/** Customer's outbox */
struct AH_OUTBOX__CBOX {
  GWEN_LIST_ELEMENT(AH_OUTBOX__CBOX);
  AH_OUTBOX *outbox;
  AH_HBCI *hbci;
  AH_CUSTOMER *customer;
  AH_JOBQUEUE_LIST *todoQueues;
  AH_JOBQUEUE_LIST *finishedQueues;

  AB_JOB_LIST2 *pendingJobs;

  AH_JOB_LIST *todoJobs;
  AH_JOB_LIST *finishedJobs;
  GWEN_TYPE_UINT32 usage;
};


AH_OUTBOX__CBOX *AH_Outbox__CBox_new(AH_HBCI *hbci,
                                     AH_CUSTOMER *cu,
                                     AH_OUTBOX *ob);
void AH_Outbox__CBox_free(AH_OUTBOX__CBOX *cbox);
void AH_Outbox__CBox_Attach(AH_OUTBOX__CBOX *cbox);

void AH_Outbox__CBox_Finish(AH_OUTBOX__CBOX *cbox);

void AH_Outbox__CBox_AddTodoJob(AH_OUTBOX__CBOX *cbox, AH_JOB *j);
void AH_Outbox__CBox_AddPendingJob(AH_OUTBOX__CBOX *cbox, AB_JOB *bj);

AB_JOB_LIST2 *AH_Outbox__CBox_GetPendingJobs(const AH_OUTBOX__CBOX *cbox);

AH_CUSTOMER*
  AH_Outbox__CBox_GetCustomer(const AH_OUTBOX__CBOX *cbox);

AH_JOB_LIST*
  AH_Outbox__CBox_TakeFinishedJobs(AH_OUTBOX__CBOX *cbox);

int AH_Outbox__CBox__Dispatch(AH_OUTBOX__CBOX *cbox,
                                   AH_MSG *msg);

/**
 * Does not take over ownership of msg
 */
int AH_Outbox__CBox_Dispatch(AH_OUTBOX__CBOX *cbox,
                                  AH_MSG *msg);

GWEN_TIME *AH_Outbox__CBox_GetEarliestPendingDate(AH_OUTBOX__CBOX *cbox);
GWEN_TIME *AH_Outbox__CBox_GetLatestPendingDate(AH_OUTBOX__CBOX *cbox);


int AH_Outbox__CBox_Prepare(AH_OUTBOX__CBOX *cbox);

AH_OUTBOX_SENDRESULT
  AH_Outbox__CBox_SendNextQueue(AH_OUTBOX__CBOX *cbox,
                                     int timeout);





struct AH_OUTBOX {
  GWEN_INHERIT_ELEMENT(AH_OUTBOX);
  AH_HBCI *hbci;
  AH_OUTBOX__CBOX_LIST *customerBoxes;
  AH_JOB_LIST *finishedJobs;

  GWEN_TYPE_UINT32 usage;
};



AH_OUTBOX__CBOX *AH_OutBox__FindCBox(const AH_OUTBOX *ob,
                                               const AH_CUSTOMER *cu);
int AH_Outbox_Prepare(AH_OUTBOX *ob);

void AH_Outbox__FinishCBox(AH_OUTBOX *ob,
                                AH_OUTBOX__CBOX *cbox);
void AH_Outbox__FinishOutBox(AH_OUTBOX *ob);

int AH_Outbox__Execute(AH_OUTBOX *ob);


int AH_Outbox_StartSending(AH_OUTBOX *ob);
AH_OUTBOX_SENDRESULT AH_Outbox_Send(AH_OUTBOX *ob, int timeout);
int AH_Outbox_Receive(AH_OUTBOX *ob, int timeout);

unsigned int AH_Outbox__CountJobList(const AH_JOB_LIST *jl);

AH_JOB_LIST *AH_Outbox_TakeFinishedJobs(AH_OUTBOX *ob);



int AH_Outbox__CBox_SendQueue(AH_OUTBOX__CBOX *cbox, int timeout,
                              AH_DIALOG *dlg,
                              AH_JOBQUEUE *jq);
int AH_Outbox__CBox_RecvQueue(AH_OUTBOX__CBOX *cbox, int timeout,
                              AH_DIALOG *dlg,
                              AH_JOBQUEUE *jq);
int AH_Outbox__CBox_PerformQueue(AH_OUTBOX__CBOX *cbox,
                                 AH_DIALOG *dlg,
                                 AH_JOBQUEUE *jq,
                                 int timeout);
void AH_Outbox__CBox_HandleQueueError(AH_OUTBOX__CBOX *cbox,
                                      AH_JOBQUEUE *jq,
                                      const char *logStr);
void AH_Outbox__CBox_HandleQueueListError(AH_OUTBOX__CBOX *cbox,
                                          AH_JOBQUEUE_LIST *jql,
                                          const char *logStr);

int AH_Outbox__CBox_SendAndRecvQueue(AH_OUTBOX__CBOX *cbox,
                                     int timeout,
                                     AH_DIALOG *dlg,
                                     AH_JOBQUEUE *jq);

int AH_Outbox__CBox_OpenDialog(AH_OUTBOX__CBOX *cbox, int timeout,
                               AH_DIALOG *dlg,
                               GWEN_TYPE_UINT32 jqFlags);
int AH_Outbox__CBox_CloseDialog(AH_OUTBOX__CBOX *cbox, int timeout,
                                AH_DIALOG *dlg,
                                GWEN_TYPE_UINT32 jqFlags);

int AH_Outbox__CBox_PerformNonDialogQueues(AH_OUTBOX__CBOX *cbox,
                                           int timeout,
                                           AH_JOBQUEUE_LIST *jql);

int AH_Outbox__CBox_PerformDialogQueue(AH_OUTBOX__CBOX *cbox,
				       int timeout,
                                       AH_JOBQUEUE *jq);

void AH_Outbox__CBox_ExtractMatchingQueues(AH_JOBQUEUE_LIST *jql,
                                           AH_JOBQUEUE_LIST *jqlWanted,
                                           AH_JOBQUEUE_LIST *jqlRest,
                                           GWEN_TYPE_UINT32 jqflags,
                                           GWEN_TYPE_UINT32 jqmask);
int AH_Outbox__CBox_SendAndRecvSelected(AH_OUTBOX__CBOX *cbox,
                                        int timeout,
                                        GWEN_TYPE_UINT32 jqflags,
                                        GWEN_TYPE_UINT32 jqmask);

int AH_Outbox__CBox_SendAndRecvDialogQueues(AH_OUTBOX__CBOX *cbox,
                                            int timeout);

int AH_Outbox__CBox_SendAndRecvBox(AH_OUTBOX__CBOX *cbox, int timeout);

int AH_Outbox_SendAndRecv(AH_OUTBOX *ob, int timeout);

AH_JOB *AH_Outbox__FindTransferJobInCheckJobList(const AH_JOB_LIST *jl,
                                                 AH_CUSTOMER *cu,
                                                 AH_ACCOUNT *a,
                                                 int isTransfer);

#endif /* AH_OUTBOX_P_H */





