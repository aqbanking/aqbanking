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


#ifndef AQBANKING_JOBSINGLETRANSFER_BE_H
#define AQBANKING_JOBSINGLETRANSFER_BE_H


#include <aqbanking/job.h>
#include <aqbanking/jobsingletransfer.h>



AQBANKING_API 
void AB_JobSingleTransfer_SetFieldLimits(AB_JOB *j,
                                         AB_TRANSACTION_LIMITS *limits);



#endif

