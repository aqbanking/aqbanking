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



#include "jobgetbalance.h"
#include "jobgetbalance_be.h"
#include "jobgetbalance_p.h"
#include "job_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_INHERIT(AB_JOB, AB_JOBGETBALANCE)



AB_JOB *AB_JobGetBalance_new(AB_ACCOUNT *a){
  AB_JOB *j;
  AB_JOBGETBALANCE *aj;

  j=AB_Job_new(AB_Job_TypeGetBalance, a);
  GWEN_NEW_OBJECT(AB_JOBGETBALANCE, aj);
  GWEN_INHERIT_SETDATA(AB_JOB, AB_JOBGETBALANCE, j, aj,
                       AB_JobGetBalance_FreeData);
  return j;
}



void AB_JobGetBalance_FreeData(void *bp, void *p) {
  AB_JOBGETBALANCE *aj;

  aj=(AB_JOBGETBALANCE*)p;
  AB_AccountStatus_free(aj->accountStatus);
  GWEN_FREE_OBJECT(aj);
}



AB_JOB *AB_JobGetBalance_fromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db){
  GWEN_DB_NODE *dbT;
  AB_JOB *j;
  AB_JOBGETBALANCE *aj;

  j=AB_Job_new(AB_Job_TypeGetBalance, a);
  GWEN_NEW_OBJECT(AB_JOBGETBALANCE, aj);
  GWEN_INHERIT_SETDATA(AB_JOB, AB_JOBGETBALANCE, j, aj,
                       AB_JobGetBalance_FreeData);

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       "result/accountstatus");
  if (dbT) {
    aj->accountStatus=AB_AccountStatus_fromDb(db);
  }
  return j;
}



int AB_JobGetBalance_toDb(const AB_JOB *j, GWEN_DB_NODE *db){
  AB_JOBGETBALANCE *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBGETBALANCE, j);
  assert(aj);

  if (aj->accountStatus) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                         "result/accountstatus");
    assert(dbT);
    if (AB_AccountStatus_toDb(aj->accountStatus, dbT)) {
      DBG_ERROR(0, "Error saving account status");
      return -1;
    }
  }
  return 0;
}



AB_ACCOUNT_STATUS *AB_JobGetBalance_GetAccountStatus(AB_JOB *j){
  AB_JOBGETBALANCE *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBGETBALANCE, j);
  assert(aj);

  return aj->accountStatus;
}



void AB_JobGetBalance_SetAccountStatus(AB_JOB *j,
                                       const AB_ACCOUNT_STATUS *as){
  AB_JOBGETBALANCE *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBGETBALANCE, j);
  assert(aj);

  AB_AccountStatus_free(aj->accountStatus);
  if (as) aj->accountStatus=AB_AccountStatus_dup(as);
  else aj->accountStatus=0;
}










