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

  jd->maxPurposeLines=-1;
  return j;
}



void AB_JobSingleTransfer_FreeData(void *bp, void *p) {
  AB_JOBSINGLETRANSFER *jd;

  jd=(AB_JOBSINGLETRANSFER*)p;
  AB_Transaction_free(jd->transaction);
  free(jd->textKeys);
  GWEN_FREE_OBJECT(jd);
}



int AB_JobSingleTransfer_SetTransaction(AB_JOB *j, const AB_TRANSACTION *t){
  AB_JOBSINGLETRANSFER *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLETRANSFER, j);
  assert(jd);

  /* TODO: check transaction */

  AB_Transaction_free(jd->transaction);
  if (t) jd->transaction=AB_Transaction_dup(t);
  else jd->transaction=0;

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

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLETRANSFER, j);
  assert(jd);

  return jd->maxPurposeLines;
}



void AB_JobSingleTransfer_SetMaxPurposeLines(AB_JOB *j, int i){
  AB_JOBSINGLETRANSFER *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLETRANSFER, j);
  assert(jd);

  jd->maxPurposeLines=i;
}



const int *AB_JobSingleTransfer_GetTextKeys(const AB_JOB *j){
  AB_JOBSINGLETRANSFER *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLETRANSFER, j);
  assert(jd);

  return jd->textKeys;
}



void AB_JobSingleTransfer_SetTextKeys(AB_JOB *j, const int *tk){
  int i;
  AB_JOBSINGLETRANSFER *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLETRANSFER, j);
  assert(jd);

  free(jd->textKeys);
  jd->textKeys=0;

  if (tk) {
    for (i=0; ; i++)
      if (*tk==-1)
        break;

    jd->textKeys=(int*)malloc(i*(sizeof(int)));
    assert(jd->textKeys);
    memmove(jd->textKeys, tk, i*(sizeof(int)));
  }
}



int AB_JobSingleTransfer_toDb(const AB_JOB *j, GWEN_DB_NODE *db) {
  AB_JOBSINGLETRANSFER *jd;
  GWEN_DB_NODE *dbT;
  int rv;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBSINGLETRANSFER, j);
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
      DBG_INFO(0, "here");
      return rv;
    }
  }

  return 0;
}



AB_JOB *AB_JobSingleTransfer_fromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db) {
  AB_JOB *j;
  AB_JOBSINGLETRANSFER *jd;
  int i;
  GWEN_DB_NODE *dbT;

  j=AB_Job_new(AB_Job_TypeTransfer, a);
  GWEN_NEW_OBJECT(AB_JOBSINGLETRANSFER, jd);
  GWEN_INHERIT_SETDATA(AB_JOB, AB_JOBSINGLETRANSFER, j, jd,
                       AB_JobSingleTransfer_FreeData);

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


















