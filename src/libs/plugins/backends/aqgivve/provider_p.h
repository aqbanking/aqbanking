/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AG_PROVIDER_P_H
#define AG_PROVIDER_P_H

#include "provider.h"
#include <aqbanking/banking.h>
#include <aqbanking/backendsupport/providerqueue.h>
#include <aqbanking/backendsupport/user.h>
#include <aqbanking/backendsupport/account.h>





struct AG_PROVIDER {
  int dummy;
};



static int AG_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int AG_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);

static int AG_Provider_SendCommands(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_IMEXPORTER_CONTEXT *ctx);
int AG_Provider_UpdateAccountSpec(AB_PROVIDER *pro, AB_ACCOUNT_SPEC *as, int doLock);

void AG_Provider_AddTransactionLimit(int limit, AB_TRANSACTION_LIMITS_LIST *tll);
int AG_Provider_ExecGetBal(AB_PROVIDER *pro, AB_IMEXPORTER_ACCOUNTINFO *ai, AB_ACCOUNT *account, char *token);
int AG_Provider_ExecGetTrans(AB_PROVIDER *pro, AB_IMEXPORTER_ACCOUNTINFO *ai, AB_ACCOUNT *account, AB_TRANSACTION *j, char *token);

#endif

