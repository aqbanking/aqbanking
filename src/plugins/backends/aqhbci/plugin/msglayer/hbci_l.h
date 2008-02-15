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


#ifndef GWHBCI_HBCI_L_H
#define GWHBCI_HBCI_L_H

#include <gwenhywfar/xml.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/plugindescr.h>
#include <gwenhywfar/ct.h>

#include <aqhbci/aqhbci.h>

#include <aqbanking/banking.h>

typedef struct AH_HBCI AH_HBCI;


#include <aqhbci/user.h>
#include <aqhbci/account.h>


#define AH_DEFAULT_KEYLEN 768

#define AH_HBCI_LAST_VERSION_NONE 0xffffffff



int AH_HBCI_AddObjectPath(const AH_HBCI *hbci,
                          const char *country,
                          const char *bankId,
                          const char *accountId,
                          const char *userId,
                          const char *customerId,
                          GWEN_BUFFER *nbuf);

int AH_HBCI_GetAccountPath(const AH_HBCI *hbci,
                           const AB_ACCOUNT *acc,
                           GWEN_BUFFER *buf);
int AH_HBCI_GetCustomerPath(const AH_HBCI *hbci,
                            const AB_USER *u,
                            GWEN_BUFFER *buf);


int AH_HBCI_AddBankPath(const AH_HBCI *hbci,
                        const AB_USER *u,
                        GWEN_BUFFER *nbuf);
int AH_HBCI_AddUserPath(const AH_HBCI *hbci,
                        const AB_USER *u,
                        GWEN_BUFFER *nbuf);
int AH_HBCI_AddCustomerPath(const AH_HBCI *hbci,
                            const AB_USER *u,
                            GWEN_BUFFER *nbuf);
int AH_HBCI_AddAccountPath(const AH_HBCI *hbci,
                           const AB_ACCOUNT *a,
                           GWEN_BUFFER *nbuf);

void AH_HBCI_AppendUniqueName(AH_HBCI *hbci, GWEN_BUFFER *nbuf);

GWEN_XMLNODE *AH_HBCI_GetDefinitions(const AH_HBCI *hbci);


int AH_HBCI_Update(AH_HBCI *hbci,
                   uint32_t lastVersion,
                   uint32_t currentVersion);


uint32_t AH_HBCI_GetLastVersion(const AH_HBCI *hbci);




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
int AH_HBCI_AddBankPath(const AH_HBCI *hbci,
                        const AB_USER *u,
                        GWEN_BUFFER *nbuf);
AQHBCI_API
int AH_HBCI_AddUserPath(const AH_HBCI *hbci,
                        const AB_USER *u,
                        GWEN_BUFFER *nbuf);
AQHBCI_API
int AH_HBCI_AddCustomerPath(const AH_HBCI *hbci,
                            const AB_USER *u,
                            GWEN_BUFFER *nbuf);

AQHBCI_API
int AH_HBCI_AddAccountPath(const AH_HBCI *hbci,
                           const AB_ACCOUNT *a,
                           GWEN_BUFFER *nbuf);



AQHBCI_API
int AH_HBCI_AddBankCertFolder(AH_HBCI *hbci,
                              const AB_USER *u,
                              GWEN_BUFFER *nbuf);
AQHBCI_API
int AH_HBCI_RemoveAllBankCerts(AH_HBCI *hbci, const AB_USER *u);


AQHBCI_API
int AH_HBCI_SaveMessage(AH_HBCI *hbci,
                        const AB_USER *u,
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
int AH_HBCI_CheckStringSanity(const char *s);


#if 0
AQHBCI_API
int AH_HBCI_GetCryptToken(AH_HBCI *hbci,
			  const char *tname,
			  const char *cname,
			  GWEN_CRYPT_TOKEN **pCt);

AQHBCI_API
void AH_HBCI_ClearCryptTokenList(AH_HBCI *hbci);
#endif


#endif /* GWHBCI_HBCI_L_H */



