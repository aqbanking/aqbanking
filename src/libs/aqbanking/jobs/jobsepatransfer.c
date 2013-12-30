/***************************************************************************
 begin       : Sun Sep 21 2008
 copyright   : (C) 2008-2013 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobsepatransfer.h"
#include "jobsepatransfer_be.h"
#include "job_l.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>




AB_JOB *AB_JobSepaTransfer_new(AB_ACCOUNT *a){
  return AB_Job_new(AB_Job_TypeSepaTransfer, a);
}



void AB_JobSepaTransfer_SetFieldLimits(AB_JOB *j,
					 AB_TRANSACTION_LIMITS *limits){
  AB_Job_SetFieldLimits(j, limits);
}



const AB_TRANSACTION_LIMITS *AB_JobSepaTransfer_GetFieldLimits(AB_JOB *j) {
  return AB_Job_GetFieldLimits(j);
}



int AB_JobSepaTransfer_SetTransaction(AB_JOB *j, const AB_TRANSACTION *t){
  return AB_Job_SetTransaction(j, t);
}



AB_TRANSACTION *AB_JobSepaTransfer_GetTransaction(const AB_JOB *j){
  return AB_Job_GetTransaction(j);
}



