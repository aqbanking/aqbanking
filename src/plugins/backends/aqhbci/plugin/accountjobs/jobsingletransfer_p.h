/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2011 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSINGLETRANSFER_P_H
#define AH_JOBSINGLETRANSFER_P_H


#include "jobsingletransfer_l.h"
#include <gwenhywfar/db.h>


typedef struct AH_JOB_SINGLETRANSFER AH_JOB_SINGLETRANSFER;
struct AH_JOB_SINGLETRANSFER {
  AB_JOB_TYPE jobType;
  char *fiid;
  char *oldFiid;
  AB_TRANSACTION *validatedTransaction;
};
static void GWENHYWFAR_CB AH_Job_SingleTransfer_FreeData(void *bp, void *p);
static int AH_Job_SingleTransfer_Process(AH_JOB *j,
					 AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_SingleTransfer_Exchange(AH_JOB *j, AB_JOB *bj,
					  AH_JOB_EXCHANGE_MODE m,
					  AB_IMEXPORTER_CONTEXT *ctx);


static int AH_Job_SingleTransfer__ValidateTransfer(AB_JOB *bj,
                                                   AH_JOB *mj,
                                                   AB_TRANSACTION *t);

static AH_JOB *AH_Job_SingleTransferBase_new(AB_USER *u,
                                             AB_ACCOUNT *account,
                                             AB_JOB_TYPE jobType);

static int AH_Job_SingleTransfer_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);

#endif /* AH_JOBSINGLETRANSFER_P_H */


