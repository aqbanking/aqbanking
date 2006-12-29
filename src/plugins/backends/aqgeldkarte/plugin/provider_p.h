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

#include "card_l.h"

#include "provider.h"
#include <aqbanking/transaction.h>

#include <chipcard3/client/client.h>


typedef struct AG_PROVIDER AG_PROVIDER;
struct AG_PROVIDER {
  AB_JOB_LIST2 *bankingJobs;
  AG_CARD_LIST *cards;
  GWEN_DB_NODE *dbConfig;
  LC_CLIENT *chipcardClient;
};

static void AG_Provider_FreeData(void *bp, void *p);


static int AG_Provider_GetBalance(AB_PROVIDER *pro,
                                  AB_IMEXPORTER_CONTEXT *ctx,
                                  LC_CARD *gc,
                                  AB_JOB *bj);
static int AG_Provider_GetTransactions(AB_PROVIDER *pro,
                                       AB_IMEXPORTER_CONTEXT *ctx,
                                       LC_CARD *gc,
                                       AB_JOB *bj);

static int AG_Provider_ProcessCard(AB_PROVIDER *pro,
                                   AB_IMEXPORTER_CONTEXT *ctx,
                                   AG_CARD *card);


static int AG_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int AG_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int AG_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j);
static int AG_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j);
static int AG_Provider_Execute(AB_PROVIDER *pro, AB_IMEXPORTER_CONTEXT *ctx);
static int AG_Provider_ResetQueue(AB_PROVIDER *pro);
static int AG_Provider_ExtendAccount(AB_PROVIDER *pro, AB_ACCOUNT *a,
                                     AB_PROVIDER_EXTEND_MODE em);


#endif

