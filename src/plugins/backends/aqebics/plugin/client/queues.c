/***************************************************************************
    begin       : Wed May 07 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "queues_p.h"


#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_LIST_FUNCTIONS(EBC_USERQUEUE, EBC_UserQueue)
GWEN_LIST_FUNCTIONS(EBC_ACCOUNTQUEUE, EBC_AccountQueue)




EBC_ACCOUNTQUEUE *EBC_AccountQueue_new(AB_ACCOUNT *a) {
  EBC_ACCOUNTQUEUE *aq;

  assert(a);
  GWEN_NEW_OBJECT(EBC_ACCOUNTQUEUE, aq);
  GWEN_LIST_INIT(EBC_ACCOUNTQUEUE, aq);
  aq->account=a;
  aq->contexts=EBC_Context_List_new();

  return aq;
}



void EBC_AccountQueue_free(EBC_ACCOUNTQUEUE *aq) {
  if (aq) {
    GWEN_LIST_FINI(EBC_ACCOUNTQUEUE, aq);
    EBC_Context_List_free(aq->contexts);
    GWEN_FREE_OBJECT(aq);
  }
}



AB_ACCOUNT *EBC_AccountQueue_GetAccount(const EBC_ACCOUNTQUEUE *aq) {
  assert(aq);
  return aq->account;
}



void EBC_AccountQueue_AddJob(EBC_ACCOUNTQUEUE *aq, AB_JOB *bj) {
  AB_JOB_TYPE jt;
  EBC_CONTEXT *ctx;

  assert(aq);
  assert(bj);

  jt=AB_Job_GetType(bj);

  ctx=EBC_AccountQueue_FindContext(aq, jt);
  if (!ctx) {
    ctx=EBC_Context_new();
    EBC_Context_SetJobType(ctx, jt);
    EBC_Context_List_Add(ctx, aq->contexts);
  }

  AB_Job_List2_PushBack(EBC_Context_GetJobs(ctx), bj);
}



EBC_CONTEXT *EBC_AccountQueue_FindContext(const EBC_ACCOUNTQUEUE *aq, AB_JOB_TYPE jt) {
  EBC_CONTEXT *ctx;

  assert(aq);

  ctx=EBC_Context_List_First(aq->contexts);
  while(ctx) {
    if (EBC_Context_GetJobType(ctx)==jt)
      return ctx;
    ctx=EBC_Context_List_Next(ctx);
  }

  return NULL;
}



EBC_CONTEXT_LIST *EBC_AccountQueue_GetContextList(const EBC_ACCOUNTQUEUE *aq) {
  assert(aq);
  return aq->contexts;
}






EBC_USERQUEUE *EBC_UserQueue_new(AB_USER *u) {
  EBC_USERQUEUE *uq;

  assert(u);
  GWEN_NEW_OBJECT(EBC_USERQUEUE, uq);
  GWEN_LIST_INIT(EBC_USERQUEUE, uq);
  uq->user=u;

  uq->accountQueues=EBC_AccountQueue_List_new();

  return uq;
}



void EBC_UserQueue_free(EBC_USERQUEUE *uq) {
  if (uq) {
    GWEN_LIST_FINI(EBC_USERQUEUE, uq);
    GWEN_FREE_OBJECT(uq);
  }
}



AB_USER *EBC_UserQueue_GetUser(const EBC_USERQUEUE *uq){
  assert(uq);
  return uq->user;
}



void EBC_UserQueue_AddJob(EBC_USERQUEUE *uq, AB_JOB *bj){
  AB_ACCOUNT *acc;
  EBC_ACCOUNTQUEUE *aq;

  assert(uq);
  assert(bj);

  acc=AB_Job_GetAccount(bj);

  aq=EBC_AccountQueue_List_First(uq->accountQueues);
  while(aq) {
    if (EBC_AccountQueue_GetAccount(aq)==acc)
      break;
    aq=EBC_AccountQueue_List_Next(aq);
  }

  if (!aq) {
    aq=EBC_AccountQueue_new(acc);
    EBC_AccountQueue_List_Add(aq, uq->accountQueues);
  }
  EBC_AccountQueue_AddJob(aq, bj);
}



EBC_ACCOUNTQUEUE *EBC_UserQueue_FindAccountQueue(const EBC_USERQUEUE *uq,
						 const AB_ACCOUNT *a) {
  EBC_ACCOUNTQUEUE *aq;

  assert(uq);
  assert(a);

  aq=EBC_AccountQueue_List_First(uq->accountQueues);
  while(aq) {
    if (EBC_AccountQueue_GetAccount(aq)==a)
      return aq;
    aq=EBC_AccountQueue_List_Next(aq);
  }

  return NULL;
}



EBC_ACCOUNTQUEUE_LIST *EBC_UserQueue_GetAccountQueues(const EBC_USERQUEUE *uq) {
  assert(uq);
  return uq->accountQueues;
}










EBC_QUEUE *EBC_Queue_new() {
  EBC_QUEUE *q;

  GWEN_NEW_OBJECT(EBC_QUEUE, q);
  q->userQueues=EBC_UserQueue_List_new();
  return q;
}



void EBC_Queue_free(EBC_QUEUE *q) {
  if (q) {
    EBC_UserQueue_List_free(q->userQueues);
    GWEN_FREE_OBJECT(q);
  }
}



EBC_USERQUEUE *EBC_Queue_FindUserQueue(EBC_QUEUE *q, const AB_USER *u) {
  EBC_USERQUEUE *uq;

  uq=EBC_UserQueue_List_First(q->userQueues);
  while(uq) {
    if (EBC_UserQueue_GetUser(uq)==u)
      break;
    uq=EBC_UserQueue_List_Next(uq);
  }
  return uq;
}



EBC_USERQUEUE *EBC_Queue_GetUserQueue(EBC_QUEUE *q, AB_USER *u) {
  EBC_USERQUEUE *uq;

  assert(q);
  assert(u);

  uq=EBC_Queue_FindUserQueue(q, u);
  if (!uq) {
    uq=EBC_UserQueue_new(u);
    EBC_UserQueue_List_Add(uq, q->userQueues);
  }
  return uq;
}



EBC_USERQUEUE_LIST *EBC_Queue_GetUserQueues(const EBC_QUEUE *q) {
  assert(q);
  return q->userQueues;
}



void EBC_Queue_AddJob(EBC_QUEUE *q, AB_USER *u, AB_JOB *bj) {
  EBC_USERQUEUE *uq;

  assert(q);
  assert(u);
  assert(bj);

  uq=EBC_Queue_GetUserQueue(q, u);
  assert(uq);
  EBC_UserQueue_AddJob(uq, bj);
}



void EBC_Queue_Clear(EBC_QUEUE *q) {
  assert(q);
  EBC_UserQueue_List_Clear(q->userQueues);
}



AB_JOB *EBC_Queue_FindFirstJobLikeThis(EBC_QUEUE *q,
                                       AB_USER *u,
				       AB_JOB *bj) {
  EBC_USERQUEUE *uq=EBC_Queue_FindUserQueue(q, u);
  if (uq) {
    EBC_ACCOUNTQUEUE *aq=EBC_UserQueue_FindAccountQueue(uq, AB_Job_GetAccount(bj));
    if (aq) {
      EBC_CONTEXT *ctx=EBC_AccountQueue_FindContext(aq, AB_Job_GetType(bj));
      if (ctx)
	return AB_Job_List2_GetFront(EBC_Context_GetJobs(ctx));
    }
  }

  return NULL;
}






