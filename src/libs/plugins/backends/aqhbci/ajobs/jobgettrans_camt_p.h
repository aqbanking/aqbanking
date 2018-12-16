/***************************************************************************
    begin       : Sat Dec 15 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBGETTRANSACTIONS_CAMT_P_H
#define AH_JOBGETTRANSACTIONS_CAMT_P_H


#include "jobgettransactions_l.h"
#include <gwenhywfar/db.h>


typedef struct AH_JOB_GETTRANS_CAMT AH_JOB_GETTRANS_CAMT;
struct AH_JOB_GETTRANS_CAMT {
  int dummy;
};
static void GWENHYWFAR_CB AH_Job_GetTransactionsCAMT_FreeData(void *bp, void *p);

static int AH_Job_GetTransactionsCAMT_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);

static int AH_Job_GetTransactionsCAMT_GetLimits(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits);
static int AH_Job_GetTransactionsCAMT_HandleCommand(AH_JOB *j, const AB_TRANSACTION *t);


static int AH_Job_GetTransCAMT__ReadTransactions(AH_JOB *j,
                                                 AB_IMEXPORTER_ACCOUNTINFO *ai,
                                                 const char *docType,
                                                 int ty,
                                                 const uint8_t *ptr,
                                                 uint32_t len);

#endif /* AH_JOBGETTRANSACTIONS_CAMT_P_H */


