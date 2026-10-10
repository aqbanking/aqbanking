/***************************************************************************
    begin       : Thu Jan 31 2019
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBGETSEPAINFO_L_H
#define AH_JOBGETSEPAINFO_L_H


#include "aqhbci/aqhbci_l.h"
#include "aqhbci/joblayer/job_l.h"


AH_JOB *AH_Job_GetAccountSepaInfo_new(AB_PROVIDER *pro, AB_USER *u);



#endif

