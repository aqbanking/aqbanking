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


#include "jobeutransfer.h"
#include "jobeutransfer_be.h"
#include "jobeutransfer_p.h"
#include "job_l.h"
#include "account_l.h"
#include "banking_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_INHERIT(AB_JOB, AB_JOBEUTRANSFER)



AB_JOB *AB_JobEuTransfer_new(AB_ACCOUNT *a){
  AB_JOB *j;
  AB_JOBEUTRANSFER *jd;

  j=AB_Job_new(AB_Job_TypeTransfer, a);
  GWEN_NEW_OBJECT(AB_JOBEUTRANSFER, jd);
  GWEN_INHERIT_SETDATA(AB_JOB, AB_JOBEUTRANSFER, j, jd,
                       AB_JobEuTransfer_FreeData);
  jd->countryInfoList=AB_EuTransferInfo_List_new();
  return j;
}



void AB_JobEuTransfer_FreeData(void *bp, void *p) {
  AB_JOBEUTRANSFER *jd;

  jd=(AB_JOBEUTRANSFER*)p;
  AB_Transaction_free(jd->transaction);
  AB_EuTransferInfo_List_free(jd->countryInfoList);
  GWEN_FREE_OBJECT(jd);
}



void AB_JobEuTransfer_SetCountryInfoList(AB_JOB *j,
                                         AB_EUTRANSFER_INFO_LIST *l){
  AB_JOBEUTRANSFER *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBEUTRANSFER, j);
  assert(jd);

  AB_EuTransferInfo_List_free(jd->countryInfoList);
  jd->countryInfoList=l;
}



const AB_EUTRANSFER_INFO *AB_JobEuTransfer_FindCountryInfo(const AB_JOB *j,
                                                           const char *cnt){
  AB_JOBEUTRANSFER *jd;
  AB_EUTRANSFER_INFO *ei;

  assert(cnt);
  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBEUTRANSFER, j);
  assert(jd);

  if (jd->countryInfoList==0)
    return 0;

  ei=AB_EuTransferInfo_List_First(jd->countryInfoList);
  while(ei) {
    const char *s;

    s=AB_EuTransferInfo_GetCountryCode(ei);
    if (s) {
      if (strcasecmp(s, cnt)==0)
        break;
    }
    ei=AB_EuTransferInfo_List_Next(ei);
  }

  if (!ei) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Country \"%s\" not found", cnt);
  }

  return ei;
}



int AB_JobEuTransfer_GetIbanAllowed(const AB_JOB *j){
  AB_JOBEUTRANSFER *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBEUTRANSFER, j);
  assert(jd);

  return jd->ibanAllowed;
}



void AB_JobEuTransfer_SetIbanAllowed(AB_JOB *j, int b){
  AB_JOBEUTRANSFER *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBEUTRANSFER, j);
  assert(jd);

  jd->ibanAllowed=b;
}



int AB_JobEuTransfer_SetTransaction(AB_JOB *j, const AB_TRANSACTION *t){
  AB_JOBEUTRANSFER *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBEUTRANSFER, j);
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



const AB_TRANSACTION *AB_JobEuTransfer_GetTransaction(const AB_JOB *j){
  AB_JOBEUTRANSFER *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBEUTRANSFER, j);
  assert(jd);

  return jd->transaction;
}



int AB_JobEuTransfer_toDb(const AB_JOB *j, GWEN_DB_NODE *db) {
  AB_JOBEUTRANSFER *jd;
  GWEN_DB_NODE *dbT;
  int rv;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBEUTRANSFER, j);
  assert(jd);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "params/ibanAllowed",
                      jd->ibanAllowed);

  /* store country info */
  dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                       "params/countryInfo");

  if (jd->countryInfoList) {
    AB_EUTRANSFER_INFO *ei;

    ei=AB_EuTransferInfo_List_First(jd->countryInfoList);
    while(ei) {
      GWEN_DB_NODE *dbEi;

      dbEi=GWEN_DB_Group_new("country");
      if (AB_EuTransferInfo_toDb(ei, dbEi)) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not store country info");
        GWEN_DB_Group_free(dbEi);
      }
      else {
        GWEN_DB_AddGroup(dbT, dbEi);
      }
      ei=AB_EuTransferInfo_List_Next(ei);
    } /* while */
  }

  /* store transaction */
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



AB_JOB *AB_JobEuTransfer_fromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db) {
  AB_JOB *j;
  AB_JOBEUTRANSFER *jd;
  GWEN_DB_NODE *dbT;

  j=AB_Job_new(AB_Job_TypeTransfer, a);
  GWEN_NEW_OBJECT(AB_JOBEUTRANSFER, jd);
  GWEN_INHERIT_SETDATA(AB_JOB, AB_JOBEUTRANSFER, j, jd,
                       AB_JobEuTransfer_FreeData);

  /* read params */
  if (jd->countryInfoList)
    AB_EuTransferInfo_List_Clear(jd->countryInfoList);
  else
    jd->countryInfoList=AB_EuTransferInfo_List_new();

  jd->ibanAllowed=GWEN_DB_GetIntValue(db, "params/ibanAllowed", 0, 0);

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       "args/countryInfo");
  if (dbT) {
    GWEN_DB_NODE *dbEi;

    dbEi=GWEN_DB_FindFirstGroup(dbT, "country");
    while(dbEi) {
      AB_EUTRANSFER_INFO *ei;

      ei=AB_EuTransferInfo_fromDb(dbEi);
      if (ei==0) {
        DBG_WARN(AQBANKING_LOGDOMAIN, "Bad country info in job");
      }
      else {
        AB_EuTransferInfo_List_Add(ei, jd->countryInfoList);
      }
      dbEi=GWEN_DB_FindNextGroup(dbEi, "country");
    }
  }

  /* read arguments */
  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       "args/transaction");
  if (dbT)
    jd->transaction=AB_Transaction_fromDb(dbT);

  return j;
}


















