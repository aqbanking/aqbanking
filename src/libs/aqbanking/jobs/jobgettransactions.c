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



void AB_JobGetTransactions_FreeData(void *bp, void *p){
  AB_JOB_GETTRANSACTIONS *aj;

  aj=(AB_JOB_GETTRANSACTIONS*)p;
  GWEN_Time_free(aj->fromTime);
  GWEN_Time_free(aj->toTime);
  if (aj->transactions)
    AB_Transaction_List2_freeAll(aj->transactions);
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




AB_JOB *AB_JobGetTransactions_fromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db){
  AB_JOB *j;
  AB_JOB_GETTRANSACTIONS *aj;
  GWEN_DB_NODE *dbT;

  j=AB_Job_new(AB_Job_TypeGetTransactions, a);
  GWEN_NEW_OBJECT(AB_JOB_GETTRANSACTIONS, aj);
  GWEN_INHERIT_SETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j, aj,
                       AB_JobGetTransactions_FreeData);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j);
  assert(aj);

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       "args/fromdate");
  if (dbT)
    aj->fromTime=GWEN_Time_fromDb(dbT);

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       "args/todate");
  if (dbT)
    aj->toTime=GWEN_Time_fromDb(dbT);

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       "result/transactions");
  if (dbT) {
    GWEN_DB_NODE *dbT2;

    aj->transactions=AB_Transaction_List2_new();

    /* read transactions */
    dbT2=GWEN_DB_FindFirstGroup(dbT, "transaction");
    while(dbT2) {
      AB_TRANSACTION *t;

      t=AB_Transaction_fromDb(dbT2);
      if (t)
        AB_Transaction_List2_PushBack(aj->transactions, t);
      dbT2=GWEN_DB_FindNextGroup(dbT2, "transaction");
    } /* while */
  } /* if transactions */

  return j;
}



int AB_JobGetTransactions_toDb(const AB_JOB *j, GWEN_DB_NODE *db){
  AB_JOB_GETTRANSACTIONS *aj;
  GWEN_DB_NODE *dbT;
  int errors;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j);
  assert(aj);

  errors=0;
  if (aj->fromTime) {
    dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                         "args/fromdate");
    assert(dbT);
    if (GWEN_Time_toDb(aj->fromTime, dbT))
      return -1;
  }

  if (aj->toTime) {
    dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                         "args/todate");
    assert(dbT);
    if (GWEN_Time_toDb(aj->toTime, dbT))
      return -1;
  }

  dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "result");
  assert(dbT);

  if (aj->transactions) {
    AB_TRANSACTION_LIST2_ITERATOR *it;
    GWEN_DB_NODE *dbT2;

    dbT2=GWEN_DB_GetGroup(dbT, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                          "transactions");
    assert(dbT2);
    it=AB_Transaction_List2_First(aj->transactions);
    if (it) {
      AB_TRANSACTION *t;

      t=AB_Transaction_List2Iterator_Data(it);
      assert(t);
      while(t) {
        GWEN_DB_NODE *dbT3;

        dbT3=GWEN_DB_GetGroup(dbT2, GWEN_PATH_FLAGS_CREATE_GROUP,
                              "transaction");
        assert(dbT3);
        if (AB_Transaction_toDb(t, dbT3)) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Error saving transaction");
          errors++;
        }
        t=AB_Transaction_List2Iterator_Next(it);
      } /* while */
      AB_Transaction_List2Iterator_free(it);
    } /* if it */
  } /* if transactions */


  return 0;
}









