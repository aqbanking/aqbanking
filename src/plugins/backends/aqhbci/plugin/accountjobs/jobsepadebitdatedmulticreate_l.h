/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2013 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSEPADEBITDATEDMULTICREATE_L_H
#define AH_JOBSEPADEBITDATEDMULTICREATE_L_H


#include "accountjob_l.h"


AH_JOB *AH_Job_SepaDebitDatedMultiCreate_new(AB_USER *u, AB_ACCOUNT *account);

int AH_Job_SepaDebitDatedMultiCreate_GetTransferCount(AH_JOB *j);
int AH_Job_SepaDebitDatedMultiCreate_GetMaxTransfers(AH_JOB *j);


#endif /* AH_JOBSEPADEBITDATEDMULTICREATE_L_H */


