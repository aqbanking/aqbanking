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


#ifndef AH_JOBGETSTANDINGORDERS_P_H
#define AH_JOBGETSTANDINGORDERS_P_H


#include <aqhbci/jobgetstandingorders.h>
#include <gwenhywfar/db.h>


typedef struct AH_JOB_GETSTANDINGORDERS AH_JOB_GETSTANDINGORDERS;
struct AH_JOB_GETSTANDINGORDERS {
  AB_TRANSACTION_LIST2 *standingOrders;
};
void AH_Job_GetStandingOrders_FreeData(void *bp, void *p);

int AH_Job_GetStandingOrders_Process(AH_JOB *j);
int AH_Job_GetStandingOrders_Exchange(AH_JOB *j, AB_JOB *bj,
                                      AH_JOB_EXCHANGE_MODE m);


#endif /* AH_JOBGETSTANDINGORDERS_P_H */


