/***************************************************************************
    begin       : Tue Dec 31 2013
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSEPADEBITDATEDMULTICREATE_P_H
#define AH_JOBSEPADEBITDATEDMULTICREATE_P_H


#include "jobsepadebitdatedmulticreate_l.h"
#include <aqbanking/types/transaction.h>

#include <gwenhywfar/db.h>



typedef struct AH_JOB_CREATESEPAMULTIDEBIT AH_JOB_CREATESEPAMULTIDEBIT;
struct AH_JOB_CREATESEPAMULTIDEBIT {
  char *fiid;

  int sumFieldNeeded;
  int singleBookingAllowed;

  AB_VALUE *sumValues;
  char *localIban;

};


#endif /* AH_JOBSEPADEBITDATEDMULTICREATE_P_H */



