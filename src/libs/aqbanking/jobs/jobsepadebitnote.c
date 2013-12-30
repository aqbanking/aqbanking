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


#include "jobsepadebitnote.h"
#include "jobsepadebitnote_be.h"
#include "job_l.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>




AB_JOB *AB_JobSepaDebitNote_new(AB_ACCOUNT *a){
  return AB_Job_new(AB_Job_TypeSepaDebitNote, a);
}



void AB_JobSepaDebitNote_SetFieldLimits(AB_JOB *j,
					 AB_TRANSACTION_LIMITS *limits){
  AB_Job_SetFieldLimits(j, limits);
}



const AB_TRANSACTION_LIMITS *AB_JobSepaDebitNote_GetFieldLimits(AB_JOB *j) {
  return AB_Job_GetFieldLimits(j);
}



int AB_JobSepaDebitNote_SetTransaction(AB_JOB *j, const AB_TRANSACTION *t){
  return AB_Job_SetTransaction(j, t);
}



AB_TRANSACTION *AB_JobSepaDebitNote_GetTransaction(const AB_JOB *j){
  return AB_Job_GetTransaction(j);
}



