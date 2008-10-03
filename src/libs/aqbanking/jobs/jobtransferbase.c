/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobtransferbase_p.h"
#include "job_l.h"
#include "account_l.h"
#include "banking_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_INHERIT(AB_JOB, AB_JOBTRANSFERBASE)



AB_JOB *AB_JobTransferBase_new(AB_JOB_TYPE jt, AB_ACCOUNT *a){
  AB_JOB *j;
  AB_JOBTRANSFERBASE *jd;

  j=AB_Job_new(jt, a);
  GWEN_NEW_OBJECT(AB_JOBTRANSFERBASE, jd);
  GWEN_INHERIT_SETDATA(AB_JOB, AB_JOBTRANSFERBASE, j, jd,
                       AB_JobTransferBase_FreeData);

  return j;
}



void GWENHYWFAR_CB AB_JobTransferBase_FreeData(void *bp, void *p) {
  AB_JOBTRANSFERBASE *jd;

  jd=(AB_JOBTRANSFERBASE*)p;
  AB_Transaction_free(jd->transaction);
  AB_TransactionLimits_free(jd->limits);
  free(jd->textKeys);
  GWEN_FREE_OBJECT(jd);
}



void AB_JobTransferBase_SetFieldLimits(AB_JOB *j,
                                       AB_TRANSACTION_LIMITS *limits){
  AB_JOBTRANSFERBASE *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBTRANSFERBASE, j);
  assert(jd);

  AB_TransactionLimits_free(jd->limits);
  if (limits) jd->limits=AB_TransactionLimits_dup(limits);
  else jd->limits=0;

  free(jd->textKeys);
  jd->textKeys=0;
}



const AB_TRANSACTION_LIMITS *AB_JobTransferBase_GetFieldLimits(AB_JOB *j) {
  AB_JOBTRANSFERBASE *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBTRANSFERBASE, j);
  assert(jd);

  return jd->limits;
}



int AB_JobTransferBase_SetTransaction(AB_JOB *j, const AB_TRANSACTION *t){
  AB_JOBTRANSFERBASE *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBTRANSFERBASE, j);
  assert(jd);

  /* TODO: check transaction */

  AB_Transaction_free(jd->transaction);
  if (t) {
    AB_ACCOUNT *a;
    AB_BANKING *ba;

    a=AB_Job_GetAccount(j);
    assert(a);
    ba=AB_Account_GetBanking(a);
    assert(ba);

    jd->transaction=AB_Transaction_dup(t);
    /* assign unique id */
    AB_Transaction_SetUniqueId(jd->transaction, AB_Banking_GetUniqueId(ba, 0));
  }
  else
    jd->transaction=0;

  return 0;
}



const AB_TRANSACTION *AB_JobTransferBase_GetTransaction(const AB_JOB *j){
  AB_JOBTRANSFERBASE *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBTRANSFERBASE, j);
  assert(jd);

  return jd->transaction;
}





