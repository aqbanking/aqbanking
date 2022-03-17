/***************************************************************************
    begin       : Sat May 08 2010
    copyright   : (C) 2022 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQPAYPAL_PROVIDER_SENDCMD_H
#define AQPAYPAL_PROVIDER_SENDCMD_H


#include <aqbanking/backendsupport/provider.h>
#include <aqbanking/backendsupport/providerqueue.h>
#include <aqbanking/types/imexporter_context.h>




int APY_Provider_SendCommands(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_IMEXPORTER_CONTEXT *ctx);


#endif

