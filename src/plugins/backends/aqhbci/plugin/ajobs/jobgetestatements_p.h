/***************************************************************************
 begin       : Tue Apr 03 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOB_GETESTATEMENTS_P_H
#define AH_JOB_GETESTATEMENTS_P_H


#include "jobgetestatements_l.h"


static int AH_Job_GetEStatements_Prepare(AH_JOB *j);
static int AH_Job_GetEStatements_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_GetEStatements_Exchange(AH_JOB *j, AB_JOB *bj,
                                          AH_JOB_EXCHANGE_MODE m,
                                          AB_IMEXPORTER_CONTEXT *ctx);





#endif /* AH_JOB_GETESTATEMENTS_P_H */


