/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AG_PROVIDER_P_H
#define AG_PROVIDER_P_H

#define AG_GELDKARTE_WCB_GENERIC "AG_GELDKARTE_WCB_GENERIC"
#define AG_OUTBOX_EXECUTE_WCB_ID "AG_OUTBOX_EXECUTE_WCB_ID"

#include "card.h"

#include <aqgeldkarte/provider.h>
#include <aqbanking/transaction.h>
#include <gwenhywfar/waitcallback.h>
#include <chipcard2-client/client/client.h>


struct AG_PROVIDER {
  AB_ACCOUNT_LIST2 *accounts;
  AB_JOB_LIST2 *bankingJobs;
  AG_CARD_LIST *cards;
  GWEN_DB_NODE *dbConfig;
  LC_CLIENT *chipcardClient;
};

void AG_Provider_FreeData(void *bp, void *p);

AB_ACCOUNT *AG_Provider_FindMyAccount(AB_PROVIDER *pro,
                                      AB_ACCOUNT *acc);

int AG_Provider_GetBalance(AB_PROVIDER *pro,
                           LC_CARD *gc,
                           AB_JOB *bj);
int AG_Provider_GetTransactions(AB_PROVIDER *pro,
                                LC_CARD *gc,
                                AB_JOB *bj);

int AG_Provider_ProcessCard(AB_PROVIDER *pro, AG_CARD *card);


int AG_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
int AG_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
int AG_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j);
int AG_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j);
int AG_Provider_Execute(AB_PROVIDER *pro);
int AG_Provider_ResetQueue(AB_PROVIDER *pro);
AB_ACCOUNT_LIST2 *AG_Provider_GetAccountList(AB_PROVIDER *pro);
int AG_Provider_UpdateAccount(AB_PROVIDER *pro, AB_ACCOUNT *a);


#endif

