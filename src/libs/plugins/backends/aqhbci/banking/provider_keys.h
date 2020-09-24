/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_PROVIDER_KEYS_H
#define AH_PROVIDER_KEYS_H


#include <aqbanking/backendsupport/provider.h>
#include <aqbanking/backendsupport/user.h>



int AH_Provider_CreateKeys(AB_PROVIDER *pro, AB_USER *u, int nounmount);


#endif

