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


#ifndef AQBANKING_JOBSINGLETRANSFER_H
#define AQBANKING_JOBSINGLETRANSFER_H


#include <aqbanking/job.h>
#include <aqbanking/transaction.h>



AB_JOB *AB_JobSingleTransfer_new(AB_ACCOUNT *a);


/** @name Arguments
 *
 *
 */
/*@{*/
/**
 *
 */
AQBANKING_API 
int AB_JobSingleTransfer_SetTransaction(AB_JOB *j, const AB_TRANSACTION *t);

AQBANKING_API 
const AB_TRANSACTION *AB_JobSingleTransfer_GetTransaction(const AB_JOB *j);
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
int AB_JobSingleTransfer_GetMaxPurposeLines(const AB_JOB *j);

/**
 * Returns a pointer to a list of INTs containing valid text keys.
 * The list returned is terminated by an entry of the value "-1".
 * If this parameter is unknown NULL is returned.
 */
AQBANKING_API 
const int *AB_JobSingleTransfer_GetTextKeys(const AB_JOB *j);
/*@}*/


#endif

