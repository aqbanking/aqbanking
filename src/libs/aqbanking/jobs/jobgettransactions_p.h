/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_JOBGETTRANSACTIONS_P_H
#define AQBANKING_JOBGETTRANSACTIONS_P_H


#include <aqbanking/job.h>
#include "jobgettransactions_be.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct AB_JOB_GETTRANSACTIONS AB_JOB_GETTRANSACTIONS;
struct AB_JOB_GETTRANSACTIONS {
  /* arguments */
  GWEN_DATE *fromDate;
  GWEN_DATE *toDate;

  /* parameters */
  int maxStoreDays;
};

static void GWENHYWFAR_CB AB_JobGetTransactions_FreeData(void *bp, void *p);



#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_JOBGETTRANSACTIONS_P_H */

