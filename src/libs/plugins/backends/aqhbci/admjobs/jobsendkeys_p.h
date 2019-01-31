/***************************************************************************
    begin       : Thu Jan 31 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSENDKEYS_P_H
#define AH_JOBSENDKEYS_P_H


#include "jobsendkeys_l.h"


static int AH_Job_SendKeys_PrepareKey(AH_JOB *j,
                                      GWEN_DB_NODE *dbKey,
				      const GWEN_CRYPT_TOKEN_KEYINFO *ki,
                                      int kn);




#endif

