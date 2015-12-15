/***************************************************************************
 begin       : Wed Jan 15 2014
 copyright   : (C) 2014 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSEPASTANDINGORDERCREATE_L_H
#define AH_JOBSEPASTANDINGORDERCREATE_L_H


#include "accountjob_l.h"


AH_JOB *AH_Job_SepaStandingOrderCreate_new(AB_USER *u, AB_ACCOUNT *account);


int AH_Job_SepaStandingOrderCreate_ExchangeParams(AH_JOB *j, AB_JOB *bj, AB_IMEXPORTER_CONTEXT *ctx);
int AH_Job_SepaStandingOrderCreate_ExchangeArgs(AH_JOB *j, AB_JOB *bj, AB_IMEXPORTER_CONTEXT *ctx);

int AH_Job_SepaStandingOrderCreate_Prepare(AH_JOB *j);
int AH_Job_SepaStandingOrderCreate_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);


#endif /* AH_JOBSEPASTANDINGORDERCREATE_L_H */


