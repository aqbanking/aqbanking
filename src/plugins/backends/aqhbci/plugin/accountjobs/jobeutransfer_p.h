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


#ifndef AH_JOBEUTRANSFER_P_H
#define AH_JOBEUTRANSFER_P_H


#include <aqhbci/jobsingletransfer.h>
#include <gwenhywfar/db.h>


typedef struct AH_JOB_EUTRANSFER AH_JOB_EUTRANSFER;
struct AH_JOB_EUTRANSFER {
  int isTransfer;
};
void AH_Job_EuTransfer_FreeData(void *bp, void *p);
int AH_Job_EuTransfer_Process(AH_JOB *j);
int AH_Job_EuTransfer_Exchange(AH_JOB *j, AB_JOB *bj,
                               AH_JOB_EXCHANGE_MODE m);


int AH_Job_EuTransfer__ToDTA(int c);
int AH_Job_EuTransfer__ValidateTransfer(AB_JOB *bj,
                                        AH_JOB *mj,
                                        AB_TRANSACTION *t);

AH_JOB *AH_Job_EuTransferBase_new(AH_CUSTOMER *cu,
                                  AH_ACCOUNT *account,
                                  int isTransfer);

#endif /* AH_JOBEUTRANSFER_P_H */


