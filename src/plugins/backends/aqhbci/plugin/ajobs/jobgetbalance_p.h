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


#ifndef AH_JOBGETBALANCE_P_H
#define AH_JOBGETBALANCE_P_H


#include "jobgetbalance_l.h"
#include <gwenhywfar/db.h>


typedef struct AH_JOB_GETBALANCE AH_JOB_GETBALANCE;
struct AH_JOB_GETBALANCE {
  int dummy;
};
static void GWENHYWFAR_CB AH_Job_GetBalance_FreeData(void *bp, void *p);
static int AH_Job_GetBalance_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_GetBalance_Exchange(AH_JOB *j, AB_JOB *bj,
				      AH_JOB_EXCHANGE_MODE m,
				      AB_IMEXPORTER_CONTEXT *ctx);

static AB_BALANCE *AH_Job_GetBalance__ReadBalance(GWEN_DB_NODE *dbT);



#endif /* AH_JOBGETBALANCE_P_H */


