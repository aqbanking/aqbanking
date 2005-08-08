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

#ifndef AG_PROVIDER_H
#define AG_PROVIDER_H


#include <aqbanking/banking_be.h>
#include <aqbanking/provider_be.h>
#include <chipcard2-client/client/card.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct AG_PROVIDER AG_PROVIDER;


AB_PROVIDER *AG_Provider_new(AB_BANKING *ab);

AB_ACCOUNT_LIST2 *AG_Provider_GetAccounts(AB_PROVIDER *pro);

void AG_Provider_AddAccount(AB_PROVIDER *pro, AB_ACCOUNT *a);
void AG_Provider_RemoveAccount(AB_PROVIDER *pro, AB_ACCOUNT *a);
int AG_Provider_HasAccount(AB_PROVIDER *pro,
                           const char *bankCode,
                           const char *accountNumber);
LC_CARD *AG_Provider_MountCard(AB_PROVIDER *pro, AB_ACCOUNT *acc);


#ifdef __cplusplus
}
#endif


#endif

