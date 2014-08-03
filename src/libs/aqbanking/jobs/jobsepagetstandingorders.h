/***************************************************************************
 begin       : Sun Aug 03 2014
 copyright   : (C) 2014 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_JOBSEPAGETSTANDINGORDERS_H
#define AQBANKING_JOBSEPAGETSTANDINGORDERS_H


#include <aqbanking/job.h>

/** @addtogroup G_AB_JOBS_XFER_SEPA_STANDINGORDERS
 *
 * This job requests active SEPA standing orders.
 */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif


AQBANKING_API 
AB_JOB *AB_JobSepaGetStandingOrders_new(AB_ACCOUNT *a);



#ifdef __cplusplus
}
#endif

/*@}*/ /* defgroup */

#endif  /* AQBANKING_JOBSEPAGETSTANDINGORDERS_H */

