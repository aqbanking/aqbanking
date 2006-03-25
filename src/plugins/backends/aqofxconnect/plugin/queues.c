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



GWEN_LIST_FUNCTIONS(AO_USERQUEUE, AO_UserQueue)


AO_USERQUEUE *AO_UserQueue_new(AB_USER *u) {
  AO_USERQUEUE *uq;

  assert(u);
  GWEN_NEW_OBJECT(AO_USERQUEUE, uq);
  GWEN_LIST_INIT(AO_USERQUEUE, uq);
  uq->user=u;
  uq->jobs=AB_Job_List2_new();

  return uq;
}



void AO_UserQueue_free(AO_USERQUEUE *uq) {
  if (uq) {
    GWEN_LIST_FINI(AO_USERQUEUE, uq);
    AB_Job_List2_free(uq->jobs);
    GWEN_FREE_OBJECT(uq);
  }
}



AB_USER *AO_UserQueue_GetUser(const AO_USERQUEUE *uq){
  assert(uq);
  return uq->user;
}



AB_JOB_LIST2 *AO_UserQueue_GetJobs(const AO_USERQUEUE *uq){
  assert(uq);
  return uq->jobs;
}



void AO_UserQueue_AddJob(AO_USERQUEUE *uq, AB_JOB *bj){
  assert(uq);
  assert(bj);
  AB_Job_List2_PushBack(uq->jobs, bj);
}





AO_QUEUE *AO_Queue_new() {
  AO_QUEUE *q;

  GWEN_NEW_OBJECT(AO_QUEUE, q);
  q->userQueues=AO_UserQueue_List_new();
  return q;
}



void AO_Queue_free(AO_QUEUE *q) {
  if (q) {
    AO_UserQueue_List_free(q->userQueues);
    GWEN_FREE_OBJECT(q);
  }
}



AO_USERQUEUE *AO_Queue_FindUserQueue(AO_QUEUE *q, const AB_USER *u) {
  AO_USERQUEUE *uq;

  uq=AO_UserQueue_List_First(q->userQueues);
  while(uq) {
    if (AO_UserQueue_GetUser(uq)==u)
      break;
    uq=AO_UserQueue_List_Next(uq);
  }
  return uq;
}



AO_USERQUEUE *AO_Queue_GetUserQueue(AO_QUEUE *q, AB_USER *u) {
  AO_USERQUEUE *uq;

  assert(q);
  assert(u);

  uq=AO_Queue_FindUserQueue(q, u);
  if (!uq) {
    uq=AO_UserQueue_new(u);
    AO_UserQueue_List_Add(uq, q->userQueues);
  }
  return uq;
}



AO_USERQUEUE *AO_Queue_FirstUserQueue(AO_QUEUE *q) {
  assert(q);
  return AO_UserQueue_List_First(q->userQueues);
}



void AO_Queue_AddJob(AO_QUEUE *q, AB_USER *u, AB_JOB *bj) {
  AO_USERQUEUE *uq;

  assert(q);
  assert(u);
  assert(bj);

  uq=AO_Queue_GetUserQueue(q, u);
  assert(uq);
  AO_UserQueue_AddJob(uq, bj);
}



void AO_Queue_Clear(AO_QUEUE *q) {
  assert(q);
  AO_UserQueue_List_Clear(q->userQueues);
}



