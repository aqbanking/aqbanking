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


#ifndef GWHBCI_HBCI_H
#define GWHBCI_HBCI_H


#include <aqhbci/aqhbci.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct AH_HBCI AH_HBCI;
#ifdef __cplusplus
}
#endif

#define AH_DEFAULT_KEYLEN 768

#define AH_HBCI_CHECKMEDIUM_WCB_ID "AH_HBCI_CHECKMEDIUM_WCB_ID"

#define AH_HBCI_LAST_VERSION_NONE 0xffffffff

#include <gwenhywfar/xml.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/plugindescr.h>
#include <gwenhywfar/crypttoken.h>

#include <aqhbci/objectref.h>

#include <aqbanking/banking.h>


#ifdef __cplusplus
extern "C" {
#endif

GWEN_INHERIT_FUNCTION_LIB_DEFS(AH_HBCI, AQHBCI_API);

#define AH_HBCI_CONN_MARK_TCP 1
#define AH_HBCI_CONN_MARK_SSL 2



typedef enum {
  AH_CryptMode_Unknown=-1,
  /** No type.  */
  AH_CryptMode_None=0,
  /** DES-DES-Verfahren  */
  AH_CryptMode_Ddv,
  /** PIN/TAN mode  */
  AH_CryptMode_Pintan,
  /** RSA-DES-Hybridverfahren  */
  AH_CryptMode_Rdh
} AH_CRYPT_MODE;
AQHBCI_API
AH_CRYPT_MODE AH_CryptMode_fromString(const char *s);
AQHBCI_API
const char *AH_CryptMode_toString(AH_CRYPT_MODE v);

#ifdef __cplusplus
}
#endif

#include <aqhbci/medium.h>
#include <aqhbci/user.h>
#include <aqhbci/account.h>
#include <aqhbci/message.h>
#include <aqhbci/customer.h>
#include <aqhbci/user.h>

#ifdef __cplusplus
extern "C" {
#endif


/** @name Constructors, Destructors
 *
 */
/*@{*/
AQHBCI_API
AH_HBCI *AH_HBCI_new(AB_PROVIDER *pro);
AQHBCI_API
void AH_HBCI_free(AH_HBCI *hbci);
/*@}*/


/** @name Informational Functions
 *
 */
/*@{*/
AQHBCI_API
const char *AH_HBCI_GetProductName(const AH_HBCI *hbci);
AQHBCI_API
void AH_HBCI_SetProductName(AH_HBCI *hbci, const char *s);

AQHBCI_API
const char *AH_HBCI_GetProductVersion(const AH_HBCI *hbci);
AQHBCI_API
void AH_HBCI_SetProductVersion(AH_HBCI *hbci, const char *s);

AQHBCI_API
AB_BANKING *AH_HBCI_GetBankingApi(const AH_HBCI *hbci);
AQHBCI_API
AB_PROVIDER *AH_HBCI_GetProvider(const AH_HBCI *hbci);

/*@}*/



AQHBCI_API
int AH_HBCI_Init(AH_HBCI *hbci);
AQHBCI_API
int AH_HBCI_Fini(AH_HBCI *hbci);



AQHBCI_API
AH_MEDIUM*
  AH_HBCI_MediumFactory(AH_HBCI *hbci,
                        const char *typeName,
			const char *subTypeName,
                        const char *mediumName);

AQHBCI_API
AH_MEDIUM*
AH_HBCI_MediumFactoryDb(AH_HBCI *hbci,
                        const char *typeName,
			const char *subTypeName,
                        GWEN_DB_NODE *db);

AQHBCI_API
AH_MEDIUM *AH_HBCI_FindMedium(const AH_HBCI *hbci,
                              const char *typeName,
                              const char *mediumName);

AQHBCI_API
AH_MEDIUM *AH_HBCI_SelectMedium(AH_HBCI *hbci,
                                const char *typeName,
                                const char *subTypeName,
                                const char *mediumName);

AQHBCI_API
AH_MEDIUM *AH_HBCI_SelectMediumDb(AH_HBCI *hbci,
                                  const char *typeName,
                                  const char *subTypeName,
                                  GWEN_DB_NODE *db);

AQHBCI_API
int AH_HBCI_AddMedium(AH_HBCI *hbci, AH_MEDIUM *m);

AQHBCI_API
const AH_MEDIUM_LIST *AH_HBCI_GetMediaList(const AH_HBCI *hbci);


/**
 * This function only removes the object from the internal list, it doesn't
 * delete its files.
 * You should only call this for objects which you just created in the same
 * session since otherwise it will reappear upon next startup.
 * This function does not destroy the object.
 */
AQHBCI_API
int AH_HBCI_RemoveMedium(AH_HBCI *hbci, AH_MEDIUM *m);


AQHBCI_API
int AH_HBCI_UnmountCurrentMedium(AH_HBCI *hbci);


AQHBCI_API
GWEN_PLUGIN_DESCRIPTION_LIST2*
AH_HBCI_GetMediumPluginDescrs(AH_HBCI *hbci,
                              GWEN_CRYPTTOKEN_DEVICE dev);

AQHBCI_API
int AH_HBCI_CheckMedium(AH_HBCI *hbci,
                        GWEN_CRYPTTOKEN_DEVICE dev,
                        GWEN_BUFFER *mtypeName,
                        GWEN_BUFFER *msubTypeName,
                        GWEN_BUFFER *mediumName);




AQHBCI_API
AH_BANK *AH_HBCI_FindBank(const AH_HBCI *hbci,
                          int country,
                          const char *bankId);

AQHBCI_API
AH_BANK_LIST2 *AH_HBCI_GetBanks(const AH_HBCI *hbci,
                                int country,
                                const char *bankId);

AQHBCI_API
int AH_HBCI_AddBank(AH_HBCI *hbci, AH_BANK *b);
AQHBCI_API

AQHBCI_API
int AH_HBCI_RemoveBank(AH_HBCI *hbci, AH_BANK *b);


AQHBCI_API
AH_CUSTOMER *AH_HBCI_FindCustomer(AH_HBCI *hbci,
                                  int country,
                                  const char *bankId,
                                  const char *userId,
                                  const char *customerId);

AQHBCI_API
AH_CUSTOMER_LIST2 *AH_HBCI_GetCustomers(AH_HBCI *hbci,
                                        int country,
                                        const char *bankId,
                                        const char *userId,
                                        const char *customerId);

AQHBCI_API
AH_ACCOUNT *AH_HBCI_FindAccount(AH_HBCI *hbci,
                                int country,
                                const char *bankId,
                                const char *accountId);

AQHBCI_API
AH_ACCOUNT_LIST2 *AH_HBCI_GetAccounts(AH_HBCI *hbci,
                                      int country,
                                      const char *bankId,
                                      const char *accountId);


AQHBCI_API
AH_USER *AH_HBCI_FindUser(AH_HBCI *hbci,
                          int country,
                          const char *bankId,
                          const char *userId);

AQHBCI_API
AH_USER_LIST2 *AH_HBCI_GetUsers(AH_HBCI *hbci,
                                int country,
                                const char *bankId,
                                const char *userId);



AQHBCI_API
int AH_HBCI_AddBankPath(const AH_HBCI *hbci,
                        const AH_BANK *b,
                        GWEN_BUFFER *nbuf);
AQHBCI_API
int AH_HBCI_AddUserPath(const AH_HBCI *hbci,
                        const AH_USER *u,
                        GWEN_BUFFER *nbuf);
AQHBCI_API
int AH_HBCI_AddCustomerPath(const AH_HBCI *hbci,
                            const AH_CUSTOMER *cu,
                            GWEN_BUFFER *nbuf);

AQHBCI_API
int AH_HBCI_AddAccountPath(const AH_HBCI *hbci,
                           const AH_ACCOUNT *a,
                           GWEN_BUFFER *nbuf);



AQHBCI_API
int AH_HBCI_AddBankCertFolder(AH_HBCI *hbci,
                              AH_BANK *b,
                              GWEN_BUFFER *nbuf);
AQHBCI_API
int AH_HBCI_RemoveAllBankCerts(AH_HBCI *hbci, AH_BANK *b);


AQHBCI_API
int AH_HBCI_SaveMessage(AH_HBCI *hbci,
                        const AH_CUSTOMER *cu,
                        GWEN_DB_NODE *dbMsg);


AQHBCI_API
GWEN_DB_NODE *AH_HBCI_GetSharedRuntimeData(const AH_HBCI *hbci);

AQHBCI_API
int AH_HBCI_GetTransferTimeout(const AH_HBCI *hbci);
AQHBCI_API
void AH_HBCI_SetTransferTimeout(AH_HBCI *hbci, int i);


AQHBCI_API
int AH_HBCI_GetConnectTimeout(const AH_HBCI *hbci);
AQHBCI_API
void AH_HBCI_SetConnectTimeout(AH_HBCI *hbci, int i);


AQHBCI_API
void AH_HBCI_HbciToUtf8(const char *p,
                        int size,
                        GWEN_BUFFER *buf);

AQHBCI_API
int AH_HBCI_CheckStringSanity(const char *s);


#ifdef __cplusplus
}
#endif

#endif /* GWHBCI_HBCI_H */



