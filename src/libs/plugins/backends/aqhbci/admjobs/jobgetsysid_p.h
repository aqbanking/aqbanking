/***************************************************************************
    begin       : Fri Feb 01 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBGETSYSID_P_H
#define AH_JOBGETSYSID_P_H


#include "jobgetsysid_l.h"


typedef struct AH_JOB_GETSYSID AH_JOB_GETSYSID;

struct AH_JOB_GETSYSID {
  char *sysId;
};

static void GWENHYWFAR_CB AH_Job_GetSysId_FreeData(void *bp, void *p);
static int AH_Job_GetSysId_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_GetSysId_NextMsg(AH_JOB *j);
static int AH_Job_GetSysId_ExtractSysId(AH_JOB *j);


#endif

