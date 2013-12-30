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


#include "jobgetstandingorders_p.h"
#include "jobgetstandingorders_be.h"
#include "job_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT(AB_JOB, AB_JOB_GETSTANDINGORDERS)



AB_JOB *AB_JobGetStandingOrders_new(AB_ACCOUNT *a) {
  AB_JOB *j;
  AB_JOB_GETSTANDINGORDERS *aj;

  j=AB_Job_new(AB_Job_TypeGetStandingOrders, a);
  GWEN_NEW_OBJECT(AB_JOB_GETSTANDINGORDERS, aj);
  GWEN_INHERIT_SETDATA(AB_JOB, AB_JOB_GETSTANDINGORDERS, j, aj,
                       AB_JobGetStandingOrders_FreeData);
  return j;
}



void GWENHYWFAR_CB AB_JobGetStandingOrders_FreeData(void *bp, void *p){
  AB_JOB_GETSTANDINGORDERS *aj;

  aj=(AB_JOB_GETSTANDINGORDERS*)p;
  if (aj->standingOrders)
    AB_Transaction_List2_freeAll(aj->standingOrders);
  GWEN_FREE_OBJECT(aj);
}



AB_TRANSACTION_LIST2*
AB_JobGetStandingOrders_GetStandingOrders(const AB_JOB *j){
  AB_JOB_GETSTANDINGORDERS *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETSTANDINGORDERS, j);
  assert(aj);

  if (aj->standingOrders) {
    if (AB_Transaction_List2_GetSize(aj->standingOrders)==0)
      return 0;
  }
  return aj->standingOrders;
}



void AB_JobGetStandingOrders_SetStandingOrders(AB_JOB *j,
                                               AB_TRANSACTION_LIST2 *tl){
  AB_JOB_GETSTANDINGORDERS *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETSTANDINGORDERS, j);
  assert(aj);

  assert(tl);
  if (aj->standingOrders)
    AB_Transaction_List2_freeAll(aj->standingOrders);
  aj->standingOrders=tl;
}



AB_JOB *AB_JobGetStandingOrders_fromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db){
  AB_JOB *j;
  AB_JOB_GETSTANDINGORDERS *aj;
  GWEN_DB_NODE *dbT;

  j=AB_Job_new(AB_Job_TypeGetStandingOrders, a);
  GWEN_NEW_OBJECT(AB_JOB_GETSTANDINGORDERS, aj);
  GWEN_INHERIT_SETDATA(AB_JOB, AB_JOB_GETSTANDINGORDERS, j, aj,
                       AB_JobGetStandingOrders_FreeData);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETSTANDINGORDERS, j);
  assert(aj);

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       "result/standingOrders");
  if (dbT) {
    GWEN_DB_NODE *dbT2;

    aj->standingOrders=AB_Transaction_List2_new();

    /* read standingOrders */
    dbT2=GWEN_DB_FindFirstGroup(dbT, "standingOrder");
    while(dbT2) {
      AB_TRANSACTION *t;

      t=AB_Transaction_fromDb(dbT2);
      if (t)
        AB_Transaction_List2_PushBack(aj->standingOrders, t);
      dbT2=GWEN_DB_FindNextGroup(dbT2, "standingOrder");
    } /* while */
  } /* if standingOrders */

  return j;
}



int AB_JobGetStandingOrders_toDb(const AB_JOB *j, GWEN_DB_NODE *db){
  AB_JOB_GETSTANDINGORDERS *aj;
  GWEN_DB_NODE *dbT;
  int errors;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETSTANDINGORDERS, j);
  assert(aj);

  errors=0;
  dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "result");
  assert(dbT);

  if (aj->standingOrders) {
    AB_TRANSACTION_LIST2_ITERATOR *it;
    GWEN_DB_NODE *dbT2;

    dbT2=GWEN_DB_GetGroup(dbT, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                          "standingOrders");
    assert(dbT2);
    it=AB_Transaction_List2_First(aj->standingOrders);
    if (it) {
      AB_TRANSACTION *t;

      t=AB_Transaction_List2Iterator_Data(it);
      assert(t);
      while(t) {
        GWEN_DB_NODE *dbT3;

        dbT3=GWEN_DB_GetGroup(dbT2, GWEN_PATH_FLAGS_CREATE_GROUP,
                              "standingOrder");
        assert(dbT3);
        if (AB_Transaction_toDb(t, dbT3)) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Error saving standing order");
          errors++;
        }
        t=AB_Transaction_List2Iterator_Next(it);
      } /* while */
      AB_Transaction_List2Iterator_free(it);
    } /* if it */
  } /* if standingOrders */


  return 0;
}









