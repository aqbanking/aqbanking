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


#ifndef AQBANKING_JOBSINGLEDEBITNOTE_P_H
#define AQBANKING_JOBSINGLEDEBITNOTE_P_H


#include <aqbanking/job.h>
#include <aqbanking/transaction.h>
#include <aqbanking/transactionlimits.h>
#include "jobsingledebitnote_l.h"

typedef struct AB_JOBSINGLEDEBITNOTE AB_JOBSINGLEDEBITNOTE;
struct AB_JOBSINGLEDEBITNOTE {
  AB_TRANSACTION *transaction;
  AB_TRANSACTION_LIMITS *limits;
  int *textKeys;
};
void AB_JobSingleDebitNote_FreeData(void *bp, void *p);


#endif

