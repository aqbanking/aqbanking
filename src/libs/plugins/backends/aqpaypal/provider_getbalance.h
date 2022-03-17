/***************************************************************************
    begin       : Sat Dec 01 2018
    copyright   : (C) 2022 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQPAYPAL_PROVIDER_GETBALANCE_H
#define AQPAYPAL_PROVIDER_GETBALANCE_H


#include <aqbanking/backendsupport/provider.h>
#include <aqbanking/backendsupport/user.h>
#include <aqbanking/types/transaction.h>
#include <aqbanking/types/imexporter_accountinfo.h>




int APY_Provider_ExecGetBal(AB_PROVIDER *pro,
                            AB_IMEXPORTER_ACCOUNTINFO *ai,
                            AB_USER *u,
                            AB_TRANSACTION *j);



#endif
