/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
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
#include "medium_l.h"

#include "adminjobs_p.h"
#include "job_l.h"
#include "jobqueue_l.h"
#include <aqhbci/hbci.h>
#include <aqhbci/job.h>
#include <aqhbci/provider.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/net2.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/inherit.h>

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

  assert(u);
  j=AH_Job_new("JobGetKeys", u, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "JobGetKeys not supported, should not happen");
    return 0;
  }

  /* create data for inheriting class */
  GWEN_NEW_OBJECT(AH_JOB_GETKEYS, jd);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_GETKEYS,
                       j, jd, AH_Job_GetKeys_FreeData);

  /* overwrite virtual functions */
  AH_Job_SetProcessFn(j, AH_Job_GetKeys_Process);
  AH_Job_SetCommitFn(j, AH_Job_GetKeys_Commit);

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

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Job_GetKeys_FreeData(void *bp, void *p){
  AH_JOB *j;
  AH_JOB_GETKEYS *jd;

  j=(AH_JOB*) bp;
  jd=(AH_JOB_GETKEYS*) p;

  GWEN_CryptKey_free(jd->signKey);
  GWEN_CryptKey_free(jd->cryptKey);
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetKeys_Process(AH_JOB *j){
  AH_JOB_GETKEYS *jd;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  int haveKey;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETKEYS, j);
  assert(jd);

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
      GWEN_CRYPTKEY *key;
      GWEN_DB_NODE *dbKey;
      unsigned int bs;
      const void *p;
      const char *ktype;
      const char defaultExpo[3]={0x01, 0x00, 0x01};

      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Got this key response:");
      if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevelDebug)
        GWEN_DB_Dump(dbKeyResponse, stderr, 2);

      dbKey=GWEN_DB_Group_new("key");
      GWEN_DB_SetCharValue(dbKey,
                           GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "type",
                           "RSA");
      /* TODO: check for the correct exponent (for now assume 65537) */
      GWEN_DB_SetBinValue(dbKey,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "data/e",
                          defaultExpo,
                          sizeof(defaultExpo));

      GWEN_DB_SetIntValue(dbKey,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "data/public",
                          1);

      GWEN_DB_SetCharValue(dbKey,
                           GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "name",
                           GWEN_DB_GetCharValue(dbKeyResponse,
                                                "keyname/keytype", 0,
                                                "V"));
      GWEN_DB_SetCharValue(dbKey,
                           GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "owner",
                           GWEN_DB_GetCharValue(dbKeyResponse, "keyname/userId", 0, ""));
      GWEN_DB_SetIntValue(dbKey,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "number",
                          GWEN_DB_GetIntValue(dbKeyResponse, "keyname/keynum", 0, 0));
      GWEN_DB_SetIntValue(dbKey,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "version",
                          GWEN_DB_GetIntValue(dbKeyResponse, "keyname/keyversion", 0, 0));

      p=GWEN_DB_GetBinValue(dbKeyResponse, "key/modulus", 0, 0, 0 , &bs);
      if (!p || !bs) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "No modulus");
          GWEN_DB_Group_free(dbKey);
          return -1;
      }
      GWEN_DB_SetBinValue(dbKey,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "data/n",
                          p, bs);

      /* now dbKey contains the key */
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Creating key");
      key=GWEN_CryptKey_fromDb(dbKey);
      GWEN_DB_Group_free(dbKey);
      if (!key) {
        DBG_ERROR(AQHBCI_LOGDOMAIN,
                  "Could not create key (maybe type RSA is not supported)");
        return -1;
      }
      ktype=GWEN_CryptKey_GetKeyName(key);
      if (strcasecmp(ktype, "S")==0) {
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Got the server's sign key");
        jd->signKey=key;
      }
      else if (strcasecmp(ktype, "V")==0) {
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Got the server's crypt key");
        jd->cryptKey=key;
      }
      else {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Got an unknown key type (\"%s\")", ktype);
        GWEN_CryptKey_free(key);
        return -1;
      }
      haveKey++;
    } /* if we have one */
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */

  if (haveKey==0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "No server keys found");
      AH_Job_SetStatus(j, AH_JobStatusError);
    return AB_ERROR_NO_DATA;
  }
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetKeys_Commit(AH_JOB *j){
  AH_JOB_GETKEYS *jd;
  AB_USER *u;
  AH_MEDIUM *m;
  AH_MEDIUM_CTX *mctx;
  int rv;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETKEYS, j);
  assert(jd);

  if (AH_Job_DefaultCommitHandler(j)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error in default commit handler, continue anyway");
  }

  if (jd->signKey==0 && jd->cryptKey==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No keys received");
    return -1;
  }

  u=AH_Job_GetUser(j);
  assert(u);

  m=AH_User_GetMedium(u);
  assert(m);
  rv=AH_Medium_Mount(m);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not mount medium (%d)", rv);
    return rv;
  }

  rv=AH_Medium_SelectContext(m, AH_User_GetContextIdx(u));
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not select user %s/%s/%s (%d)",
              AB_User_GetCountry(u),
              AB_User_GetBankCode(u),
              AB_User_GetUserId(u),
              rv);
    AH_Medium_Unmount(m, 0);
    return rv;
  }

  mctx=AH_Medium_GetCurrentContext(m);
  assert(mctx);

  if (jd->signKey) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Setting sign key");
    GWEN_CryptKey_SetStatus(jd->signKey, GWEN_CRYPTTOKEN_KEYSTATUS_ACTIVE);
    AH_Medium_SetPubSignKey(m, jd->signKey);
  }

  if (jd->cryptKey) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Setting crypt key");
    GWEN_CryptKey_SetStatus(jd->cryptKey, GWEN_CRYPTTOKEN_KEYSTATUS_ACTIVE);
    AH_Medium_SetPubCryptKey(m, jd->cryptKey);
  }

  rv=AH_Medium_Unmount(m, 0);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not unmount medium (%d)", rv);
    return -1;
  }

  if (!AH_User_GetPeerId(u)) {
    if (jd->signKey)
      AH_User_SetPeerId(u, GWEN_CryptKey_GetOwner(jd->signKey));
    else if (jd->cryptKey)
      AH_User_SetPeerId(u, GWEN_CryptKey_GetOwner(jd->cryptKey));
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Keys saved");
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_CRYPTKEY *AH_Job_GetKeys_GetSignKey(const AH_JOB *j){
  AH_JOB_GETKEYS *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETKEYS, j);
  assert(jd);

  return jd->signKey;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_CRYPTKEY *AH_Job_GetKeys_GetCryptKey(const AH_JOB *j){
  AH_JOB_GETKEYS *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETKEYS, j);
  assert(jd);

  return jd->cryptKey;
}






/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_SendKeys
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */



/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_SendKeys_new(AB_USER *u,
                            const GWEN_CRYPTKEY *cryptKey,
                            const GWEN_CRYPTKEY *signKey){
  AH_JOB *j;
  GWEN_DB_NODE *dbKey;

  assert(u);
  j=AH_Job_new("JobSendKeys", u, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "JobSendKeys not supported, should not happen");
    return 0;
  }

  /* set arguments */
  dbKey=GWEN_DB_GetGroup(AH_Job_GetArguments(j),
                         GWEN_DB_FLAGS_DEFAULT,
                         "signKey");
  assert(dbKey);
  if (AH_Job_SendKeys_PrepareKey(j, dbKey, signKey)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not prepare signkey");
    AH_Job_free(j);
    return 0;
  }

  dbKey=GWEN_DB_GetGroup(AH_Job_GetArguments(j),
                         GWEN_DB_FLAGS_DEFAULT,
                         "cryptKey");
  assert(dbKey);
  if (AH_Job_SendKeys_PrepareKey(j, dbKey, cryptKey)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not prepare cryptkey");
    AH_Job_free(j);
    return 0;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "JobSendKeys created");
  return j;
}




int AH_Job_SendKeys_PrepareKey(AH_JOB *j,
                               GWEN_DB_NODE *dbKey,
                               const GWEN_CRYPTKEY *key) {
  int isCryptKey;
  GWEN_DB_NODE *dbTmp;
  unsigned int bsize;
  const void *p;
  AB_USER *u;
  GWEN_ERRORCODE err;
  const char *userId;
  const AB_COUNTRY *pcountry;
  int country;

  assert(j);
  assert(dbKey);
  assert(key);

  u=AH_Job_GetUser(j);
  assert(u);

  userId=AB_User_GetUserId(u);
  assert(userId);
  assert(*userId);

  if (strcasecmp(GWEN_CryptKey_GetKeyName(key), "V")==0)
    isCryptKey=1;
  else if (strcasecmp(GWEN_CryptKey_GetKeyName(key), "S")==0)
    isCryptKey=0;
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Invalid key type \"%s\"", GWEN_CryptKey_GetKeyName(key));
    return -1;
  }

  /* set keyname */
  pcountry=AB_Banking_FindCountryByName(AH_Job_GetBankingApi(j),
                                        AB_User_GetCountry(u));
  if (pcountry)
    country=AB_Country_GetNumericCode(pcountry);
  else
    country=280;

  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "keyName/country", country);
  GWEN_DB_SetCharValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "keyName/bankCode", AB_User_GetBankCode(u));

  GWEN_DB_SetCharValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "keyName/userid", userId);
  GWEN_DB_SetCharValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "keyName/keyType", GWEN_CryptKey_GetKeyName(key));
  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "keyName/keyNum", GWEN_CryptKey_GetNumber(key));
  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "keyName/keyVersion", GWEN_CryptKey_GetVersion(key));

  /* set key */
  if (isCryptKey) {
    GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "key/purpose", 5);

  }
  else {
    GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "key/purpose", 6);
  }
  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "key/opmode", 16);
  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "key/type", 10);
  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "key/modname", 12);
  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "key/expname", 13);

  dbTmp=GWEN_DB_Group_new("keydata");
  err=GWEN_CryptKey_toDb(key, dbTmp, 1);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not write key to DB");
    GWEN_DB_Group_free(dbTmp);
    return -1;
  }

  p=GWEN_DB_GetBinValue(dbTmp, "data/n", 0, 0, 0, &bsize);
  if (!p) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No modulus in key");
    GWEN_DB_Group_free(dbTmp);
    return -1;
  }

  GWEN_DB_SetBinValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "key/modulus", p, bsize);

  p=GWEN_DB_GetBinValue(dbTmp, "data/e", 0, 0, 0, &bsize);
  if (!p) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No exponent in key");
    GWEN_DB_Group_free(dbTmp);
    return -1;
  }

  GWEN_DB_SetBinValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "key/exponent", p, bsize);
  GWEN_DB_Group_free(dbTmp);

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
  j=AH_Job_new("JobUpdateBankInfo", u, 0);
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



void AH_Job_UpdateBank_FreeData(void *bp, void *p){
  AH_JOB_UPDATEBANK *jd;

  jd=(AH_JOB_UPDATEBANK*)p;
  AB_Account_List2_FreeAll(jd->accountList);
}



int AH_Job_UpdateBank_Process(AH_JOB *j){
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
                                   "data/AccountData");
    if (dbAccountData) {
      const char *accountId;
      const char *userName;
      const char *accountName;
      const char *bankCode;
      AB_ACCOUNT *acc;
  
      DBG_INFO(AQHBCI_LOGDOMAIN, "Found an account");
      accs++;

      /* account data found */
      accountId=GWEN_DB_GetCharValue(dbAccountData, "accountId", 0, 0);
      assert(accountId);
      accountName=GWEN_DB_GetCharValue(dbAccountData, "account/name", 0, 0);
      userName=GWEN_DB_GetCharValue(dbAccountData, "name1", 0, 0);
      bankCode=GWEN_DB_GetCharValue(dbAccountData, "bankCode", 0, 0);
      assert(bankCode);

      acc=AB_Banking_CreateAccount(ab, AH_PROVIDER_NAME);
      assert(acc);
      AB_Account_SetBankCode(acc, bankCode);
      AB_Account_SetAccountNumber(acc, accountId);

      if (accountName) {
        GWEN_BUFFER *xbuf;

        xbuf=GWEN_Buffer_new(0, 32, 0, 1);
        AH_HBCI_HbciToUtf8(accountName, 0, xbuf);
        AB_Account_SetAccountName(acc, GWEN_Buffer_GetStart(xbuf));
        GWEN_Buffer_free(xbuf);
      }
      if (userName) {
        GWEN_BUFFER *xbuf;

        xbuf=GWEN_Buffer_new(0, 32, 0, 1);
        AH_HBCI_HbciToUtf8(userName, 0, xbuf);
        AB_Account_SetOwnerName(acc, GWEN_Buffer_GetStart(xbuf));
        GWEN_Buffer_free(xbuf);
      }

      AB_Account_List2_PushBack(jd->accountList, acc);
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
  j=AH_Job_new("JobSync", u, 0);
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



void AH_Job_GetSysId_FreeData(void *bp, void *p){
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
  if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevelDebug)
    GWEN_DB_Dump(dbResponses, stderr, 2);

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


int AH_Job_GetSysId_Process(AH_JOB *j){
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
    j=AH_Job_new("JobDialogInitAnon", u, 0);
  else
    j=AH_Job_new("JobDialogInit", u, 0);
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



void AH_Job_TestVersion_FreeData(void *bp, void *p){
  AH_JOB_TESTVERSION *jd;

  jd=(AH_JOB_TESTVERSION*)p;
  GWEN_FREE_OBJECT(jd);
}



int AH_Job_TestVersion_Process(AH_JOB *j){
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
  if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevelDebug)
    GWEN_DB_Dump(dbResponses, stderr, 2);

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
        code=GWEN_DB_GetIntValue(dbResult, "code", 0, -1);
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

  j=AH_Job_new("JobGetStatus", u, 0);
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



void AH_Job_GetStatus_FreeData(void *bp, void *p){
  AH_JOB_GETSTATUS *aj;

  aj=(AH_JOB_GETSTATUS*)p;
  AH_Result_List_free(aj->results);
  GWEN_FREE_OBJECT(aj);
}



int AH_Job_GetStatus_Process(AH_JOB *j){
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
                              AH_JOB_EXCHANGE_MODE m){
  AH_JOB_GETSTATUS *aj;

  DBG_WARN(AQHBCI_LOGDOMAIN, "Exchanging (%d), should not happen...", m);
  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETSTATUS, j);
  assert(aj);

  return 0;
}










