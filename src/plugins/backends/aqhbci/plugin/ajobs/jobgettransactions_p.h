/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBGETTRANSACTIONS_P_H
#define AH_JOBGETTRANSACTIONS_P_H


#include "jobgettransactions_l.h"
#include <gwenhywfar/db.h>


typedef struct AH_JOB_GETTRANSACTIONS AH_JOB_GETTRANSACTIONS;
struct AH_JOB_GETTRANSACTIONS {
  int dummy;
};
static void GWENHYWFAR_CB AH_Job_GetTransactions_FreeData(void *bp, void *p);

static int AH_Job_GetTransactions_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_GetTransactionsCreditCard_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);

static int AH_Job_GetTransactions_GetLimits(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits);
static int AH_Job_GetTransactions_HandleCommand(AH_JOB *j, const AB_TRANSACTION *t);


static int
  AH_Job_GetTransactions__ReadTransactions(AH_JOB *j,
                                           AB_IMEXPORTER_ACCOUNTINFO *ai,
                                           const char *docType,
                                           int noted,
					   GWEN_BUFFER *buf);



#endif /* AH_JOBGETTRANSACTIONS_P_H */


