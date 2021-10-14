/***************************************************************************
    begin       : Tue Dec 31 2013
    copyright   : (C) 2004-2013 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSEPAINTERNALXFER_P_H
#define AH_JOBSEPAINTERNALXFER_P_H


#include <gwenhywfar/db.h>
#include "jobsepainternalxfer_l.h"


static int AH_Job_SepaInternalTransfer_Prepare(AH_JOB *j);
static int AH_Job_SepaInternalTransfer_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);


#endif /* AH_JOBSEPAXFERSINGLE_P_H */



