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

#ifndef AH_ACCOUNT_H
#define AH_ACCOUNT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gwenhywfar/misc.h>
#include <gwenhywfar/misc2.h>
#include <aqhbci/aqhbci.h> /* for AQHBCI_API */

typedef struct AH_ACCOUNT AH_ACCOUNT;
GWEN_LIST2_FUNCTION_LIB_DEFS(AH_ACCOUNT, AH_Account, AQHBCI_API);

AQHBCI_API /* needed here since this function really is defined in the lib */
void AH_Account_List2_freeAll(AH_ACCOUNT_LIST2 *al);

#ifdef __cplusplus
}
#endif

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/stringlist.h>
#include <aqhbci/bank.h>


#ifdef __cplusplus
extern "C" {
#endif




AQHBCI_API
AH_ACCOUNT *AH_Account_new(AH_BANK *b,
                           const char *bankId,
                           const char *accountId);
AQHBCI_API
  void AH_Account_free(AH_ACCOUNT *a);

AQHBCI_API
void AH_Account_Attach(AH_ACCOUNT *a);

AQHBCI_API
AH_BANK *AH_Account_GetBank(const AH_ACCOUNT *a);

/**
 * Use with care, changing the bank may render this object useless.
 */
AQHBCI_API
void AH_Account_SetBank(AH_ACCOUNT *a, AH_BANK *b);

AQHBCI_API
const char *AH_Account_GetBankId(const AH_ACCOUNT *a);

AQHBCI_API
void AH_Account_SetBankId(AH_ACCOUNT *a, const char *s);

AQHBCI_API
const char *AH_Account_GetAccountId(const AH_ACCOUNT *a);

/**
 * Use with care, changing the account id may render this object useless.
 */
AQHBCI_API
void AH_Account_SetAccountId(AH_ACCOUNT *a, const char *s);

AQHBCI_API
const char *AH_Account_GetSuffix(const AH_ACCOUNT *a);
AQHBCI_API
void AH_Account_SetSuffix(AH_ACCOUNT *a, const char *s);

AQHBCI_API
const char *AH_Account_GetAccountName(const AH_ACCOUNT *a);
AQHBCI_API
void AH_Account_SetAccountName(AH_ACCOUNT *a,
                               const char *s);
AQHBCI_API
const char *AH_Account_GetOwnerName(const AH_ACCOUNT *a);
AQHBCI_API
void AH_Account_SetOwnerName(AH_ACCOUNT *a,
                             const char *s);

AQHBCI_API
const GWEN_STRINGLIST *AH_Account_GetCustomers(const AH_ACCOUNT *a);
AQHBCI_API
void AH_Account_AddCustomer(AH_ACCOUNT *a, const char *cid);
AQHBCI_API
void AH_Account_ClearCustomers(AH_ACCOUNT *a);

AQHBCI_API
void AH_Account_CleanUp(AH_ACCOUNT *a);

#ifdef __cplusplus
}
#endif





#endif /* AH_ACCOUNT_H */


