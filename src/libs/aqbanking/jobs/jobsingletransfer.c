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


#include "jobsingletransfer.h"
#include "jobsingletransfer_be.h"
#include "jobsingletransfer_p.h"
#include "job_l.h"
#include "account_l.h"
#include "banking_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_INHERIT(AB_JOB, AB_JOBSINGLETRANSFER)



AB_JOB *AB_JobSingleTransfer_new(AB_ACCOUNT *a){
  AB_JOB *j;
  AB_JOBSINGLETRANSFER *jd;

  j=AB_Job_new(AB_Job_TypeTransfer, a);
  GWEN_NEW_OBJECT(AB_JOBSINGLETRANSFER, jd);
  GWEN_INHERIT_SETDATA(AB_JOB, AB_JOBSINGLETRANSFER, j, jd,
                       AB_JobSingleTransfer_FreeData);

  return j;
}



void AB_JobSingleTransfer_FreeData(void *bp, void *p) {
  AB_JOBSINGLETRANSFER *jd;

  jd=(AB_JOBSINGLETRANSFER*)p;
  AB_Transaction_free(jd->transaction);
  AB_TransactionLimits_free(jd->limits);
  free(jd->textKeys);
  GWEN_FREE_OBJECT(jd);
}



void AB_JobSingleTransfer_SetFieldLimits(AB_JOB *j,
                                         AB_TRANSACTION_LIMITS *limits){
  AB_JOBSINGLETRANSFER *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLETRANSFER, j);
  assert(jd);

  AB_TransactionLimits_free(jd->limits);
  if (limits) jd->limits=AB_TransactionLimits_dup(limits);
  else jd->limits=0;

  free(jd->textKeys);
  jd->textKeys=0;
}



const AB_TRANSACTION_LIMITS *AB_JobSingleTransfer_GetFieldLimits(AB_JOB *j) {
  AB_JOBSINGLETRANSFER *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLETRANSFER, j);
  assert(jd);

  return jd->limits;
}



int AB_JobSingleTransfer_SetTransaction(AB_JOB *j, const AB_TRANSACTION *t){
  AB_JOBSINGLETRANSFER *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLETRANSFER, j);
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



const AB_TRANSACTION *AB_JobSingleTransfer_GetTransaction(const AB_JOB *j){
  AB_JOBSINGLETRANSFER *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLETRANSFER, j);
  assert(jd);

  return jd->transaction;
}



int AB_JobSingleTransfer_GetMaxPurposeLines(const AB_JOB *j){
  AB_JOBSINGLETRANSFER *jd;

  DBG_ERROR(AQBANKING_LOGDOMAIN,
            "AB_JobSingleTransfer_GetMaxPurposeLines is deprecated");

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLETRANSFER, j);
  assert(jd);

  if (jd->limits) {
    int i;

    i=AB_TransactionLimits_GetMaxLinesPurpose(jd->limits);
    if (i==0)
      i=-1;
    return i;
  }
  return -1;
}



void AB_JobSingleTransfer_SetMaxPurposeLines(AB_JOB *j, int i){
  DBG_ERROR(AQBANKING_LOGDOMAIN,
            "AB_JobSingleTransfer_SetMaxPurposeLines is deprecated");
}



const int *AB_JobSingleTransfer_GetTextKeys(const AB_JOB *j){
  AB_JOBSINGLETRANSFER *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLETRANSFER, j);
  assert(jd);

  if (jd->textKeys==0) {
    const GWEN_STRINGLIST *sl;
    GWEN_STRINGLISTENTRY *se;
    int *p;

    if (jd->limits==0)
      return 0;
    sl=AB_TransactionLimits_GetValuesTextKey(jd->limits);
    if (sl==0)
      return 0;
    if (GWEN_StringList_Count(sl)==0)
      return 0;
    jd->textKeys=(int*)malloc(sizeof(int)*(GWEN_StringList_Count(sl)+1));
    assert(jd->textKeys);
    se=GWEN_StringList_FirstEntry(sl);
    assert(se);
    p=jd->textKeys;
    while(se) {
      const char *s;
      int i;

      s=GWEN_StringListEntry_Data(se);
      assert(s);
      if (1==sscanf(s, "%d", &i))
        *p=i;
      p++;
    } /* while se */
    *p=-1;
  }

  return jd->textKeys;
}



int AB_JobSingleTransfer_toDb(const AB_JOB *j, GWEN_DB_NODE *db) {
  AB_JOBSINGLETRANSFER *jd;
  GWEN_DB_NODE *dbT;
  int rv;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLETRANSFER, j);
  assert(jd);

  /* write params */
  if (jd->limits) {
    dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                         "params/limits");
    assert(dbT);
    rv=AB_TransactionLimits_toDb(jd->limits, dbT);
    if (rv) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return rv;
    }
  }

  /* write arguments */
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



AB_JOB *AB_JobSingleTransfer_fromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db) {
  AB_JOB *j;
  AB_JOBSINGLETRANSFER *jd;
  GWEN_DB_NODE *dbT;

  j=AB_Job_new(AB_Job_TypeTransfer, a);
  GWEN_NEW_OBJECT(AB_JOBSINGLETRANSFER, jd);
  GWEN_INHERIT_SETDATA(AB_JOB, AB_JOBSINGLETRANSFER, j, jd,
                       AB_JobSingleTransfer_FreeData);

  /* read params */
  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       "params/limits");
  if (dbT)
    jd->limits=AB_TransactionLimits_fromDb(dbT);

  /* read arguments */
  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       "args/transaction");
  if (dbT)
    jd->transaction=AB_Transaction_fromDb(dbT);

  return j;
}


















