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

#include <aqdtaus/aqdtaus.h>
#include <aqbanking/account_be.h>

#ifdef __cplusplus
extern "C" {
#endif

AQDTAUS_API int AD_Account_GetMaxTransfersPerJob(const AB_ACCOUNT *acc);
AQDTAUS_API void AD_Account_SetMaxTransfersPerJob(AB_ACCOUNT *acc, int i);

AQDTAUS_API int AD_Account_GetMaxPurposeLines(const AB_ACCOUNT *acc);
AQDTAUS_API void AD_Account_SetMaxPurposeLines(AB_ACCOUNT *acc, int i);

AQDTAUS_API int AD_Account_GetDebitAllowed(const AB_ACCOUNT *acc);
AQDTAUS_API void AD_Account_SetDebitAllowed(AB_ACCOUNT *acc, int i);

AQDTAUS_API int AD_Account_GetMountAllowed(const AB_ACCOUNT *acc);
AQDTAUS_API void AD_Account_SetMountAllowed(AB_ACCOUNT *acc, int i);

AQDTAUS_API const char *AD_Account_GetMountCommand(const AB_ACCOUNT *acc);
AQDTAUS_API void AD_Account_SetMountCommand(AB_ACCOUNT *acc, const char *s);

AQDTAUS_API const char *AD_Account_GetUnmountCommand(const AB_ACCOUNT *acc);
AQDTAUS_API void AD_Account_SetUnmountCommand(AB_ACCOUNT *acc, const char *s);


AQDTAUS_API const char *AD_Account_GetFolder(const AB_ACCOUNT *acc);
AQDTAUS_API void AD_Account_SetFolder(AB_ACCOUNT *acc, const char *s);

AQDTAUS_API int AD_Account_GetUseDisc(const AB_ACCOUNT *acc);
AQDTAUS_API void AD_Account_SetUseDisc(AB_ACCOUNT *acc, int i);

AQDTAUS_API int AD_Account_GetPrintAllTransactions(const AB_ACCOUNT *acc);
AQDTAUS_API void AD_Account_SetPrintAllTransactions(AB_ACCOUNT *acc, int b);


#ifdef __cplusplus
}
#endif


#endif
