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

typedef enum {
  AB_AccountType_Unknown=0,
  AB_AccountType_Bank,
  AB_AccountType_CreditCard,
  AB_AccountType_Checking,
  AB_AccountType_Savings,
  AB_AccountType_Investment,
  AB_AccountType_Cash
} AB_ACCOUNT_TYPE;

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


AQBANKING_API 
void AB_Account_free(AB_ACCOUNT *acc);

AQBANKING_API 
AB_BANKING *AB_Account_GetBanking(const AB_ACCOUNT *acc);

AQBANKING_API 
int AB_Account_CheckAvailability(AB_ACCOUNT *a);

AQBANKING_API 
AB_ACCOUNT_TYPE AB_Account_GetAccountType(const AB_ACCOUNT *acc);
AQBANKING_API 
void AB_Account_SetAccountType(AB_ACCOUNT *acc, AB_ACCOUNT_TYPE t);

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

/** Returns the name of the account product (really:
 "Kontoproduktbezeichnung" according to HBCI spec). This may or
 may not be useful for your application. The bank may freely
 choose what to say in here. */
AQBANKING_API 
const char *AB_Account_GetAccountName(const AB_ACCOUNT *acc);
AQBANKING_API 
void AB_Account_SetAccountName(AB_ACCOUNT *acc, const char *s);

/** Returns the name of the bank, or NULL if none was set. */
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
