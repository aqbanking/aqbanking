/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobtransferbase_l.h"
#include "jobsingledebitnote.h"
#include "jobsingledebitnote_be.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>




AB_JOB *AB_JobSingleDebitNote_new(AB_ACCOUNT *a){
  return AB_JobTransferBase_new(AB_Job_TypeDebitNote, a);
}



void AB_JobSingleDebitNote_SetFieldLimits(AB_JOB *j,
                                          AB_TRANSACTION_LIMITS *limits){
  AB_JobTransferBase_SetFieldLimits(j, limits);
}



const AB_TRANSACTION_LIMITS *AB_JobSingleDebitNote_GetFieldLimits(AB_JOB *j) {
  return AB_JobTransferBase_GetFieldLimits(j);
}



int AB_JobSingleDebitNote_SetTransaction(AB_JOB *j, const AB_TRANSACTION *t){
  return AB_JobTransferBase_SetTransaction(j, t);
}



AB_TRANSACTION *AB_JobSingleDebitNote_GetTransaction(const AB_JOB *j){
  return AB_JobTransferBase_GetTransaction(j);
}



