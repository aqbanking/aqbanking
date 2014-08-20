/***************************************************************************
    begin       : Tue Dec 31 2013
    copyright   : (C) 2004-2013 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSEPADEBITDATEDMULTICREATE_P_H
#define AH_JOBSEPADEBITDATEDMULTICREATE_P_H


#include "jobsepadebitdatedmulticreate_l.h"
#include <aqbanking/transaction.h>
#include <gwenhywfar/db.h>



typedef struct AH_JOB_CREATESEPAMULTIDEBIT AH_JOB_CREATESEPAMULTIDEBIT;
struct AH_JOB_CREATESEPAMULTIDEBIT {
  char *fiid;

  int sumFieldNeeded;
  int singleBookingAllowed;

  AB_VALUE *sumValues;
  char *localIban;

};
static void GWENHYWFAR_CB AH_Job_SepaDebitDatedMultiCreate_FreeData(void *bp, void *p);

static int AH_Job_SepaDebitDatedMultiCreate_ExchangeParams(AH_JOB *j, AB_JOB *bj,
                                                           AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_SepaDebitDatedMultiCreate_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);

static int AH_Job_SepaDebitDatedMultiCreate_Prepare(AH_JOB *j);


#endif /* AH_JOBSEPADEBITDATEDMULTICREATE_P_H */



