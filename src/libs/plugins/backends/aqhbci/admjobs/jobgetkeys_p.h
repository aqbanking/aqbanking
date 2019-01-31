/***************************************************************************
    begin       : Thu Jan 31 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBGETKEYS_P_H
#define AH_JOBGETKEYS_P_H


#include "jobgetkeys_l.h"


typedef struct AH_JOB_GETKEYS AH_JOB_GETKEYS;

struct AH_JOB_GETKEYS {
  char *peerId;
  GWEN_CRYPT_TOKEN_KEYINFO *signKeyInfo;
  GWEN_CRYPT_TOKEN_KEYINFO *cryptKeyInfo;
  GWEN_CRYPT_TOKEN_KEYINFO *authKeyInfo;
};

static void GWENHYWFAR_CB AH_Job_GetKeys_FreeData(void *bp, void *p);
static int AH_Job_GetKeys_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);




#endif

