/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_PROVIDER_ACCSPEC_H
#define AH_PROVIDER_ACCSPEC_H

#include <aqbanking/backendsupport/provider.h>
#include <aqbanking/backendsupport/user.h>
#include <aqbanking/backendsupport/account.h>
#include <aqbanking/types/account_spec.h>


int AH_Provider_UpdateAccountSpec(AB_PROVIDER *pro, AB_ACCOUNT_SPEC *as, int doLock);

AB_ACCOUNT_SPEC *AH_Provider_CreateAccountSpecWithUserAndAccount(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a);
int AH_Provider_CreateAndWriteAccountSpecWithUserAndAccount(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a);


#endif

