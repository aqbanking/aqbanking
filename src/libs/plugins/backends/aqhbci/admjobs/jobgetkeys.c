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


#include "jobgetkeys_p.h"

#include "aqhbci/joblayer/job_crypt.h"


GWEN_INHERIT(AH_JOB, AH_JOB_GETKEYS);




AH_JOB *AH_Job_GetKeys_new(AB_PROVIDER *pro, AB_USER *u)
{
  AH_JOB *j;
  AH_JOB_GETKEYS *jd;
  GWEN_DB_NODE *args;
  int version;

  assert(u);
  j=AH_Job_new("JobGetKeys", pro, u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "JobGetKeys not supported, should not happen");
    return NULL;
  }

  /* create data for inheriting class */
  GWEN_NEW_OBJECT(AH_JOB_GETKEYS, jd);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_GETKEYS,
                       j, jd, AH_Job_GetKeys_FreeData);

  /* overwrite virtual function */
  AH_Job_SetProcessFn(j, AH_Job_GetKeys_Process);

  /* set args */
  args=AH_Job_GetArguments(j);
  assert(args);
  GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS, "open/ident/customerId", "9999999999");
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS, "open/ident/status", 0);
  GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS, "open/ident/systemId", "0");

  version=AH_User_GetRdhType(u);
  if (version==0)
    version=1;
  switch (AH_User_GetCryptMode(u)) {
  case AH_CryptMode_Rdh:
    GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS, "open/cryptKey/secprofile/code", "RDH");
    GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS, "open/signKey/secprofile/code", "RDH");
    break;

  case AH_CryptMode_Rah:
    GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS, "open/cryptKey/secprofile/code", "RAH");
    GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS, "open/signKey/secprofile/code", "RAH");
    break;
  default:
    break;
  }

  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS, "open/cryptKey/secprofile/version", version);
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS, "open/signKey/secprofile/version", version);

  return j;
}



void GWENHYWFAR_CB GWENHYWFAR_CB AH_Job_GetKeys_FreeData(void *bp, void *p)
{
  AH_JOB_GETKEYS *jd;

  jd=(AH_JOB_GETKEYS *) p;

  GWEN_Crypt_Token_KeyInfo_free(jd->signKeyInfo);
  GWEN_Crypt_Token_KeyInfo_free(jd->cryptKeyInfo);
  GWEN_Crypt_Token_KeyInfo_free(jd->authKeyInfo);
  free(jd->peerId);
  GWEN_FREE_OBJECT(jd);
}



int AH_Job_GetKeys_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  AH_HBCI *h;
  AH_JOB_GETKEYS *jd;
  AB_USER *u;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  int rv;
  int haveKey;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *cctx;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETKEYS, j);
  assert(jd);

  h=AH_Job_GetHbci(j);
  u=AH_Job_GetUser(j);
  rv=AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h),
                              AH_User_GetTokenType(u),
                              AH_User_GetTokenName(u),
                              &ct);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  cctx=GWEN_Crypt_Token_GetContext(ct, AH_User_GetTokenContextId(u), 0);
  if (cctx==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "CT context %d not found",
              AH_User_GetTokenContextId(u));
    return GWEN_ERROR_NOT_FOUND;
  }

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  /* search for "GetKeyResponse" */
  haveKey=0;
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while (dbCurr) {
    GWEN_DB_NODE *dbKeyResponse;
    int rv;

    rv=AH_Job_CheckEncryption(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (encryption)");
      return rv;
    }
    rv=AH_Job_CheckSignature(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (signature)");
      return rv;
    }

    dbKeyResponse=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data/GetKeyResponse");
    if (dbKeyResponse) {
      unsigned int bs;
      const uint8_t *p;

      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Got this key response:");
      if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Debug)
        GWEN_DB_Dump(dbKeyResponse, 2);

      p=GWEN_DB_GetBinValue(dbKeyResponse, "key/modulus", 0, 0, 0, &bs);
      if (!p || !bs) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "No modulus");
        return GWEN_ERROR_BAD_DATA;
      }
      else {
        const uint8_t defaultExpo[3]= {0x01, 0x00, 0x01};
        const char *s;
        uint32_t keyId;
        GWEN_CRYPT_TOKEN_KEYINFO *ki;
        int keySize;
        uint32_t flags=0;

        /* skip zero bytes if any */
        while (bs && *p==0) {
          p++;
          bs--;
        }

        /* calculate key size in bytes */
        if (bs<=96)
          keySize=96;
        else {
          keySize=bs;
        }

        s=GWEN_DB_GetCharValue(dbKeyResponse, "keyname/keytype", 0, "V");
        if (strcasecmp(s, "V")==0)
          keyId=GWEN_Crypt_Token_Context_GetEncipherKeyId(cctx);
        else if (strcasecmp(s, "S")==0)
          keyId=GWEN_Crypt_Token_Context_GetVerifyKeyId(cctx);
        else
          keyId=GWEN_Crypt_Token_Context_GetAuthVerifyKeyId(cctx);

        ki=GWEN_Crypt_Token_KeyInfo_new(keyId, GWEN_Crypt_CryptAlgoId_Rsa, keySize);

        GWEN_Crypt_Token_KeyInfo_SetModulus(ki, p, bs);
        GWEN_Crypt_Token_KeyInfo_SetExponent(ki, defaultExpo, sizeof(defaultExpo));

        flags|=
          GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
          GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
          GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
          GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
          GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER;
        if (strcasecmp(s, "V")==0) {
          flags|=GWEN_CRYPT_TOKEN_KEYFLAGS_CANENCIPHER;
          jd->cryptKeyInfo=ki;
          s=GWEN_DB_GetCharValue(dbKeyResponse, "keyname/userId", 0, NULL);
          free(jd->peerId);
          if (s)
            jd->peerId=strdup(s);
          else
            jd->peerId=NULL;
        }
        else if (strcasecmp(s, "S")==0) {
          flags|=GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY;
          jd->signKeyInfo=ki;
        }
        else {
          flags|=GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY;
          jd->authKeyInfo=ki;
        }
        GWEN_Crypt_Token_KeyInfo_SetFlags(ki, flags);
        GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, GWEN_DB_GetIntValue(dbKeyResponse, "keyname/keynum", 0, 0));
        GWEN_Crypt_Token_KeyInfo_SetKeyVersion(ki, GWEN_DB_GetIntValue(dbKeyResponse, "keyname/keyversion", 0, 0));
      }
      haveKey++;
    } /* if we have one */
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */

  if (haveKey==0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No server keys found");
    AH_Job_SetStatus(j, AH_JobStatusError);
    return GWEN_ERROR_NO_DATA;
  }
  return 0;
}



GWEN_CRYPT_TOKEN_KEYINFO *AH_Job_GetKeys_GetSignKeyInfo(const AH_JOB *j)
{
  AH_JOB_GETKEYS *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETKEYS, j);
  assert(jd);

  return jd->signKeyInfo;
}



GWEN_CRYPT_TOKEN_KEYINFO *AH_Job_GetKeys_GetCryptKeyInfo(const AH_JOB *j)
{
  AH_JOB_GETKEYS *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETKEYS, j);
  assert(jd);

  return jd->cryptKeyInfo;
}



GWEN_CRYPT_TOKEN_KEYINFO *AH_Job_GetKeys_GetAuthKeyInfo(const AH_JOB *j)
{
  AH_JOB_GETKEYS *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETKEYS, j);
  assert(jd);

  return jd->authKeyInfo;
}



const char *AH_Job_GetKeys_GetPeerId(const AH_JOB *j)
{
  AH_JOB_GETKEYS *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETKEYS, j);
  assert(jd);

  return jd->peerId;
}


