/***************************************************************************
 begin       : Wed Nov 28 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_BANKING_TRANSACTION_H
#define AQBANKING_BANKING_TRANSACTION_H

#include <aqbanking/error.h>
#include <aqbanking/types/transaction.h>
#include <aqbanking/types/transactionlimits.h>


#ifdef __cplusplus
extern "C" {
#endif



/** @name Working with Transactions
 *
 */
/*@{*/


/**
 * Check transaction against limits: Check purpose.
 * @return 0 if okay, errorcode otherwise.
 * @param t transaction to check
 * @param lim limits to check against.
 */
AQBANKING_API int AB_Banking_CheckTransactionAgainstLimits_Purpose(const AB_TRANSACTION *t,
                                                                   const AB_TRANSACTION_LIMITS *lim);

/**
 * Check transaction against limits: Check local and remote name.
 * @return 0 if okay, errorcode otherwise.
 * @param t transaction to check
 * @param lim limits to check against.
 */
AQBANKING_API int AB_Banking_CheckTransactionAgainstLimits_Names(const AB_TRANSACTION *t,
                                                                 const AB_TRANSACTION_LIMITS *lim);

/**
 * Check transaction against limits: Check recurrence.
 * @return 0 if okay, errorcode otherwise.
 * @param t transaction to check
 * @param lim limits to check against.
 */
AQBANKING_API int AB_Banking_CheckTransactionAgainstLimits_Recurrence(const AB_TRANSACTION *t,
                                                                      const AB_TRANSACTION_LIMITS *lim);

/**
 * Check transaction against limits: Check execution date.
 * @return 0 if okay, errorcode otherwise.
 * @param t transaction to check
 * @param lim limits to check against.
 */
AQBANKING_API int AB_Banking_CheckTransactionAgainstLimits_ExecutionDate(const AB_TRANSACTION *t,
                                                                         const AB_TRANSACTION_LIMITS *lim);

/**
 * Check transaction against limits: Check date.
 * @return 0 if okay, errorcode otherwise.
 * @param t transaction to check
 * @param lim limits to check against.
 */
AQBANKING_API int AB_Banking_CheckTransactionAgainstLimits_Date(const AB_TRANSACTION *t,
                                                                const AB_TRANSACTION_LIMITS *lim);

/**
 * Check transaction against limits: Check sequence setup (debit notes).
 * @return 0 if okay, errorcode otherwise.
 * @param t transaction to check
 * @param lim limits to check against.
 */
AQBANKING_API int AB_Banking_CheckTransactionAgainstLimits_Sequence(const AB_TRANSACTION *t,
                                                                    const AB_TRANSACTION_LIMITS *lim);

/**
 * Check transaction for SEPA conformity (IBAN, BIC, names)
 * @return 0 if okay, errorcode otherwise.
 * @param t transaction to check
 * @param lim limits to check against.
 */
AQBANKING_API int AB_Banking_CheckTransactionForSepaConformity(const AB_TRANSACTION *t, int restricted);


/**
 * Fill local account info from account spec.
 */
AQBANKING_API void AB_Banking_FillTransactionFromAccountSpec(AB_TRANSACTION *t, const AB_ACCOUNT_SPEC *as);


/**
 * Construct data for a memo field in your finance application using a template.
 *
 * The template may contain text
 * to always be written into the memo field and also refererences to fields of the AB_TRANSACTION object given.
 * Typical variables are:
 * - $(purpose): purpose field
 * - $(purpose:32): purpose field (use max 32 chars of it)
 * - $(transactionText): short description of the transaction ("Buchungstext")
 * - $(remoteName): name of the other party in a transaction
 * - $(remoteIban): iban of the other party in a transaction
 * - $(customerReference)
 * - $(bankReference)
 * - $(endToEndReference)
 * - any other field defined in src/libs/aqbanking/types/transaction.xml (NAME in XML elements <member name=NAME")
 * @param t transaction for which to generate a memo
 * @param sTemplate template string (e.g. "$(purpose) $(transactionText))
 * @return GWEN_BUFFER containing the result (get a char pointer with GWEN_Buffer_GetStart(), release with GWEN_Buffer_free())
 *         or NULL on error
 */
AQBANKING_API GWEN_BUFFER *AB_Banking_ConstructMemoFromTemplate(const AB_TRANSACTION *t, const char *sTemplate);

/*@}*/


#ifdef __cplusplus
}
#endif

#endif
