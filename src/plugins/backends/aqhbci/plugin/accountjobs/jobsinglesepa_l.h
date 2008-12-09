/***************************************************************************
    begin       : Sat Nov 29 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSINGLESEPA_L_H
#define AH_JOBSINGLESEPA_L_H


#include "accountjob_l.h"

#include <gwenhywfar/gwentime.h>


AH_JOB *AH_Job_SingleSepaTransfer_new(AB_USER *u, AB_ACCOUNT *account);
AH_JOB *AH_Job_SingleSepaDebitNote_new(AB_USER *u, AB_ACCOUNT *account);

const char *AH_Job_SingleSepaTransfer_GetFiid(AH_JOB *j);

const char *AH_Job_SingleSepaTransfer_GetOldFiid(AH_JOB *j);


#endif /* AH_JOBSINGLESEPA_L_H */


