/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_JOBMODIFYSTO_H
#define AQBANKING_JOBMODIFYSTO_H


#include <aqbanking/job.h>


/** @addtogroup G_AB_JOBS_STO_MOD
 *
 * This job modifies an already existing standing order.
 */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif


AQBANKING_API
AB_JOB *AB_JobModifyStandingOrder_new(AB_ACCOUNT *a);


#ifdef __cplusplus
}
#endif

/*@}*/ /* defgroup */


#endif

