/***************************************************************************
    begin       : Tue Dec 31 2013
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSEPACOR1DATEDMULTICREATE_P_H
#define AH_JOBSEPACOR1DATEDMULTICREATE_P_H


#include "jobsepacor1datedmulticreate_l.h"

#include <aqbanking/types/transaction.h>

#include <gwenhywfar/db.h>



typedef struct AH_JOB_CREATESEPAMULTICOR1 AH_JOB_CREATESEPAMULTICOR1;
struct AH_JOB_CREATESEPAMULTICOR1 {
  char *fiid;

  int sumFieldNeeded;
  int singleBookingAllowed;

  AB_VALUE *sumValues;
  char *localIban;

};


#endif /* AH_JOBSEPACOR1DATEDMULTICREATE_P_H */



