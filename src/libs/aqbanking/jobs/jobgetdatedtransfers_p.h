/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004-2013 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_JOBGETDATEDTRANSFERS_P_H
#define AQBANKING_JOBGETDATEDTRANSFERS_P_H


#include <aqbanking/job.h>
#include "jobgetdatedtransfers_l.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct AB_JOB_GETDATEDTRANSFERS AB_JOB_GETDATEDTRANSFERS;
struct AB_JOB_GETDATEDTRANSFERS {
  /* arguments */

  /* parameters */

  /* responses */
  AB_TRANSACTION_LIST2 *datedTransfers;

};

static void GWENHYWFAR_CB AB_JobGetDatedTransfers_FreeData(void *bp, void *p);



#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_JOBGETDATEDTRANSFERS_P_H */

