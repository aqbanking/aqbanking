/***************************************************************************

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBLOCKKEYS_P_H
#define AH_JOBLOCKKEYS_P_H


#include "jobchangekeys_l.h"

typedef struct AH_JOB_CHANGEKEYS AH_JOB_CHANGEKEYS;

struct AH_JOB_CHANGEKEYS {
  uint16_t flags;
  uint8_t *canceled;
  AB_PROVIDER *pro;
  AB_USER *u;
  AB_USER *uTmp;
  const char *fm;
  char *tokenType;
  char *tokenName;
  uint32_t currentCryptKeyVersion;
  uint32_t currentSignKeyVersion;
  uint32_t currentAuthKeyVersion;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  int tokenCtxId;
  const GWEN_CRYPT_TOKEN_KEYINFO *cryptKeyInfo;
  const GWEN_CRYPT_TOKEN_KEYINFO *signKeyInfo;
  const GWEN_CRYPT_TOKEN_KEYINFO *authKeyInfo;
  int8_t resp;
  GWEN_BUFFER *emsg;
};


#endif

