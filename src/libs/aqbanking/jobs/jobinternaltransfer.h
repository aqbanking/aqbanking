/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_JOBINTERNALTRANSFER_H
#define AQBANKING_JOBINTERNALTRANSFER_H


#include <aqbanking/job.h>


/** @addtogroup G_AB_JOBS_XFER_INTERNAL
 *
 * An internal transfer is a transfer between two accounts of the same
 * customer at the same bank. Some banks only allow to use this job for
 * this kind of transfer while others only allow normal transfers (as
 * described in @ref G_AB_JOBS_XFER_TRANSFER).
 */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif


AQBANKING_API
AB_JOB *AB_JobInternalTransfer_new(AB_ACCOUNT *a);


#ifdef __cplusplus
}
#endif

/*@}*/ /* defgroup */


#endif

