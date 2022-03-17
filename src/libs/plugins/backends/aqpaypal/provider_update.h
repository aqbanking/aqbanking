/***************************************************************************
    begin       : Sun Dec 02 2018
    copyright   : (C) 2022 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQPAYPAL_PROVIDER_UPDATE_H
#define AQPAYPAL_PROVIDER_UPDATE_H


#include <aqbanking/backendsupport/provider.h>

#include <inttypes.h>


int APY_Provider_UpdatePreInit(AB_PROVIDER *pro, uint32_t lastVersion, uint32_t currentVersion);

int APY_Provider_UpdatePostInit(AB_PROVIDER *pro, uint32_t lastVersion, uint32_t currentVersion);



#endif

