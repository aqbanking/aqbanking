/***************************************************************************
    begin       : Tue Dec 31 2013
    copyright   : (C) 2018 by Martin Preuss
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
};
static void GWENHYWFAR_CB AH_Job_TransferBase_FreeData(void *bp, void *p);

static int AH_Job_TransferBase_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);

static int AH_Job_TransferBase_HandleResults(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);

/**
 * Set given status on all transfers and add copies of them to the given context.
 */
static void AH_Job_TransferBase_SetStatusOnTransfersAndAddToCtx(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx, AB_TRANSACTION_STATUS status);



#endif /* AH_JOBTRANSFERBASE_P_H */



