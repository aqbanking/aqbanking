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


#include "jobsingledebitnote.h"
#include "jobsingledebitnote_be.h"
#include "jobsingledebitnote_p.h"
#include "job_l.h"
#include "account_l.h"
#include "banking_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_INHERIT(AB_JOB, AB_JOBSINGLEDEBITNOTE)



AB_JOB *AB_JobSingleDebitNote_new(AB_ACCOUNT *a){
  AB_JOB *j;
  AB_JOBSINGLEDEBITNOTE *jd;

  j=AB_Job_new(AB_Job_TypeDebitNote, a);
  GWEN_NEW_OBJECT(AB_JOBSINGLEDEBITNOTE, jd);
  GWEN_INHERIT_SETDATA(AB_JOB, AB_JOBSINGLEDEBITNOTE, j, jd,
                       AB_JobSingleDebitNote_FreeData);

  jd->maxPurposeLines=-1;
  return j;
}



void AB_JobSingleDebitNote_FreeData(void *bp, void *p) {
  AB_JOBSINGLEDEBITNOTE *jd;

  jd=(AB_JOBSINGLEDEBITNOTE*)p;
  AB_Transaction_free(jd->transaction);
  free(jd->textKeys);
  GWEN_FREE_OBJECT(jd);
}



int AB_JobSingleDebitNote_SetTransaction(AB_JOB *j, const AB_TRANSACTION *t){
  AB_JOBSINGLEDEBITNOTE *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLEDEBITNOTE, j);
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
    AB_Transaction_SetUniqueId(jd->transaction, AB_Banking_GetUniqueId(ba));
  }
  else
    jd->transaction=0;

  return 0;
}



const AB_TRANSACTION *AB_JobSingleDebitNote_GetTransaction(const AB_JOB *j){
  AB_JOBSINGLEDEBITNOTE *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLEDEBITNOTE, j);
  assert(jd);

  return jd->transaction;
}



int AB_JobSingleDebitNote_GetMaxPurposeLines(const AB_JOB *j){
  AB_JOBSINGLEDEBITNOTE *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLEDEBITNOTE, j);
  assert(jd);

  return jd->maxPurposeLines;
}



void AB_JobSingleDebitNote_SetMaxPurposeLines(AB_JOB *j, int i){
  AB_JOBSINGLEDEBITNOTE *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLEDEBITNOTE, j);
  assert(jd);

  jd->maxPurposeLines=i;
}



const int *AB_JobSingleDebitNote_GetTextKeys(const AB_JOB *j){
  AB_JOBSINGLEDEBITNOTE *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLEDEBITNOTE, j);
  assert(jd);

  return jd->textKeys;
}



void AB_JobSingleDebitNote_SetTextKeys(AB_JOB *j, const int *tk){
  int i;
  AB_JOBSINGLEDEBITNOTE *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLEDEBITNOTE, j);
  assert(jd);

  free(jd->textKeys);
  jd->textKeys=0;

  if (tk) {
    for (i=0; ; i++) {
      if (tk[i]==-1)
	break;
    }
    if (i) {
      jd->textKeys=(int*)malloc((i+1)*(sizeof(int)));
      assert(jd->textKeys);
      memmove(jd->textKeys, tk, (i+1)*(sizeof(int)));
    }
  }
}



int AB_JobSingleDebitNote_toDb(const AB_JOB *j, GWEN_DB_NODE *db) {
  AB_JOBSINGLEDEBITNOTE *jd;
  GWEN_DB_NODE *dbT;
  int rv;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLEDEBITNOTE, j);
  assert(jd);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "params/maxPurposeLines",
                      jd->maxPurposeLines);

  if (jd->textKeys) {
    int i;

    GWEN_DB_DeleteVar(db, "textkeys");
    for (i=0; ; i++) {
      if (jd->textKeys[i]==-1)
        break;
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
                          "params/textKeys",
                          jd->textKeys[i]);
    }
  }

  if (jd->transaction) {
    dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                         "args/transaction");
    assert(dbT);
    rv=AB_Transaction_toDb(jd->transaction, dbT);
    if (rv) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return rv;
    }
  }

  return 0;
}



AB_JOB *AB_JobSingleDebitNote_fromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db) {
  AB_JOB *j;
  AB_JOBSINGLEDEBITNOTE *jd;
  int i;
  GWEN_DB_NODE *dbT;

  j=AB_Job_new(AB_Job_TypeDebitNote, a);
  GWEN_NEW_OBJECT(AB_JOBSINGLEDEBITNOTE, jd);
  GWEN_INHERIT_SETDATA(AB_JOB, AB_JOBSINGLEDEBITNOTE, j, jd,
                       AB_JobSingleDebitNote_FreeData);

  jd->maxPurposeLines=GWEN_DB_GetIntValue(db, "params/maxPurposeLines",0, -1);

  /* count text keys */
  for (i=0;;i++) {
    int k;

    k=GWEN_DB_GetIntValue(db, "params/textKeys",i, -1);
    if (k==-1)
      break;
  }
  /* read text keys */
  if (i) {
    int k;

    jd->textKeys=(int*)malloc((i+1)*(sizeof(int)));
    assert(jd->textKeys);
    for (k=0; k<i; k++)
      jd->textKeys[k]=GWEN_DB_GetIntValue(db, "params/textKeys",i, -1);
    jd->textKeys[k]=-1;
  }

  /* read arguments */
  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       "args/transaction");
  if (dbT)
    jd->transaction=AB_Transaction_fromDb(dbT);

  return j;
}


















