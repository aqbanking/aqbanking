/***************************************************************************
    begin       : Thu Jan 31 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBTAN_P_H
#define AH_JOBTAN_P_H

#include "jobtan_l.h"

#include "aqhbci_l.h"
#include "job_l.h"


typedef struct AH_JOB_TAN AH_JOB_TAN;
struct AH_JOB_TAN {
  char *challenge;
  char *challengeHhd;
  char *reference;
  int tanMethod;
  char *tanMediumId;
};
static void GWENHYWFAR_CB AH_Job_Tan_FreeData(void *bp, void *p);
static int AH_Job_Tan_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);





#endif

