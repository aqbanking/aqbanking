/***************************************************************************
 begin       : Tue Apr 03 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOB_GETESTATEMENTS_L_H
#define AH_JOB_GETESTATEMENTS_L_H


#include "accountjob_l.h"

/* This job uses HBCI/FinTS job "HKEKP" */
AH_JOB *AH_Job_GetEStatements_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account);


/* This job uses HBCI/FinTS job "HKEKA" */
AH_JOB *AH_Job_GetEStatements2_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account);


#endif /* AH_JOB_GETESTATEMENTS_L_H */


