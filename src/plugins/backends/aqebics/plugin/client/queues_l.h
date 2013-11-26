/***************************************************************************
    begin       : Wed May 07 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AO_QUEUES_H
#define AO_QUEUES_H

#include <gwenhywfar/misc.h>
#include <aqebics/user.h>

#include "context_l.h"


typedef struct EBC_QUEUE EBC_QUEUE;
typedef struct EBC_USERQUEUE EBC_USERQUEUE;
typedef struct EBC_ACCOUNTQUEUE EBC_ACCOUNTQUEUE;

GWEN_LIST_FUNCTION_DEFS(EBC_USERQUEUE, EBC_UserQueue)
GWEN_LIST_FUNCTION_DEFS(EBC_ACCOUNTQUEUE, EBC_AccountQueue)


EBC_ACCOUNTQUEUE *EBC_AccountQueue_new(AB_ACCOUNT *a);
void EBC_AccountQueue_free(EBC_ACCOUNTQUEUE *aq);
AB_ACCOUNT *EBC_AccountQueue_GetAccount(const EBC_ACCOUNTQUEUE *aq);

EBC_CONTEXT *EBC_AccountQueue_FindContext(const EBC_ACCOUNTQUEUE *aq, AB_JOB_TYPE jt);
void EBC_AccountQueue_AddJob(EBC_ACCOUNTQUEUE *aq, AB_JOB *bj);

EBC_CONTEXT_LIST *EBC_AccountQueue_GetContextList(const EBC_ACCOUNTQUEUE *aq);



EBC_USERQUEUE *EBC_UserQueue_new(AB_USER *u);
void EBC_UserQueue_free(EBC_USERQUEUE *uq);
AB_USER *EBC_UserQueue_GetUser(const EBC_USERQUEUE *uq);

EBC_ACCOUNTQUEUE *EBC_UserQueue_FindAccountQueue(const EBC_USERQUEUE *uq,
						 const AB_ACCOUNT *a);

EBC_ACCOUNTQUEUE_LIST *EBC_UserQueue_GetAccountQueues(const EBC_USERQUEUE *uq);

void EBC_UserQueue_AddJob(EBC_USERQUEUE *uq, AB_JOB *bj);

int EBC_UserQueue_IsLocked(const EBC_USERQUEUE *uq);
void EBC_UserQueue_SetIsLocked(EBC_USERQUEUE *uq, int i);


EBC_QUEUE *EBC_Queue_new();
void EBC_Queue_free(EBC_QUEUE *q);

EBC_USERQUEUE *EBC_Queue_FindUserQueue(EBC_QUEUE *q, const AB_USER *u);
EBC_USERQUEUE *EBC_Queue_GetUserQueue(EBC_QUEUE *q, AB_USER *u);
EBC_USERQUEUE_LIST *EBC_Queue_GetUserQueues(const EBC_QUEUE *q);

void EBC_Queue_AddJob(EBC_QUEUE *q, AB_USER *u, AB_JOB *bj);
void EBC_Queue_Clear(EBC_QUEUE *q);

AB_JOB *EBC_Queue_FindFirstJobLikeThis(EBC_QUEUE *q,
				       AB_USER *u,
				       AB_JOB *bj);


#endif
