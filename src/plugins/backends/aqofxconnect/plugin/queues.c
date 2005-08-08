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



GWEN_LIST_FUNCTIONS(AO_BANKQUEUE, AO_BankQueue)
GWEN_LIST_FUNCTIONS(AO_USERQUEUE, AO_UserQueue)


AO_USERQUEUE *AO_UserQueue_new(AO_USER *u) {
  AO_USERQUEUE *uq;

  assert(u);
  GWEN_NEW_OBJECT(AO_USERQUEUE, uq);
  uq->user=u;
  uq->jobs=AB_Job_List2_new();

  return uq;
}



void AO_UserQueue_free(AO_USERQUEUE *uq) {
  if (uq) {
    AB_Job_List2_free(uq->jobs);
    GWEN_FREE_OBJECT(uq);
  }
}



AO_USER *AO_UserQueue_GetUser(const AO_USERQUEUE *uq){
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




AO_BANKQUEUE *AO_BankQueue_new(AO_BANK *b) {
  AO_BANKQUEUE *bq;

  assert(b);
  GWEN_NEW_OBJECT(AO_BANKQUEUE, bq);
  bq->bank=b;
  bq->userQueues=AO_UserQueue_List_new();

  return bq;
}



void AO_BankQueue_free(AO_BANKQUEUE *bq) {
  if (bq) {
    AO_UserQueue_List_free(bq->userQueues);
    GWEN_FREE_OBJECT(bq);
  }
}



AO_BANK *AO_BankQueue_GetBank(const AO_BANKQUEUE *bq){
  assert(bq);
  return bq->bank;
}



AO_USERQUEUE_LIST *AO_BankQueue_GetUserQueues(const AO_BANKQUEUE *bq) {
  assert(bq);
  return bq->userQueues;
}



AO_USERQUEUE *AO_BankQueue_FindUserQueue(const AO_BANKQUEUE *bq,
                                         const char *uid){
  AO_USERQUEUE *uq;

  assert(bq);
  uq=AO_UserQueue_List_First(bq->userQueues);
  while(uq) {
    const char *s;
    s=AO_User_GetUserId(AO_UserQueue_GetUser(uq));
    assert(s);
    if (strcasecmp(s, uid)==0)
      break;
    uq=AO_UserQueue_List_Next(uq);
  }

  return uq;
}



void AO_BankQueue_AddUserQueue(AO_BANKQUEUE *bq, AO_USERQUEUE *uq) {
  assert(bq);
  AO_UserQueue_List_Add(uq, bq->userQueues);
}



void AO_BankQueue_AddJob(AO_BANKQUEUE *bq, const char *uid, AB_JOB *bj) {
  AO_USERQUEUE *uq;

  assert(bq);
  assert(uid);
  assert(*uid);
  assert(bj);

  uq=AO_BankQueue_FindUserQueue(bq, uid);
  if (!uq) {
    AO_USER *u;

    u=AO_Bank_FindUser(bq->bank, uid);
    assert(u);
    uq=AO_UserQueue_new(u);
    AO_UserQueue_List_Add(uq, bq->userQueues);
  }
  AO_UserQueue_AddJob(uq, bj);
}




