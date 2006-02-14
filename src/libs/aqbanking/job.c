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
#include "i18n_l.h"
#include "account_l.h"
#include "banking_l.h"
#include "provider_l.h"
#include "jobs/jobgetdatedtransfers_l.h"
#include "jobs/jobgettransactions_l.h"
#include "jobs/jobgetstandingorders_l.h"
#include "jobs/jobgetbalance_l.h"
#include "jobs/jobtransferbase_l.h"
#include "jobs/jobsingletransfer_l.h"
#include "jobs/jobsingledebitnote_l.h"
#include "jobs/jobeutransfer_l.h"
#include "jobs/jobcreatesto_l.h"
#include "jobs/jobmodifysto_l.h"
#include "jobs/jobdeletesto_l.h"
#include "jobs/jobcreatedatedtransfer_l.h"
#include "jobs/jobmodifydatedtransfer_l.h"
#include "jobs/jobdeletedatedtransfer_l.h"
#include "jobs/jobinternaltransfer_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>

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



AB_JOB *AB_Job_new_l(AB_JOB_TYPE jt, AB_ACCOUNT *a){
  AB_JOB *j;
  GWEN_BUFFER *lbuf;
  const char *bankCode;
  const char *accountNumber;

  j=AB_Job_new(jt, a);
  assert(j);

  bankCode=AB_Account_GetBankCode(a);
  if (!bankCode || !*bankCode)
    bankCode="[no bankcode]";
  accountNumber=AB_Account_GetAccountNumber(a);

  lbuf=GWEN_Buffer_new(0, 32, 0, 1);
  GWEN_Buffer_AppendString(lbuf, "Created job for account \"");
  GWEN_Buffer_AppendString(lbuf, accountNumber);
  GWEN_Buffer_AppendString(lbuf, "\" at \"");
  GWEN_Buffer_AppendString(lbuf, bankCode);
  GWEN_Buffer_AppendString(lbuf, "\"");

  AB_Job_Log(j, AB_Banking_LogLevelInfo, "aqbanking",
             GWEN_Buffer_GetStart(lbuf));
  GWEN_Buffer_free(lbuf);

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
      free(j->usedTan);
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
    GWEN_BUFFER *lbuf;

    GWEN_Time_free(j->lastStatusChange);
    lbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(lbuf, "Status changed from \"");
    GWEN_Buffer_AppendString(lbuf, AB_Job_Status2Char(j->status));
    GWEN_Buffer_AppendString(lbuf, "\" to \"");
    GWEN_Buffer_AppendString(lbuf, AB_Job_Status2Char(st));
    GWEN_Buffer_AppendString(lbuf, "\"");

    AB_Job_Log(j, AB_Banking_LogLevelInfo, "aqbanking",
	       GWEN_Buffer_GetStart(lbuf));
    GWEN_Buffer_free(lbuf);
    j->lastStatusChange=GWEN_CurrentTime();
    j->status=st;

  }
}



AB_JOB_TYPE AB_Job_GetType(const AB_JOB *j){
  assert(j);
  return j->jobType;
}



const char *AB_Job_GetUsedTan(const AB_JOB *j) {
  assert(j);
  return j->usedTan;
}



void AB_Job_SetUsedTan(AB_JOB *j, const char *s) {
  assert(j);
  free(j->usedTan);
  if (s) j->usedTan=strdup(s);
  else j->usedTan=0;
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
  case AB_Job_TypeGetBalance:          s="getbalance"; break;
  case AB_Job_TypeGetTransactions:     s="gettransactions"; break;
  case AB_Job_TypeTransfer:            s="transfer"; break;
  case AB_Job_TypeDebitNote:           s="debitnote"; break;
  case AB_Job_TypeEuTransfer:          s="eutransfer"; break;
  case AB_Job_TypeGetStandingOrders:   s="getstandingorders"; break;
  case AB_Job_TypeGetDatedTransfers:   s="getdatedtransfers"; break;
  case AB_Job_TypeCreateStandingOrder: s="createstandingorder"; break;
  case AB_Job_TypeModifyStandingOrder: s="modifystandingorder"; break;
  case AB_Job_TypeDeleteStandingOrder: s="deletestandingorder"; break;
  case AB_Job_TypeCreateDatedTransfer: s="createdatedtransfer"; break;
  case AB_Job_TypeModifyDatedTransfer: s="modifydatedtransfer"; break;
  case AB_Job_TypeDeleteDatedTransfer: s="deletedatedtransfer"; break;
  case AB_Job_TypeInternalTransfer:    s="internaltransfer"; break;
  default:
  case AB_Job_TypeUnknown:             s="unknown"; break;
  }

  return s;
}



const char *AB_Job_Type2LocalChar(AB_JOB_TYPE i) {
  const char *s;

  switch(i) {
  case AB_Job_TypeGetBalance:          s=I18N("Get Balance"); break;
  case AB_Job_TypeGetTransactions:     s=I18N("Get Transactions"); break;
  case AB_Job_TypeTransfer:            s=I18N("Transfer"); break;
  case AB_Job_TypeDebitNote:           s=I18N("Debit Note"); break;
  case AB_Job_TypeEuTransfer:          s=I18N("EU Transfer"); break;
  case AB_Job_TypeGetStandingOrders:   s=I18N("Get Standing Orders"); break;
  case AB_Job_TypeGetDatedTransfers:   s=I18N("Get Dated Transfers"); break;
  case AB_Job_TypeCreateStandingOrder: s=I18N("Create Standing Order"); break;
  case AB_Job_TypeModifyStandingOrder: s=I18N("Modify Standing Order"); break;
  case AB_Job_TypeDeleteStandingOrder: s=I18N("Delete Standing Order"); break;
  case AB_Job_TypeCreateDatedTransfer: s=I18N("Create Dated Transfer"); break;
  case AB_Job_TypeModifyDatedTransfer: s=I18N("Modify Dated Transfer"); break;
  case AB_Job_TypeDeleteDatedTransfer: s=I18N("Delete Dated Transfer"); break;
  case AB_Job_TypeInternalTransfer:    s=I18N("Internal Transfer"); break;
  default:
  case AB_Job_TypeUnknown:             s=I18N("unknown"); break;
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
  else if (strcasecmp(s, "getdatedtransfers")==0) i=AB_Job_TypeGetDatedTransfers;
  else if (strcasecmp(s, "createstandingorder")==0)
    i=AB_Job_TypeCreateStandingOrder;
  else if (strcasecmp(s, "modifystandingorder")==0)
    i=AB_Job_TypeModifyStandingOrder;
  else if (strcasecmp(s, "deletestandingorder")==0)
    i=AB_Job_TypeDeleteStandingOrder;
  else if (strcasecmp(s, "createdatedtransfer")==0)
    i=AB_Job_TypeCreateDatedTransfer;
  else if (strcasecmp(s, "modifydatedtransfer")==0)
    i=AB_Job_TypeModifyDatedTransfer;
  else if (strcasecmp(s, "deletedatedtransfer")==0)
    i=AB_Job_TypeDeleteDatedTransfer;
  else if (strcasecmp(s, "internaltransfer")==0)
    i=AB_Job_TypeInternalTransfer;

  else i=AB_Job_TypeUnknown;

  return i;
}



int AB_Job_toDb(const AB_JOB *j, GWEN_DB_NODE *db){
  const char *p;
  GWEN_DB_NODE *dbT;
  GWEN_DB_NODE *dbL;

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "jobId", j->jobId);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "idForProvider", j->idForProvider);
  p=AB_Job_Type2Char(j->jobType);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "jobType", p);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "createdBy", j->createdBy);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "usedTan", j->usedTan);

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

  case AB_Job_TypeGetDatedTransfers:
    if (AB_JobGetDatedTransfers_toDb(j, db)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    break;

  case AB_Job_TypeCreateStandingOrder:
    if (AB_JobCreateStandingOrder_toDb(j, db)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    break;

  case AB_Job_TypeModifyStandingOrder:
    if (AB_JobModifyStandingOrder_toDb(j, db)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    break;

  case AB_Job_TypeDeleteStandingOrder:
    if (AB_JobDeleteStandingOrder_toDb(j, db)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    break;

  case AB_Job_TypeCreateDatedTransfer:
    if (AB_JobCreateDatedTransfer_toDb(j, db)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    break;

  case AB_Job_TypeModifyDatedTransfer:
    if (AB_JobModifyDatedTransfer_toDb(j, db)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    break;

  case AB_Job_TypeDeleteDatedTransfer:
    if (AB_JobDeleteDatedTransfer_toDb(j, db)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    break;

  case AB_Job_TypeInternalTransfer:
    if (AB_JobInternalTransfer_toDb(j, db)) {
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

  dbL=GWEN_DB_GetGroup(j->dbData, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "logs");
  if (dbL)
    GWEN_DB_UnlinkGroup(dbL);
  GWEN_DB_AddGroupChildren(dbT, j->dbData);
  if (dbL) {
    if (j->status!=AB_Job_StatusFinished ||
        getenv("AQBANKING_STORE_JOBLOGS")) {
      const char *s;
      unsigned int i;
      GWEN_DB_NODE *dbN;

      dbN=GWEN_DB_GetGroup(dbT, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "logs");
      assert(dbN);
      for (i=0; ; i++) {
        s=GWEN_DB_GetCharValue(dbL, "log", i, 0);
        if (!s)
          break;
        GWEN_DB_SetCharValue(dbN,
                             GWEN_DB_FLAGS_DEFAULT |
                             GWEN_PATH_FLAGS_CREATE_VAR,
                             "log", s);
      }
    }
    GWEN_DB_AddGroup(j->dbData, dbL);
  }

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
    DBG_INFO(AQBANKING_LOGDOMAIN, "Account \"%08x\" not found, ignoring job", accountId);
    return 0;
  }

  p=GWEN_DB_GetCharValue(db, "jobType", 0, "unknown");
  jt=AB_Job_Char2Type(p);
  if (jt==AB_Job_TypeUnknown) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Unknown job type \"%s\", ignoring job", p);
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

  case AB_Job_TypeGetDatedTransfers:
    j=AB_JobGetDatedTransfers_fromDb(a, db);
    assert(j);
    break;

  case AB_Job_TypeCreateStandingOrder:
    j=AB_JobCreateStandingOrder_fromDb(a, db);
    assert(j);
    break;

  case AB_Job_TypeModifyStandingOrder:
    j=AB_JobModifyStandingOrder_fromDb(a, db);
    assert(j);
    break;

  case AB_Job_TypeDeleteStandingOrder:
    j=AB_JobDeleteStandingOrder_fromDb(a, db);
    assert(j);
    break;

  case AB_Job_TypeCreateDatedTransfer:
    j=AB_JobCreateDatedTransfer_fromDb(a, db);
    assert(j);
    break;

  case AB_Job_TypeModifyDatedTransfer:
    j=AB_JobModifyDatedTransfer_fromDb(a, db);
    assert(j);
    break;

  case AB_Job_TypeDeleteDatedTransfer:
    j=AB_JobDeleteDatedTransfer_fromDb(a, db);
    assert(j);
    break;

  case AB_Job_TypeInternalTransfer:
    j=AB_JobInternalTransfer_fromDb(a, db);
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
  p=GWEN_DB_GetCharValue(db, "usedTan", 0, 0);
  if (p)
    j->usedTan=strdup(p);

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



void AB_Job_Log(AB_JOB *j,
		AB_BANKING_LOGLEVEL ll,
                const char *who,
		const char *txt) {
  GWEN_DB_NODE *db;
  char buffer[32];
  GWEN_TIME *ti;
  GWEN_BUFFER *lbuf;

  assert(j);

  db=GWEN_DB_GetGroup(j->dbData, GWEN_DB_FLAGS_DEFAULT, "logs");
  assert(db);

  lbuf=GWEN_Buffer_new(0, 128, 0, 1);
  snprintf(buffer, sizeof(buffer), "%02d", ll);
  GWEN_Buffer_AppendString(lbuf, buffer);
  GWEN_Buffer_AppendByte(lbuf, ':');
  ti=GWEN_CurrentTime();
  assert(ti);
  GWEN_Time_toString(ti, "YYYYMMDD:hhmmss:", lbuf);
  GWEN_Time_free(ti);
  GWEN_Text_EscapeToBufferTolerant(who, lbuf);
  GWEN_Buffer_AppendByte(lbuf, ':');
  GWEN_Text_EscapeToBufferTolerant(txt, lbuf);

  GWEN_DB_SetCharValue(db,
                       GWEN_DB_FLAGS_DEFAULT,
                       "log", GWEN_Buffer_GetStart(lbuf));

  GWEN_Buffer_free(lbuf);
}



void AB_Job_LogRaw(AB_JOB *j, const char *txt) {
  GWEN_DB_NODE *db;

  assert(j);

  db=GWEN_DB_GetGroup(j->dbData, GWEN_DB_FLAGS_DEFAULT, "logs");
  assert(db);

  GWEN_DB_SetCharValue(db,
                       GWEN_DB_FLAGS_DEFAULT,
                       "log", txt);
}



GWEN_STRINGLIST *AB_Job_GetLogs(const AB_JOB *j) {
  const char *s;
  int i;
  GWEN_STRINGLIST *sl;
  GWEN_DB_NODE *db;

  assert(j);

  db=GWEN_DB_GetGroup(j->dbData, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "logs");
  if (!db)
    return 0;

  sl=GWEN_StringList_new();
  for (i=0; ; i++) {
    s=GWEN_DB_GetCharValue(db, "log", i, 0);
    if (!s)
      break;
    GWEN_StringList_AppendString(sl, s, 0, 0);
  }
  if (GWEN_StringList_Count(sl)==0) {
    GWEN_StringList_free(sl);
    return 0;
  }

  return sl;
}

