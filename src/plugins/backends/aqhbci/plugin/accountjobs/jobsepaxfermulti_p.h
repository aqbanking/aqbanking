/***************************************************************************
    begin       : Tue Dec 31 2013
    copyright   : (C) 2004-2013 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSEPAXFERMULTI_P_H
#define AH_JOBSEPAXFERMULTI_P_H


#include "jobsepaxfermulti_l.h"
#include <aqbanking/transaction.h>
#include <gwenhywfar/db.h>



typedef struct AH_JOB_SEPAXFERMULTI AH_JOB_SEPAXFERMULTI;
struct AH_JOB_SEPAXFERMULTI {
  int sumFieldNeeded;
  int singleBookingAllowed;

  AB_VALUE *sumValues;
  char *localIban;

};
static void GWENHYWFAR_CB AH_Job_SepaTransferMulti_FreeData(void *bp, void *p);

static int AH_Job_SepaTransferMulti_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);

static int AH_Job_SepaTransferMulti_Prepare(AH_JOB *j);


#endif /* AH_JOBSEPAXFERMULTI_P_H */



