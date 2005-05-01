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
#include "jobs/jobgetstandingorders_l.h"
#include "jobs/jobgetbalance_l.h"
#include "jobs/jobsingletransfer_l.h"
#include "jobs/jobsingledebitnote_l.h"
#include "jobs/jobeutransfer_l.h"

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
  AB_Account_Attach(j->account);
  j->createdBy=strdup(AB_Banking_GetAppName(AB_Account_GetBanking(a)));
  j->dbData=GWEN_DB_Group_new("data");

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
      DBG_VERBOUS(AQBANKING_LOGDOMAIN, "Destroying AB_JOB");
      GWEN_INHERIT_FINI(AB_JOB, j);
      GWEN_LIST_FINI(AB_JOB, j);
      AB_Account_free(j->account);
      GWEN_DB_Group_free(j->dbData);
      free(j->resultText);
      free(j->createdBy);
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
  if (j->status!=st) {
    GWEN_Time_free(j->lastStatusChange);
    j->lastStatusChange=GWEN_CurrentTime();
    j->status=st;
  }
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
  case AB_Job_StatusDeferred: s="deferred"; break;
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
  else if (strcasecmp(s, "deferred")==0) i=AB_Job_StatusDeferred;
  else i=AB_Job_StatusUnknown;

  return i;
}



const char *AB_Job_Type2Char(AB_JOB_TYPE i) {
  const char *s;

  switch(i) {
  case AB_Job_TypeGetBalance:        s="getbalance"; break;
  case AB_Job_TypeGetTransactions:   s="gettransactions"; break;
  case AB_Job_TypeTransfer:          s="transfer"; break;
  case AB_Job_TypeDebitNote:         s="debitnote"; break;
  case AB_Job_TypeEuTransfer:        s="eutransfer"; break;
  case AB_Job_TypeGetStandingOrders: s="getstandingorders"; break;
  default:
  case AB_Job_TypeUnknown:           s="unknown"; break;
  }

  return s;
}



AB_JOB_TYPE AB_Job_Char2Type(const char *s) {
  AB_JOB_TYPE i;

  if (strcasecmp(s, "getbalance")==0) i=AB_Job_TypeGetBalance;
  else if (strcasecmp(s, "gettransactions")==0) i=AB_Job_TypeGetTransactions;
  else if (strcasecmp(s, "transfer")==0) i=AB_Job_TypeTransfer;
  else if (strcasecmp(s, "debitnote")==0) i=AB_Job_TypeDebitNote;
  else if (strcasecmp(s, "eutransfer")==0) i=AB_Job_TypeEuTransfer;
  else if (strcasecmp(s, "getstandingorders")==0) i=AB_Job_TypeGetStandingOrders;
  else i=AB_Job_TypeUnknown;

  return i;
}



int AB_Job_toDb(const AB_JOB *j, GWEN_DB_NODE *db){
  const char *p;
  GWEN_DB_NODE *dbT;

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
  if (j->resultText)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "resultText", j->resultText);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "accountId", AB_Account_GetUniqueId(j->account));

  if (j->lastStatusChange)
    AB_Job_DateToDb(j->lastStatusChange, db, "lastStatusChange");

  /* let every job store its data */
  switch(j->jobType) {
  case AB_Job_TypeGetBalance:
    if (AB_JobGetBalance_toDb(j, db)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    break;

  case AB_Job_TypeGetTransactions:
    if (AB_JobGetTransactions_toDb(j, db)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    break;

  case AB_Job_TypeTransfer:
    if (AB_JobSingleTransfer_toDb(j, db)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    break;

  case AB_Job_TypeDebitNote:
    if (AB_JobSingleDebitNote_toDb(j, db)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    break;

  case AB_Job_TypeEuTransfer:
    if (AB_JobEuTransfer_toDb(j, db)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    break;

  case AB_Job_TypeGetStandingOrders:
    if (AB_JobGetStandingOrders_toDb(j, db)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    break;

  default:
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unknown job type %d", j->jobType);
    return -1;
  }

  dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "data");
  assert(dbT);
  GWEN_DB_AddGroupChildren(dbT, j->dbData);

  return 0;
}



void AB_Job_DateToDb(const GWEN_TIME *ti, GWEN_DB_NODE *db, const char *name){
  if (ti) {
    GWEN_BUFFER *dbuf;
    int rv;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    rv=GWEN_Time_toUtcString(ti, "YYYYMMDD hh:mm:ss", dbuf);
    assert(rv==0);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         name, GWEN_Buffer_GetStart(dbuf));
    GWEN_Buffer_free(dbuf);
  }
}



GWEN_TIME *AB_Job_DateFromDb(GWEN_DB_NODE *db, const char *name) {
  const char *p;

  p=GWEN_DB_GetCharValue(db, name, 0, 0);
  if (p) {
    GWEN_TIME *ti;

    ti=GWEN_Time_fromUtcString(p, "YYYYMMDD hh:mm:ss");
    assert(ti);
    return ti;
  }
  else {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, name);
    if (dbT) {
      GWEN_TIME *ti;

      ti=GWEN_Time_fromDb(dbT);
      assert(ti);
      return ti;
    }
  }

  return 0;
}



void AB_Job_DateOnlyToDb(const GWEN_TIME *ti,
                         GWEN_DB_NODE *db,
                         const char *name){
  if (ti) {
    GWEN_BUFFER *tbuf;
    int rv;

    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
    rv=GWEN_Time_toUtcString(ti, "YYYYMMDD", tbuf);
    assert(rv==0);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         name, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
}



GWEN_TIME *AB_Job_DateOnlyFromDb(GWEN_DB_NODE *db, const char *name) {
  const char *p;

  p=GWEN_DB_GetCharValue(db, name, 0, 0);
  if (p) {
    GWEN_TIME *ti;
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(dbuf, p);
    GWEN_Buffer_AppendString(dbuf, "-12:00");

    ti=GWEN_Time_fromUtcString(GWEN_Buffer_GetStart(dbuf), "YYYYMMDD-hh:mm");
    assert(ti);
    GWEN_Buffer_free(dbuf);
    return ti;
  }
  else {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, name);
    if (dbT) {
      GWEN_TIME *ti;

      ti=GWEN_Time_fromDb(dbT);
      assert(ti);
      return ti;
    }
  }
  return 0;
}




AB_JOB *AB_Job_fromDb(AB_BANKING *ab, GWEN_DB_NODE *db){
  AB_JOB *j;
  AB_JOB_TYPE jt;
  AB_ACCOUNT *a;
  GWEN_TYPE_UINT32 accountId;
  GWEN_DB_NODE *dbT;
  const char *p;

  accountId=GWEN_DB_GetIntValue(db, "accountId", 0, 0);
  assert(accountId);
  a=AB_Banking_GetAccount(ab, accountId);
  if (!a) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Account \"%08x\" not found, ignoring job", accountId);
    return 0;
  }

  p=GWEN_DB_GetCharValue(db, "jobType", 0, "unknown");
  jt=AB_Job_Char2Type(p);
  if (jt==AB_Job_TypeUnknown) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unknown job type \"%s\", ignoring job", p);
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
    j=AB_JobSingleDebitNote_fromDb(a, db);
    assert(j);
    break;

  case AB_Job_TypeEuTransfer:
    j=AB_JobEuTransfer_fromDb(a, db);
    assert(j);
    break;

  case AB_Job_TypeGetStandingOrders:
    j=AB_JobGetStandingOrders_fromDb(a, db);
    assert(j);
    break;

  default:
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unknown job type %d", j->jobType);
    return 0;
  } /* switch */

  j->jobId=GWEN_DB_GetIntValue(db, "jobId", 0, 0);
  j->idForProvider=GWEN_DB_GetIntValue(db, "idForProvider", 0, 0);
  j->status=AB_Job_Char2Status(GWEN_DB_GetCharValue(db,
                                                    "jobStatus", 0,
						    "unknown"));
  p=GWEN_DB_GetCharValue(db, "resultText", 0, 0);
  if (p)
    j->resultText=strdup(p);
  j->lastStatusChange=AB_Job_DateFromDb(db, "lastStatusChange");

  p=GWEN_DB_GetCharValue(db, "createdBy", 0, 0);
  assert(p);
  free(j->createdBy);
  j->createdBy=strdup(p);

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
  if (dbT)
    GWEN_DB_AddGroupChildren(j->dbData, dbT);

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



GWEN_DB_NODE *AB_Job_GetAppData(AB_JOB *j){
  const char *s;
  GWEN_DB_NODE *db;

  assert(j);
  s=AB_Banking_GetEscapedAppName(AB_Account_GetBanking(AB_Job_GetAccount(j)));
  assert(s);
  db=GWEN_DB_GetGroup(j->dbData, GWEN_DB_FLAGS_DEFAULT, "apps");
  assert(db);
  return GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, s);
}



GWEN_DB_NODE *AB_Job_GetProviderData(AB_JOB *j, AB_PROVIDER *pro){
  const char *s;
  GWEN_DB_NODE *db;

  assert(j);
  assert(pro);
  s=AB_Provider_GetEscapedName(pro);

  db=GWEN_DB_GetGroup(j->dbData, GWEN_DB_FLAGS_DEFAULT, "backends");
  assert(db);
  return GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, s);
}



const GWEN_TIME *AB_Job_GetLastStatusChange(const AB_JOB *j){
  assert(j);
  return j->lastStatusChange;
}






