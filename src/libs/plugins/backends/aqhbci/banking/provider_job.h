/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_PROVIDER_JOB_H
#define AH_PROVIDER_JOB_H

#include <aqbanking/backendsupport/provider.h>
#include <aqbanking/backendsupport/user.h>
#include <aqbanking/backendsupport/account.h>

#include "aqhbci/joblayer/job_l.h"
#include "aqhbci/applayer/outbox_l.h"



int AH_Provider_CreateHbciJob(AB_PROVIDER *pro, AB_USER *mu, AB_ACCOUNT *ma, int cmd, AH_JOB **pHbciJob);
int AH_Provider_GetMultiHbciJob(AB_PROVIDER *pro,
                                AH_OUTBOX *outbox,
                                AB_USER *mu,
                                AB_ACCOUNT *ma,
                                int cmd,
                                AH_JOB **pHbciJob);


#endif

