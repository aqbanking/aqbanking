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

#ifndef AY_ACCOUNT_H
#define AY_ACCOUNT_H

#include <aqyellownet/aqyellownet.h>
#include <aqbanking/account.h>


#ifdef __cplusplus
extern "C" {
#endif


AQYELLOWNET_API int AY_Account_GetMaxPurposeLines(const AB_ACCOUNT *a);
AQYELLOWNET_API void AY_Account_SetMaxPurposeLines(AB_ACCOUNT *a, int i);

AQYELLOWNET_API int AY_Account_GetDebitAllowed(const AB_ACCOUNT *a);
AQYELLOWNET_API void AY_Account_SetDebitAllowed(AB_ACCOUNT *a, int i);


#ifdef __cplusplus
}
#endif


#endif
