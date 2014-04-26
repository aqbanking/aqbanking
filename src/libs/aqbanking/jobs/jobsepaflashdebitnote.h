/***************************************************************************
 begin       : Sat Apr 26 2014
 copyright   : (C) 2014 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_JOBSEPAFLASHDEBITNOTE_H
#define AQBANKING_JOBSEPAFLASHDEBITNOTE_H


#include <aqbanking/job.h>

/** @addtogroup G_AB_JOBS_XFER_SEPA_FLASH_DEBITNOTE
 *
 * This is a SEPA debitnote with very low setup time. This means the date of the execution
 * is requested be shorter than for normal SEPA debit notes.
 */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif


AQBANKING_API 
AB_JOB *AB_JobSepaFlashDebitNote_new(AB_ACCOUNT *a);


#ifdef __cplusplus
}
#endif

/*@}*/ /* defgroup */

#endif

