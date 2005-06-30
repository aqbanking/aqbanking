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


#include "jobtransferbase_l.h"
#include "jobdeletesto.h"
#include "jobdeletesto_be.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>




AB_JOB *AB_JobDeleteStandingOrder_new(AB_ACCOUNT *a){
  return AB_JobTransferBase_new(AB_Job_TypeDeleteStandingOrder, a);
}



void AB_JobDeleteStandingOrder_SetFieldLimits(AB_JOB *j,
					 AB_TRANSACTION_LIMITS *limits){
  AB_JobTransferBase_SetFieldLimits(j, limits);
}



const AB_TRANSACTION_LIMITS *AB_JobDeleteStandingOrder_GetFieldLimits(AB_JOB *j) {
  return AB_JobTransferBase_GetFieldLimits(j);
}



int AB_JobDeleteStandingOrder_SetTransaction(AB_JOB *j, const AB_TRANSACTION *t){
  return AB_JobTransferBase_SetTransaction(j, t);
}



const AB_TRANSACTION *AB_JobDeleteStandingOrder_GetTransaction(const AB_JOB *j){
  return AB_JobTransferBase_GetTransaction(j);
}



int AB_JobDeleteStandingOrder_toDb(const AB_JOB *j, GWEN_DB_NODE *db) {
  return AB_JobTransferBase_toDb(j, db);
}



AB_JOB *AB_JobDeleteStandingOrder_fromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db) {
  return AB_JobTransferBase_fromDb(a, db);
}




