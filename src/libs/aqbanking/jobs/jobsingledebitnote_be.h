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


#ifndef AQBANKING_JOBSINGLEDEBITNOTE_BE_H
#define AQBANKING_JOBSINGLEDEBITNOTE_BE_H


#include <aqbanking/job.h>
#include <aqbanking/jobsingledebitnote.h>



AQBANKING_API 
void AB_JobSingleDebitNote_SetMaxPurposeLines(AB_JOB *j, int i);

AQBANKING_API 
void AB_JobSingleDebitNote_SetTextKeys(AB_JOB *j, const int *tk);



#endif

