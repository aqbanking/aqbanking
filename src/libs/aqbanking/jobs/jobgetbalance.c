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
#include "job_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


AB_JOB *AB_JobGetBalance_new(AB_ACCOUNT *a){
  AB_JOB *j;

  j=AB_Job_new(AB_Job_TypeGetBalance, a);
  AB_Job_Update(j);
  return j;
}



AB_ACCOUNT_STATUS *AB_JobGetBalance_GetAccountStatus(AB_JOB *j){
  GWEN_DB_NODE *db;
  AB_ACCOUNT_STATUS *as;

  assert(j);
  db=AB_Job_GetData(j);
  assert(db);
  db=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "status");
  if (!db)
    return 0;
  as=AB_AccountStatus_FromDb(db);
  if (!as) {
    DBG_ERROR(0, "Bad data");
  }
  return as;
}



void AB_JobGetBalance_SetAccountStatus(AB_JOB *j,
                                       const AB_ACCOUNT_STATUS *as){
  GWEN_DB_NODE *db;

  assert(j);
  db=AB_Job_GetData(j);
  assert(db);
  db=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "status");
  assert(db);
  if (AB_AccountStatus_ToDb(as, db)) {
    DBG_ERROR(0, "Bad data");
  }
}



