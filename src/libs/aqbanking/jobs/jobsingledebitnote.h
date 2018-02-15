/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_JOBSINGLEDEBITNOTE_H
#define AQBANKING_JOBSINGLEDEBITNOTE_H


#include <aqbanking/job.h>


/** @addtogroup G_AB_JOBS_XFER_DEBIT
 *
 * <p>
 * Debit notes are inverse transfers: You specify an account from which you
 * want to draw money to on of your accounts.
 * </p>
 * <p>
 * Obviously not every customer is allowed to draw from any other account.
 * This feature is only reserved for business customers of credit institutes
 * (not restricted by us but by your credit institute).
 * </p>
 * <p>
 * In most cases your are required to sign a special document with your bank
 * to be able to use such a jib.
 * </p>
 *
 */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif


AQBANKING_API 
AB_JOB *AB_JobSingleDebitNote_new(AB_ACCOUNT *a);


#ifdef __cplusplus
}
#endif


/*@}*/ /* addtogroup */


#endif

