/***************************************************************************
    copyright   : (C) 2026 by AqBanking Contributors

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSEPAB2BDEBIT_L_H
#define AH_JOBSEPAB2BDEBIT_L_H


#include "accountjob_l.h"


AH_JOB *AH_Job_SepaB2bDebitDatedSingleCreate_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account);
AH_JOB *AH_Job_SepaB2bDebitDatedMultiCreate_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account);


#endif /* AH_JOBSEPAB2BDEBIT_L_H */
