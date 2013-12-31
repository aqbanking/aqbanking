/***************************************************************************
    begin       : Tue Dec 31 2013
    copyright   : (C) 2004-2013 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSEPADEBITSINGLE_P_H
#define AH_JOBSEPADEBITSINGLE_P_H


#include "jobsepadebitsingle_l.h"

#include <gwenhywfar/db.h>


typedef struct AH_JOB_SEPADEBITSINGLE AH_JOB_SEPADEBITSINGLE;
struct AH_JOB_SEPADEBITSINGLE {
  char *fiid;
  AB_TRANSACTION *validatedTransaction;
};
static void GWENHYWFAR_CB AH_Job_SepaDebitSingle_FreeData(void *bp, void *p);

static int AH_Job_SepaDebitSingle_ExchangeParams(AH_JOB *j, AB_JOB *bj,
                                                    AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_SepaDebitSingle_ExchangeArgs(AH_JOB *j, AB_JOB *bj,
                                                  AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_SepaDebitSingle_ExchangeResults(AH_JOB *j, AB_JOB *bj,
                                                     AB_IMEXPORTER_CONTEXT *ctx);

static int AH_Job_SepaDebitSingle_Exchange(AH_JOB *j, AB_JOB *bj,
                                              AH_JOB_EXCHANGE_MODE m,
                                              AB_IMEXPORTER_CONTEXT *ctx);

static int AH_Job_SepaDebitSingle_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);

static int AH_Job_SepaDebitSingle_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);


#endif /* AH_JOBSEPADEBITSINGLE_P_H */



