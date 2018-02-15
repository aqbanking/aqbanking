/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_JOBSINGLETRANSFER_H
#define AQBANKING_JOBSINGLETRANSFER_H


#include <aqbanking/job.h>

/** @addtogroup G_AB_JOBS_XFER_TRANSFER
 *
 * This is a single national transfer (i.e. the banks of the payee and the
 * recipient are in the same country).
 */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif


AQBANKING_API 
AB_JOB *AB_JobSingleTransfer_new(AB_ACCOUNT *a);


#ifdef __cplusplus
}
#endif

/*@}*/ /* defgroup */

#endif

