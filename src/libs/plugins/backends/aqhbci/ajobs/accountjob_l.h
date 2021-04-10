/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_ACCOUNTJOBS_L_H
#define AH_ACCOUNTJOBS_L_H


#include "aqhbci/joblayer/job_l.h"
#include <aqbanking/backendsupport/account.h>
#include <gwenhywfar/buffer.h>


AH_JOB *AH_AccountJob_new(const char *name,
                          AB_PROVIDER *pro,
                          AB_USER *u,
                          AB_ACCOUNT *account);
int AH_AccountJob_IsAccountJob(const AH_JOB *j);

AB_ACCOUNT *AH_AccountJob_GetAccount(const AH_JOB *j);


#endif /* AH_ACCOUNTJOBS_L_H */


