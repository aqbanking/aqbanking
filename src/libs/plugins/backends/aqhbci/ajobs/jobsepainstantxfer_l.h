/***************************************************************************
    copyright   : (C) 2026 by AqBanking Contributors

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSEPAINSTANTXFER_L_H
#define AH_JOBSEPAINSTANTXFER_L_H


#include "accountjob_l.h"


AH_JOB *AH_Job_SepaInstantTransferSingle_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account);
AH_JOB *AH_Job_SepaInstantTransferMulti_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account);


#endif /* AH_JOBSEPAINSTANTXFER_L_H */
