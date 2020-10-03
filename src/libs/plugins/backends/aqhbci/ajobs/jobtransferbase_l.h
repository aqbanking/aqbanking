/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBTRANSFERBASE_L_H
#define AH_JOBTRANSFERBASE_L_H


#include "accountjob_l.h"



AH_JOB *AH_Job_TransferBase_new(const char *jobName,
                                AB_TRANSACTION_TYPE tt,
                                AB_TRANSACTION_SUBTYPE tst,
                                AB_PROVIDER *pro,
                                AB_USER *u, AB_ACCOUNT *account);

const char *AH_Job_TransferBase_GetFiid(const AH_JOB *j);

/**
 * Select SEPA PAIN profile to be used.
 *
 * Lookupo SWIFT descriptor which is supported by the user BPD/UPD and by the XML imexporter.
 */
int AH_Job_TransferBase_SelectPainProfile(AH_JOB *j, int version1);


void AH_Job_TransferBase_SetLocalInstrumentationCode(AH_JOB *j, const char *s);


/**
 * Export the transactions stored with this job and export them to SEPA using the
 * previously selected SEPA profile (see @ref AH_Job_TransferBase_SelectPainProfile).
 */
int AH_Job_TransferBase_SepaExportTransactions(AH_JOB *j);

/**
 * Returns AB_TRANSACTION_LIMITS for undated SEPA transfers and debit notes.
 */
int AH_Job_TransferBase_GetLimits_SepaUndated(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits);

/**
 * Returns AB_TRANSACTION_LIMITS for dated SEPA transfers and debit notes.
 */
int AH_Job_TransferBase_GetLimits_SepaDated(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits);

/**
 * Returns AB_TRANSACTION_LIMITS for SEPA standing order jobs.
 */
int AH_Job_TransferBase_GetLimits_SepaStandingOrder(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits);



/**
 * Implementation of AH_Job_HandleCommand() for undated SEPA jobs.
 * It checks the given transaction and adds it to the internal list of transfers for the given job.
 * Checks performed here are:
 * - @ref AB_Transaction_CheckForSepaConformity
 * - @ref AB_Transaction_CheckPurposeAgainstLimits
 * - @ref AB_Transaction_CheckNamesAgainstLimits
 */
int AH_Job_TransferBase_HandleCommand_SepaUndated(AH_JOB *j, const AB_TRANSACTION *t);

/**
 * Implementation of AH_Job_HandleCommand() for dated SEPA jobs.
 * It checks the given transaction and adds it to the internal list of transfers for the given job.
 * Checks performed here are:
 * - @ref AB_Transaction_CheckForSepaConformity
 * - @ref AB_Transaction_CheckPurposeAgainstLimits
 * - @ref AB_Transaction_CheckNamesAgainstLimits
 * - @ref AB_Transaction_CheckDateAgainstLimits
 */
int AH_Job_TransferBase_HandleCommand_SepaDated(AH_JOB *j, const AB_TRANSACTION *t);

/**
 * Implementation of AH_Job_HandleCommand() for dated SEPA debit noted jobs.
 * It checks the given transaction and adds it to the internal list of transfers for the given job.
 * Checks performed here are:
 * - @ref AB_Transaction_CheckForSepaConformity
 * - @ref AB_Transaction_CheckPurposeAgainstLimits
 * - @ref AB_Transaction_CheckNamesAgainstLimits
 * - @ref AB_Transaction_CheckDateAgainstSequenceLimits
 */
int AH_Job_TransferBase_HandleCommand_SepaDatedDebit(AH_JOB *j, const AB_TRANSACTION *t);

/**
 * Implementation of AH_Job_HandleCommand() for SEPA standing order jobs.
 * It checks the given transaction and adds it to the internal list of transfers for the given job.
 * Checks performed here are:
 * - @ref AB_Transaction_CheckForSepaConformity
 * - @ref AB_Transaction_CheckPurposeAgainstLimits
 * - @ref AB_Transaction_CheckNamesAgainstLimits
 * - @ref AB_Transaction_CheckRecurrenceAgainstLimits
 * - @ref AB_Transaction_CheckFirstExecutionDateAgainstLimits (only for CreateStandingOrder)
 */
int AH_Job_TransferBase_HandleCommand_SepaStandingOrder(AH_JOB *j, const AB_TRANSACTION *t);





/**
 * Add challenge parameters type 29 (used in dated transfers/debit notes).
 */
int AH_Job_TransferBase_AddChallengeParams29(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);



/**
 * Add challenge parameters type 35 (used in SEPA standing orders).
 */
int AH_Job_TransferBase_AddChallengeParams35(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);


#endif /* AH_JOBTRANSFERBASE_L_H */


