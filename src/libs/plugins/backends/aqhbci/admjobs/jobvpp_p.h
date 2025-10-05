/***************************************************************************
    begin       : Fri Oct 3 2025
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_JOBVPP_P_H
#define AH_JOBVPP_P_H

#include "jobvpp.h"

#include "aqhbci/aqhbci_l.h"
#include "aqhbci/joblayer/job_l.h"


typedef struct AH_JOB_VPP AH_JOB_VPP;
struct AH_JOB_VPP {
  char *pollingId;
  char *paymentStatusFormat;
  char *vopMsg;

  uint8_t *ptrVopId;
  unsigned int lenVopId;
};





#endif

