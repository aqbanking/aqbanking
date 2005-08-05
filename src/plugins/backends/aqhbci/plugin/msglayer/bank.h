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


#ifndef AH_BANK_H
#define AH_BANK_H

/** @defgroup MOD_BANK HBCI Bank
 * @ingroup MOD_MSGLAYER
 *
 * @short This group contains functions and definitions for a HBCI bank
 *
 * The following graph shows the basic structure of a bank:
 *  @image html bank.png "Internal structure of a AH_BANK"
 * <p>
 * As you can see a bank includes a list of @ref MOD_USER which in turn
 * contains a list of @ref MOD_CUSTOMER.
 * </p>
 */
/*@{*/

#include <gwenhywfar/misc.h>
#include <gwenhywfar/list2.h>
#include <gwenhywfar/inherit.h>
#include <aqhbci/aqhbci.h> /* for AQHBCI_API */

#ifdef __cplusplus
extern "C" {
#endif
typedef struct AH_BANK AH_BANK;

GWEN_LIST_FUNCTION_LIB_DEFS(AH_BANK, AH_Bank, AQHBCI_API);
GWEN_LIST2_FUNCTION_LIB_DEFS(AH_BANK, AH_Bank, AQHBCI_API);
GWEN_INHERIT_FUNCTION_LIB_DEFS(AH_BANK, AQHBCI_API);

#ifdef __cplusplus
}
#endif


#include <gwenhywfar/netconnection.h>
#include <aqhbci/hbci.h>
#include <aqhbci/medium.h>
#include <aqhbci/message.h>
#include <aqhbci/user.h>
#include <aqhbci/account.h>
#include <aqhbci/bpd.h>


#ifdef __cplusplus
extern "C" {
#endif


/** @name Contructor And Destructor
 *
 */
/*@{*/
AQHBCI_API
AH_BANK *AH_Bank_new(AH_HBCI *hbci,
                     int country,
                     const char *bankId);

AQHBCI_API
void AH_Bank_free(AH_BANK *b);
AQHBCI_API
void AH_Bank_Attach(AH_BANK *b);

/*@}*/


/** @name Informational Functions
 *
 */
/*@{*/
AQHBCI_API
AH_HBCI *AH_Bank_GetHbci(const AH_BANK *b);

AQHBCI_API
int AH_Bank_GetCountry(const AH_BANK *b);
AQHBCI_API
const char *AH_Bank_GetBankId(const AH_BANK *b);

AQHBCI_API
const char *AH_Bank_GetBankName(const AH_BANK *b);
AQHBCI_API
void AH_Bank_SetBankName(AH_BANK *b, const char *s);








AQHBCI_API
AH_CUSTOMER *AH_Bank_FindCustomer(const AH_BANK *b,
                                  const char *userId,
                                  const char *customerId);

AQHBCI_API
AH_CUSTOMER_LIST2 *AH_Bank_GetCustomers(const AH_BANK *b,
                                        const char *userId,
                                        const char *customerId);



AQHBCI_API
AH_USER *AH_Bank_FindUser(const AH_BANK *b, const char *userId);

AQHBCI_API
AH_USER_LIST2 *AH_Bank_GetUsers(const AH_BANK *b, const char *userId);

AQHBCI_API
int AH_Bank_AddUser(AH_BANK *b, AH_USER *u);

AQHBCI_API
int AH_Bank_RemoveUser(AH_BANK *b, AH_USER *u);


AQHBCI_API
AH_ACCOUNT *AH_Bank_FindAccount(const AH_BANK *b, const char *accountId);

AQHBCI_API
AH_ACCOUNT_LIST2 *AH_Bank_GetAccounts(const AH_BANK *b,
                                      const char *accountId);

AQHBCI_API
int AH_Bank_AddAccount(AH_BANK *b, AH_ACCOUNT *a);

AQHBCI_API
int AH_Bank_RemoveAccount(AH_BANK *b, AH_ACCOUNT *a);

/*@}*/



/*@}*/ /* defgroup */

#ifdef __cplusplus
}
#endif

#endif /* AH_BANK_H */


