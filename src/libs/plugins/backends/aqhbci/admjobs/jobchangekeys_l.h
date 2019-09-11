/***************************************************************************

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBCHANGEKEYS_L_H
#define AH_JOBCHANGEKEYS_L_H


#include "aqhbci_l.h"
#include "job_l.h"


AH_JOB *AH_Job_ChangeKeys_new(AB_PROVIDER *pro, AB_USER *u, GWEN_DB_NODE *args, uint8_t *canceled);

int AH_Job_ChangeKeys_finish(AB_PROVIDER *pro, AH_JOB *job, int res);



#endif

