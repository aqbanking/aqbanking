/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_ACCOUNT_L_H
#define AH_ACCOUNT_L_H


#include "account.h"
#include "hbci_l.h"
#include <aqbanking/provider_be.h>


AH_HBCI *AH_Account_GetHbci(const AB_ACCOUNT *a);


GWEN_DB_NODE *AH_Account_GetDbTempUpd(const AB_ACCOUNT *a);
void AH_Account_SetDbTempUpd(AB_ACCOUNT *a, GWEN_DB_NODE *db);


#endif /* AH_ACCOUNT_L_H */


