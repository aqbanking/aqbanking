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


GWEN_LIST_FUNCTIONS(AB_JOB, AB_Job);
GWEN_LIST2_FUNCTIONS(AB_JOB, AB_Job);



AB_JOB *AB_Job_new(AB_JOB_TYPE jt, AB_ACCOUNT *a){
  AB_JOB *j;
  AB_PROVIDER *pro;

  assert(a);
  GWEN_NEW_OBJECT(AB_JOB, j);
  j->usage=1;
  GWEN_LIST_INIT(AB_JOB, j);
  j->jobType=jt;
  j->data=GWEN_DB_Group_new("JobData");
  j->account=a;

  /* check whether job is available */
  pro=AB_Account_GetProvider(a);
  assert(pro);
  j->availability=AB_Provider_UpdateJob(pro, j);

  return j;
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
      GWEN_DB_Group_free(j->data);
      free(j->resultText);
      GWEN_LIST_FINI(AB_JOB, j);
      GWEN_FREE_OBJECT(j);
    }
  }
}



GWEN_DB_NODE *AB_Job_GetData(const AB_JOB *j){
  GWEN_DB_NODE *dbT;

  assert(j);
  dbT=GWEN_DB_GetGroup(j->data, GWEN_DB_FLAGS_DEFAULT, "static/ext");
  assert(dbT);
  return dbT;
}



int AB_Job_CheckAvailability(AB_JOB *j){
  assert(j);
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
  case AB_Job_StatusEnqueued: s="enqueued"; break;
  case AB_Job_StatusSent:     s="sent"; break;
  case AB_Job_StatusAnswered: s="answered"; break;
  case AB_Job_StatusError:    s="error"; break;
  default:                    s="unknown"; break;
  }
  return s;
}



AB_JOB_STATUS AB_Job_Char2Status(const char *s) {
  AB_JOB_STATUS i;

  if (strcasecmp(s, "new")==0) i=AB_Job_StatusNew;
  else if (strcasecmp(s, "enqueued")==0) i=AB_Job_StatusEnqueued;
  else if (strcasecmp(s, "sent")==0) i=AB_Job_StatusSent;
  else if (strcasecmp(s, "answered")==0) i=AB_Job_StatusAnswered;
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
  GWEN_DB_NODE *dbTsrc;
  const char *p;

  p=AB_Job_Type2Char(j->jobType);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "jobType", p);

  p=AB_Job_Status2Char(j->jobType);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "jobStatus", p);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "accountId", AB_Account_GetUniqueId(j->account));
  dbTsrc=GWEN_DB_GetGroup(j->data, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "static");
  if (dbTsrc) {
    GWEN_DB_NODE *dbTdst;

    dbTdst=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "data");
    assert(dbTdst);
    GWEN_DB_AddGroupChildren(dbTdst, dbTsrc);
  }

  return 0;
}



AB_JOB *AB_Job_fromDb(AB_BANKING *ab, GWEN_DB_NODE *db){
  AB_JOB *j;
  AB_PROVIDER *pro;
  AB_JOB_TYPE jt;
  AB_ACCOUNT *a;
  GWEN_TYPE_UINT32 accountId;
  const char *p;
  GWEN_DB_NODE *dbTsrc;

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

  GWEN_NEW_OBJECT(AB_JOB, j);
  j->jobType=jt;
  j->status=AB_Job_Char2Status(GWEN_DB_GetCharValue(db,
                                                    "jobStatus", 0,
                                                    "unknown"));
  j->account=a;

  /* read job data */
  j->data=GWEN_DB_Group_new("JobData");
  dbTsrc=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
  if (dbTsrc) {
    GWEN_DB_NODE *dbTdst;

    dbTdst=GWEN_DB_GetGroup(j->data, GWEN_DB_FLAGS_DEFAULT, "static");
    assert(dbTdst);
    GWEN_DB_AddGroupChildren(dbTdst, dbTsrc);
  }

  /* check whether job is available */
  pro=AB_Account_GetProvider(a);
  assert(pro);
  j->availability=AB_Provider_UpdateJob(pro, j);

  return j;
}



GWEN_TYPE_UINT32 AB_Job_IdForProvider(const AB_JOB *j) {
  assert(j);
  return GWEN_DB_GetIntValue(j->data, "static/idForProvider", 0, 0);
}



void AB_Job_SetIdForProvider(AB_JOB *j, GWEN_TYPE_UINT32 i) {
  assert(j);
  GWEN_DB_SetIntValue(j->data, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "static/idForProvider", i);
}



AB_ACCOUNT *AB_Job_GetAccount(const AB_JOB *j){
  assert(j);
  return j->account;
}




AB_JOB *AB_Job__freeAll_cb(AB_JOB *j) {
  AB_Job_free(j);
  return 0;
}



void AB_Job_List2_FreeAll(AB_JOB_LIST2 *jl){
  AB_Job_List2_ForEach(jl, AB_Job__freeAll_cb);
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













