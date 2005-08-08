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

#ifndef AD_PROVIDER_H
#define AD_PROVIDER_H


#include <aqbanking/banking_be.h>
#include <aqbanking/provider_be.h>


#define AQDTAUS_LOGDOMAIN "aqdtaus"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AD_PROVIDER AD_PROVIDER;


AB_PROVIDER *AD_Provider_new(AB_BANKING *ab);

AB_ACCOUNT_LIST2 *AD_Provider_GetAccounts(AB_PROVIDER *pro);

void AD_Provider_AddAccount(AB_PROVIDER *pro, AB_ACCOUNT *a);
void AD_Provider_RemoveAccount(AB_PROVIDER *pro, AB_ACCOUNT *a);
int AD_Provider_HasAccount(AB_PROVIDER *pro,
                           const char *bankCode,
                           const char *accountNumber);


#ifdef __cplusplus
}
#endif


#endif

