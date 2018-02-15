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
  GWEN_Date_free(aj->fromDate);
  GWEN_Date_free(aj->toDate);
  GWEN_FREE_OBJECT(aj);
}



const GWEN_DATE *AB_JobGetTransactions_GetFromDate(const AB_JOB *j){
  AB_JOB_GETTRANSACTIONS *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j);
  assert(aj);

  return aj->fromDate;
}



void AB_JobGetTransactions_SetFromDate(AB_JOB *j, const GWEN_DATE *dt){
  AB_JOB_GETTRANSACTIONS *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j);
  assert(aj);

  GWEN_Date_free(aj->fromDate);
  if (dt) aj->fromDate=GWEN_Date_dup(dt);
  else aj->fromDate=0;
}


const GWEN_DATE *AB_JobGetTransactions_GetToDate(const AB_JOB *j){
  AB_JOB_GETTRANSACTIONS *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j);
  assert(aj);

  return aj->toDate;
}



void AB_JobGetTransactions_SetToDate(AB_JOB *j, const GWEN_DATE *dt){
  AB_JOB_GETTRANSACTIONS *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j);
  assert(aj);

  GWEN_Date_free(aj->toDate);
  if (dt) aj->toDate=GWEN_Date_dup(dt);
  else aj->toDate=0;
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






