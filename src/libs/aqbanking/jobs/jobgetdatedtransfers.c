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


#include "jobgetdatedtransfers_p.h"
#include "jobgetdatedtransfers_be.h"
#include "job_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT(AB_JOB, AB_JOB_GETDATEDTRANSFERS)



AB_JOB *AB_JobGetDatedTransfers_new(AB_ACCOUNT *a) {
  AB_JOB *j;
  AB_JOB_GETDATEDTRANSFERS *aj;

  j=AB_Job_new(AB_Job_TypeGetDatedTransfers, a);
  GWEN_NEW_OBJECT(AB_JOB_GETDATEDTRANSFERS, aj);
  GWEN_INHERIT_SETDATA(AB_JOB, AB_JOB_GETDATEDTRANSFERS, j, aj,
                       AB_JobGetDatedTransfers_FreeData);
  return j;
}



void GWENHYWFAR_CB AB_JobGetDatedTransfers_FreeData(void *bp, void *p){
  AB_JOB_GETDATEDTRANSFERS *aj;

  aj=(AB_JOB_GETDATEDTRANSFERS*)p;
  if (aj->datedTransfers)
    AB_Transaction_List2_freeAll(aj->datedTransfers);
  GWEN_FREE_OBJECT(aj);
}



AB_TRANSACTION_LIST2*
AB_JobGetDatedTransfers_GetDatedTransfers(const AB_JOB *j){
  AB_JOB_GETDATEDTRANSFERS *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETDATEDTRANSFERS, j);
  assert(aj);

  if (aj->datedTransfers) {
    if (AB_Transaction_List2_GetSize(aj->datedTransfers)==0)
      return 0;
  }
  return aj->datedTransfers;
}



void AB_JobGetDatedTransfers_SetDatedTransfers(AB_JOB *j,
                                               AB_TRANSACTION_LIST2 *tl){
  AB_JOB_GETDATEDTRANSFERS *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETDATEDTRANSFERS, j);
  assert(aj);

  assert(tl);
  if (aj->datedTransfers)
    AB_Transaction_List2_freeAll(aj->datedTransfers);
  aj->datedTransfers=tl;
}



AB_JOB *AB_JobGetDatedTransfers_fromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db){
  AB_JOB *j;
  AB_JOB_GETDATEDTRANSFERS *aj;
  GWEN_DB_NODE *dbT;

  j=AB_Job_new(AB_Job_TypeGetDatedTransfers, a);
  GWEN_NEW_OBJECT(AB_JOB_GETDATEDTRANSFERS, aj);
  GWEN_INHERIT_SETDATA(AB_JOB, AB_JOB_GETDATEDTRANSFERS, j, aj,
                       AB_JobGetDatedTransfers_FreeData);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETDATEDTRANSFERS, j);
  assert(aj);

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       "result/datedTransfers");
  if (dbT) {
    GWEN_DB_NODE *dbT2;

    aj->datedTransfers=AB_Transaction_List2_new();

    /* read datedTransfers */
    dbT2=GWEN_DB_FindFirstGroup(dbT, "datedTransfer");
    while(dbT2) {
      AB_TRANSACTION *t;

      t=AB_Transaction_fromDb(dbT2);
      if (t)
        AB_Transaction_List2_PushBack(aj->datedTransfers, t);
      dbT2=GWEN_DB_FindNextGroup(dbT2, "datedTransfer");
    } /* while */
  } /* if datedTransfers */

  return j;
}



int AB_JobGetDatedTransfers_toDb(const AB_JOB *j, GWEN_DB_NODE *db){
  AB_JOB_GETDATEDTRANSFERS *aj;
  GWEN_DB_NODE *dbT;
  int errors;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETDATEDTRANSFERS, j);
  assert(aj);

  errors=0;
  dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "result");
  assert(dbT);

  if (aj->datedTransfers) {
    AB_TRANSACTION_LIST2_ITERATOR *it;
    GWEN_DB_NODE *dbT2;

    dbT2=GWEN_DB_GetGroup(dbT, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                          "datedTransfers");
    assert(dbT2);
    it=AB_Transaction_List2_First(aj->datedTransfers);
    if (it) {
      AB_TRANSACTION *t;

      t=AB_Transaction_List2Iterator_Data(it);
      assert(t);
      while(t) {
        GWEN_DB_NODE *dbT3;

        dbT3=GWEN_DB_GetGroup(dbT2, GWEN_PATH_FLAGS_CREATE_GROUP,
                              "datedTransfer");
        assert(dbT3);
        if (AB_Transaction_toDb(t, dbT3)) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Error saving dated transfer");
          errors++;
        }
        t=AB_Transaction_List2Iterator_Next(it);
      } /* while */
      AB_Transaction_List2Iterator_free(it);
    } /* if it */
  } /* if datedTransfers */


  return 0;
}









