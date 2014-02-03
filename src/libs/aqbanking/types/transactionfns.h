/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004-2013 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/



#ifndef AQBANKING_TRANSACTIONFNS_H
#define AQBANKING_TRANSACTIONFNS_H

#include <aqbanking/transaction.h>
#include <aqbanking/account.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @return 0 if both transactions are equal, 1 otherwise (and -1 on error)
 */
AQBANKING_API 
int AB_Transaction_Compare(const AB_TRANSACTION *t1,
                           const AB_TRANSACTION *t0);

/**
 * Fills "local account" parts of the given transaction with the data
 * from the given account. In particular, the following fields are set
 * through this function: SetLocalCountry, SetRemoteCountry,
 * SetLocalBankCode, SetLocalAccountNumber, and SetLocalName.
 */
AQBANKING_API
void AB_Transaction_FillLocalFromAccount(AB_TRANSACTION *t, const AB_ACCOUNT *a);



AQBANKING_API
int AB_Transaction_CheckPurposeAgainstLimits(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim);

AQBANKING_API
int AB_Transaction_CheckNamesAgainstLimits(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim);

AQBANKING_API
int AB_Transaction_CheckTextKeyAgainstLimits(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim);

AQBANKING_API
int AB_Transaction_CheckRecurrenceAgainstLimits(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim);

AQBANKING_API
int AB_Transaction_CheckFirstExecutionDateAgainstLimits(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim);

AQBANKING_API
int AB_Transaction_CheckDateAgainstLimits(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim);

AQBANKING_API
int AB_Transaction_CheckDateAgainstSequenceLimits(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim);


/**
 * Checks whether a given transaction conforms to SEPA specs. This functions especially checks whether local and remote
 * BIC and IBAN are present and whether the local and remote names conform to the retricted SEPA character set.
 */
AQBANKING_API
int AB_Transaction_CheckForSepaConformity(const AB_TRANSACTION *t, int restricted);


AQBANKING_API
int AB_Transaction_WriteToFile(const AB_TRANSACTION *t, const char *tFile);


#ifdef __cplusplus
} /* __cplusplus */
#endif


#endif /* AQBANKING_TRANSACTIONFNS_H */
