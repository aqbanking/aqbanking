/***************************************************************************
    begin       : Fri Oct 3 2025
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBVPA_H
#define AH_JOBVPA_H


#include "aqhbci/aqhbci_l.h"
#include "aqhbci/joblayer/job_l.h"



AH_JOB *AH_Job_VPA_new(AB_PROVIDER *pro, AB_USER *u, int jobVersion, const char *vopId);



#endif

