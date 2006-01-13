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

#include <aqhbci/aqhbci.h> /* for AQHBCI_API */
#include <aqbanking/provider.h>

#ifdef __cplusplus
extern "C" {
#endif


#define AH_BANK_FLAGS_PREFER_SINGLE_TRANSFER  0x00000001
#define AH_BANK_FLAGS_PREFER_SINGLE_DEBITNOTE 0x00000002

AQHBCI_API
void AH_Account_Flags_toDb(GWEN_DB_NODE *db, const char *name,
                           GWEN_TYPE_UINT32 flags);

AQHBCI_API
GWEN_TYPE_UINT32 AH_Account_Flags_fromDb(GWEN_DB_NODE *db, const char *name);



AQHBCI_API
const char *AH_Account_GetSuffix(const AB_ACCOUNT *a);
AQHBCI_API
void AH_Account_SetSuffix(AB_ACCOUNT *a, const char *s);

AQHBCI_API
GWEN_TYPE_UINT32 AH_Account_GetFlags(const AB_ACCOUNT *a);

AQHBCI_API
void AH_Account_SetFlags(AB_ACCOUNT *a, GWEN_TYPE_UINT32 flags);

AQHBCI_API
void AH_Account_AddFlags(AB_ACCOUNT *a, GWEN_TYPE_UINT32 flags);

AQHBCI_API
void AH_Account_SubFlags(AB_ACCOUNT *a, GWEN_TYPE_UINT32 flags);


#ifdef __cplusplus
}
#endif





#endif /* AH_ACCOUNT_H */


