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


#ifndef AQBANKING_JOBSINGLEDEBITNOTE_H
#define AQBANKING_JOBSINGLEDEBITNOTE_H


#include <aqbanking/job.h>
#include <aqbanking/transaction.h>

#ifdef __cplusplus
extern "C" {
#endif


AB_JOB *AB_JobSingleDebitNote_new(AB_ACCOUNT *a);


/** @name Arguments
 *
 *
 */
/*@{*/
/**
 * This function sets the transfer to be performed.
 * Please note that the backend might later replace the transaction given
 * here with a validated version (upon execution of the job).
 * So if you want to be sure that you have the recent version of the
 * transaction you should call @ref AB_JobSingleDebitNote_GetTransaction.
 */
AQBANKING_API 
int AB_JobSingleDebitNote_SetTransaction(AB_JOB *j, const AB_TRANSACTION *t);

/**
 * Returns the currently stored transaction for this job. After the job has
 * been executed by the backend the transaction returned will very likely
 * be a pointer to the validated replacement for the initially given
 * transaction.
 */
AQBANKING_API 
const AB_TRANSACTION *AB_JobSingleDebitNote_GetTransaction(const AB_JOB *j);
/*@}*/


/** @name Parameters
 *
 * The functions in this group are only available after the function
 * @ref AB_Job_CheckAvailability has been called and only if that call flagged
 * success (i.e. that the job is available).
 */
/*@{*/

/**
 * Returns the number of lines the purpose field of a transaction is allowed
 * to have. If -1 then this limit is unknown. It is best in that case to
 * assume a limit of 2 (or to leave it to the user).
 */
AQBANKING_API 
int AB_JobSingleDebitNote_GetMaxPurposeLines(const AB_JOB *j);

/**
 * Returns a pointer to a list of INTs containing valid text keys.
 * The list returned is terminated by an entry of the value "-1".
 * If this parameter is unknown NULL is returned.
 */
AQBANKING_API 
const int *AB_JobSingleDebitNote_GetTextKeys(const AB_JOB *j);
/*@}*/


#ifdef __cplusplus
}
#endif


#endif

