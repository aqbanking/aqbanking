/***************************************************************************
    begin       : Fri Feb 01 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBGETSYSID_L_H
#define AH_JOBGETSYSID_L_H


#include "aqhbci_l.h"
#include "job_l.h"


AH_JOB *AH_Job_GetSysId_new(AB_PROVIDER *pro, AB_USER *u);

const char *AH_Job_GetSysId_GetSysId(AH_JOB *j);



#endif

