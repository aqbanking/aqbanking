/***************************************************************************
    begin       : Tue Dec 31 2013
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSEPAXFERMULTI_P_H
#define AH_JOBSEPAXFERMULTI_P_H


#include "jobsepaxfermulti_l.h"

#include <aqbanking/types/transaction.h>

#include <gwenhywfar/db.h>



typedef struct AH_JOB_SEPAXFERMULTI AH_JOB_SEPAXFERMULTI;
struct AH_JOB_SEPAXFERMULTI {
  int sumFieldNeeded;
  int singleBookingAllowed;

  AB_VALUE *sumValues;
  char *localIban;

};


#endif /* AH_JOBSEPAXFERMULTI_P_H */



