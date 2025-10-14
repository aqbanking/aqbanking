/***************************************************************************
    begin       : Fri Oct 3 2025
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBVPP_H
#define AH_JOBVPP_H


#include "aqhbci/aqhbci_l.h"
#include "aqhbci/joblayer/job_l.h"



AH_JOB *AH_Job_VPP_new(AB_PROVIDER *pro, AB_USER *u, int jobVersion);

const char *AH_Job_VPP_GetVopId(const AH_JOB *j);
const char *AH_Job_VPP_GetVopMsg(const AH_JOB *j);

int AH_Job_VPP_IsNeededForCode(const AH_JOB *j, const char *code);

const uint8_t *AH_Job_VPP_GetPtrVopId(const AH_JOB *j);
unsigned int AH_Job_VPP_GetLenVopId(const AH_JOB *j);


#endif

