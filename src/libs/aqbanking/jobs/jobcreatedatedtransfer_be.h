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


#ifndef AQBANKING_JOBCREATEDATEDTRANSFER_BE_H
#define AQBANKING_JOBCREATEDATEDTRANSFER_BE_H


#include <aqbanking/jobcreatedatedtransfer.h>


AQBANKING_API
void AB_JobCreateDatedTransfer_SetFieldLimits(AB_JOB *j,
                                              AB_TRANSACTION_LIMITS *limits);


#endif

