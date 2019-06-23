/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBQUEUE_L_H
#define AH_JOBQUEUE_L_H

#include "job_l.h"


typedef struct AH_JOBQUEUE AH_JOBQUEUE;

/** jobs in the queue need encrytion */
#define AH_JOBQUEUE_FLAGS_CRYPT       0x00000001
/** jobs in the queue need signature */
#define AH_JOBQUEUE_FLAGS_SIGN        0x00000002
/** job queue contains a dialog job */
#define AH_JOBQUEUE_FLAGS_ISDIALOG    0x00000004
/** jobs in the queue need a TAN */
#define AH_JOBQUEUE_FLAGS_NEEDTAN     0x00000010
/** set systemid "0" instead of real value */
#define AH_JOBQUEUE_FLAGS_NOSYSID     0x00000020
/** dont select iTAN mode for this queue (i.e. use single step mode) */
#define AH_JOBQUEUE_FLAGS_NOITAN      0x00000040
/** use "1" as signature counter value in signatures, not the real sequence counter */
#define AH_JOBQUEUE_FLAGS_SIGNSEQONE  0x00000080

#define AH_JOBQUEUE_FLAGS_COPYMASK    0x0000ffff
/** job queue has been sent to outbox */
#define AH_JOBQUEUE_FLAGS_OUTBOX      0x00040000

/** at least on job in the queue has warnings */
#define AH_JOBQUEUE_FLAGS_HASWARNINGS 0x00080000

/** at least on job in the queue has errors */
#define AH_JOBQUEUE_FLAGS_HASERRORS   0x00100000
/** a dialog job has been started */

#include <gwenhywfar/misc.h>

#include "message_l.h"

typedef enum {
  AH_JobQueueAddResultOk=0,
  AH_JobQueueAddResultJobLimit,
  AH_JobQueueAddResultQueueFull,
  AH_JobQueueAddResultError,
} AH_JOBQUEUE_ADDRESULT;

GWEN_LIST_FUNCTION_DEFS(AH_JOBQUEUE, AH_JobQueue);


AH_JOBQUEUE *AH_JobQueue_new(AB_USER *u);
void AH_JobQueue_free(AH_JOBQUEUE *jq);
void AH_JobQueue_Attach(AH_JOBQUEUE *jq);

AH_JOBQUEUE *AH_JobQueue_fromQueue(AH_JOBQUEUE *oldq);

AB_USER *AH_JobQueue_GetUser(const AH_JOBQUEUE *jq);

AH_JOBQUEUE_ADDRESULT AH_JobQueue_AddJob(AH_JOBQUEUE *jq,
                                         AH_JOB *j);

const AH_JOB_LIST *AH_JobQueue_GetJobList(const AH_JOBQUEUE *jq);
AH_JOB_LIST *AH_JobQueue_TakeJobList(AH_JOBQUEUE *jq);

uint32_t AH_JobQueue_GetMsgNum(const AH_JOBQUEUE *jq);

AH_MSG *AH_JobQueue_ToMessage(AH_JOBQUEUE *jq, AH_DIALOG *dlg);

int AH_JobQueue_DispatchMessage(AH_JOBQUEUE *jq,
                                AH_MSG *msg,
                                GWEN_DB_NODE *db);

unsigned int AH_JobQueue_GetCount(const AH_JOBQUEUE *jq);

void AH_JobQueue_Dump(AH_JOBQUEUE *jq, FILE *f, unsigned int insert);



void AH_JobQueue_AddSigner(AH_JOBQUEUE *jq, const char *s);

uint32_t AH_JobQueue_GetFlags(AH_JOBQUEUE *jq);
void AH_JobQueue_SetFlags(AH_JOBQUEUE *jq, uint32_t f);
void AH_JobQueue_AddFlags(AH_JOBQUEUE *jq, uint32_t f);
void AH_JobQueue_SubFlags(AH_JOBQUEUE *jq, uint32_t f);

void AH_JobQueue_SetJobStatusOnMatch(AH_JOBQUEUE *jq,
                                     AH_JOB_STATUS matchSt,
                                     AH_JOB_STATUS newSt);


#endif /* AH_JOBQUEUE_L_H */




