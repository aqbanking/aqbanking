/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_BANKING_OB_H
#define AQBANKING_BANKING_OB_H

#include <aqbanking/error.h>
#include <aqbanking/account_spec.h>


#ifdef __cplusplus
extern "C" {
#endif




/** @name Account Spec Functions
 *
 * AqBanking holds a list of account specs for every account managed by AqBanking.
 * You can retrieve the complete list of account specs for all accounts known to AqBanking (@ref AB_Banking6_GetAccountSpecList)
 * or directly request an account spec by its unique id (@ref AB_Banking6_GetAccountSpecByUniqueId).
 */
/*@{*/

/**
 * Returns the list of AB_ACCOUNT_SPEC objects for all known accounts. The caller is responsible for freeing the list returned (if any)
 * via @ref AB_AccountSpec_List_free.
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab pointer to the AB_BANKING object
 * @param pAccountSpecList Pointer to a variable receiving the list of account specs.
 */
AQBANKING_API int AB_Banking_GetAccountSpecList(AB_BANKING *ab, AB_ACCOUNT_SPEC_LIST** pAccountSpecList);


/**
 * Returns an AB_ACCOUNT_SPEC object for the account with the given id (or NULL if not found).
 * The caller is responsible for freeing the data returned (if any) via @ref ABS_AccountSpec_free.
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab pointer to the AB_BANKING object
 * @param uniqueAccountId Unique account id
 * @param pAccountSpec Pointer to a variable to receive the matching account spec.
 */
AQBANKING_API int AB_Banking_GetAccountSpecByUniqueId(AB_BANKING *ab, uint32_t uniqueAccountId, AB_ACCOUNT_SPEC** pAccountSpec);


/*@}*/



/** @name Sending Banking Commands
 *
 */
/*@{*/

/**
 * <p>
 * This function sends all jobs from the given list to their
 * respective backend. The results will be stored in the given im-/exporter
 * context.
 * </p>
 * <p>
 * This function does @b not take over or free the commands.
 * </p>
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab pointer to the AB_BANKING object
 * @param commandList list of commands to execute
 * @param ctx im-/exporter context to receive the results
 */
AQBANKING_API int AB_Banking_SendCommands(AB_BANKING *ab,
                                          const AB_TRANSACTION_LIST* commandList,
                                          AB_IMEXPORTER_CONTEXT *ctx);

/*@}*/



#ifdef __cplusplus
}
#endif

#endif

