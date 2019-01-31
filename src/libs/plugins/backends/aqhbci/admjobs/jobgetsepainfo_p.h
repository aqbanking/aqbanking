/***************************************************************************
    begin       : Thu Jan 31 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBGETSEPAINFO_P_H
#define AH_JOBGETSEPAINFO_P_H


#include "aqhbci_l.h"
#include "jobgetsepainfo_l.h"


typedef struct AH_JOB_GETACCSEPAINFO AH_JOB_GETACCSEPAINFO;
struct AH_JOB_GETACCSEPAINFO {
  AB_ACCOUNT *account;
  int scanned;
};
static void GWENHYWFAR_CB AH_Job_GetAccountSepaInfo_FreeData(void *bp, void *p);

static int AH_Job_GetAccountSepaInfo_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);



#endif

