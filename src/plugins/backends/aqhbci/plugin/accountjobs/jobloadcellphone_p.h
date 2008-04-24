/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: jobgetbalance_p.h 1138 2007-01-22 15:05:39Z christian $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBLOADCELLPHONE_P_H
#define AH_JOBLOADCELLPHONE_P_H


#include "jobloadcellphone_l.h"
#include <gwenhywfar/db.h>


typedef struct AH_JOB_LOADCELLPHONE AH_JOB_LOADCELLPHONE;
struct AH_JOB_LOADCELLPHONE {
  int dummy;
};
static void GWENHYWFAR_CB AH_Job_LoadCellPhone_FreeData(void *bp, void *p);

static int AH_Job_LoadCellPhone_ExchangeParams(AH_JOB *j, AB_JOB *bj,
					       AB_IMEXPORTER_CONTEXT *ctx,
					       uint32_t guiid);
static int AH_Job_LoadCellPhone_ExchangeArgs(AH_JOB *j, AB_JOB *bj,
					     AB_IMEXPORTER_CONTEXT *ctx,
					     uint32_t guiid);

static int AH_Job_LoadCellPhone_Exchange(AH_JOB *j, AB_JOB *bj,
					 AH_JOB_EXCHANGE_MODE m,
					 AB_IMEXPORTER_CONTEXT *ctx,
					 uint32_t guiid);


#endif /* AH_JOBLOADCELLPHONE_P_H */


