/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQBANKING_JOBGETTRANSACTIONS_P_H
#define AQBANKING_JOBGETTRANSACTIONS_P_H


#include <aqbanking/job.h>
#include <aqbanking/accstatus.h>
#include "jobgettransactions_l.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct AB_JOB_GETTRANSACTIONS AB_JOB_GETTRANSACTIONS;
struct AB_JOB_GETTRANSACTIONS {
  GWEN_TIME *fromTime;
  GWEN_TIME *toTime;
};

void AB_JobGetTransactions_FreeData(void *bp, void *p);



#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_JOBGETTRANSACTIONS_P_H */

