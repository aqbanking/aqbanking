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

#ifndef AO_PROVIDER_H
#define AO_PROVIDER_H


#include <aqbanking/banking_be.h>
#include <aqbanking/provider_be.h>
#include <aqofxconnect/bank.h>


#define AQOFXCONNECT_LOGDOMAIN "aqofxconnect"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AO_PROVIDER AO_PROVIDER;


AB_PROVIDER *AO_Provider_new(AB_BANKING *ab);

AB_ACCOUNT_LIST2 *AO_Provider_GetAccounts(AB_PROVIDER *pro);

int AO_Provider_AddAccount(AB_PROVIDER *pro, AB_ACCOUNT *a);
int AO_Provider_RemoveAccount(AB_PROVIDER *pro, AB_ACCOUNT *a);
int AO_Provider_HasAccount(AB_PROVIDER *pro,
                           const char *country,
                           const char *bankCode,
                           const char *accountNumber);

AO_BANK_LIST *AO_Provider_GetBanks(const AB_PROVIDER *pro);
int AO_Provider_AddBank(AB_PROVIDER *pro, AO_BANK *b);

AO_BANK *AO_Provider_FindMyBank(AB_PROVIDER *pro,
                                const char *country,
                                const char *bid);
AB_ACCOUNT *AO_Provider_FindMyAccount(AB_PROVIDER *pro,
                                      const char *country,
                                      const char *bankCode,
                                      const char *accountNumber);

int AO_Provider_RequestAccounts(AB_PROVIDER *pro,
                                const char *country,
                                const char *bankId,
                                const char *userId);

#ifdef __cplusplus
}
#endif


#endif

