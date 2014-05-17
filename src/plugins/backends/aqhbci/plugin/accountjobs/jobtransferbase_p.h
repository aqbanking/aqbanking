/***************************************************************************
    begin       : Tue Dec 31 2013
    copyright   : (C) 2004-2013 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBTRANSFERBASE_P_H
#define AH_JOBTRANSFERBASE_P_H


#include "jobtransferbase_l.h"
#include <gwenhywfar/db.h>


typedef struct AH_JOB_TRANSFERBASE AH_JOB_TRANSFERBASE;
struct AH_JOB_TRANSFERBASE {
  AB_TRANSACTION_TYPE transactionType;
  AB_TRANSACTION_SUBTYPE transactionSubType;
  char *fiid;
  AH_JOB_TRANSFERBASE_EXCHANGE_FN exchangeParamsFn;
  AH_JOB_TRANSFERBASE_EXCHANGE_FN exchangeArgsFn;
  AH_JOB_TRANSFERBASE_EXCHANGE_FN exchangeResultsFn;
};
static void GWENHYWFAR_CB AH_Job_TransferBase_FreeData(void *bp, void *p);

static int AH_Job_TransferBase_ExchangeResults(AH_JOB *j, AB_JOB *bj,
                                           AB_IMEXPORTER_CONTEXT *ctx);

static int AH_Job_TransferBase_Exchange(AH_JOB *j, AB_JOB *bj,
                                    AH_JOB_EXCHANGE_MODE m,
                                    AB_IMEXPORTER_CONTEXT *ctx);

static int AH_Job_TransferBase_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);


#endif /* AH_JOBTRANSFERBASE_P_H */



