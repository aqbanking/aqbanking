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


#ifndef AQBANKING_BANKING_SIMPLE_H
#define AQBANKING_BANKING_SIMPLE_H


#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup G_AB_SIMPLE
 *
 * @short This group contains a very simple API.
 *
 * <p>
 * This API is intended to be used when adding support for AqBanking to
 * already existing applications. Basically this API just defines some
 * functions which enqueue requests (such as "Get balance").
 * </p>
 * <p>
 * This way your application only needs to provide the functionality to
 * import such a context.
 * </p>
 * <p>
 * Mixing the Job-API with this API is allowed.
 * </p>
 * <p>
 * A program should first call @ref AB_Banking_Init to allow AqBanking
 * to load its configuration files and initialize itself.
 * </p>
 * <p>
 * You need to setup AqBanking before using functions of this group (i.e.
 * accounts must be setup).
 * </p>
 * After that you may call any other function of this group (most likely
 * the program will request a list of managed account via
 * @ref AB_Banking_GetAccounts).
 * </p>
 * <p>
 * When the program has finished its work it should call @ref AB_Banking_Fini
 * as the last function of AqBanking (just before calling
 * @ref AB_Banking_free).
 * </p>
 * <p>
 * The following is a small example of how the High Level API might be used.
 * It requests the status (e.g. balance etc) of an account.
 * </p>
 * @code
 * TODO: Add example
 * @endcode
 */
/*@{*/

/** @name Request Functions
 *
 */
/*@{*/
/**
 * This function enqueues a request for the balance of an account.
 * You need to call @ref AB_Banking_ExecuteQueue to actually perform the
 * job.
 */
AQBANKING_API 
int AB_Banking_RequestBalance(AB_BANKING *ab,
                              const char *bankCode,
                              const char *accountNumber);

/**
 * This function enqueues a request for the transactions of an account.
 * You need to call @ref AB_Banking_ExecuteQueue to actually perform the
 * job.
 * Please note that not all backends and all banks allow a time span to be
 * given to this function. In such cases the dates are simply ignored.
 */
AQBANKING_API 
int AB_Banking_RequestTransactions(AB_BANKING *ab,
                                   const char *bankCode,
                                   const char *accountNumber,
                                   const GWEN_TIME *firstDate,
                                   const GWEN_TIME *lastDate);

/**
 * This function enqueues a request for the standing orders of an account.
 * You need to call @ref AB_Banking_ExecuteQueue to actually perform the
 * job.
 */
AQBANKING_API 
int AB_Banking_RequestStandingOrders(AB_BANKING *ab,
                                     const char *bankCode,
                                     const char *accountNumber);

/**
 * This function enqueues a request for the list of pending dated transfers of
 * an account.
 * You need to call @ref AB_Banking_ExecuteQueue to actually perform the
 * job.
 */
AQBANKING_API 
int AB_Banking_RequestDatedTransfers(AB_BANKING *ab,
                                     const char *bankCode,
                                     const char *accountNumber);
/*@}*/



/** @name Mapping Application Accounts to Online Accounts
 *
 * Functions in this group provide an optional service for account mapping.
 * Most applications assign unique ids to their own accounts. This unique
 * id can be mapped to an account of AqBanking.
 */
/*@{*/
/**
 * <p>
 * Sets an alias for the given AqBanking account. You can later use
 * @ref AB_Banking_GetAccountByAlias to refer to an online account by using
 * the unique id of your application's account.
 * </p>
 * <p>
 * AqBanking separates the aliases for each application.
 * </p>
 * @param ab AqBanking main object
 * @param a online account of AqBanking you wish to map your account to
 * @param alias unique id of your application's own account structure
 */
AQBANKING_API 
void AB_Banking_SetAccountAlias(AB_BANKING *ab,
                                AB_ACCOUNT *a, const char *alias);

/**
 * This function returns the AqBanking account to which the given
 * alias (=unique id of your application's own account data) has been
 * mapped.
 *
 * AqBanking remains the owner of the object returned (if any), so you must
 * not free it.
 *
 * Please also note that the object returned is only valid until
 * AB_Banking_Fini() has been called (or until the corresponding backend for
 * this particular account has been deactivated).
 *
 * @return corresponding AqBanking (or 0 if none)
 * @param ab AqBanking main object
 * @param alias unique id of your application's own account structure
 */
AQBANKING_API 
AB_ACCOUNT *AB_Banking_GetAccountByAlias(AB_BANKING *ab,
                                         const char *alias);
/*@}*/


/*@}*/ /* addtogroup */

#ifdef __cplusplus
}
#endif


#endif

