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


#ifndef AQBANKING_JOBGETTRANSACTIONS_BE_H
#define AQBANKING_JOBGETTRANSACTIONS_BE_H


#include <aqbanking/job.h>
#include <aqbanking/jobgettransactions.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * Takes over ownership of the given list
 */
AQBANKING_API 
void AB_JobGetTransactions_SetTransactions(AB_JOB *j,
                                           AB_TRANSACTION_LIST2 *tl);

AQBANKING_API 
void AB_JobGetTransactions_SetMaxStoreDays(AB_JOB *j, int i);



#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_JOBGETTRANSACTIONS_BE_H */

