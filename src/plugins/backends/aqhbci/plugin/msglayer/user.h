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

#ifndef AH_USER_H
#define AH_USER_H

#include <aqhbci/aqhbci.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/list2.h>


/** @defgroup MOD_USER HBCI User
 *
 * @ingroup MOD_MSGLAYER
 *
 */
/*@{*/

#ifdef __cplusplus
extern "C" {
#endif
typedef struct AH_USER AH_USER;
GWEN_LIST2_FUNCTION_LIB_DEFS(AH_USER, AH_User, AQHBCI_API);
#ifdef __cplusplus
}
#endif


#include <gwenhywfar/db.h>
#include <aqhbci/bank.h>
#include <aqhbci/medium.h>
#include <aqhbci/customer.h>


#ifdef __cplusplus
extern "C" {
#endif



typedef enum {
  AH_UserStatusNew=0,
  AH_UserStatusEnabled,
  AH_UserStatusPending,
  AH_UserStatusDisabled,
  AH_UserStatusUnknown=999
} AH_USER_STATUS;



AQHBCI_API
AH_USER *AH_User_new(AH_BANK *b,
                     const char *userId,
                     AH_CRYPT_MODE cm,
                     AH_MEDIUM *m);

AQHBCI_API
void AH_User_Attach(AH_USER *u);

AQHBCI_API
void AH_User_free(AH_USER *u);


AQHBCI_API
AH_USER_STATUS AH_User_GetStatus(const AH_USER *u);

AQHBCI_API
void AH_User_SetStatus(AH_USER *u, AH_USER_STATUS i);

const char *AH_User_StatusName(AH_USER_STATUS st);
AH_USER_STATUS AH_User_StatusFromName(const char *s);


AQHBCI_API
AH_MEDIUM *AH_User_GetMedium(const AH_USER *u);

AQHBCI_API
AH_BANK *AH_User_GetBank(const AH_USER *u);

AQHBCI_API
const char *AH_User_GetUserId(const AH_USER *u);

/** This function should NEVER be called with users which are already
 * enlisted in AqHBCI!
 */
AQHBCI_API
void AH_User_SetUserId(AH_USER *u, const char *s);

AQHBCI_API
const char *AH_User_GetPeerId(const AH_USER *u);

AQHBCI_API
void AH_User_SetPeerId(AH_USER *u, const char *s);

AQHBCI_API
int AH_User_GetContextIdx(const AH_USER *u);
AQHBCI_API
void AH_User_SetContextIdx(AH_USER *u, int idx);


AH_CRYPT_MODE AH_User_GetCryptMode(const AH_USER *u);
void AH_User_SetCryptMode(AH_USER *u, AH_CRYPT_MODE m);


AQHBCI_API
AH_CUSTOMER *AH_User_FindCustomer(const AH_USER *u,
                                  const char *customerId);

AQHBCI_API
AH_CUSTOMER_LIST2 *AH_User_GetCustomers(const AH_USER *u,
                                        const char *customerId);

AQHBCI_API
const AH_BPD_ADDR *AH_User_GetAddress(const AH_USER *u);
AQHBCI_API
void AH_User_SetAddress(AH_USER *u, const AH_BPD_ADDR *a);


AQHBCI_API
int AH_User_AddCustomer(AH_USER *u, AH_CUSTOMER *cu);


/**
 * This function only removes the object from the internal list, it doesn't
 * delete its files.
 * You should only call this for objects which you just created in the same
 * session since otherwise it will reappear upon next startup.
 * This function does not destroy the object.
 */
AQHBCI_API
int AH_User_RemoveCustomer(AH_USER *u, AH_CUSTOMER *cu);

/*@}*/ /* defgroup */

#ifdef __cplusplus
}
#endif

#endif /* AH_USER_H */






