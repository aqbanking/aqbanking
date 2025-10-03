/***************************************************************************
    begin       : Fri Oct 3 2025
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBVPP_L_H
#define AH_JOBVPP_L_H


#include "aqhbci/aqhbci_l.h"
#include "aqhbci/joblayer/job_l.h"



AH_JOB *AH_Job_Vpp_new(AB_PROVIDER *pro, AB_USER *u, int process, int jobVersion);



#endif

