/***************************************************************************
    begin       : Tue Dec 31 2013
    copyright   : (C) 2004-2013 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSEPADEBITDATEDSINGLECREATE_P_H
#define AH_JOBSEPADEBITDATEDSINGLECREATE_P_H


#include "jobsepadebitdatedsinglecreate_l.h"
#include <gwenhywfar/db.h>


typedef struct AH_JOB_CREATESEPASINGLEDEBIT AH_JOB_CREATESEPASINGLEDEBIT;
struct AH_JOB_CREATESEPASINGLEDEBIT {
  char *fiid;
  AB_TRANSACTION *validatedTransaction;
};
static void GWENHYWFAR_CB AH_Job_SepaDebitDatedSingleCreate_FreeData(void *bp, void *p);

static int AH_Job_SepaDebitDatedSingleCreate_ExchangeParams(AH_JOB *j, AB_JOB *bj,
                                                            AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_SepaDebitDatedSingleCreate_ExchangeArgs(AH_JOB *j, AB_JOB *bj,
                                                          AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_SepaDebitDatedSingleCreate_ExchangeResults(AH_JOB *j, AB_JOB *bj,
                                                             AB_IMEXPORTER_CONTEXT *ctx);

static int AH_Job_SepaDebitDatedSingleCreate_Exchange(AH_JOB *j, AB_JOB *bj,
                                                          AH_JOB_EXCHANGE_MODE m,
                                                          AB_IMEXPORTER_CONTEXT *ctx);

static int AH_Job_SepaDebitDatedSingleCreate_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);

static int AH_Job_SepaDebitDatedSingleCreate_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);


#endif /* AH_JOBSEPADEBITDATEDSINGLECREATE_P_H */



