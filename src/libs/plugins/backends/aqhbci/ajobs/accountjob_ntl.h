/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_ACCOUNTJOBS_NTL_L_H
#define AH_ACCOUNTJOBS_NTL_L_H


#include "aqhbci/joblayer/job_l.h"
#include "aqhbci/ajobs/accountjob_ntl.h"
#include <aqbanking/backendsupport/account.h>


/**
 * National account jobs are non-SEPA jobs which use accountId and bankCode instead of
 * IBAN and BIC.
 */
AH_JOB *AH_NationalAccountJob_new(const char *name,
                                  AB_PROVIDER *pro,
                                  AB_USER *u,
                                  AB_ACCOUNT *account);


#endif /* AH_ACCOUNTJOBS_L_H */


