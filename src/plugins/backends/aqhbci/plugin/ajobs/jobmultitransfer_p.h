/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBMULTITRANSFER_P_H
#define AH_JOBMULTITRANSFER_P_H


#include "jobmultitransfer_l.h"
#include <gwenhywfar/db.h>


typedef struct AH_JOB_MULTITRANSFER AH_JOB_MULTITRANSFER;
struct AH_JOB_MULTITRANSFER {
  int isTransfer;

  AB_VALUE *sumRemoteAccountId;
  AB_VALUE *sumRemoteBankCode;
  AB_VALUE *sumValues;
};
static void GWENHYWFAR_CB AH_Job_MultiTransfer_FreeData(void *bp, void *p);
static int AH_Job_MultiTransfer_Process(AH_JOB *j,
					AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_MultiTransfer_Exchange(AH_JOB *j, AB_JOB *bj,
					 AH_JOB_EXCHANGE_MODE m,
					 AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_MultiTransfer_Prepare(AH_JOB *j);

static int AH_Job_MultiTransfer_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);


static int AH_Job_MultiTransfer__ValidateTransfer(AB_JOB *bj,
                                                  AH_JOB *mj,
                                                  AB_TRANSACTION *t);

static AH_JOB *AH_Job_MultiTransferBase_new(AB_USER *u,
                                            AB_ACCOUNT *account,
                                            int isTransfer);


#endif /* AH_JOBMULTITRANSFER_P_H */


