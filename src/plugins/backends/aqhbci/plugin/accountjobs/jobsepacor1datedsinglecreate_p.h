/***************************************************************************
    begin       : Tue Dec 31 2013
    copyright   : (C) 2004-2013 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSEPACOR1DEBITDATEDSINGLECREATE_P_H
#define AH_JOBSEPACOR1DEBITDATEDSINGLECREATE_P_H


#include "jobsepacor1datedsinglecreate_l.h"
#include <gwenhywfar/db.h>



static int AH_Job_SepaCor1DebitDatedSingleCreate_ExchangeParams(AH_JOB *j, AB_JOB *bj,
                                                                AB_IMEXPORTER_CONTEXT *ctx);

static int AH_Job_SepaCor1DebitDatedSingleCreate_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);
static int AH_Job_SepaCor1DebitDatedSingleCreate_Prepare(AH_JOB *j);



#endif /* AH_JOBSEPACOR1DEBITDATEDSINGLECREATE_P_H */



