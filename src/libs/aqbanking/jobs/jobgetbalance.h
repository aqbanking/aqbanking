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


#ifndef AQBANKING_JOBGETBALANCE_H
#define AQBANKING_JOBGETBALANCE_H


#include <aqbanking/job.h>
#include <aqbanking/accstatus.h>

#ifdef __cplusplus
extern "C" {
#endif


AQBANKING_API
AB_JOB *AB_JobGetBalance_new(AB_ACCOUNT *a);
AQBANKING_API
const AB_ACCOUNT_STATUS *AB_JobGetBalance_GetAccountStatus(AB_JOB *j);
AQBANKING_API
void AB_JobGetBalance_SetAccountStatus(AB_JOB *j,
                                       const AB_ACCOUNT_STATUS *as);

#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_JOBGETBALANCE_H */

