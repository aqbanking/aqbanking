/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_JOBDELETESTO_H
#define AQBANKING_JOBDELETESTO_H


#include <aqbanking/job.h>


/** @addtogroup G_AB_JOBS_STO_DEL
 *
 */
/*@{*/

#ifdef __cplusplus
extern "C" {
#endif


AQBANKING_API
AB_JOB *AB_JobDeleteStandingOrder_new(AB_ACCOUNT *a);


#ifdef __cplusplus
}
#endif

/*@}*/ /* addtogroup */


#endif

