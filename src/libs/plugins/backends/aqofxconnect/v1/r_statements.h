/***************************************************************************
 begin       : Mon Jan 13 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AO_V1_R_STATEMENTS_H
#define AO_V1_R_STATEMENTS_H


/* plugin headers */
#include <aqofxconnect/aqofxconnect.h>

/* aqbanking headers */
#include <aqbanking/backendsupport/provider.h>
#include <aqbanking/backendsupport/user.h>
#include <aqbanking/backendsupport/account.h>
#include <aqbanking/types/imexporter_context.h>
#include <aqbanking/types/transaction.h>




int AO_V1_RequestStatements(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j,
                            AB_IMEXPORTER_CONTEXT *ictx);



#endif

