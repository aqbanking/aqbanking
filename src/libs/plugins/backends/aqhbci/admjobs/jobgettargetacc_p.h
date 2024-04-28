/***************************************************************************
 begin       : Tue Oct 12 2021
 copyright   : (C) 2021 by Stefan Bayer, Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBGETTARGETACC_P_H
#define AH_JOBGETTARGETACC_P_H


#include "aqhbci_l.h"
#include "jobgettargetacc_l.h"


typedef struct AH_JOB_GETTARGETACC AH_JOB_GETTARGETACC;
struct AH_JOB_GETTARGETACC {
  AB_ACCOUNT *account;
  int scanned;
};



#endif

