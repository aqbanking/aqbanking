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


#ifndef AQBANKING_JOBGETTRANSACTIONS_H
#define AQBANKING_JOBGETTRANSACTIONS_H


#include <aqbanking/job.h>
#include <aqbanking/transaction.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Creates a job which retrieves transaction reports for the given timespan
 * (if any).
 * @param a account for which you want the reports
 * @param fromTime if !=0 then this is the first date for which you want
 *  the reports (the time doesn't matter, only the date component of the
 *  given GWEN_TIME is used). If NULL then the first day for which the bank
 *  has reports is assumed.
 * @param toTime if !=0 then this is the last date for which you want
 *  the reports (the time doesn't matter, only the date component of the
 *  given GWEN_TIME is used). If NULL then the last day for which the bank
 *  has reports is assumed.
 */
AQBANKING_API
AB_JOB *AB_JobGetTransactions_new(AB_ACCOUNT *a,
                                  const GWEN_TIME *fromTime,
                                  const GWEN_TIME *toTime);

AQBANKING_API 
const AB_TRANSACTION_LIST2*
  AB_JobGetTransactions_GetTransactions(const AB_JOB *j);

/**
 * Returns the maximum number of days the bank stores your transaction
 * data for the account associated with the given job.
 * @return 0 if unknown, number of days otherwise
 * @param j job
 */
AQBANKING_API 
int AB_JobGetTransactions_GetMaxStoreDays(const AB_JOB *j);

AQBANKING_API 
const GWEN_TIME *AB_JobGetTransactions_GetFromTime(const AB_JOB *j);

AQBANKING_API
const GWEN_TIME *AB_JobGetTransactions_GetToTime(const AB_JOB *j);



#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_JOBGETTRANSACTIONS_H */

