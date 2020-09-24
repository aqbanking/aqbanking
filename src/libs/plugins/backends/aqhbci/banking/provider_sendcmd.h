/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_PROVIDER_SENDCMD_H
#define AH_PROVIDER_SENDCMD_H

#include <aqbanking/backendsupport/provider.h>
#include <aqbanking/backendsupport/providerqueue.h>
#include <aqbanking/types/imexporter_context.h>


#include "aqhbci/joblayer/job_l.h"
#include "aqhbci/applayer/outbox_l.h"



int AH_Provider_SendCommands(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_IMEXPORTER_CONTEXT *ctx);


#endif

