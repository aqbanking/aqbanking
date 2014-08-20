/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOB_FOREIGNXFERWH_P_H
#define AH_JOB_FOREIGNXFERWH_P_H


#include "jobforeignxferwh_l.h"
#include <gwenhywfar/db.h>


#define AH_JOBFOREIGNXFERWH_MAXTRANS  256


typedef struct AH_JOB_FOREIGNXFERWH AH_JOB_FOREIGNXFERWH;
struct AH_JOB_FOREIGNXFERWH {
  int maxTransfers;
};

static void GWENHYWFAR_CB AH_Job_ForeignTransferWH_FreeData(void *bp,
							    void *p);

static int AH_Job_ForeignTransferWH_Process(AH_JOB *j,
					    AB_IMEXPORTER_CONTEXT *ctx);

static int AH_Job_ForeignTransferWH_Exchange(AH_JOB *j, AB_JOB *bj,
					     AH_JOB_EXCHANGE_MODE m,
					     AB_IMEXPORTER_CONTEXT *ctx);




#endif /* AH_JOB_FOREIGNXFERWH_P_H */


