/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004-2013 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "jobgettransactions_p.h"
#include "jobgettransactions_be.h"
#include "job_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT(AB_JOB, AB_JOB_GETTRANSACTIONS)



AB_JOB *AB_JobGetTransactions_new(AB_ACCOUNT *a) {
  AB_JOB *j;
  AB_JOB_GETTRANSACTIONS *aj;

  j=AB_Job_new(AB_Job_TypeGetTransactions, a);
  GWEN_NEW_OBJECT(AB_JOB_GETTRANSACTIONS, aj);
  GWEN_INHERIT_SETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j, aj,
                       AB_JobGetTransactions_FreeData);
  return j;
}



void GWENHYWFAR_CB AB_JobGetTransactions_FreeData(void *bp, void *p){
  AB_JOB_GETTRANSACTIONS *aj;

  aj=(AB_JOB_GETTRANSACTIONS*)p;
  GWEN_Time_free(aj->fromTime);
  GWEN_Time_free(aj->toTime);
  if (aj->transactions)
    AB_Transaction_List2_freeAll(aj->transactions);
  if (aj->accountStatusList)
    AB_AccountStatus_List2_freeAll(aj->accountStatusList);
  GWEN_FREE_OBJECT(aj);
}



AB_TRANSACTION_LIST2*
AB_JobGetTransactions_GetTransactions(const AB_JOB *j){
  AB_JOB_GETTRANSACTIONS *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j);
  assert(aj);

  if (aj->transactions) {
    if (AB_Transaction_List2_GetSize(aj->transactions)==0)
      return 0;
  }
  return aj->transactions;
}



void AB_JobGetTransactions_SetTransactions(AB_JOB *j,
                                           AB_TRANSACTION_LIST2 *tl){
  AB_JOB_GETTRANSACTIONS *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j);
  assert(aj);

  assert(tl);
  if (aj->transactions)
    AB_Transaction_List2_freeAll(aj->transactions);
  aj->transactions=tl;
}



AB_ACCOUNT_STATUS_LIST2*
AB_JobGetTransactions_GetAccountStatusList(const AB_JOB *j){
  AB_JOB_GETTRANSACTIONS *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j);
  assert(aj);

  if (aj->accountStatusList) {
    if (AB_AccountStatus_List2_GetSize(aj->accountStatusList)==0)
      return 0;
  }
  return aj->accountStatusList;
}



void AB_JobGetTransactions_SetAccountStatusList(AB_JOB *j,
                                                AB_ACCOUNT_STATUS_LIST2 *tl){
  AB_JOB_GETTRANSACTIONS *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j);
  assert(aj);

  assert(tl);
  if (aj->accountStatusList)
    AB_AccountStatus_List2_freeAll(aj->accountStatusList);
  aj->accountStatusList=tl;
}



const GWEN_TIME *AB_JobGetTransactions_GetFromTime(const AB_JOB *j){
  AB_JOB_GETTRANSACTIONS *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j);
  assert(aj);

  return aj->fromTime;
}



void AB_JobGetTransactions_SetFromTime(AB_JOB *j, const GWEN_TIME *t){
  AB_JOB_GETTRANSACTIONS *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j);
  assert(aj);

  GWEN_Time_free(aj->fromTime);
  if (t) aj->fromTime=GWEN_Time_dup(t);
  else aj->fromTime=0;
}


const GWEN_TIME *AB_JobGetTransactions_GetToTime(const AB_JOB *j){
  AB_JOB_GETTRANSACTIONS *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j);
  assert(aj);

  return aj->toTime;
}



void AB_JobGetTransactions_SetToTime(AB_JOB *j, const GWEN_TIME *t){
  AB_JOB_GETTRANSACTIONS *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j);
  assert(aj);

  GWEN_Time_free(aj->toTime);
  if (t) aj->toTime=GWEN_Time_dup(t);
  else aj->toTime=0;
}



int AB_JobGetTransactions_GetMaxStoreDays(const AB_JOB *j){
  AB_JOB_GETTRANSACTIONS *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j);
  assert(aj);

  return aj->maxStoreDays;
}



void AB_JobGetTransactions_SetMaxStoreDays(AB_JOB *j, int i){
  AB_JOB_GETTRANSACTIONS *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j);
  assert(aj);

  aj->maxStoreDays=i;
}






