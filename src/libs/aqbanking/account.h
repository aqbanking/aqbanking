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


#ifndef AQBANKING_ACCOUNT_H
#define AQBANKING_ACCOUNT_H

#include <gwenhywfar/misc.h>
#include <gwenhywfar/misc2.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/db.h>
#include <aqbanking/error.h> /* for AQBANKING_API */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AB_ACCOUNT AB_ACCOUNT;
GWEN_INHERIT_FUNCTION_LIB_DEFS(AB_ACCOUNT, AQBANKING_API)
GWEN_LIST2_FUNCTION_LIB_DEFS(AB_ACCOUNT, AB_Account, AQBANKING_API)
/* Do not terminate these lines with semicolon because they are
   macros, not functions, and ISO C89 does not allow a semicolon
   there. */

#ifdef __cplusplus
}
#endif


#include <aqbanking/banking.h>
#include <aqbanking/provider.h>
#include <aqbanking/job.h>


#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup AB_ACCOUNT AB_ACCOUNT (Managing accounts)
 * @ingroup AB_C_INTERFACE
 *
 * @brief This group represents accounts.
 *
 * Accounts are only created by AB_PROVIDERs, not by the application.
 */
/*@{*/


/**
 *  This constructor MUST NOT be used by applications. Only backends
 * (see @ref AB_PROVIDER) and AB_BANKING need to create accounts.
 */
AQBANKING_API 
AB_ACCOUNT *AB_Account_new(AB_BANKING *ab,
                           AB_PROVIDER *pro,
                           const char *idForProvider);
AQBANKING_API 
void AB_Account_free(AB_ACCOUNT *acc);

AQBANKING_API 
int AB_Account_CheckAvailability(AB_ACCOUNT *a);

AQBANKING_API 
GWEN_TYPE_UINT32 AB_Account_GetUniqueId(const AB_ACCOUNT *acc);
AQBANKING_API 
void AB_Account_SetUniqueId(AB_ACCOUNT *acc, GWEN_TYPE_UINT32 id);

AQBANKING_API 
AB_PROVIDER *AB_Account_GetProvider(const AB_ACCOUNT *acc);

AQBANKING_API 
GWEN_DB_NODE *AB_Account_GetAppData(const AB_ACCOUNT *acc);
AQBANKING_API 
GWEN_DB_NODE *AB_Account_GetProviderData(const AB_ACCOUNT *acc);

AQBANKING_API 
const char *AB_Account_GetAccountNumber(const AB_ACCOUNT *acc);
AQBANKING_API 
void AB_Account_SetAccountNumber(AB_ACCOUNT *acc, const char *s);

AQBANKING_API 
const char *AB_Account_GetBankCode(const AB_ACCOUNT *acc);
AQBANKING_API 
void AB_Account_SetBankCode(AB_ACCOUNT *acc, const char *s);

AQBANKING_API 
const char *AB_Account_GetAccountName(const AB_ACCOUNT *acc);
AQBANKING_API 
void AB_Account_SetAccountName(AB_ACCOUNT *acc, const char *s);

AQBANKING_API 
const char *AB_Account_GetBankName(const AB_ACCOUNT *acc);
AQBANKING_API 
void AB_Account_SetBankName(AB_ACCOUNT *acc, const char *s);

AQBANKING_API 
const char *AB_Account_GetOwnerName(const AB_ACCOUNT *acc);
AQBANKING_API 
void AB_Account_SetOwnerName(AB_ACCOUNT *acc, const char *s);

AQBANKING_API 
const char *AB_Account_GetCurrency(const AB_ACCOUNT *acc);

AQBANKING_API 
void AB_Account_SetCurrency(AB_ACCOUNT *acc, const char *s);

/*@}*/


#ifdef __cplusplus
}
#endif



#endif /* AQBANKING_ACCOUNT_H */
