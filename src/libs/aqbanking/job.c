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
#include "job_be.h"
#include "account_l.h"
#include "banking_l.h"
#include "provider_l.h"
#include "jobs/jobgettransactions_l.h"
#include "jobs/jobgetbalance_l.h"
#include "jobs/jobsingletransfer_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_LIST_FUNCTIONS(AB_JOB, AB_Job)
GWEN_LIST2_FUNCTIONS(AB_JOB, AB_Job)
GWEN_INHERIT_FUNCTIONS(AB_JOB)



AB_JOB *AB_Job_new(AB_JOB_TYPE jt, AB_ACCOUNT *a){
  AB_JOB *j;

  assert(a);
  GWEN_NEW_OBJECT(AB_JOB, j);
  j->usage=1;
  GWEN_INHERIT_INIT(AB_JOB, j);
  GWEN_LIST_INIT(AB_JOB, j);
  j->jobType=jt;
  j->account=a;
  j->createdBy=strdup(AB_Banking_GetAppName(AB_Account_GetBanking(a)));

  return j;
}



int AB_Job_Update(AB_JOB *j){
  AB_PROVIDER *pro;

  assert(j);

  /* check whether the job is available */
  pro=AB_Account_GetProvider(j->account);
  assert(pro);
  j->availability=AB_Provider_UpdateJob(pro, j);
  return j->availability;
}



void AB_Job_Attach(AB_JOB *j){
  assert(j);
  assert(j->usage);
  j->usage++;
}



void AB_Job_free(AB_JOB *j){
  if (j) {
    assert(j->usage);
    if (--(j->usage)==0) {
      GWEN_INHERIT_FINI(AB_JOB, j);
      free(j->resultText);
      free(j->createdBy);
      GWEN_LIST_FINI(AB_JOB, j);
      GWEN_FREE_OBJECT(j);
    }
  }
}



int AB_Job_CheckAvailability(AB_JOB *j){
  assert(j);
  AB_Job_Update(j);
  return j->availability;
}



AB_JOB_STATUS AB_Job_GetStatus(const AB_JOB *j){
  assert(j);
  return j->status;
}



void  AB_Job_SetStatus(AB_JOB *j, AB_JOB_STATUS st){
  assert(j);
  j->status=st;
}



AB_JOB_TYPE AB_Job_GetType(const AB_JOB *j){
  assert(j);
  return j->jobType;
}



const char *AB_Job_Status2Char(AB_JOB_STATUS i) {
  const char *s;

  switch(i) {
  case AB_Job_StatusNew:      s="new"; break;
  case AB_Job_StatusUpdated:  s="updated"; break;
  case AB_Job_StatusEnqueued: s="enqueued"; break;
  case AB_Job_StatusSent:     s="sent"; break;
  case AB_Job_StatusPending:  s="pending"; break;
  case AB_Job_StatusFinished: s="finished"; break;
  case AB_Job_StatusError:    s="error"; break;
  default:                    s="unknown"; break;
  }
  return s;
}



AB_JOB_STATUS AB_Job_Char2Status(const char *s) {
  AB_JOB_STATUS i;

  if (strcasecmp(s, "new")==0) i=AB_Job_StatusNew;
  else if (strcasecmp(s, "updated")==0) i=AB_Job_StatusUpdated;
  else if (strcasecmp(s, "enqueued")==0) i=AB_Job_StatusEnqueued;
  else if (strcasecmp(s, "sent")==0) i=AB_Job_StatusSent;
  else if (strcasecmp(s, "pending")==0) i=AB_Job_StatusPending;
  else if (strcasecmp(s, "finished")==0) i=AB_Job_StatusFinished;
  else if (strcasecmp(s, "error")==0) i=AB_Job_StatusError;
  else i=AB_Job_StatusUnknown;

  return i;
}



const char *AB_Job_Type2Char(AB_JOB_TYPE i) {
  const char *s;

  switch(i) {
  case AB_Job_TypeGetBalance:      s="getbalance"; break;
  case AB_Job_TypeGetTransactions: s="gettransactions"; break;
  case AB_Job_TypeTransfer:        s="transfer"; break;
  case AB_Job_TypeDebitNote:       s="debitnote"; break;
  default:
  case AB_Job_TypeUnknown:         s="unknown"; break;
  }

  return s;
}



AB_JOB_TYPE AB_Job_Char2Type(const char *s) {
  AB_JOB_TYPE i;

  if (strcasecmp(s, "getbalance")==0) i=AB_Job_TypeGetBalance;
  else if (strcasecmp(s, "gettransactions")==0) i=AB_Job_TypeGetTransactions;
  else if (strcasecmp(s, "transfer")==0) i=AB_Job_TypeTransfer;
  else if (strcasecmp(s, "debitnote")==0) i=AB_Job_TypeDebitNote;
  else i=AB_Job_TypeUnknown;

  return i;
}



int AB_Job_toDb(const AB_JOB *j, GWEN_DB_NODE *db){
  const char *p;

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "jobId", j->jobId);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "idForProvider", j->idForProvider);
  p=AB_Job_Type2Char(j->jobType);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "jobType", p);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "createdBy", j->createdBy);

  p=AB_Job_Status2Char(j->status);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "jobStatus", p);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "accountId", AB_Account_GetUniqueId(j->account));

  /* let every job store its data */
  switch(j->jobType) {
  case AB_Job_TypeGetBalance:
    if (AB_JobGetBalance_toDb(j, db)) {
      DBG_INFO(0, "here");
      return -1;
    }
    break;

  case AB_Job_TypeGetTransactions:
    if (AB_JobGetTransactions_toDb(j, db)) {
      DBG_INFO(0, "here");
      return -1;
    }
    break;

  case AB_Job_TypeTransfer:
    if (AB_JobSingleTransfer_toDb(j, db)) {
      DBG_INFO(0, "here");
      return -1;
    }
    break;

  case AB_Job_TypeDebitNote:
    DBG_ERROR(0, "Job type not yet supported");
    return -1;

  default:
    DBG_ERROR(0, "Unknown job type %d", j->jobType);
    return -1;
  }

  return 0;
}



AB_JOB *AB_Job_fromDb(AB_BANKING *ab, GWEN_DB_NODE *db){
  AB_JOB *j;
  AB_JOB_TYPE jt;
  AB_ACCOUNT *a;
  GWEN_TYPE_UINT32 accountId;
  const char *p;

  accountId=GWEN_DB_GetIntValue(db, "accountId", 0, 0);
  assert(accountId);
  a=AB_Banking_GetAccount(ab, accountId);
  if (!a) {
    DBG_ERROR(0, "Account \"%08x\" not found, ignoring job", accountId);
    return 0;
  }

  p=GWEN_DB_GetCharValue(db, "jobType", 0, "unknown");
  jt=AB_Job_Char2Type(p);
  if (jt==AB_Job_TypeUnknown) {
    DBG_ERROR(0, "Unknown job type \"%s\", ignoring job", p);
    return 0;
  }

  /* read job */
  j=0;
  switch(jt) {
  case AB_Job_TypeGetBalance:
    j=AB_JobGetBalance_fromDb(a, db);
    assert(j);
    break;

  case AB_Job_TypeGetTransactions:
    j=AB_JobGetTransactions_fromDb(a, db);
    assert(j);
    break;

  case AB_Job_TypeTransfer:
    j=AB_JobSingleTransfer_fromDb(a, db);
    assert(j);
    break;

  case AB_Job_TypeDebitNote:
    DBG_ERROR(0, "Unsupported job type %d", j->jobType);
    return 0;
  default:
    DBG_ERROR(0, "Unknown job type %d", j->jobType);
    return 0;
  } /* switch */

  j->jobId=GWEN_DB_GetIntValue(db, "jobId", 0, 0);
  j->idForProvider=GWEN_DB_GetIntValue(db, "idForProvider", 0, 0);
  j->status=AB_Job_Char2Status(GWEN_DB_GetCharValue(db,
                                                    "jobStatus", 0,
                                                    "unknown"));
  p=GWEN_DB_GetCharValue(db, "createdBy", 0, 0);
  assert(p);
  j->createdBy=strdup(p);

  return j;
}



GWEN_TYPE_UINT32 AB_Job_GetIdForProvider(const AB_JOB *j) {
  assert(j);
  return j->idForProvider;
}



void AB_Job_SetIdForProvider(AB_JOB *j, GWEN_TYPE_UINT32 i) {
  assert(j);
  j->idForProvider=i;
}



AB_ACCOUNT *AB_Job_GetAccount(const AB_JOB *j){
  assert(j);
  return j->account;
}




AB_JOB *AB_Job__freeAll_cb(AB_JOB *j, void *userData) {
  AB_Job_free(j);
  return 0;
}



void AB_Job_List2_FreeAll(AB_JOB_LIST2 *jl){
  AB_Job_List2_ForEach(jl, AB_Job__freeAll_cb, 0);
  AB_Job_List2_free(jl);
}



const char *AB_Job_GetResultText(const AB_JOB *j){
  assert(j);
  return j->resultText;
}



void AB_Job_SetResultText(AB_JOB *j, const char *s){
  assert(j);
  free(j->resultText);
  if (s) j->resultText=strdup(s);
  else j->resultText=0;
}



GWEN_TYPE_UINT32 AB_Job_GetJobId(const AB_JOB *j) {
  assert(j);
  return j->jobId;
}



const char *AB_Job_GetCreatedBy(const AB_JOB *j) {
  assert(j);
  return j->createdBy;
}



void AB_Job_SetUniqueId(AB_JOB *j, GWEN_TYPE_UINT32 jid){
  assert(j);
  j->jobId=jid;
}






