/***************************************************************************
    begin       : Sun Dec 02 2018
    copyright   : (C) 2022 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQPAYPAL_PROVIDER_ACCSPEC_H
#define AQPAYPAL_PROVIDER_ACCSPEC_H


#include <aqbanking/backendsupport/provider.h>
#include <aqbanking/types/account_spec.h>

#include <inttypes.h>


int APY_Provider_UpdateAccountSpec(AB_PROVIDER *pro, AB_ACCOUNT_SPEC *as, int doLock);


#endif

