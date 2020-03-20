/***************************************************************************
 begin       : Mon Jan 13 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AO_V1_R_ACCOUNTS_H
#define AO_V1_R_ACCOUNTS_H


/* plugin headers */
#include <aqofxconnect/aqofxconnect.h>

/* aqbanking headers */
#include <aqbanking/backendsupport/provider.h>
#include <aqbanking/backendsupport/user.h>
#include <aqbanking/types/imexporter_context.h>




int AO_V1_RequestAccounts(AB_PROVIDER *pro, AB_USER *u, AB_IMEXPORTER_CONTEXT *ctx);



#endif

