/***************************************************************************
    begin       : Thu Jan 31 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobsendkeys_p.h"



AH_JOB *AH_Job_SendKeys_new(AB_PROVIDER *pro,
                            AB_USER *u,
                            GWEN_CRYPT_TOKEN_KEYINFO *cryptKeyInfo,
                            GWEN_CRYPT_TOKEN_KEYINFO *signKeyInfo,
                            GWEN_CRYPT_TOKEN_KEYINFO *authKeyInfo)
{
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;
  GWEN_DB_NODE *dbKey;
  int version;

  assert(u);

  if (authKeyInfo)
    j=AH_Job_new("JobSendKeysWithAuthKey", pro, u, 0, 0);
  else
    j=AH_Job_new("JobSendKeys", pro, u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "JobSendKeys not supported, should not happen");
    return NULL;
  }

  /* set arguments */
  dbArgs=AH_Job_GetArguments(j);
  dbKey=GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_DEFAULT, "cryptKey");
  assert(dbKey);
  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing crypt key");
  if (AH_Job_SendKeys_PrepareKey(j, dbKey, cryptKeyInfo, 0)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not prepare cryptkey");
    AH_Job_free(j);
    return 0;
  }

  dbKey=GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_DEFAULT, "signKey");
  assert(dbKey);
  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing sign key");
  if (AH_Job_SendKeys_PrepareKey(j, dbKey, signKeyInfo, 1)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not prepare signkey");
    AH_Job_free(j);
    return 0;
  }

  if (authKeyInfo) {
    dbKey=GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_DEFAULT, "authKey");
    assert(dbKey);
    DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing auth key");
    if (AH_Job_SendKeys_PrepareKey(j, dbKey, authKeyInfo, 2)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not prepare authkey");
      AH_Job_free(j);
      return 0;
    }
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No auth key info");
  }

  version=AH_User_GetRdhType(u);
  if (version==0)
    version=1;
  GWEN_DB_SetIntValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "authKey/secprofile/version", version);
  GWEN_DB_SetIntValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "cryptKey/secprofile/version", version);
  GWEN_DB_SetIntValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "signKey/secprofile/version", version);

  switch (AH_User_GetCryptMode(u)) {
  case AH_CryptMode_Rdh:
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "cryptKey/secprofile/code", "RDH");
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "signKey/secprofile/code", "RDH");
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "authKey/secprofile/code", "RDH");
    break;
  case AH_CryptMode_Rah:
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "cryptKey/secprofile/code", "RAH");
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "signKey/secprofile/code", "RAH");
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "authKey/secprofile/code", "RAH");
    break;
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "We need to send public keys only for RDH or RAH!\n");

  }
  DBG_INFO(AQHBCI_LOGDOMAIN, "JobSendKeys created");
  return j;
}



int AH_Job_SendKeys_PrepareKey(AH_JOB *j,
                               GWEN_DB_NODE *dbKey,
                               GWEN_CRYPT_TOKEN_KEYINFO *ki,
                               int kn)
{
  uint32_t bsize;
  const uint8_t *p;
  AB_USER *u;
  const char *userId;
  int country;
  int hbciVersion;

  assert(j);
  assert(dbKey);
  assert(ki);

  u=AH_Job_GetUser(j);
  assert(u);

  userId=AB_User_GetUserId(u);
  assert(userId);
  assert(*userId);

  /* set keyname */
  country=280; /* fixed value for "Germany", since HBCI is only used here */

  hbciVersion=AH_User_GetHbciVersion(u);
  if (hbciVersion==0)
    hbciVersion=220;

  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "keyName/country", country);
  GWEN_DB_SetCharValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "keyName/bankCode", AB_User_GetBankCode(u));

  GWEN_DB_SetCharValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "keyName/userid", userId);

  switch (kn) {
  case 0:
    GWEN_DB_SetCharValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "keyName/keyType", "V");
    break;
  case 1:
    GWEN_DB_SetCharValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "keyName/keyType", "S");
    break;
  case 2:
  default:
    GWEN_DB_SetCharValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "keyName/keyType", "D");
    break;
  }

  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "keyName/keyNum", GWEN_Crypt_Token_KeyInfo_GetKeyNumber(ki));
  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "keyName/keyVersion",
                      GWEN_Crypt_Token_KeyInfo_GetKeyVersion(ki));

  /* set key */
  if (kn==0) { /* crypt key */
    GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/purpose", 5);
    switch (AH_User_GetRdhType(u)) {
    case 10:
      GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/opmode", 2);
      break;
    /**** RDH7 Block Start******/
    case 7:
      GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/opmode", 18);
      break;
    /**** RDH7 Block End***/
    case 5:
      GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/opmode", 18);
      break;
    case 2:
      GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/opmode", 2);
      break;
    case 1:
      if (hbciVersion<300)
        /* for HBCI up until 2.20 the opmode has to be set to 16 even for crypt keys */
        GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/opmode", 16);
      else
        /* since FinTS 3.00 the crypt key uses the value "2" (meaning CBC) */
        GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/opmode", 2);
      break;

    case 0:
    default:
      GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/opmode", 2);
      break;
    }
  }
  else {
    /* sign key */
    GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/purpose", 6);
    switch (AH_User_GetRdhType(u)) {
    case 10:
      GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/opmode", 19);
      break;
    /**** RDH7 Block Start******/
    case 7:
      GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/opmode", 19);
      break;
    /**** RDH7 Block End***/
    case 5:
      GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/opmode", 18);
      break;
    case 2:
      GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/opmode", 17);
      break;
    case 0:
    case 1:
    default:
      GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/opmode", 16);
      break;
    }
  }

  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/type", 10);
  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/modname", 12);
  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/expname", 13);

  p=GWEN_Crypt_Token_KeyInfo_GetModulusData(ki);
  bsize=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
  if (!p || !bsize) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No modulus in key");
    return GWEN_ERROR_INVALID;
  }
  GWEN_DB_SetBinValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/modulus", p, bsize);

  p=GWEN_Crypt_Token_KeyInfo_GetExponentData(ki);
  bsize=GWEN_Crypt_Token_KeyInfo_GetExponentLen(ki);
  if (!p || !bsize) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No exponent in key");
    return GWEN_ERROR_INVALID;
  }
  GWEN_DB_SetBinValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/exponent", p, bsize);

  p=GWEN_Crypt_Token_KeyInfo_GetCertificateData(ki);
  bsize=GWEN_Crypt_Token_KeyInfo_GetCertificateLen(ki);
  if (p && bsize) {
    GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "cert/type", GWEN_Crypt_Token_KeyInfo_GetCertType(ki));
    GWEN_DB_SetBinValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS, "cert/cert", p, bsize);
  }

  return 0;
}



