/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBGETSTANDINGORDERS_P_H
#define AH_JOBGETSTANDINGORDERS_P_H


#include "jobgetstandingorders_l.h"
#include <gwenhywfar/db.h>


typedef struct AH_JOB_GETSTANDINGORDERS AH_JOB_GETSTANDINGORDERS;
struct AH_JOB_GETSTANDINGORDERS {
  int dummy;
};
static void GWENHYWFAR_CB AH_Job_GetStandingOrders_FreeData(void *bp, void *p);

static int AH_Job_GetStandingOrders_Process(AH_JOB *j,
					    AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_GetStandingOrders_Exchange(AH_JOB *j, AB_JOB *bj,
					     AH_JOB_EXCHANGE_MODE m,
					     AB_IMEXPORTER_CONTEXT *ctx);


#endif /* AH_JOBGETSTANDINGORDERS_P_H */


