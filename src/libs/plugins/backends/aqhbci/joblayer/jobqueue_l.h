/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBQUEUE_L_H
#define AH_JOBQUEUE_L_H

#include "aqhbci/joblayer/job_l.h"


typedef struct AH_JOBQUEUE AH_JOBQUEUE;

/** jobs in the queue need encrytion */
#define AH_JOBQUEUE_FLAGS_CRYPT          0x00000001
/** jobs in the queue need signature */
#define AH_JOBQUEUE_FLAGS_SIGN           0x00000002
/** job queue contains a dialog job */
#define AH_JOBQUEUE_FLAGS_ISDIALOG       0x00000004
/** jobs in the queue need a TAN */
#define AH_JOBQUEUE_FLAGS_NEEDTAN        0x00000010
/** set systemid "0" instead of real value */
#define AH_JOBQUEUE_FLAGS_NOSYSID        0x00000020
/** dont select iTAN mode for this queue (i.e. use single step mode) */
#define AH_JOBQUEUE_FLAGS_NOITAN         0x00000040
/** use "1" as signature counter value in signatures, not the real sequence counter */
#define AH_JOBQUEUE_FLAGS_SIGNSEQONE     0x00000080

#define AH_JOBQUEUE_FLAGS_COPYMASK       0x0000ffff
/** job queue has been sent to outbox */
#define AH_JOBQUEUE_FLAGS_OUTBOX         0x00040000

/** at least on job in the queue has warnings */
#define AH_JOBQUEUE_FLAGS_HASWARNINGS    0x00080000

/** at least on job in the queue has errors */
#define AH_JOBQUEUE_FLAGS_HASERRORS      0x00100000

/** dialog was aborted by peer */
#define AH_JOBQUEUE_FLAGS_DIALOG_ABORTED 0x00200000

/** received an error code about access problems (e.g. user key blocked at the bank) */
#define AH_JOBQUEUE_FLAGS_ACCESS_PROBLEM 0x00400000

/** used tan was not really used, free to reuse */
#define AH_JOBQUEUE_FLAGS_RECYCLE_TAN    0x00800000

/** bad pin flagged by server */
#define AH_JOBQUEUE_FLAGS_BAD_PIN        0x01000000

#define AH_JOBQUEUE_FLAGS_IGNOREACCOUNTS 0x02000000


#include <gwenhywfar/misc.h>

#include "aqhbci/msglayer/message_l.h"

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

AH_JOBQUEUE_ADDRESULT AH_JobQueue_AddJob(AH_JOBQUEUE *jq, AH_JOB *j);

AH_JOB_LIST *AH_JobQueue_GetJobList(const AH_JOBQUEUE *jq);
AH_JOB_LIST *AH_JobQueue_TakeJobList(AH_JOBQUEUE *jq);
AH_JOB *AH_JobQueue_GetFirstJob(const AH_JOBQUEUE *jq);

unsigned int AH_JobQueue_GetCount(const AH_JOBQUEUE *jq);

void AH_JobQueue_Dump(AH_JOBQUEUE *jq, FILE *f, unsigned int insert);
void AH_JobQueue_DumpJobList(const AH_JOBQUEUE *jq, FILE *f, unsigned int insert);



uint32_t AH_JobQueue_GetFlags(const AH_JOBQUEUE *jq);
void AH_JobQueue_SetFlags(AH_JOBQUEUE *jq, uint32_t f);
void AH_JobQueue_AddFlags(AH_JOBQUEUE *jq, uint32_t f);
void AH_JobQueue_SubFlags(AH_JOBQUEUE *jq, uint32_t f);

void AH_JobQueue_SetJobStatusOnMatch(AH_JOBQUEUE *jq,
                                     AH_JOB_STATUS matchSt,
                                     AH_JOB_STATUS newSt);

const char *AH_JobQueue_GetUsedTan(const AH_JOBQUEUE *jq);
void AH_JobQueue_SetUsedTan(AH_JOBQUEUE *jq, const char *s);

const char *AH_JobQueue_GetUsedPin(const AH_JOBQUEUE *jq);
void AH_JobQueue_SetUsedPin(AH_JOBQUEUE *jq, const char *s);


int AH_JobQueue_GetSecProfile(const AH_JOBQUEUE *jq);
void AH_JobQueue_SetSecProfile(AH_JOBQUEUE *jq, int i);

int AH_JobQueue_GetSecClass(const AH_JOBQUEUE *jq);
void AH_JobQueue_SetSecClass(AH_JOBQUEUE *jq, int i);

GWEN_STRINGLIST *AH_JobQueue_GetSigners(const AH_JOBQUEUE *jq);
void AH_JobQueue_SetSigners(AH_JOBQUEUE *jq, GWEN_STRINGLIST *signers);


AH_JOBQUEUE *AH_JobQueue_GetReferenceQueue(const AH_JOBQUEUE *jq);
/**
 * Internally calls @ref AH_JobQueue_Attach.
 */
void AH_JobQueue_SetReferenceQueue(AH_JOBQUEUE *jq, AH_JOBQUEUE *refq);


GWEN_DB_NODE *AH_JobQueue_GetDbAllResponses(const AH_JOBQUEUE *jq);
void AH_JobQueue_AddToAllResponses(AH_JOBQUEUE *jq, GWEN_DB_NODE *dbResponse);
void AH_JobQueue_AddAllResponsesToJob(const AH_JOBQUEUE *jq, AH_JOB *j);


#endif /* AH_JOBQUEUE_L_H */




