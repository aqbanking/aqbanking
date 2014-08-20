/***************************************************************************
 begin       : Wed Jan 15 2014
 copyright   : (C) 2014 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSEPASTANDINGORDERCREATE_P_H
#define AH_JOBSEPASTANDINGORDERCREATE_P_H


#include "jobsepastandingordercreate_l.h"

#include <gwenhywfar/db.h>


static int AH_Job_SepaStandingOrderCreate_ExchangeParams(AH_JOB *j, AB_JOB *bj, AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_SepaStandingOrderCreate_ExchangeArgs(AH_JOB *j, AB_JOB *bj, AB_IMEXPORTER_CONTEXT *ctx);

static int AH_Job_SepaStandingOrderCreate_Prepare(AH_JOB *j);
static int AH_Job_SepaStandingOrderCreate_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);


#endif /* AH_JOBSEPASTANDINGORDERCREATE_P_H */



