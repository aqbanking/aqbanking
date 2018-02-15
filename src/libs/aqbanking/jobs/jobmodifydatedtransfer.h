/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_JOBMODIFYDATEDTRANSFER_H
#define AQBANKING_JOBMODIFYDATEDTRANSFER_H


#include <aqbanking/job.h>

/** @addtogroup G_AB_JOBS_DATED_TRANSFER_MOD
 *
 * This job modifies an already existing dated transfer at the bank.
 */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif


AQBANKING_API
AB_JOB *AB_JobModifyDatedTransfer_new(AB_ACCOUNT *a);


#ifdef __cplusplus
}
#endif

/*@}*/


#endif

