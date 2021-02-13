/***************************************************************************
    begin       : Sun Dec 27 2020
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBGETDEPOT_L_H
#define AH_JOBGETDEPOT_L_H


#include "accountjob_l.h"

#include <gwenhywfar/gwentime.h>


AH_JOB *AH_Job_GetDepot_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account);


#endif /* AH_JOBGETDEPOT_H */


