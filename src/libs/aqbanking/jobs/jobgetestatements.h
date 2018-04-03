/***************************************************************************
 begin       : Tue Apr 03 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_JOBGETESTATEMENTS_H
#define AQBANKING_JOBGETESTATEMENTS_H


#include <aqbanking/job.h>

/** @addtogroup G_AB_JOBS_STO_GET
 *
 */
/*@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Creates a job which retrieves a electronic statements
 * for the given account ("Elektronischer Kontoauszug").
 * @param a account for which you want the standing orders
 */
AQBANKING_API
AB_JOB *AB_JobGetEStatements_new(AB_ACCOUNT *a);


#ifdef __cplusplus
}
#endif

/*@}*/

#endif /* AQBANKING_JOBGETESTATEMENTS_H */

