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


#ifndef AQBANKING_JOBEUTRANSFER_BE_H
#define AQBANKING_JOBEUTRANSFER_BE_H


#include <aqbanking/job.h>
#include <aqbanking/jobeutransfer.h>


/**
 * This function takes over the given list and all its members.
 */
AQBANKING_API 
void AB_JobEuTransfer_SetCountryInfoList(AB_JOB *j,
                                         AB_EUTRANSFER_INFO_LIST *l);

AQBANKING_API 
void AB_JobEuTransfer_SetIbanAllowed(AB_JOB *j, int b);


#endif

