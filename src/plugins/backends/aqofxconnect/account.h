/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AO_ACCOUNT_H
#define AO_ACCOUNT_H

#include <aqbanking/account.h>
#include <aqofxconnect/provider.h>


#ifdef __cplusplus
extern "C" {
#endif

AQOFXCONNECT_API AB_ACCOUNT *AO_Account_new(AB_PROVIDER *pro);


AQOFXCONNECT_API int AO_Account_GetMaxPurposeLines(const AB_ACCOUNT *a);
AQOFXCONNECT_API void AO_Account_SetMaxPurposeLines(AB_ACCOUNT *a, int i);

AQOFXCONNECT_API int AO_Account_GetDebitAllowed(const AB_ACCOUNT *a);
AQOFXCONNECT_API void AO_Account_SetDebitAllowed(AB_ACCOUNT *a, int i);



#ifdef __cplusplus
}
#endif


#endif
