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


#ifndef AQBANKING_JOBGETDATEDTRANSFERS_BE_H
#define AQBANKING_JOBGETDATEDTRANSFERS_BE_H


#include <aqbanking/job.h>
#include <aqbanking/jobgetdatedtransfers.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * Takes over ownership of the given list
 */
AQBANKING_API 
void AB_JobGetDatedTransfers_SetDatedTransfers(AB_JOB *j,
                                               AB_TRANSACTION_LIST2 *tl);


#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_JOBGETDATEDTRANSFERS_BE_H */

