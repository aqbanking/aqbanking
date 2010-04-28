/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBEUTRANSFER_P_H
#define AH_JOBEUTRANSFER_P_H


#include "jobsingletransfer_l.h"
#include <gwenhywfar/db.h>


typedef struct AH_JOB_EUTRANSFER AH_JOB_EUTRANSFER;
struct AH_JOB_EUTRANSFER {
  int isTransfer;
};
static void GWENHYWFAR_CB AH_Job_EuTransfer_FreeData(void *bp, void *p);
static int AH_Job_EuTransfer_Process(AH_JOB *j,
				     AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_EuTransfer_Exchange(AH_JOB *j, AB_JOB *bj,
				      AH_JOB_EXCHANGE_MODE m,
				      AB_IMEXPORTER_CONTEXT *ctx);


static int AH_Job_EuTransfer__ValidateTransfer(AB_JOB *bj,
                                               AH_JOB *mj,
                                               AB_TRANSACTION *t);

static AH_JOB *AH_Job_EuTransferBase_new(AB_USER *u,
                                         AB_ACCOUNT *account,
                                         int isTransfer);

#endif /* AH_JOBEUTRANSFER_P_H */


