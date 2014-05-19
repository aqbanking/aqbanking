/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "aqhbci_l.h"

#include "adminjobs_p.h"
#include "job_l.h"
#include "jobqueue_l.h"
#include "hbci_l.h"
#include <aqhbci/provider.h>

#include <aqbanking/banking_be.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/text.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>




/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_GetKeys
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


GWEN_INHERIT(AH_JOB, AH_JOB_GETKEYS);


/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_GetKeys_new(AB_USER *u){
  AH_JOB *j;
  AH_JOB_GETKEYS *jd;
  GWEN_DB_NODE *args;
  int version;

  assert(u);
  j=AH_Job_new("JobGetKeys", u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "JobGetKeys not supported, should not happen");
    return 0;
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
  GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "open/ident/customerId",
                       "9999999999");
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "open/ident/status",
                      0);
  GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "open/ident/systemId",
		       "0");

  version=AH_User_GetRdhType(u);
  if (version==0)
    version=1;
  GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "open/cryptKey/secprofile/code", "RDH");
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "open/cryptKey/secprofile/version", version);
  GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "open/signKey/secprofile/code", "RDH");
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "open/signKey/secprofile/version", version);

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
void GWENHYWFAR_CB GWENHYWFAR_CB AH_Job_GetKeys_FreeData(void *bp, void *p){
//  AH_JOB *j;
  AH_JOB_GETKEYS *jd;

//  j=(AH_JOB*) bp;
  jd=(AH_JOB_GETKEYS*) p;

  GWEN_Crypt_Token_KeyInfo_free(jd->signKeyInfo);
  GWEN_Crypt_Token_KeyInfo_free(jd->cryptKeyInfo);
  GWEN_Crypt_Token_KeyInfo_free(jd->authKeyInfo);
  free(jd->peerId);
  GWEN_FREE_OBJECT(jd);
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetKeys_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){
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
  while(dbCurr) {
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

    dbKeyResponse=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                   "data/GetKeyResponse");
    if (dbKeyResponse) {
      unsigned int bs;
      const uint8_t *p;

      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Got this key response:");
      if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Debug)
        GWEN_DB_Dump(dbKeyResponse, 2);

      p=GWEN_DB_GetBinValue(dbKeyResponse, "key/modulus", 0, 0, 0 , &bs);
      if (!p || !bs) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "No modulus");
	return GWEN_ERROR_BAD_DATA;
      }
      else {
	const uint8_t defaultExpo[3]={0x01, 0x00, 0x01};
        const char *s;
	uint32_t keyId;
	GWEN_CRYPT_TOKEN_KEYINFO *ki;
	int keySize;
	uint32_t flags=0;

	/* skip zero bytes if any */
	while(bs && *p==0) {
	  p++;
	  bs--;
	}

	/* calculate key size in bytes */
	if (bs<=96)
	  keySize=96;
	else {
	  keySize=bs;
	}

	s=GWEN_DB_GetCharValue(dbKeyResponse,
			       "keyname/keytype", 0,
			       "V");
	if (strcasecmp(s, "V")==0)
	  keyId=GWEN_Crypt_Token_Context_GetEncipherKeyId(cctx);
	else if (strcasecmp(s, "S")==0)
	  keyId=GWEN_Crypt_Token_Context_GetVerifyKeyId(cctx);
        else
	  keyId=GWEN_Crypt_Token_Context_GetAuthVerifyKeyId(cctx);

	ki=GWEN_Crypt_Token_KeyInfo_new(keyId,
					GWEN_Crypt_CryptAlgoId_Rsa,
					keySize);

	GWEN_Crypt_Token_KeyInfo_SetModulus(ki, p, bs);
	GWEN_Crypt_Token_KeyInfo_SetExponent(ki,
					     defaultExpo,
					     sizeof(defaultExpo));

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
	GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki,
					      GWEN_DB_GetIntValue(dbKeyResponse,
								  "keyname/keynum",
								  0, 0));
	GWEN_Crypt_Token_KeyInfo_SetKeyVersion(ki,
					       GWEN_DB_GetIntValue(dbKeyResponse,
								   "keyname/keyversion",
								   0, 0));
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



/* --------------------------------------------------------------- FUNCTION */
GWEN_CRYPT_TOKEN_KEYINFO *AH_Job_GetKeys_GetSignKeyInfo(const AH_JOB *j){
  AH_JOB_GETKEYS *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETKEYS, j);
  assert(jd);

  return jd->signKeyInfo;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_CRYPT_TOKEN_KEYINFO *AH_Job_GetKeys_GetCryptKeyInfo(const AH_JOB *j){
  AH_JOB_GETKEYS *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETKEYS, j);
  assert(jd);

  return jd->cryptKeyInfo;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_CRYPT_TOKEN_KEYINFO *AH_Job_GetKeys_GetAuthKeyInfo(const AH_JOB *j){
  AH_JOB_GETKEYS *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETKEYS, j);
  assert(jd);

  return jd->authKeyInfo;
}



const char *AH_Job_GetKeys_GetPeerId(const AH_JOB *j) {
  AH_JOB_GETKEYS *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETKEYS, j);
  assert(jd);

  return jd->peerId;
}






/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_SendKeys
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */



/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_SendKeys_new(AB_USER *u,
			    const GWEN_CRYPT_TOKEN_KEYINFO *cryptKeyInfo,
			    const GWEN_CRYPT_TOKEN_KEYINFO *signKeyInfo,
			    const GWEN_CRYPT_TOKEN_KEYINFO *authKeyInfo) {
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;
  GWEN_DB_NODE *dbKey;
  int version;

  assert(u);

  if (authKeyInfo)
    j=AH_Job_new("JobSendKeysWithAuthKey", u, 0, 0);
  else
    j=AH_Job_new("JobSendKeys", u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "JobSendKeys not supported, should not happen");
    return NULL;
  }

  /* set arguments */
  dbArgs=AH_Job_GetArguments(j);
  dbKey=GWEN_DB_GetGroup(dbArgs,
			 GWEN_DB_FLAGS_DEFAULT,
			 "cryptKey");
  assert(dbKey);
  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing crypt key");
  if (AH_Job_SendKeys_PrepareKey(j, dbKey, cryptKeyInfo, 0)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not prepare cryptkey");
    AH_Job_free(j);
    return 0;
  }

  dbKey=GWEN_DB_GetGroup(dbArgs,
                         GWEN_DB_FLAGS_DEFAULT,
                         "signKey");
  assert(dbKey);
  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing sign key");
  if (AH_Job_SendKeys_PrepareKey(j, dbKey, signKeyInfo, 1)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not prepare signkey");
    AH_Job_free(j);
    return 0;
  }

  if (authKeyInfo) {
    dbKey=GWEN_DB_GetGroup(dbArgs,
			   GWEN_DB_FLAGS_DEFAULT,
			   "authKey");
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
  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "cryptKey/secprofile/code", "RDH");
  GWEN_DB_SetIntValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "cryptKey/secprofile/version", version);

  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "signKey/secprofile/code", "RDH");
  GWEN_DB_SetIntValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "signKey/secprofile/version", version);
  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "authKey/secprofile/code", "RDH");
  GWEN_DB_SetIntValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "authKey/secprofile/version", version);

  DBG_INFO(AQHBCI_LOGDOMAIN, "JobSendKeys created");
  return j;
}



int AH_Job_SendKeys_PrepareKey(AH_JOB *j,
			       GWEN_DB_NODE *dbKey,
			       const GWEN_CRYPT_TOKEN_KEYINFO *ki,
			       int kn) {
  uint32_t bsize;
  const uint8_t *p;
  AB_USER *u;
  const char *userId;
  const AB_COUNTRY *pcountry;
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
  pcountry=AB_Banking_FindCountryByName(AH_Job_GetBankingApi(j),
					AB_User_GetCountry(u));
  if (pcountry)
    country=AB_Country_GetNumericCode(pcountry);
  else
    country=280;

  hbciVersion=AH_User_GetHbciVersion(u);
  if (hbciVersion==0)
    hbciVersion=220;

  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "keyName/country", country);
  GWEN_DB_SetCharValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "keyName/bankCode", AB_User_GetBankCode(u));

  GWEN_DB_SetCharValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "keyName/userid", userId);

  switch(kn) {
  case 0:
    GWEN_DB_SetCharValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "keyName/keyType", "V");
    break;
  case 1:
    GWEN_DB_SetCharValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "keyName/keyType", "S");
    break;
  case 2:
  default:
    GWEN_DB_SetCharValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "keyName/keyType", "D");
    break;
  }

  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "keyName/keyNum",
		      GWEN_Crypt_Token_KeyInfo_GetKeyNumber(ki));
  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "keyName/keyVersion",
		      GWEN_Crypt_Token_KeyInfo_GetKeyVersion(ki));

  /* set key */
  if (kn==0) { /* crypt key */
    GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"key/purpose", 5);
    switch(AH_User_GetRdhType(u)) {
    case 10:
      GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "key/opmode", 2);
      break;
    case 5:
      GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "key/opmode", 18);
      break;
    case 2:
      GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "key/opmode", 2);
      break;
    case 1:
      if (hbciVersion<300)
	/* for HBCI up until 2.20 the opmode has to be set to 16 even for crypt keys */
	GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			    "key/opmode", 16);
      else
	/* since FinTS 3.00 the crypt key uses the value "2" (meaning CBC) */
	GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			    "key/opmode", 2);
      break;

    case 0:
    default:
      GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "key/opmode", 2);
      break;
    }
  }
  else {
    /* sign key */
    GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"key/purpose", 6);
    switch(AH_User_GetRdhType(u)) {
    case 10:
      GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "key/opmode", 19);
      break;
    case 5:
      GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "key/opmode", 18);
      break;
    case 2:
      GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "key/opmode", 17);
      break;
    case 0:
    case 1:
    default:
      GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "key/opmode", 16);
      break;
    }
  }

  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "key/type", 10);
  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "key/modname", 12);
  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "key/expname", 13);

  p=GWEN_Crypt_Token_KeyInfo_GetModulusData(ki);
  bsize=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
  if (!p || !bsize) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No modulus in key");
    return GWEN_ERROR_INVALID;
  }
  GWEN_DB_SetBinValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "key/modulus", p, bsize);

  p=GWEN_Crypt_Token_KeyInfo_GetExponentData(ki);
  bsize=GWEN_Crypt_Token_KeyInfo_GetExponentLen(ki);
  if (!p || !bsize) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No exponent in key");
    return GWEN_ERROR_INVALID;
  }
  GWEN_DB_SetBinValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "key/exponent", p, bsize);

  return 0;
}




/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_UpdateBank
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */



GWEN_INHERIT(AH_JOB, AH_JOB_UPDATEBANK)

/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_UpdateBank_new(AB_USER *u) {
  AH_JOB *j;
  GWEN_DB_NODE *args;
  AH_JOB_UPDATEBANK *jd;

  assert(u);
  j=AH_Job_new("JobUpdateBankInfo", u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "JobUpdateBankInfo not supported, should not happen");
    return 0;
  }

  GWEN_NEW_OBJECT(AH_JOB_UPDATEBANK, jd);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_UPDATEBANK, j, jd,
                       AH_Job_UpdateBank_FreeData)
  AH_Job_SetProcessFn(j, AH_Job_UpdateBank_Process);

  jd->accountList=AB_Account_List2_new();

  /* set arguments */
  args=AH_Job_GetArguments(j);
  assert(args);
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "open/prepare/bpdversion", 0);
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "open/prepare/updversion", 0);

  DBG_INFO(AQHBCI_LOGDOMAIN, "JobUpdateBankInfo created");
  return j;
}



void GWENHYWFAR_CB AH_Job_UpdateBank_FreeData(void *bp, void *p){
  AH_JOB_UPDATEBANK *jd;

  jd=(AH_JOB_UPDATEBANK*)p;
  AB_Account_List2_FreeAll(jd->accountList);
  GWEN_FREE_OBJECT(jd);
}



int AH_Job_UpdateBank_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_UPDATEBANK *jd;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  GWEN_DB_NODE *dbAccountData;
  AB_USER *u;
  AB_BANKING *ab;
  int accs;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_UPDATEBANK, j);
  assert(jd);

  if (jd->scanned)
    return 0;

  jd->scanned=1;

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  u=AH_Job_GetUser(j);
  assert(u);

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  /* search for "AccountData" */
  accs=0;
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while(dbCurr) {
    dbAccountData=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
				   "data/AccountData2");
    if (dbAccountData==NULL)
      dbAccountData=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
				     "data/AccountData");
    if (dbAccountData) {
      const char *accountId;
      const char *userName;
      const char *accountName;
      const char *bankCode;
      const char *accountSuffix;
      AB_ACCOUNT *acc;
  
      DBG_INFO(AQHBCI_LOGDOMAIN, "Found an account candidate");

      /* account data found */
      accountId=GWEN_DB_GetCharValue(dbAccountData, "accountId", 0, 0);
      accountSuffix=GWEN_DB_GetCharValue(dbAccountData, "accountsubid", 0, 0);
      accountName=GWEN_DB_GetCharValue(dbAccountData, "account/name", 0, 0);
      userName=GWEN_DB_GetCharValue(dbAccountData, "name1", 0, 0);
      bankCode=GWEN_DB_GetCharValue(dbAccountData, "bankCode", 0, 0);

      if (accountId && *accountId &&
	  bankCode && *bankCode) {
	acc=AB_Banking_CreateAccount(ab, AH_PROVIDER_NAME);
	assert(acc);
        if (strcasecmp(GWEN_DB_GroupName(dbAccountData), "AccountData2")==0)
          /* KTV in version 2 available */
          AH_Account_AddFlags(acc, AH_BANK_FLAGS_KTV2);
        else
          AH_Account_SubFlags(acc, AH_BANK_FLAGS_KTV2);
	AB_Account_SetBankCode(acc, bankCode);
	AB_Account_SetAccountNumber(acc, accountId);

	if (accountName)
	  AB_Account_SetAccountName(acc, accountName);
        if (accountSuffix)
          AB_Account_SetSubAccountId(acc, accountSuffix);
        if (userName)
	  AB_Account_SetOwnerName(acc, userName);

        accs++;
	AB_Account_List2_PushBack(jd->accountList, acc);
      }
      else {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing information in account: BLZ=[%s], Kto=[%s], AccName=[%s]",
		  bankCode?bankCode:"",
		  accountId?accountId:"",
		  accountName?accountName:"");
      }
    }
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }
  if (!accs) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "No accounts found");
  }

  return 0;
}



AB_ACCOUNT_LIST2 *AH_Job_UpdateBank_GetAccountList(const AH_JOB *j){
  AH_JOB_UPDATEBANK *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_UPDATEBANK, j);
  assert(jd);

  return jd->accountList;
}



AB_ACCOUNT_LIST2 *AH_Job_UpdateBank_TakeAccountList(AH_JOB *j){
  AH_JOB_UPDATEBANK *jd;
  AB_ACCOUNT_LIST2 *tal;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_UPDATEBANK, j);
  assert(jd);

  tal=jd->accountList;
  jd->accountList=0;
  return tal;
}






/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_GetSysId
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

GWEN_INHERIT(AH_JOB, AH_JOB_GETSYSID)

AH_JOB *AH_Job_GetSysId_new(AB_USER *u){
  AH_JOB *j;
  GWEN_DB_NODE *args;
  AH_JOB_GETSYSID *jd;

  assert(u);
  j=AH_Job_new("JobSync", u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "JobSync not supported, should not happen");
    return 0;
  }

  GWEN_NEW_OBJECT(AH_JOB_GETSYSID, jd);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_GETSYSID, j, jd,
		       AH_Job_GetSysId_FreeData);
  AH_Job_SetProcessFn(j, AH_Job_GetSysId_Process);
  AH_Job_SetNextMsgFn(j, AH_Job_GetSysId_NextMsg);

  /* set arguments */
  args=AH_Job_GetArguments(j);
  assert(args);
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "open/ident/country", 280);
  GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "open/ident/bankCode",
                       AB_User_GetBankCode(u));
  GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "open/ident/customerId",
                       AB_User_GetCustomerId(u));

  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "open/sync/mode", 0);
  GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "open/sync/systemId", "0");

  DBG_INFO(AQHBCI_LOGDOMAIN, "JobGetSysId created");
  return j;
}



void GWENHYWFAR_CB AH_Job_GetSysId_FreeData(void *bp, void *p){
  AH_JOB_GETSYSID *jd;

  jd=(AH_JOB_GETSYSID*)p;
  free(jd->sysId);
  GWEN_FREE_OBJECT(jd);
}



int AH_Job_GetSysId_ExtractSysId(AH_JOB *j){
  AH_JOB_GETSYSID *jd;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  GWEN_DB_NODE *dbSyncResponse;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETSYSID, j);
  assert(jd);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Extracting system-id from this response:");
  if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Debug)
    GWEN_DB_Dump(dbResponses, 2);

  /* search for "SyncResponse" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while(dbCurr) {
    dbSyncResponse=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                    "data/SyncResponse");
    if (dbSyncResponse) {
      const char *s;

      DBG_INFO(AQHBCI_LOGDOMAIN, "Found a sync response");
      s=GWEN_DB_GetCharValue(dbSyncResponse, "systemId", 0, 0);
      if (s) {
        free(jd->sysId);
        jd->sysId=strdup(s);
        return 0;
      }
      else {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "No system id found");
        free(jd->sysId);
        jd->sysId=0;
        AH_Job_SetStatus(j, AH_JobStatusError);
        return -1;
      }
    }
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }
  DBG_ERROR(AQHBCI_LOGDOMAIN, "No syncresponse found");
  AH_Job_SetStatus(j, AH_JobStatusError);
  return -1;
}



int AH_Job_GetSysId_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_GETSYSID *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETSYSID, j);
  assert(jd);

  return AH_Job_GetSysId_ExtractSysId(j);
}



const char *AH_Job_GetSysId_GetSysId(AH_JOB *j){
  AH_JOB_GETSYSID *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETSYSID, j);
  assert(jd);
  return jd->sysId;
}




int AH_Job_GetSysId_NextMsg(AH_JOB *j) {
  AH_JOB_GETSYSID *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETSYSID, j);
  assert(jd);

  if (AH_Job_GetSysId_ExtractSysId(j)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not extract system id");
    return 0;
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Setting system id [%s]", jd->sysId);
    AH_User_SetSystemId(AH_Job_GetUser(j), jd->sysId);
  }

  return 1;
}







/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_TestVersion
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

GWEN_INHERIT(AH_JOB, AH_JOB_TESTVERSION);


AH_JOB *AH_Job_TestVersion_new(AB_USER *u, int anon){
  AH_JOB *j;
  GWEN_DB_NODE *args;
  AH_JOB_TESTVERSION *jd;

  assert(u);
  if (anon)
    j=AH_Job_new("JobDialogInitAnon", u, 0, 0);
  else
    j=AH_Job_new("JobDialogInit", u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "JobTestVersion not supported, should not happen");
    return 0;
  }
  AH_Job_AddFlags(j, AH_JOB_FLAGS_DLGJOB);
  GWEN_NEW_OBJECT(AH_JOB_TESTVERSION, jd);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_TESTVERSION, j, jd,
                       AH_Job_TestVersion_FreeData);
  AH_Job_SetProcessFn(j, AH_Job_TestVersion_Process);

  jd->versionSupported=AH_JobTestVersion_ResultUnknown;

  /* set arguments */
  args=AH_Job_GetArguments(j);
  assert(args);
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "prepare/bpdversion", 0);
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "prepare/updversion", 0);

  DBG_INFO(AQHBCI_LOGDOMAIN, "JobTestVersion created");
  return j;
}



void GWENHYWFAR_CB AH_Job_TestVersion_FreeData(void *bp, void *p){
  AH_JOB_TESTVERSION *jd;

  jd=(AH_JOB_TESTVERSION*)p;
  GWEN_FREE_OBJECT(jd);
}



int AH_Job_TestVersion_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_TESTVERSION *jd;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  GWEN_DB_NODE *dbMsgResponse;
  int hadAGoodResult=0;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TESTVERSION, j);
  assert(jd);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Parsing this response");
  if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Debug)
    GWEN_DB_Dump(dbResponses, 2);

  /* search for "MsgResult" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);

  while(dbCurr && jd->versionSupported==AH_JobTestVersion_ResultUnknown) {
    dbMsgResponse=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
				   "data/MsgResult");
    if (dbMsgResponse){
      GWEN_DB_NODE *dbResult;

      dbResult=GWEN_DB_FindFirstGroup(dbMsgResponse, "result");
      while(dbResult &&
            jd->versionSupported==AH_JobTestVersion_ResultUnknown){
	int code;

	DBG_INFO(AQHBCI_LOGDOMAIN, "Found message result");
        code=GWEN_DB_GetIntValue(dbResult, "resultCode", 0, -1);
	if (code>=9000) {
	  if (code==9180) {
	    /* version is definately not supported */
	    jd->versionSupported=AH_JobTestVersion_ResultNotSupported;
	  }
	  else {
	    if (code>=9300 && code<9400) {
	      /* error with the signature/encryption, so there was
	       * no complaint about the version */
	      jd->versionSupported=AH_JobTestVersion_ResultMaybeSupported;
	    }
	    else {
	      const char *s;

	      /* any other error, check for substring "version" */
	      s=GWEN_DB_GetCharValue(dbResult, "text", 0, 0);
	      if (s) {
		if (strstr(s, "version") || strstr(s, "Version")) {
		  /* seems to be a complaint about the version */
		  jd->versionSupported=AH_JobTestVersion_ResultNotSupported;
		}
	      }
	      /* still undecided ? */
	      if (jd->versionSupported==AH_JobTestVersion_ResultUnknown)
		/* yes, so there was no complaint about the version */
		jd->versionSupported=AH_JobTestVersion_ResultMaybeSupported;
	    }
	  }
	} /* if error */
	else {
	  /* not an error, so the version is definately supported */
	  hadAGoodResult=1;
	}
	dbResult=GWEN_DB_FindNextGroup(dbResult, "result");
      } /* while result */
    }
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */

  /* still undecided ? */
  if (jd->versionSupported==AH_JobTestVersion_ResultUnknown)
    if (hadAGoodResult)
      /* in dubio pro reo */
      jd->versionSupported=AH_JobTestVersion_ResultSupported;

  return 0;
}



AH_JOB_TESTVERSION_RESULT AH_Job_TestVersion_GetResult(const AH_JOB *j){
  AH_JOB_TESTVERSION *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TESTVERSION, j);
  assert(jd);

  return jd->versionSupported;
}



/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_GetStatus
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


GWEN_INHERIT(AH_JOB, AH_JOB_GETSTATUS);


AH_JOB *AH_Job_GetStatus_new(AB_USER *u,
                             const GWEN_TIME *fromDate,
                             const GWEN_TIME *toDate) {
  AH_JOB *j;
  AH_JOB_GETSTATUS *aj;
  GWEN_DB_NODE *dbArgs;

  j=AH_Job_new("JobGetStatus", u, 0, 0);
  if (!j)
    return 0;

  GWEN_NEW_OBJECT(AH_JOB_GETSTATUS, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_GETSTATUS, j, aj,
                       AH_Job_GetStatus_FreeData);
  aj->results=AH_Result_List_new();

  if (fromDate)
    aj->fromDate=GWEN_Time_dup(fromDate);
  if (toDate)
    aj->toDate=GWEN_Time_dup(toDate);

  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, AH_Job_GetStatus_Process);
  AH_Job_SetExchangeFn(j, AH_Job_GetStatus_Exchange);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  if (fromDate) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    if (GWEN_Time_toString(fromDate, "YYYYMMDD", dbuf)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
		"Error in fromDate");
    }
    else {
      GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
			   "fromDate", GWEN_Buffer_GetStart(dbuf));
    }
    GWEN_Buffer_free(dbuf);
  }

  if (toDate) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    if (GWEN_Time_toString(toDate, "YYYYMMDD", dbuf)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
		"Error in toDate");
    }
    else {
      GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
			   "toDate", GWEN_Buffer_GetStart(dbuf));
    }
    GWEN_Buffer_free(dbuf);
  }

  return j;
}



void GWENHYWFAR_CB AH_Job_GetStatus_FreeData(void *bp, void *p){
  AH_JOB_GETSTATUS *aj;

  aj=(AH_JOB_GETSTATUS*)p;
  AH_Result_List_free(aj->results);
  GWEN_FREE_OBJECT(aj);
}



int AH_Job_GetStatus_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_GETSTATUS *aj;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobGetStatus");
  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETSTATUS, j);
  assert(aj);

  /* nothing to do here (because this is not a real job and it is processed
   * by AH_Outbox) */
  return 0;
}



int AH_Job_GetStatus_Exchange(AH_JOB *j, AB_JOB *bj,
			      AH_JOB_EXCHANGE_MODE m,
			      AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_GETSTATUS *aj;

  DBG_WARN(AQHBCI_LOGDOMAIN, "Exchanging (%d), should not happen...", m);
  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETSTATUS, j);
  assert(aj);

  return 0;
}



/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_Tan
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


GWEN_INHERIT(AH_JOB, AH_JOB_TAN);


AH_JOB *AH_Job_Tan_new(AB_USER *u, int process, int jobVersion) {
  AH_JOB *j;
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbArgs;
  GWEN_DB_NODE *dbParams;
  const char *s;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Looking for TAN job in version %d", jobVersion);
  j=AH_Job_new("JobTan", u, 0, jobVersion);
  if (!j) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "TAN job in version %d not found", jobVersion);
    return NULL;
  }

  GWEN_NEW_OBJECT(AH_JOB_TAN, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_TAN, j, aj,
                       AH_Job_Tan_FreeData);
  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, AH_Job_Tan_Process);
  AH_Job_SetExchangeFn(j, AH_Job_Tan_Exchange);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);
  dbParams=AH_Job_GetParams(j);
  assert(dbParams);

  GWEN_DB_SetIntValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "process", process);
  if (process==1 || process==2)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "moreTans", "N");

  /* set tanmedium id (if any) */
  s=AH_User_GetTanMediumId(u);
  if (s && *s)
    AH_Job_Tan_SetTanMediumId(j, s);

  return j;
}



void GWENHYWFAR_CB AH_Job_Tan_FreeData(void *bp, void *p){
  AH_JOB_TAN *aj;

  aj=(AH_JOB_TAN*)p;
  free(aj->tanMediumId);
  free(aj->reference);
  free(aj->challenge);
  free(aj->challengeHhd);
  GWEN_FREE_OBJECT(aj);
}



int AH_Job_Tan_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  int rv;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Processing JobTan");
  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  /* search for "TanResponse" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while(dbCurr) {
    GWEN_DB_NODE *dbTanResponse;

    rv=AH_Job_CheckEncryption(j, dbCurr);
    if (rv) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Compromised security (encryption)");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }
    rv=AH_Job_CheckSignature(j, dbCurr);
    if (rv) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Compromised security (signature)");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }

    dbTanResponse=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                   "data/tanResponse");
    if (dbTanResponse) {
      const char *s;

      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Got a TAN response");
      if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevel_Debug)
        GWEN_DB_Dump(dbTanResponse, 2);

      s=GWEN_DB_GetCharValue(dbTanResponse, "challenge", 0, 0);
      if (s) {
        free(aj->challenge);
        aj->challenge=strdup(s);
      }

      /* get special HHD challenge */
      if (GWEN_DB_VariableExists(dbTanResponse, "challengeHHD")) {
	const uint8_t *p;
	unsigned int l;

	p=GWEN_DB_GetBinValue(dbTanResponse, "challengeHHD", 0, NULL, 0, &l);
	if (p && l) {
	  GWEN_BUFFER *bbuf;

          DBG_INFO(AQHBCI_LOGDOMAIN, "Job has a challengeHHD string:");
          GWEN_Text_LogString((const char*)p, l, AQHBCI_LOGDOMAIN, GWEN_LoggerLevel_Info);

          bbuf=GWEN_Buffer_new(0, 256, 0, 1);
#if 0
          GWEN_Text_ToHexBuffer((const char*) p, l, bbuf, 0, 0, 0);
#else
          /* data is a string, no need to transform to hex */
          GWEN_Buffer_AppendBytes(bbuf, (const char*) p, l);
#endif
	  free(aj->challengeHhd);
	  aj->challengeHhd=strdup(GWEN_Buffer_GetStart(bbuf));
	  GWEN_Buffer_free(bbuf);
	}
      }

      s=GWEN_DB_GetCharValue(dbTanResponse, "jobReference", 0, 0);
      if (s) {
        free(aj->reference);
        aj->reference=strdup(s);
      }

      break; /* break loop, we found the tanResponse */
    } /* if "TanResponse" */
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  return 0;
}



int AH_Job_Tan_GetTanMethod(const AH_JOB *j) {
  AH_JOB_TAN *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  return aj->tanMethod;
}



void AH_Job_Tan_SetTanMethod(AH_JOB *j, int i) {
  AH_JOB_TAN *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  aj->tanMethod=i;
}



int AH_Job_Tan_Exchange(AH_JOB *j, AB_JOB *bj,
			AH_JOB_EXCHANGE_MODE m,
			AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_TAN *aj;

  DBG_WARN(AQHBCI_LOGDOMAIN, "Exchanging (%d)", m);
  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  switch(m) {
  case AH_Job_ExchangeModeParams:
    break;
  case AH_Job_ExchangeModeArgs:
    break;
  case AH_Job_ExchangeModeResults:
    break;
  default:
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Unsupported exchange mode");
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */

  return 0;
}



void AH_Job_Tan_SetHash(AH_JOB *j,
                        const unsigned char *p,
                        unsigned int len) {
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbArgs;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  GWEN_DB_SetBinValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "jobHash", p, len);

}



void AH_Job_Tan_SetReference(AH_JOB *j, const char *p) {
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbArgs;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "jobReference", p);
}



void AH_Job_Tan_SetTanList(AH_JOB *j, const char *p) {
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbArgs;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "tanList", p);
}



void AH_Job_Tan_SetTanInfo(AH_JOB *j, const char *p) {
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbArgs;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "tanInfo", p);
}



void AH_Job_Tan_SetSegCode(AH_JOB *j, const char *p) {
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbArgs;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  if (p && *p) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Setting segment id in TAN to [%s]", p);
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "segmentId", p);
  }
  else {
    GWEN_DB_DeleteVar(dbArgs, "segmentId");
  }
}



const char *AH_Job_Tan_GetChallenge(const AH_JOB *j) {
  AH_JOB_TAN *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  return aj->challenge;
}



const char *AH_Job_Tan_GetHhdChallenge(const AH_JOB *j) {
  AH_JOB_TAN *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  return aj->challengeHhd;
}



const char *AH_Job_Tan_GetReference(const AH_JOB *j) {
  AH_JOB_TAN *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  return aj->reference;
}



void AH_Job_Tan_SetTanMediumId(AH_JOB *j, const char *s) {
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbArgs;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  if (s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "tanMediumId", s);
  else
    GWEN_DB_DeleteVar(dbArgs, "tanMediumId");
}



void AH_Job_Tan_SetLocalAccountInfo(AH_JOB *j,
                                    const char *bankCode,
                                    const char *accountId,
                                    const char *accountSubId) {
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbArgs;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  if (bankCode && *bankCode)
      GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "localAccount/bankCode", bankCode);
  if (accountId && *accountId)
      GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "localAccount/accountId", accountId);
  if (accountSubId && *accountSubId)
      GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "localAccount/accountSubId", accountSubId);
}



void AH_Job_Tan_SetSmsAccountInfo(AH_JOB *j,
                                  const char *bankCode,
                                  const char *accountId,
                                  const char *accountSubId) {
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbArgs;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  if (bankCode && *bankCode)
      GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "smsAccount/bankCode", bankCode);
  if (accountId && *accountId)
      GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "smsAccount/accountId", accountId);
  if (accountSubId && *accountSubId)
      GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "smsAccount/accountSubId", accountSubId);
}



int AH_Job_Tan_FinishSetup(AH_JOB *j, AH_JOB *accJob) {
  AH_JOB_TAN *aj;
  int rv;
  GWEN_DB_NODE *args;
  GWEN_DB_NODE *dbParams;
  GWEN_DB_NODE *dbMethod;
  const char *s;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  args=AH_Job_GetArguments(j);
  assert(args);
  
  dbParams=AH_Job_GetParams(j);
  assert(dbParams);
  
  /* get data for the selected tan method */
  dbMethod=GWEN_DB_FindFirstGroup(dbParams, "tanMethod");
  while(dbMethod) {
    int tm;
  
    tm=GWEN_DB_GetIntValue(dbMethod, "function", 0, -1);
    if (tm!=-1 && tm==aj->tanMethod)
      break;
  
    dbMethod=GWEN_DB_FindNextGroup(dbMethod, "tanMethod");
  }
  
  if (!dbMethod) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No info for the selected iTAN method");
    return GWEN_ERROR_GENERIC;
  }
  
  rv=AH_Job_AddChallengeParams(accJob, AH_Job_GetSegmentVersion(j), dbMethod);
  if (rv<0) {
    if (rv==GWEN_ERROR_NOT_SUPPORTED) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Challenge parameters not supported by job, ignoring");
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    GWEN_STRINGLIST *sl;

    /* add challenge params as provided by addChallengeParams function */
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Using result of AddChallengeParams function");
    sl=AH_Job_GetChallengeParams(accJob);
    if (sl) {
      GWEN_STRINGLISTENTRY *e;

      e=GWEN_StringList_FirstEntry(sl);
      while(e) {
        GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_DEFAULT,
                             "challengeParams/param",
                             GWEN_StringListEntry_Data(e));
        e=GWEN_StringListEntry_Next(e);
      }
    }
  }

  /* set challenge class */
  s=GWEN_DB_GetCharValue(dbMethod, "needChallengeClass", 0, "N");
  if (strcasecmp(s, "J")==0)
    GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "challengeClass", AH_Job_GetChallengeClass(accJob));

  return 0;
}






/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_GetItanModes
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

GWEN_INHERIT(AH_JOB, AH_JOB_GETITANMODES);


AH_JOB *AH_Job_GetItanModes_new(AB_USER *u){
  AH_JOB *j;
  GWEN_DB_NODE *args;
  AH_JOB_GETITANMODES *jd;

  assert(u);
  j=AH_Job_new("JobGetItanModes", u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "JobGetItanModes not supported, should not happen");
    return 0;
  }
  GWEN_NEW_OBJECT(AH_JOB_GETITANMODES, jd);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_GETITANMODES, j, jd,
                       AH_Job_GetItanModes_FreeData);
  AH_Job_SetProcessFn(j, AH_Job_GetItanModes_Process);

  /* set arguments */
  args=AH_Job_GetArguments(j);
  assert(args);
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "prepare/bpdversion", 0);
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "prepare/updversion", 0);

  jd->modeList[0]=-1;
  jd->modeCount=0;

  DBG_INFO(AQHBCI_LOGDOMAIN, "JobGetItanModes created");
  return j;
}



void GWENHYWFAR_CB AH_Job_GetItanModes_FreeData(void *bp, void *p){
  AH_JOB_GETITANMODES *jd;

  jd=(AH_JOB_GETITANMODES*)p;
  GWEN_FREE_OBJECT(jd);
}



int AH_Job_GetItanModes_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_GETITANMODES *jd;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  GWEN_DB_NODE *dbMsgResponse;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETITANMODES, j);
  assert(jd);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Parsing this response");
  if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Debug)
    GWEN_DB_Dump(dbResponses, 2);

  /* search for "SegResult" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);

  while(dbCurr) {
    dbMsgResponse=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                   "data/SegResult");
    if (dbMsgResponse){
      GWEN_DB_NODE *dbRes;

      dbRes=GWEN_DB_FindFirstGroup(dbMsgResponse, "result");
      while(dbRes){
        int code;

        code=GWEN_DB_GetIntValue(dbRes, "resultCode", 0, -1);
        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Found message result (%d)", code);
        if (code==3920) {
          int i;

	  jd->modeList[0]=-1;
	  jd->modeCount=0;

	  for (i=0; ; i++) {
	    int k;

	    k=GWEN_DB_GetIntValue(dbRes, "param", i, 0);
	    if (k==0)
	      break;
	    if (jd->modeCount<AH_JOB_GETITANMODES_MAXMODES) {
	      jd->modeList[jd->modeCount++]=k;
              jd->modeList[jd->modeCount]=-1;
	    }
	    else
              break;
	  } /* for */
	  if (i==0) {
	    DBG_ERROR(AQHBCI_LOGDOMAIN,
		      "Bad server response: No TAN method reported");
	    return -1;
	  }
	} /* if correct result found */

	dbRes=GWEN_DB_FindNextGroup(dbRes, "result");
      } /* while result */
    }
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */

  return 0;
}



const int *AH_Job_GetItanModes_GetModes(const AH_JOB *j){
  AH_JOB_GETITANMODES *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETITANMODES, j);
  assert(jd);

  return jd->modeList;
}



/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_ChangePin
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */



/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_ChangePin_new(AB_USER *u, const char *newPin){
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;

  assert(u);
  j=AH_Job_new("JobChangePin", u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "JobChangePin not supported, should not happen");
    return 0;
  }

  /* set challenge class */
  AH_Job_SetChallengeClass(j, 90);

  /* set arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "newPin", newPin);

  DBG_INFO(AQHBCI_LOGDOMAIN, "JobChangePin created");
  return j;
}







/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_GetAccountSepaInfo
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */



GWEN_INHERIT(AH_JOB, AH_JOB_GETACCSEPAINFO)

/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_GetAccountSepaInfo_new(AB_USER *u, AB_ACCOUNT *acc) {
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;
  AH_JOB_GETACCSEPAINFO *jd;
  const char *s;

  assert(u);
  j=AH_Job_new("JobGetAccountSepaInfo", u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "JobGetAccountSepaInfo not supported, should not happen");
    return 0;
  }

  GWEN_NEW_OBJECT(AH_JOB_GETACCSEPAINFO, jd);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_GETACCSEPAINFO, j, jd,
                       AH_Job_GetAccountSepaInfo_FreeData)
  AH_Job_SetProcessFn(j, AH_Job_GetAccountSepaInfo_Process);

  jd->account=acc;

  /* set arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  s=AB_Account_GetAccountNumber(jd->account);
  if (s && *s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "accountId", s);

  s=AB_Account_GetSubAccountId(jd->account);
  if (s && *s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "accountSubId", s);

  s=AB_Account_GetBankCode(jd->account);
  if (s && *s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "bankCode", s);

  GWEN_DB_SetIntValue(dbArgs, GWEN_DB_FLAGS_DEFAULT,
                      "country", 280);

  s=AB_Account_GetIBAN(jd->account);
  if (s && *s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "iban", s);

  s=AB_Account_GetBIC(jd->account);
  if (s && *s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "bic", s);


  DBG_INFO(AQHBCI_LOGDOMAIN, "JobGetAccountSepaInfo created");
  return j;
}



void GWENHYWFAR_CB AH_Job_GetAccountSepaInfo_FreeData(void *bp, void *p){
  AH_JOB_GETACCSEPAINFO *jd;

  jd=(AH_JOB_GETACCSEPAINFO*)p;
  GWEN_FREE_OBJECT(jd);
}



int AH_Job_GetAccountSepaInfo_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_GETACCSEPAINFO *jd;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  AB_USER *u;
  AB_BANKING *ab;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETACCSEPAINFO, j);
  assert(jd);

  if (jd->scanned)
    return 0;

  jd->scanned=1;

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  u=AH_Job_GetUser(j);
  assert(u);

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  /* search for "GetAccountSepaInfoResponse" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while(dbCurr) {
    GWEN_DB_NODE *dbXA;
    int rv;

    rv=AH_Job_CheckEncryption(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (encryption)");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }
    rv=AH_Job_CheckSignature(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (signature)");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }

    dbXA=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                          "data/GetAccountSepaInfoResponse");
    if (dbXA) {
      const char *accountId;
      const char *bankCode;
      const char *accountSuffix;
      const char *iban;
      const char *bic;
      const char *sSepa;
      int useWithSepa=0;

      /* account data found */
      DBG_INFO(AQHBCI_LOGDOMAIN, "Found a GetAccountSepaInfoResponse segment");
      accountId=GWEN_DB_GetCharValue(dbXA, "accountId", 0, 0);
      accountSuffix=GWEN_DB_GetCharValue(dbXA, "accountsubid", 0, 0);
      bankCode=GWEN_DB_GetCharValue(dbXA, "bankCode", 0, 0);
      sSepa=GWEN_DB_GetCharValue(dbXA, "sepa", 0, "n");
      if (strcasecmp(sSepa, "j")==0)
        useWithSepa=1;

      iban=GWEN_DB_GetCharValue(dbXA, "iban", 0, 0);
      bic=GWEN_DB_GetCharValue(dbXA, "bic", 0, 0);

      rv=AB_Banking_BeginExclUseAccount(ab, jd->account);
      if (rv<0) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Unable to lock account");
      }
      else {
        if (accountSuffix)
          AB_Account_SetSubAccountId(jd->account, accountSuffix);
        AH_Account_AddFlags(jd->account, AH_BANK_FLAGS_KTV2); /* we have a sub id (even if emtpy), set flag */

	if (useWithSepa) {
	  DBG_INFO(AQHBCI_LOGDOMAIN, "SEPA available with this account");
	  AH_Account_AddFlags(jd->account, AH_BANK_FLAGS_SEPA); /* we have a sub id (even if emtpy), set flag */
	}
	else {
	  DBG_INFO(AQHBCI_LOGDOMAIN, "SEPA not available with this account");
	  AH_Account_SubFlags(jd->account, AH_BANK_FLAGS_SEPA); /* we have a sub id (even if emtpy), set flag */
	}

        if (iban && *iban) {
	  if (bic && *bic) {
	    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Setting IBAN and BIC: %s/%s", iban, bic);
	    AB_Account_SetIBAN(jd->account, iban);
	    AB_Account_SetBIC(jd->account, bic);
	  }
	  else {
	    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Setting IBAN (no BIC): %s", iban);
	    AB_Account_SetIBAN(jd->account, iban);
	  }
        }
        else {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing information in account: BLZ=[%s], Kto=[%s], IBAN=[%s], BIC=[%s]",
                    bankCode?bankCode:"",
                    accountId?accountId:"",
                    iban?iban:"",
                    bic?bic:"");
        }
        AB_Banking_EndExclUseAccount(ab, jd->account, 0);
      }
    }

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  return 0;
}



