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


int AH_Account_Extend(AB_ACCOUNT *a, AB_PROVIDER *pro,
                      AB_PROVIDER_EXTEND_MODE um,
                      GWEN_DB_NODE *db);

AB_ACCOUNT *AH_Account_new(AB_BANKING *ab, AB_PROVIDER *pro);

AB_ACCOUNT *AH_Account_fromDb(AB_BANKING *ab, AB_PROVIDER *pro, GWEN_DB_NODE *db);
int AH_Account_toDb(const AB_ACCOUNT *a, GWEN_DB_NODE *db);

int AH_Account_ReadDb(AB_ACCOUNT *a, GWEN_DB_NODE *db);


AH_HBCI *AH_Account_GetHbci(const AB_ACCOUNT *a);


GWEN_DB_NODE *AH_Account_GetDbTempUpd(const AB_ACCOUNT *a);
void AH_Account_SetDbTempUpd(AB_ACCOUNT *a, GWEN_DB_NODE *db);


#endif /* AH_ACCOUNT_L_H */


