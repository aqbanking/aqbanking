/***************************************************************************
    begin       : Thu Jan 31 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBGETKEYS_L_H
#define AH_JOBGETKEYS_L_H


#include "aqhbci/aqhbci_l.h"
#include "aqhbci/joblayer/job_l.h"


AH_JOB *AH_Job_GetKeys_new(AB_PROVIDER *pro, AB_USER *u);
GWEN_CRYPT_TOKEN_KEYINFO *AH_Job_GetKeys_GetSignKeyInfo(const AH_JOB *j);
GWEN_CRYPT_TOKEN_KEYINFO *AH_Job_GetKeys_GetCryptKeyInfo(const AH_JOB *j);
GWEN_CRYPT_TOKEN_KEYINFO *AH_Job_GetKeys_GetAuthKeyInfo(const AH_JOB *j);
const char *AH_Job_GetKeys_GetPeerId(const AH_JOB *j);



#endif

