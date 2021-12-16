/***************************************************************************
    begin       : Thu Dec 16 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSEPAGENERICDMULTICREATE_L_H
#define AH_JOBSEPAGENERICDMULTICREATE_L_H


#include "accountjob_l.h"


AH_JOB *AH_Job_SepaGenericMultiCreate_new(const char *jobName,
                                          AB_TRANSACTION_TYPE tt,
                                          AB_TRANSACTION_SUBTYPE tst,
                                          const char *instrumentationCode,
                                          int painMessageGroup,
                                          AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account);


#endif /* AH_JOBSEPACOR1DATEDMULTICREATE_L_H */


