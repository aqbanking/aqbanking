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


#ifndef AQBANKING_JOBEUTRANSFER_H
#define AQBANKING_JOBEUTRANSFER_H


#include <aqbanking/job.h>
#include <aqbanking/transaction.h>
#include <aqbanking/eutransferinfo.h>


/** @addtogroup G_AB_JOBS_XFER_EU
 *
 */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  AB_JobEuTransfer_ChargeWhom_Unknown=0,
  AB_JobEuTransfer_ChargeWhom_Local,
  AB_JobEuTransfer_ChargeWhom_Remote,
  AB_JobEuTransfer_ChargeWhom_Share
} AB_JOBEUTRANSFER_CHARGE_WHOM;


AQBANKING_API 
AB_JOB *AB_JobEuTransfer_new(AB_ACCOUNT *a);


/** @name Arguments
 *
 *
 */
/*@{*/
/**
 * <p>
 * This function sets the transfer to be performed.
 * </p>
 * <p>
 * Please note that the backend might later replace the transaction given
 * here with a validated version (upon execution of the job).
 * So if you want to be sure that you have the recent version of the
 * transaction you should call @ref AB_JobEuTransfer_GetTransaction.
 * </p>
 * The remote country code in the transaction must refer to the destination
 * country (see @ref AB_Transaction_SetRemoteCountry).
 * <p>
 * This transaction MUST NOT contain splits.
 * </p>
 */
AQBANKING_API 
int AB_JobEuTransfer_SetTransaction(AB_JOB *j, const AB_TRANSACTION *t);

/**
 * Returns the currently stored transaction for this job. After the job has
 * been executed by the backend the transaction returned will very likely
 * be a pointer to the validated replacement for the initially given
 * transaction.
 */
AQBANKING_API 
const AB_TRANSACTION *AB_JobEuTransfer_GetTransaction(const AB_JOB *j);
/*@}*/


/** @name Parameters
 *
 * The functions in this group are only available after the function
 * @ref AB_Job_CheckAvailability has been called and only if that call flagged
 * success (i.e. that the job is available).
 */
/*@{*/

AQBANKING_API 
const AB_EUTRANSFER_INFO *AB_JobEuTransfer_FindCountryInfo(const AB_JOB *j,
                                                           const char *cnt);

AQBANKING_API 
  const AB_EUTRANSFER_INFO_LIST*
  AB_JobEuTransfer_GetCountryInfoList(const AB_JOB *j);

/**
 * Returns !=0 if the IBAN field of a transaction can be used to specify
 * the destination account. If 0 then the remote account info must set the
 * traditional way (see @ref AB_Transaction_SetRemoteBankCode).
 */
AQBANKING_API 
int AB_JobEuTransfer_GetIbanAllowed(const AB_JOB *j);

AQBANKING_API 
AB_JOBEUTRANSFER_CHARGE_WHOM AB_JobEuTransfer_GetChargeWhom(const AB_JOB *j);

AQBANKING_API 
void AB_JobEuTransfer_SetChargeWhom(AB_JOB *j,
                                    AB_JOBEUTRANSFER_CHARGE_WHOM i);


/*@}*/


#ifdef __cplusplus
}
#endif


/*@}*/ /* addtogroup */


#endif

