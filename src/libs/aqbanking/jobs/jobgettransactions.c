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
#include "job_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT(AB_JOB, AB_JOB_GETTRANSACTIONS)



AB_JOB *AB_JobGetTransactions_new(AB_ACCOUNT *a,
                                  GWEN_TIME *fromTime,
                                  GWEN_TIME *toTime){
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
  GWEN_FREE_OBJECT(aj);
}




int AB_Job_GetTransactions_ReadDb(AB_JOB *j, GWEN_DB_NODE *db) {
  AB_JOB_GETTRANSACTIONS *aj;
  GWEN_DB_NODE *dbT;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j);
  assert(aj);

  GWEN_Time_free(aj->fromTime);
  aj->fromTime=0;
  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "fromdate");
  if (dbT)
    aj->fromTime=GWEN_Time_fromDb(dbT);

  GWEN_Time_free(aj->toTime);
  aj->toTime=0;
  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "todate");
  if (dbT)
    aj->toTime=GWEN_Time_fromDb(dbT);

  return 0;
}



int AB_Job_GetTransactions_WriteDb(const AB_JOB *j, GWEN_DB_NODE *db){
  AB_JOB_GETTRANSACTIONS *aj;
  GWEN_DB_NODE *dbT;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOB_GETTRANSACTIONS, j);
  assert(aj);

  if (aj->fromTime) {
    dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "fromdate");
    assert(dbT);
    if (GWEN_Time_toDb(aj->fromTime, dbT))
      return -1;
  }

  if (aj->toTime) {
    dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "todate");
    assert(dbT);
    if (GWEN_Time_toDb(aj->toTime, dbT))
      return -1;
  }

  return 0;
}






