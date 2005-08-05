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


#ifndef AH_JOBMULTITRANSFER_P_H
#define AH_JOBMULTITRANSFER_P_H


#include "jobmultitransfer_l.h"
#include <gwenhywfar/db.h>


typedef struct AH_JOB_MULTITRANSFER AH_JOB_MULTITRANSFER;
struct AH_JOB_MULTITRANSFER {
  int isTransfer;
  int transferCount;
  int maxTransfers;
};
void AH_Job_MultiTransfer_FreeData(void *bp, void *p);
int AH_Job_MultiTransfer_Process(AH_JOB *j);
int AH_Job_MultiTransfer_Exchange(AH_JOB *j, AB_JOB *bj,
                                  AH_JOB_EXCHANGE_MODE m);


int AH_Job_MultiTransfer__ToDTA(int c);
int AH_Job_MultiTransfer__ValidateTransfer(AB_JOB *bj,
                                           AH_JOB *mj,
                                           AB_TRANSACTION *t);

AH_JOB *AH_Job_MultiTransferBase_new(AH_CUSTOMER *cu,
                                     AH_ACCOUNT *account,
                                     int isTransfer);


#endif /* AH_JOBMULTITRANSFER_P_H */


