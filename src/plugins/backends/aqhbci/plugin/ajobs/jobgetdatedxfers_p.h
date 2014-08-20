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


#ifndef AH_JOBGETDATEDTRANSFERS_P_H
#define AH_JOBGETDATEDTRANSFERS_P_H


#include "jobgetdatedxfers_l.h"
#include <gwenhywfar/db.h>


typedef struct AH_JOB_GETDATEDTRANSFERS AH_JOB_GETDATEDTRANSFERS;
struct AH_JOB_GETDATEDTRANSFERS {
  int dummy;
};
static void GWENHYWFAR_CB AH_Job_GetDatedTransfers_FreeData(void *bp, void *p);

static int AH_Job_GetDatedTransfers_Process(AH_JOB *j,
					    AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_GetDatedTransfers_Exchange(AH_JOB *j, AB_JOB *bj,
					     AH_JOB_EXCHANGE_MODE m,
					     AB_IMEXPORTER_CONTEXT *ctx);


#endif /* AH_JOBGETDATEDTRANSFERS_P_H */


