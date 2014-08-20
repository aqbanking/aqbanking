/***************************************************************************
    begin       : Tue Dec 31 2013
    copyright   : (C) 2004-2013 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSEPADEBITDATEDSINGLECREATE_P_H
#define AH_JOBSEPADEBITDATEDSINGLECREATE_P_H


#include "jobsepadebitdatedsinglecreate_l.h"
#include <gwenhywfar/db.h>


static int AH_Job_SepaDebitDatedSingleCreate_ExchangeParams(AH_JOB *j, AB_JOB *bj,
                                                            AB_IMEXPORTER_CONTEXT *ctx);

static int AH_Job_SepaDebitDatedSingleCreate_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);
static int AH_Job_SepaDebitDatedSingleCreate_Prepare(AH_JOB *j);


#endif /* AH_JOBSEPADEBITDATEDSINGLECREATE_P_H */



