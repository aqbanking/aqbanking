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

#ifndef AO_ACCOUNT_H
#define AO_ACCOUNT_H

#include <aqbanking/account_be.h>
#include <aqofxconnect/provider.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct AO_ACCOUNT AO_ACCOUNT;


AB_ACCOUNT *AO_Account_new(AB_BANKING *ab,
                           AB_PROVIDER *pro,
                           const char *idForProvider);


AB_ACCOUNT *AO_Account_fromDb(AB_BANKING *ab,
                              GWEN_DB_NODE *db);

int AO_Account_toDb(const AB_ACCOUNT *acc, GWEN_DB_NODE *db);


int AO_Account_GetMaxPurposeLines(const AB_ACCOUNT *acc);
void AO_Account_SetMaxPurposeLines(AB_ACCOUNT *acc, int i);

int AO_Account_GetDebitAllowed(const AB_ACCOUNT *acc);
void AO_Account_SetDebitAllowed(AB_ACCOUNT *acc, int i);

const char *AO_Account_GetUserId(const AB_ACCOUNT *acc);
void AO_Account_SetUserId(AB_ACCOUNT *acc, const char *s);




#ifdef __cplusplus
}
#endif


#endif
