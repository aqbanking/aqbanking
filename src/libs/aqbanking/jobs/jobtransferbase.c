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


#include "jobtransferbase_p.h"
#include "job_l.h"
#include "account_l.h"
#include "banking_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



AB_JOB *AB_JobTransferBase_new(AB_JOB_TYPE jt, AB_ACCOUNT *a){
  return AB_Job_new(jt, a);
}



void AB_JobTransferBase_SetFieldLimits(AB_JOB *j,
                                       AB_TRANSACTION_LIMITS *limits){
  AB_Job_SetFieldLimits(j, limits);
}



const AB_TRANSACTION_LIMITS *AB_JobTransferBase_GetFieldLimits(AB_JOB *j) {
  return AB_Job_GetFieldLimits(j);
}



int AB_JobTransferBase_SetTransaction(AB_JOB *j, const AB_TRANSACTION *t){
  return AB_Job_SetTransaction(j, t);
}



AB_TRANSACTION *AB_JobTransferBase_GetTransaction(const AB_JOB *j){
  return AB_JobTransferBase_GetTransaction(j);
}





