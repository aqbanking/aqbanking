/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_PROVIDER_L_H
#define AH_PROVIDER_L_H

#include "provider.h"
#include "aqhbci/tan/tanmethod.h"
#include "aqhbci/joblayer/job_l.h"
#include "aqhbci/msglayer/hbci_l.h"


AH_HBCI *AH_Provider_GetHbci(const AB_PROVIDER *pro);


int AH_Provider_SendDtazv(AB_PROVIDER *pro,
                          AB_USER *u,
                          AB_ACCOUNT *a,
                          AB_IMEXPORTER_CONTEXT *ctx,
                          const uint8_t *dataPtr,
                          uint32_t dataLen,
                          int withProgress, int nounmount, int doLock);

int AH_Provider_WriteValueToDb(const AB_VALUE *v, GWEN_DB_NODE *dbV);


#endif

