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


#ifndef AQBANKING_JOBGETSTANDINGORDERS_BE_H
#define AQBANKING_JOBGETSTANDINGORDERS_BE_H


#include <aqbanking/job.h>
#include <aqbanking/jobgetstandingorders.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * Takes over ownership of the given list
 */
AQBANKING_API 
void AB_JobGetStandingOrders_SetStandingOrders(AB_JOB *j,
                                               AB_TRANSACTION_LIST2 *tl);


#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_JOBGETSTANDINGORDERS_BE_H */

