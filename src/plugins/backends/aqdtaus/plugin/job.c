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

#include "job_p.h"


#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_LIST_FUNCTIONS(AD_JOB, AD_Job)




AD_JOB *AD_Job_new(AB_ACCOUNT *acc, int isDebitJob, uint32_t jid){
  AD_JOB *dj;

  assert(acc);
  GWEN_NEW_OBJECT(AD_JOB, dj);
  GWEN_LIST_INIT(AD_JOB, dj);
  dj->account=acc;
  dj->transactions=AB_Transaction_List2_new();
  dj->isDebitNote=isDebitJob;
  dj->jobId=jid;
  return dj;
}



void AD_Job_free(AD_JOB *dj){
  if (dj) {
    GWEN_LIST_FINI(AD_JOB, dj);
    free(dj->resultText);
    AB_Transaction_List2_freeAll(dj->transactions);
    GWEN_FREE_OBJECT(dj);
  }
}



void AD_Job_AddTransfer(AD_JOB *dj, AB_TRANSACTION *t) {
  assert(dj);

  AB_Transaction_List2_PushBack(dj->transactions, t);
}



int AD_Job_GetTransferCount(const AD_JOB *dj) {
  assert(dj);
  return AB_Transaction_List2_GetSize(dj->transactions);
}



AB_TRANSACTION_LIST2 *AD_Job_GetTransfers(const AD_JOB *dj) {
  assert(dj);
  return dj->transactions;
}



int AD_Job_GetIsDebitNote(const AD_JOB *dj){
  assert(dj);
  return dj->isDebitNote;
}



AB_ACCOUNT *AD_Job_GetAccount(const AD_JOB *dj){
  assert(dj);
  return dj->account;
}



uint32_t AD_Job_GetJobId(const AD_JOB *dj){
  assert(dj);
  return dj->jobId;
}



void AD_Job_SetResult(AD_JOB *dj, int code, const char *text){
  assert(dj);
  free(dj->resultText);
  if (text) dj->resultText=strdup(text);
  else dj->resultText=0;
  dj->resultCode=code;
}



int AD_Job_GetResultCode(const AD_JOB *dj){
  assert(dj);
  return dj->resultCode;
}



const char *AD_Job_GetResultText(const AD_JOB *dj){
  assert(dj);
  return dj->resultText;
}











