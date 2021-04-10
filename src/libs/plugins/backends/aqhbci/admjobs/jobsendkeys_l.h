/***************************************************************************
    begin       : Thu Jan 31 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSENDKEYS_L_H
#define AH_JOBSENDKEYS_L_H


#include "aqhbci/aqhbci_l.h"
#include "aqhbci/joblayer/job_l.h"


AH_JOB *AH_Job_SendKeys_new(AB_PROVIDER *pro, AB_USER *u,
                            GWEN_CRYPT_TOKEN_KEYINFO *cryptKeyInfo,
                            GWEN_CRYPT_TOKEN_KEYINFO *signKeyInfo,
                            GWEN_CRYPT_TOKEN_KEYINFO *authKeyInfo);





#endif

