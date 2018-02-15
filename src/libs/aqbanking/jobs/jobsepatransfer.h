/***************************************************************************
 begin       : Sun Sep 21 2008
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_JOBSEPATRANSFER_H
#define AQBANKING_JOBSEPATRANSFER_H


#include <aqbanking/job.h>

/** @addtogroup G_AB_JOBS_XFER_SEPA_TRANSFER
 *
 * This is a SEPA transfer.
 */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif


AQBANKING_API 
AB_JOB *AB_JobSepaTransfer_new(AB_ACCOUNT *a);


#ifdef __cplusplus
}
#endif

/*@}*/ /* defgroup */

#endif

