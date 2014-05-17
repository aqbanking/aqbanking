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
  AB_USER *user;
  AH_JOBQUEUE_LIST *todoQueues;
  AH_JOBQUEUE_LIST *finishedQueues;

  AB_JOB_LIST2 *pendingJobs;

  AH_JOB_LIST *todoJobs;
  AH_JOB_LIST *finishedJobs;

  int isLocked;

  uint32_t usage;
};


static AH_OUTBOX__CBOX *AH_Outbox__CBox_new(AH_HBCI *hbci,
                                            AB_USER *u,
                                            AH_OUTBOX *ob);
static void AH_Outbox__CBox_free(AH_OUTBOX__CBOX *cbox);

static void AH_Outbox__CBox_Finish(AH_OUTBOX__CBOX *cbox);

static void AH_Outbox__CBox_AddTodoJob(AH_OUTBOX__CBOX *cbox, AH_JOB *j);
static void AH_Outbox__CBox_AddPendingJob(AH_OUTBOX__CBOX *cbox, AB_JOB *bj);

static AB_USER*
  AH_Outbox__CBox_GetUser(const AH_OUTBOX__CBOX *cbox);

static AH_JOB_LIST*
  AH_Outbox__CBox_TakeFinishedJobs(AH_OUTBOX__CBOX *cbox);


static GWEN_TIME*
  AH_Outbox__CBox_GetEarliestPendingDate(AH_OUTBOX__CBOX *cbox);
static GWEN_TIME *AH_Outbox__CBox_GetLatestPendingDate(AH_OUTBOX__CBOX *cbox);


static int AH_Outbox__CBox_Prepare(AH_OUTBOX__CBOX *cbox);

static int AH_Outbox__CBox__Hash(int mode,
				 const uint8_t *p,
				 unsigned int l,
				 AH_MSG *msg);



struct AH_OUTBOX {
  GWEN_INHERIT_ELEMENT(AH_OUTBOX);
  AH_HBCI *hbci;
  AH_OUTBOX__CBOX_LIST *userBoxes;
  AH_JOB_LIST *finishedJobs;
  AB_IMEXPORTER_CONTEXT *context;

  uint32_t usage;
};



static int AH_Outbox_Prepare(AH_OUTBOX *ob);

static void AH_Outbox__FinishCBox(AH_OUTBOX *ob,
				  AH_OUTBOX__CBOX *cbox);

static int AH_Outbox__Execute(AH_OUTBOX *ob);


static int AH_Outbox_StartSending(AH_OUTBOX *ob);

static unsigned int AH_Outbox__CountJobList(const AH_JOB_LIST *jl);


static int AH_Outbox__CBox_SendQueue(AH_OUTBOX__CBOX *cbox,
                                     AH_DIALOG *dlg,
				     AH_JOBQUEUE *jq);
static int AH_Outbox__CBox_RecvQueue(AH_OUTBOX__CBOX *cbox, 
                                     AH_DIALOG *dlg,
				     AH_JOBQUEUE *jq);
static int AH_Outbox__CBox_PerformQueue(AH_OUTBOX__CBOX *cbox,
                                        AH_DIALOG *dlg,
                                        AH_JOBQUEUE *jq);
static void AH_Outbox__CBox_HandleQueueError(AH_OUTBOX__CBOX *cbox,
                                             AH_JOBQUEUE *jq,
                                             const char *logStr);
static void AH_Outbox__CBox_HandleQueueListError(AH_OUTBOX__CBOX *cbox,
                                                 AH_JOBQUEUE_LIST *jql,
                                                 const char *logStr);

static int AH_Outbox__CBox_SendAndRecvQueue(AH_OUTBOX__CBOX *cbox,
                                            AH_DIALOG *dlg,
					    AH_JOBQUEUE *jq);

static int AH_Outbox__CBox_OpenDialog(AH_OUTBOX__CBOX *cbox, 
				      AH_DIALOG *dlg,
				      uint32_t jqFlags);
static int AH_Outbox__CBox_CloseDialog(AH_OUTBOX__CBOX *cbox, 
                                       AH_DIALOG *dlg,
				       uint32_t jqFlags);

static int AH_Outbox__CBox_PerformNonDialogQueues(AH_OUTBOX__CBOX *cbox,
						  AH_JOBQUEUE_LIST *jql);

static int AH_Outbox__CBox_PerformDialogQueue(AH_OUTBOX__CBOX *cbox,
					      AH_JOBQUEUE *jq);

static void AH_Outbox__CBox_ExtractMatchingQueues(AH_JOBQUEUE_LIST *jql,
                                                  AH_JOBQUEUE_LIST *jqlWanted,
                                                  AH_JOBQUEUE_LIST *jqlRest,
                                                  uint32_t jqflags,
                                                  uint32_t jqmask);
static int AH_Outbox__CBox_SendAndRecvSelected(AH_OUTBOX__CBOX *cbox,
                                               uint32_t jqflags,
					       uint32_t jqmask);

static int AH_Outbox__CBox_SendAndRecvDialogQueues(AH_OUTBOX__CBOX *cbox);

static int AH_Outbox__CBox_SendAndRecvBox(AH_OUTBOX__CBOX *cbox);

static int AH_Outbox_SendAndRecv(AH_OUTBOX *ob);

static AH_JOB *AH_Outbox__FindTransferJobInCheckJobList(const AH_JOB_LIST *jl,
                                                        AB_USER *u,
                                                        AB_ACCOUNT *a,
                                                        const char *jobName);


static int AH_Outbox__CBox_JobToMessage(AH_JOB *j, AH_MSG *msg);
static int AH_Outbox__CBox_Itan_SendMsg(AH_OUTBOX__CBOX *cbox,
                                        AH_DIALOG *dlg,
                                        AH_MSG *msg);
static int AH_Outbox__CBox_Itan1(AH_OUTBOX__CBOX *cbox,
                                 AH_DIALOG *dlg,
                                 AH_JOBQUEUE *jq);

static int AH_Outbox__CBox_Itan2(AH_OUTBOX__CBOX *cbox,
                                 AH_DIALOG *dlg,
                                 AH_JOBQUEUE *qJob);

static int AH_Outbox__CBox_Itan(AH_OUTBOX__CBOX *cbox,
                                AH_DIALOG *dlg,
                                AH_JOBQUEUE *qJob);

static int AH_Outbox__CBox_SelectItanMode(AH_OUTBOX__CBOX *cbox,
                                          AH_DIALOG *dlg);


static void AH_Outbox__CBox_CopyJobResultsToJobList(const AH_JOB *j,
						    const AH_JOB_LIST *qjl);


static int AH_Outbox_LockUsers(AH_OUTBOX *ob, AB_USER_LIST2 *lockedUsers);
static int AH_Outbox_UnlockUsers(AH_OUTBOX *ob, AB_USER_LIST2 *lockedUsers, int abandon);


#endif /* AH_OUTBOX_P_H */





