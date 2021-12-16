/***************************************************************************
    begin       : Thu Dec 16 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSEPAGENERICDMULTICREATE_P_H
#define AH_JOBSEPAGENERICDMULTICREATE_P_H


#include "jobsepagenericmulticreate_l.h"

#include <aqbanking/types/transaction.h>

#include <gwenhywfar/db.h>



typedef struct AH_JOB_CREATESEPAMULTIGENERIC AH_JOB_CREATESEPAMULTIGENERIC;
struct AH_JOB_CREATESEPAMULTIGENERIC {
  char *fiid;

  int sumFieldNeeded;
  int singleBookingAllowed;

  AB_VALUE *sumValues;

  char *instrumentationCode;
  int painMessageGroup;
};


#endif /* AH_JOBSEPAGENERICDMULTICREATE_P_H */



