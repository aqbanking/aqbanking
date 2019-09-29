/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "aqbanking/i18n_l.h"
#include "user_p.h"
#include "hbci_l.h"
#include "provider_l.h"
#include "hbci-updates_l.h"
#include "msgengine_l.h"
#include "tanmethod_l.h"
#include "aqhbci/banking/provider.h"
#include "adminjobs_l.h"

#include "hhd_l.h"

#include <aqbanking/banking_be.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>

#include <assert.h>


GWEN_INHERIT(AB_USER, AH_USER)


const char *AH_User_Status_toString(AH_USER_STATUS st)
{
  switch (st) {
  case AH_UserStatusNew:
    return "new";
  case AH_UserStatusEnabled:
    return "enabled";
  case AH_UserStatusPending:
    return "pending";
  case AH_UserStatusDisabled:
    return "disabled";
  default:
    return "unknown";
  } /* switch */
}



AH_USER_STATUS AH_User_Status_fromString(const char *s)
{
  assert(s);
  if (strcasecmp(s, "new")==0)
    return AH_UserStatusNew;
  else if (strcasecmp(s, "enabled")==0)
    return AH_UserStatusEnabled;
  else if (strcasecmp(s, "pending")==0)
    return AH_UserStatusPending;
  else if (strcasecmp(s, "disabled")==0)
    return AH_UserStatusDisabled;
  else
    return AH_UserStatusUnknown;
}



void AH_User_Flags_toDb(GWEN_DB_NODE *db, const char *name,
                        uint32_t flags)
{
  GWEN_DB_DeleteVar(db, name);
  if (flags & AH_USER_FLAGS_BANK_DOESNT_SIGN)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "bankDoesntSign");
  if (flags & AH_USER_FLAGS_BANK_USES_SIGNSEQ)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "bankUsesSignSeq");
  if (flags & AH_USER_FLAGS_IGNORE_UPD)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "ignoreUpd");
  if (flags & AH_USER_FLAGS_NO_BASE64)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "noBase64");
  if (flags & AH_USER_FLAGS_KEEP_MULTIPLE_BLANKS)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "keepMultipleBlanks");
  if (flags & AH_USER_FLAGS_TAN_OMIT_SMS_ACCOUNT)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "omitSmsAccount");
  if (flags & AH_USER_FLAGS_USE_STRICT_SEPA_CHARSET)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "useStrictSepaCharset");
  if (flags & AH_USER_FLAGS_TLS_IGN_PREMATURE_CLOSE)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "tlsIgnPrematureClose");
  if (flags & AH_USER_FLAGS_VERIFY_NO_BANKSIGNKEY)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "verifyNoBankSignKey");
}



uint32_t AH_User_Flags_fromDb(GWEN_DB_NODE *db, const char *name)
{
  uint32_t fl=0;
  int i;

  for (i=0; ; i++) {
    const char *s;

    s=GWEN_DB_GetCharValue(db, name, i, 0);
    if (!s)
      break;
    if (strcasecmp(s, "bankDoesntSign")==0)
      fl|=AH_USER_FLAGS_BANK_DOESNT_SIGN;
    else if (strcasecmp(s, "bankUsesSignSeq")==0)
      fl|=AH_USER_FLAGS_BANK_USES_SIGNSEQ;
    else if (strcasecmp(s, "ignoreUpd")==0)
      fl|=AH_USER_FLAGS_IGNORE_UPD;
    else if (strcasecmp(s, "noBase64")==0)
      fl|=AH_USER_FLAGS_NO_BASE64;
    else if (strcasecmp(s, "keepMultipleBlanks")==0)
      fl|=AH_USER_FLAGS_KEEP_MULTIPLE_BLANKS;
    else if (strcasecmp(s, "omitSmsAccount")==0)
      fl|=AH_USER_FLAGS_TAN_OMIT_SMS_ACCOUNT;
    else if (strcasecmp(s, "useStrictSepaCharset")==0)
      fl|=AH_USER_FLAGS_USE_STRICT_SEPA_CHARSET;
    else if (strcasecmp(s, "tlsIgnPrematureClose")==0)
      fl|=AH_USER_FLAGS_TLS_IGN_PREMATURE_CLOSE;
    else if (strcasecmp(s, "verifyNoBankSignKey")==0)
      fl|=AH_USER_FLAGS_VERIFY_NO_BANKSIGNKEY;
    else {
      DBG_WARN(AQHBCI_LOGDOMAIN, "Unknown user flag \"%s\"", s);
    }
  }

  return fl;
}





AB_USER *AH_User_new(AB_PROVIDER *pro)
{
  AB_USER *u;
  AH_USER *ue;

  assert(pro);
  u=AB_User_new();
  assert(u);
  GWEN_NEW_OBJECT(AH_USER, ue);
  GWEN_INHERIT_SETDATA(AB_USER, AH_USER, u, ue, AH_User_freeData);

  AB_User_SetProvider(u, pro);
  AB_User_SetBackendName(u, "aqhbci");

  ue->readFromDbFn=AB_User_SetReadFromDbFn(u, AH_User_ReadFromDb);
  ue->writeToDbFn=AB_User_SetWriteToDbFn(u, AH_User_WriteToDb);

  ue->tanMethodList[0]=-1;
  ue->tanMethodCount=0;

  ue->hbci=AH_Provider_GetHbci(pro);
  ue->tanMethodDescriptions=AH_TanMethod_List_new();
  ue->sepaDescriptors=GWEN_StringList_new();

  AB_User_SetCountry(u, "de");

  ue->msgEngine=AH_MsgEngine_new();
  GWEN_MsgEngine_SetEscapeChar(ue->msgEngine, '?');
  GWEN_MsgEngine_SetCharsToEscape(ue->msgEngine, ":+\'@");
  AH_MsgEngine_SetUser(ue->msgEngine, u);
  GWEN_MsgEngine_SetDefinitions(ue->msgEngine, AH_HBCI_GetDefinitions(ue->hbci), 0);

  ue->hbciVersion=210;
  ue->bpd=AH_Bpd_new();
  ue->dbUpd=GWEN_DB_Group_new("upd");
  ue->maxTransfersPerJob=AH_USER_MAX_TRANSFERS_PER_JOB;
  ue->maxDebitNotesPerJob=AH_USER_MAX_DEBITNOTES_PER_JOB;

  return u;
}



void GWENHYWFAR_CB AH_User_freeData(void *bp, void *p)
{
  AH_USER *ue;

  ue=(AH_USER *)p;
  free(ue->tanMediumId);
  free(ue->peerId);
  free(ue->systemId);
  free(ue->httpContentType);
  free(ue->httpUserAgent);
  free(ue->tokenType);
  free(ue->tokenName);
  free(ue->prompt);
  GWEN_Url_free(ue->serverUrl);
  GWEN_DB_Group_free(ue->dbUpd);
  GWEN_Crypt_Key_free(ue->bankPubSignKey);
  GWEN_Crypt_Key_free(ue->bankPubCryptKey);
  AH_Bpd_free(ue->bpd);
  GWEN_MsgEngine_free(ue->msgEngine);
  AH_TanMethod_List_free(ue->tanMethodDescriptions);
  GWEN_StringList_free(ue->sepaDescriptors);
  GWEN_FREE_OBJECT(ue);
}



int AH_User_ReadFromDb(AB_USER *u, GWEN_DB_NODE *db)
{
  AH_USER *ue;
  int rv;
  GWEN_DB_NODE *dbP;
  AB_PROVIDER *pro;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  /* save provider, because AB_User_ReadFromDb clears it */
  pro=AB_User_GetProvider(u);

  /* read data for base class */
  rv=(ue->readFromDbFn)(u, db);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* set provider again */
  AB_User_SetProvider(u, pro);

  /* read data for provider */
  dbP=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "data/backend");
  AH_User__ReadDb(u, dbP);

  AH_User_LoadTanMethods(u);
  AH_User_LoadSepaDescriptors(u);

  return 0;
}



int AH_User_WriteToDb(const AB_USER *u, GWEN_DB_NODE *db)
{
  AH_USER *ue;
  int rv;
  GWEN_DB_NODE *dbP;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  /* write data for base class */
  rv=(ue->writeToDbFn)(u, db);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }


  /* write data for provider */
  dbP=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "data/backend");
  AH_User__WriteDb(u, dbP);

  return 0;
}



void AH_User__ReadDb(AB_USER *u, GWEN_DB_NODE *db)
{
  AH_USER *ue;
  const char *s;
  GWEN_DB_NODE *gr;
  int i;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  s=GWEN_DB_GetCharValue(db, "cryptMode", 0, "unknown");
  ue->cryptMode=AH_CryptMode_fromString(s);

  s=GWEN_DB_GetCharValue(db, "status", 0, "unknown");
  ue->status=AH_User_Status_fromString(s);

  ue->hbciVersion=GWEN_DB_GetIntValue(db, "hbciVersion", 0, 210);

  /* load server address */
  GWEN_Url_free(ue->serverUrl);
  s=GWEN_DB_GetCharValue(db, "server", 0, 0);
  if (s) {
    ue->serverUrl=GWEN_Url_fromString(s);
    assert(ue->serverUrl);
    if (GWEN_Url_GetPort(ue->serverUrl)==0) {
      if (AH_User_GetCryptMode(u)==AH_CryptMode_Pintan) {
        GWEN_Url_SetPort(ue->serverUrl, 443);
        GWEN_Url_SetProtocol(ue->serverUrl, "https");
      }
      else {
        GWEN_Url_SetProtocol(ue->serverUrl, "hbci");
        GWEN_Url_SetPort(ue->serverUrl, 3000);
      }
    }
  }
  else
    ue->serverUrl=NULL;

  /* load bankPubCryptKey */
  GWEN_Crypt_Key_free(ue->bankPubCryptKey);
  gr=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "bankPubCryptKey");
  if (gr==NULL)
    gr=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "bankPubKey");
  if (gr) {
    ue->bankPubCryptKey=GWEN_Crypt_KeyRsa_fromDb(gr);
    assert(ue->bankPubCryptKey);
  }
  else
    ue->bankPubCryptKey=NULL;

  /* load bankPubKey */
  GWEN_Crypt_Key_free(ue->bankPubSignKey);
  gr=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "bankPubSignKey");
  if (gr) {
    ue->bankPubSignKey=GWEN_Crypt_KeyRsa_fromDb(gr);
    assert(ue->bankPubSignKey);
  }
  else
    ue->bankPubSignKey=NULL;

  /* load BPD */
  AH_Bpd_free(ue->bpd);
  gr=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "bpd");
  if (gr) {
    ue->bpd=AH_Bpd_FromDb(gr);
    assert(ue->bpd);
  }
  else
    ue->bpd=AH_Bpd_new();

  /* load UPD */
  if (ue->dbUpd)
    GWEN_DB_Group_free(ue->dbUpd);
  gr=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "upd");
  if (gr) {
    ue->dbUpd=GWEN_DB_Group_dup(gr);
  }
  else
    ue->dbUpd=GWEN_DB_Group_new("upd");

  /* get peer id */
  free(ue->peerId);
  s=GWEN_DB_GetCharValue(db, "peerId", 0, 0);
  if (s)
    ue->peerId=strdup(s);
  else
    ue->peerId=NULL;

  /* get system id */
  free(ue->systemId);
  s=GWEN_DB_GetCharValue(db, "systemId", 0, 0);
  if (s)
    ue->systemId=strdup(s);
  else
    ue->systemId=NULL;

  ue->updVersion=GWEN_DB_GetIntValue(db, "updVersion", 0, 0);

  /* setup HTTP version */
  ue->httpVMajor=GWEN_DB_GetIntValue(db, "httpVMajor", 0, -1);
  ue->httpVMinor=GWEN_DB_GetIntValue(db, "httpVMinor", 0, -1);
  if (ue->httpVMajor==-1 || ue->httpVMinor==-1) {
    ue->httpVMajor=1;
    ue->httpVMinor=0;
  }

  free(ue->httpContentType);
  s=GWEN_DB_GetCharValue(db, "httpContentType", 0, 0);
  if (s)
    ue->httpContentType=strdup(s);
  else
    ue->httpContentType=NULL;

  /* read user flags */
  ue->flags=AH_User_Flags_fromDb(db, "userFlags");

  /* setup medium stuff */
  free(ue->tokenType);
  s=GWEN_DB_GetCharValue(db, "tokenType", 0, 0);
  if (s)
    ue->tokenType=strdup(s);
  else
    ue->tokenType=NULL;

  free(ue->tokenName);
  s=GWEN_DB_GetCharValue(db, "tokenName", 0, 0);
  if (s)
    ue->tokenName=strdup(s);
  else
    ue->tokenName=NULL;

  ue->tokenContextId=GWEN_DB_GetIntValue(db, "tokenContextId", 0, 1);

  /* get rdh type */
  ue->rdhType=GWEN_DB_GetIntValue(db, "rdhType", 0, -1);
  if (ue->rdhType<1)
    ue->rdhType=1;

  /* read supported TAN methods */
  for (i=0; i<AH_USER_MAX_TANMETHODS; i++)
    ue->tanMethodList[i]=-1;
  ue->tanMethodCount=0;

  for (i=0; i<AH_USER_MAX_TANMETHODS; i++) {
    int method;

    method=GWEN_DB_GetIntValue(db, "tanMethodList", i, -1);
    if (method==-1)
      break;
    ue->tanMethodList[ue->tanMethodCount++]=method;
    ue->tanMethodList[ue->tanMethodCount]=-1;
  }

  ue->selectedTanMethod=GWEN_DB_GetIntValue(db, "selectedTanMethod", 0, 0);

  /* read some settings */
  ue->maxTransfersPerJob=GWEN_DB_GetIntValue(db, "maxTransfersPerJob", 0, AH_USER_MAX_TRANSFERS_PER_JOB);
  ue->maxDebitNotesPerJob=GWEN_DB_GetIntValue(db, "maxDebitNotesPerJob", 0, AH_USER_MAX_DEBITNOTES_PER_JOB);

  free(ue->sepaTransferProfile);
  s=GWEN_DB_GetCharValue(db, "sepaTransferProfile", 0, NULL);
  if (s)
    ue->sepaTransferProfile=strdup(s);
  else
    ue->sepaTransferProfile=NULL;

  free(ue->sepaDebitNoteProfile);
  s=GWEN_DB_GetCharValue(db, "sepaDebitNoteProfile", 0, NULL);
  if (s)
    ue->sepaDebitNoteProfile=strdup(s);
  else
    ue->sepaDebitNoteProfile=NULL;

  free(ue->tanMediumId);
  s=GWEN_DB_GetCharValue(db, "tanMediumId", 0, NULL);
  if (s)
    ue->tanMediumId=strdup(s);
  else
    ue->tanMediumId=NULL;
}



void AH_User__WriteDb(const AB_USER *u, GWEN_DB_NODE *db)
{
  const AH_USER *ue;
  int i;
  GWEN_DB_NODE *gr;
  const char *s;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  /* save crypt mode */
  s=AH_CryptMode_toString(ue->cryptMode);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "cryptMode", s);

  /* save status */
  s=AH_User_Status_toString(ue->status);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "status", s);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "hbciVersion", ue->hbciVersion);

  if (ue->httpContentType)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "httpContentType", ue->httpContentType);
  else
    GWEN_DB_DeleteVar(db, "httpContentType");

  /* save URL */
  if (ue->serverUrl) {
    GWEN_BUFFER *nbuf;

    nbuf=GWEN_Buffer_new(0, 256, 0, 1);
    if (GWEN_Url_toString(ue->serverUrl, nbuf)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not store url");
      GWEN_Buffer_free(nbuf);
      assert(0);
    }
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "server", GWEN_Buffer_GetStart(nbuf));
    GWEN_Buffer_free(nbuf);
  } /* if serverUrl */

  /* save bankPubCryptKey */
  if (ue->bankPubCryptKey) {
    assert(ue->bankPubCryptKey);
    gr=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "bankPubCryptKey");
    assert(gr);
    GWEN_Crypt_KeyRsa_toDb(ue->bankPubCryptKey, gr, 1);
  }
  else
    GWEN_DB_DeleteVar(db, "bankPubCryptKey");

  /* save bankPubSignKey */
  if (ue->bankPubSignKey) {
    assert(ue->bankPubSignKey);
    gr=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "bankPubSignKey");
    assert(gr);
    GWEN_Crypt_KeyRsa_toDb(ue->bankPubSignKey, gr, 1);
  }
  else
    GWEN_DB_DeleteVar(db, "bankPubSignKey");

  /* save BPD */
  assert(ue->bpd);
  gr=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "bpd");
  assert(gr);
  AH_Bpd_ToDb(ue->bpd, gr);

  /* save UPD */
  if (ue->dbUpd) {
    gr=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "upd");
    assert(gr);
    GWEN_DB_AddGroupChildren(gr, ue->dbUpd);
  }

  if (ue->peerId)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "peerId", ue->peerId);

  if (ue->systemId)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "systemId", ue->systemId);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "updVersion", ue->updVersion);

  /* save http settings */
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "httpVMajor", ue->httpVMajor);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "httpVMinor", ue->httpVMinor);
  if (ue->httpUserAgent)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "httpUserAgent", ue->httpUserAgent);

  /* save flags */
  AH_User_Flags_toDb(db, "userFlags", ue->flags);

  /* save crypt token settings */
  if (ue->tokenType)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "tokenType", ue->tokenType);
  if (ue->tokenName)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "tokenName", ue->tokenName);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "tokenContextId", ue->tokenContextId);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "rdhType", ue->rdhType);

  /* store list of supported/allowed tan methods */
  GWEN_DB_DeleteVar(db, "tanMethodList");
  for (i=0; i<ue->tanMethodCount; i++) {
    if (ue->tanMethodList[i]==-1)
      break;
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
                        "tanMethodList",
                        ue->tanMethodList[i]);
  }

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "selectedTanMethod",
                      ue->selectedTanMethod);

  /* store some settings */
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "maxTransfersPerJob",
                      ue->maxTransfersPerJob);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "maxDebitNotesPerJob",
                      ue->maxDebitNotesPerJob);
  if (ue->sepaTransferProfile)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "sepaTransferProfile", ue->sepaTransferProfile);
  if (ue->sepaDebitNoteProfile)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "sepaDebitNoteProfile", ue->sepaDebitNoteProfile);
  if (ue->tanMediumId)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "tanMediumId", ue->tanMediumId);
}



const char *AH_User_GetPeerId(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->peerId;
}



void AH_User_SetPeerId(AB_USER *u, const char *s)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  free(ue->peerId);
  if (s)
    ue->peerId=strdup(s);
  else
    ue->peerId=NULL;
}



uint32_t AH_User_GetTokenContextId(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->tokenContextId;
}



void AH_User_SetTokenContextId(AB_USER *u, uint32_t id)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->tokenContextId=id;
}



AH_CRYPT_MODE AH_User_GetCryptMode(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->cryptMode;
}



void AH_User_SetCryptMode(AB_USER *u, AH_CRYPT_MODE m)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->cryptMode=m;
}



AH_USER_STATUS AH_User_GetStatus(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->status;
}



void AH_User_SetStatus(AB_USER *u, AH_USER_STATUS i)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->status=i;
}



AH_HBCI *AH_User_GetHbci(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->hbci;
}



const GWEN_URL *AH_User_GetServerUrl(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->serverUrl;
}



void AH_User_SetServerUrl(AB_USER *u, const GWEN_URL *url)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  GWEN_Url_free(ue->serverUrl);
  if (url)
    ue->serverUrl=GWEN_Url_dup(url);
  else
    ue->serverUrl=0;
}



int AH_User_GetUpdVersion(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->updVersion;
}



void AH_User_SetUpdVersion(AB_USER *u, int i)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->updVersion=i;
}



GWEN_DB_NODE *AH_User_GetUpd(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->dbUpd;
}



GWEN_DB_NODE *AH_User_GetUpdForAccountIdAndSuffix(const AB_USER *u,
                                                  const char *sAccountNumber,
                                                  const char *sAccountSuffix)
{
  AH_USER *ue;
  GWEN_DB_NODE *db;
  GWEN_BUFFER *tbuf;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  db=AH_User_GetUpd(u);
  if (db==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No upd");
    return NULL;
  }

  tbuf=GWEN_Buffer_new(0, 64, 0, 1);
  GWEN_Buffer_AppendString(tbuf, sAccountNumber);
  GWEN_Buffer_AppendString(tbuf, "-");
  /* take into account the "Unterkontomerkmal", don't rely solely on account id */
  if (sAccountSuffix && *sAccountSuffix)
    GWEN_Buffer_AppendString(tbuf, sAccountSuffix);
  else
    GWEN_Buffer_AppendString(tbuf, "none");
  DBG_INFO(AQHBCI_LOGDOMAIN, "Checking upd for account \"%s\"", GWEN_Buffer_GetStart(tbuf));
  db=GWEN_DB_GetGroup(db,
                      GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                      GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  if (db==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Falling back to old storage of UPD for account \"%s\"", sAccountNumber);
    db=AH_User_GetUpd(u);
    db=GWEN_DB_GetGroup(db,
                        GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                        sAccountNumber);
  }

  return db;
}



GWEN_DB_NODE *AH_User_GetUpdForAccount(const AB_USER *u, const AB_ACCOUNT *acc)
{
  GWEN_DB_NODE *db=NULL;

  db=AH_User_GetUpdForAccountUniqueId(u, AB_Account_GetUniqueId(acc));
  if (db==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Falling back to previous storage of UPD for account \"%u\"",
             AB_Account_GetUniqueId(acc));
    db=AH_User_GetUpdForAccountIdAndSuffix(u,
                                           AB_Account_GetAccountNumber(acc),
                                           AB_Account_GetSubAccountId(acc));
  }

  return db;
}



GWEN_DB_NODE *AH_User_GetUpdForAccountUniqueId(const AB_USER *u, uint32_t uid)
{
  AH_USER *ue;
  GWEN_DB_NODE *db;
  char numbuf[32];

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  db=AH_User_GetUpd(u);
  if (db==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No upd");
    return NULL;
  }

  snprintf(numbuf, sizeof(numbuf)-1, "uaid-%08llx",
           (unsigned long long int) uid);
  numbuf[sizeof(numbuf)-1]=0;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Checking upd for \"%s\"", numbuf);
  db=GWEN_DB_GetGroup(db,
                      GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                      numbuf);

  return db;
}



GWEN_CRYPT_KEY *AH_User_GetBankPubCryptKey(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->bankPubCryptKey;
}

void AH_User_SetBankPubCryptKey(AB_USER *u, GWEN_CRYPT_KEY *bankPubCryptKey)
{
  AH_USER *ue;

  assert(bankPubCryptKey);
  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  if (ue->bankPubCryptKey!=bankPubCryptKey) {
    //GWEN_Crypt_KeyRsa_free(ue->bankPubKey);
    if (ue->bankPubCryptKey)
      GWEN_Crypt_Key_free(ue->bankPubCryptKey);
    ue->bankPubCryptKey=GWEN_Crypt_KeyRsa_dup(bankPubCryptKey);
  }
}

GWEN_CRYPT_KEY *AH_User_GetBankPubSignKey(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->bankPubSignKey;
}

void AH_User_SetBankPubSignKey(AB_USER *u, GWEN_CRYPT_KEY *bankPubSignKey)
{
  AH_USER *ue;

  assert(bankPubSignKey);
  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  if (ue->bankPubSignKey!=bankPubSignKey) {
    //GWEN_Crypt_KeyRsa_free(ue->bankPubKey);
    if (ue->bankPubSignKey)
      GWEN_Crypt_Key_free(ue->bankPubSignKey);
    ue->bankPubSignKey=GWEN_Crypt_KeyRsa_dup(bankPubSignKey);
  }
}

AH_BPD *AH_User_GetBpd(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->bpd;
}

void AH_User_SetBpd(AB_USER *u, AH_BPD *bpd)
{
  AH_USER *ue;

  assert(bpd);
  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  if (ue->bpd!=bpd) {
    AH_Bpd_free(ue->bpd);
    ue->bpd=AH_Bpd_dup(bpd);
  }
}



int AH_User_GetBpdVersion(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  assert(ue->bpd);
  return AH_Bpd_GetBpdVersion(ue->bpd);
}



void AH_User_SetBpdVersion(AB_USER *u, int i)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  assert(ue->bpd);
  AH_Bpd_SetBpdVersion(ue->bpd, i);
}



const char *AH_User_GetSystemId(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->systemId;
}



void AH_User_SetSystemId(AB_USER *u, const char *s)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  free(ue->systemId);
  if (s)
    ue->systemId=strdup(s);
  else
    ue->systemId=NULL;
}



GWEN_MSGENGINE *AH_User_GetMsgEngine(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->msgEngine;
}



uint32_t AH_User_GetFlags(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->flags;
}



void AH_User_SetFlags(AB_USER *u, uint32_t flags)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->flags=flags;
}



void AH_User_AddFlags(AB_USER *u, uint32_t flags)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->flags|=flags;
}



void AH_User_SubFlags(AB_USER *u, uint32_t flags)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->flags&=~flags;
}



int AH_User_GetHbciVersion(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->hbciVersion;
}



void AH_User_SetHbciVersion(AB_USER *u, int i)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->hbciVersion=i;
}



const char *AH_User_GetHttpUserAgent(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->httpUserAgent;
}



void AH_User_SetHttpUserAgent(AB_USER *u, const char *s)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  free(ue->httpUserAgent);
  if (s)
    ue->httpUserAgent=strdup(s);
  else
    ue->httpUserAgent=NULL;
}



int AH_User_GetHttpVMajor(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->httpVMajor;
}



void AH_User_SetHttpVMajor(AB_USER *u, int i)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->httpVMajor=i;
}



int AH_User_GetHttpVMinor(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->httpVMinor;
}



void AH_User_SetHttpVMinor(AB_USER *u, int i)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->httpVMinor=i;
}



uint32_t AH_User_GetTanMethods(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->tanMethods;
}



void AH_User_SetTanMethods(AB_USER *u, uint32_t m)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->tanMethods=m;
}



void AH_User_AddTanMethods(AB_USER *u, uint32_t m)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->tanMethods|=m;
}



void AH_User_SubTanMethods(AB_USER *u, uint32_t m)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->tanMethods&=~m;
}



int AH_User_GetSelectedTanInputMechanism(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->selectedTanInputMechanism;
}



void AH_User_SetSelectedTanInputMechanism(AB_USER *u, int i)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->selectedTanInputMechanism=i;
}



int AH_User_GetRdhType(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->rdhType;
}



void AH_User_SetRdhType(AB_USER *u, int i)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->rdhType=i;
}



const char *AH_User_GetTokenType(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->tokenType;
}



void AH_User_SetTokenType(AB_USER *u, const char *s)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  free(ue->tokenType);
  if (s)
    ue->tokenType=strdup(s);
  else
    ue->tokenType=NULL;
}



const char *AH_User_GetTokenName(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->tokenName;
}



void AH_User_SetTokenName(AB_USER *u, const char *s)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  free(ue->tokenName);
  if (s)
    ue->tokenName=strdup(s);
  else
    ue->tokenName=NULL;
}



int AH_User_MkPasswdName(const AB_USER *u, GWEN_BUFFER *buf)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  if (ue->tokenType==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing tokenType or tokenName");
    return GWEN_ERROR_NO_DATA;
  }

  if (strcasecmp(ue->tokenType, "pintan")==0) {
    const char *s;

    GWEN_Buffer_AppendString(buf, "PIN_");
    s=AB_User_GetBankCode(u);
    if (s)
      GWEN_Buffer_AppendString(buf, s);
    GWEN_Buffer_AppendString(buf, "_");
    GWEN_Buffer_AppendString(buf, AB_User_GetUserId(u));
    return 0;
  }
  else {
    if (ue->tokenName) {
      GWEN_Buffer_AppendString(buf, "PASSWORD_");
      GWEN_Buffer_AppendString(buf, ue->tokenType);
      GWEN_Buffer_AppendString(buf, "_");
      GWEN_Buffer_AppendString(buf, ue->tokenName);
      return 0;
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing tokenName");
      return GWEN_ERROR_NO_DATA;
    }
  }
}



int AH_User_MkPinName(const AB_USER *u, GWEN_BUFFER *buf)
{
  return AH_User_MkPasswdName(u, buf);
}



int AH_User_MkTanName(const AB_USER *u,
                      const char *challenge,
                      GWEN_BUFFER *buf)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  if (ue->tokenType && ue->tokenName) {
    GWEN_Buffer_AppendString(buf, "TAN_");
    GWEN_Buffer_AppendString(buf, ue->tokenType);
    GWEN_Buffer_AppendString(buf, "_");
    GWEN_Buffer_AppendString(buf, ue->tokenName);
    if (challenge) {
      GWEN_Buffer_AppendString(buf, "_");
      GWEN_Buffer_AppendString(buf, challenge);
    }
    return 0;
  }
  else {
    const char *s;

    DBG_DEBUG(AQHBCI_LOGDOMAIN, "No tokenType or tokenName");
    GWEN_Buffer_AppendString(buf, "TAN_");
    s=AB_User_GetBankCode(u);
    if (s)
      GWEN_Buffer_AppendString(buf, s);
    GWEN_Buffer_AppendString(buf, "_");
    GWEN_Buffer_AppendString(buf, AB_User_GetUserId(u));
    if (challenge) {
      GWEN_Buffer_AppendString(buf, "_");
      GWEN_Buffer_AppendString(buf, challenge);
    }
    return 0;
  }
}



int AH_User_InputPin(AB_USER *u,
                     char *pwbuffer,
                     int minLen, int maxLen,
                     int flags)
{
  AB_PROVIDER *pro;
  AB_BANKING *ab;
  GWEN_BUFFER *nbuf;
  int rv;
  const char *numeric_warning = "";
  char buffer[512];
  const char *un;
  const char *bn=NULL;
  AB_BANKINFO *bi;

  assert(u);
  un=AB_User_GetUserId(u);

  pro=AB_User_GetProvider(u);
  assert(pro);
  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  /* find bank name */
  bi=AB_Banking_GetBankInfo(ab, "de", "*", AB_User_GetBankCode(u));
  if (bi)
    bn=AB_BankInfo_GetBankName(bi);
  if (!bn)
    AB_User_GetBankCode(u);

  buffer[0]=0;
  buffer[sizeof(buffer)-1]=0;
  if (flags & GWEN_GUI_INPUT_FLAGS_NUMERIC) {
    numeric_warning = I18N(" You must only enter numbers, not letters.");
  }
  if (flags & GWEN_GUI_INPUT_FLAGS_CONFIRM) {
    snprintf(buffer, sizeof(buffer)-1,
             I18N("Please enter a new PIN for \n"
                  "user %s at %s\n"
                  "The input must be at least %d characters long.%s"
                  "<html>"
                  "<p>"
                  "Please enter a new PIN for user <i>%s</i> at "
                  "<i>%s</i>."
                  "</p>"
                  "<p>"
                  "The input must be at least %d characters long.%s"
                  "</p>"
                  "</html>"),
             un, bn,
             minLen,
             numeric_warning,
             un, bn,
             minLen,
             numeric_warning);
  }
  else {
    snprintf(buffer, sizeof(buffer)-1,
             I18N("Please enter the PIN for \n"
                  "user %s at %s\n"
                  "%s"
                  "<html>"
                  "Please enter the PIN for user <i>%s</i> at "
                  "<i>%s</i>.<br>"
                  "%s"
                  "</html>"),
             un, bn,
             numeric_warning,
             un, bn,
             numeric_warning);
  }
  buffer[sizeof(buffer)-1]=0;

  AB_BankInfo_free(bi);

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  AH_User_MkPinName(u, nbuf);

  rv=GWEN_Gui_GetPassword(flags,
                          GWEN_Buffer_GetStart(nbuf),
                          I18N("Enter PIN"),
                          buffer,
                          pwbuffer,
                          minLen,
                          maxLen,
                          GWEN_Gui_PasswordMethod_Text, NULL,
                          0);
  GWEN_Buffer_free(nbuf);

  return rv;
}



int AH_User_InputTan(AB_USER *u,
                     char *pwbuffer,
                     int minLen,
                     int maxLen)
{
  AB_PROVIDER *pro;
  AB_BANKING *ab;
  int rv;
  char buffer[512];
  const char *un;
  const char *bn=NULL;
  GWEN_BUFFER *nbuf;
  AB_BANKINFO *bi;

  assert(u);
  un=AB_User_GetUserId(u);

  pro=AB_User_GetProvider(u);
  assert(pro);
  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  /* find bank name */
  bi=AB_Banking_GetBankInfo(ab,  "de", "*", AB_User_GetBankCode(u));
  if (bi)
    bn=AB_BankInfo_GetBankName(bi);
  if (!bn)
    AB_User_GetBankCode(u);

  buffer[0]=0;
  buffer[sizeof(buffer)-1]=0;
  snprintf(buffer, sizeof(buffer)-1,
           I18N("Please enter the next TAN\n"
                "for user %s at %s."
                "<html>"
                "Please enter the next TAN for user <i>%s</i> at "
                "<i>%s</i>."
                "</html>"),
           un, bn,
           un, bn);
  buffer[sizeof(buffer)-1]=0;

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  AH_User_MkTanName(u, NULL, nbuf);

  rv=GWEN_Gui_GetPassword(GWEN_GUI_INPUT_FLAGS_TAN |
                          /*GWEN_GUI_INPUT_FLAGS_NUMERIC |*/
                          GWEN_GUI_INPUT_FLAGS_SHOW,
                          GWEN_Buffer_GetStart(nbuf),
                          I18N("Enter TAN"),
                          buffer,
                          pwbuffer,
                          minLen,
                          maxLen,
                          GWEN_Gui_PasswordMethod_Text, NULL,
                          0);
  GWEN_Buffer_free(nbuf);
  AB_BankInfo_free(bi);
  return rv;
}


int AH_User_SetTanStatus(AB_USER *u,
                         const char *challenge,
                         const char *tan,
                         GWEN_GUI_PASSWORD_STATUS status)
{
  GWEN_BUFFER *nbuf;
  int rv;

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  AH_User_MkTanName(u, challenge, nbuf);
  rv=GWEN_Gui_SetPasswordStatus(GWEN_Buffer_GetStart(nbuf),
                                tan,
                                status,
                                0);
  GWEN_Buffer_free(nbuf);
  return rv;
}



int AH_User_SetPinStatus(AB_USER *u,
                         const char *pin,
                         GWEN_GUI_PASSWORD_STATUS status)
{
  GWEN_BUFFER *nbuf;
  int rv;

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  AH_User_MkPinName(u, nbuf);
  rv=GWEN_Gui_SetPasswordStatus(GWEN_Buffer_GetStart(nbuf),
                                pin,
                                status,
                                0);
  GWEN_Buffer_free(nbuf);
  return rv;
}



const char *AH_User_GetHttpContentType(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->httpContentType;
}



void AH_User_SetHttpContentType(AB_USER *u, const char *s)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  free(ue->httpContentType);
  if (s)
    ue->httpContentType=strdup(s);
  else
    ue->httpContentType=NULL;
}



const int *AH_User_GetTanMethodList(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->tanMethodList;
}



int AH_User_HasTanMethod(const AB_USER *u, int method)
{
  AH_USER *ue;
  int i;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  for (i=0; i<AH_USER_MAX_TANMETHODS; i++) {
    if (ue->tanMethodList[i]==method)
      return 1;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "TAN method %d not available", method);
  return 0;
}



int AH_User_HasTanMethodOtherThan(const AB_USER *u, int method)
{
  AH_USER *ue;
  int i;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  for (i=0; i<AH_USER_MAX_TANMETHODS; i++) {
    if (ue->tanMethodList[i]!=method && ue->tanMethodList[i]!=-1)
      return 1;
  }

  return 0;
}



void AH_User_AddTanMethod(AB_USER *u, int method)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  if (!AH_User_HasTanMethod(u, method)) {
    if (ue->tanMethodCount<AH_USER_MAX_TANMETHODS) {
      ue->tanMethodList[ue->tanMethodCount++]=method;
      ue->tanMethodList[ue->tanMethodCount]=-1;
    }
  }
}



void AH_User_ClearTanMethodList(AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->tanMethodList[0]=-1;
  ue->tanMethodCount=0;
}



int AH_User_GetTanMethodCount(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->tanMethodCount;
}



int AH_User_GetSelectedTanMethod(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->selectedTanMethod;
}



void AH_User_SetSelectedTanMethod(AB_USER *u, int i)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->selectedTanMethod=i;
}



const char *AH_User_GetTanMediumId(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->tanMediumId;
}



void AH_User_SetTanMediumId(AB_USER *u, const char *s)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  free(ue->tanMediumId);
  if (s)
    ue->tanMediumId=strdup(s);
  else
    ue->tanMediumId=NULL;
}



void AH_User_LoadTanMethods(AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  /* read directly from BPD */
  if (ue->cryptMode==AH_CryptMode_Pintan) {
    GWEN_DB_NODE *db;
    int rv;

    AH_TanMethod_List_Clear(ue->tanMethodDescriptions);
    db=GWEN_DB_Group_new("bpd");
    rv=AH_Job_SampleBpdVersions("JobTan", u, db);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "No BPD for TAN job");
    }
    else {
      GWEN_DB_NODE *dbV;

      dbV=GWEN_DB_GetFirstGroup(db);
      while (dbV) {
        int version;

        version=atoi(GWEN_DB_GroupName(dbV));
        if (version>0) {
          GWEN_DB_NODE *dbT;

          dbT=GWEN_DB_FindFirstGroup(dbV, "tanMethod");
          if (!dbT) {
            DBG_INFO(AQHBCI_LOGDOMAIN, "No tan method found");
          }
          while (dbT) {
            AH_TAN_METHOD *tm;
            const char *s;

            tm=AH_TanMethod_new();
            AH_TanMethod_SetFunction(tm, GWEN_DB_GetIntValue(dbT, "function", 0, 0));
            AH_TanMethod_SetProcess(tm, GWEN_DB_GetIntValue(dbT, "process", 0, 0));
            AH_TanMethod_SetMethodId(tm, GWEN_DB_GetCharValue(dbT, "methodId", 0, 0));
            AH_TanMethod_SetZkaTanName(tm, GWEN_DB_GetCharValue(dbT, "zkaTanName", 0, 0));
            AH_TanMethod_SetZkaTanVersion(tm, GWEN_DB_GetCharValue(dbT, "zkaTanVersion", 0, 0));
            AH_TanMethod_SetMethodName(tm, GWEN_DB_GetCharValue(dbT, "methodName", 0, 0));
            AH_TanMethod_SetTanMaxLen(tm, GWEN_DB_GetIntValue(dbT, "tanMaxLen", 0, 0));
            AH_TanMethod_SetFormatId(tm, GWEN_DB_GetCharValue(dbT, "formatId", 0, 0));
            AH_TanMethod_SetPrompt(tm, GWEN_DB_GetCharValue(dbT, "prompt", 0, 0));
            AH_TanMethod_SetReturnMaxLen(tm, GWEN_DB_GetIntValue(dbT, "returnMaxLen", 0, 0));
            AH_TanMethod_SetMaxActiveLists(tm, GWEN_DB_GetIntValue(dbT, "maxActiveLists", 0, 0));
            /*AH_TanMethod_SetGvVersion(tm, GWEN_DB_GetIntValue(dbT, "gvVersion", 0, 0));*/
            s=GWEN_DB_GetCharValue(dbT, "multiTanAllowed", 0, NULL);
            if (s && strcasecmp(s, "j")==0)
              AH_TanMethod_SetMultiTanAllowed(tm, 1);
            AH_TanMethod_SetTimeShiftAllowed(tm, GWEN_DB_GetIntValue(dbT, "timeShiftAllowed", 0, 0));
            AH_TanMethod_SetTanListMode(tm, GWEN_DB_GetIntValue(dbT, "tanListMode", 0, 0));
            s=GWEN_DB_GetCharValue(dbT, "stornoAllowed", 0, NULL);
            if (s && strcasecmp(s, "j")==0)
              AH_TanMethod_SetStornoAllowed(tm, 1);
            s=GWEN_DB_GetCharValue(dbT, "needChallengeClass", 0, NULL);
            if (s && strcasecmp(s, "j")==0)
              AH_TanMethod_SetNeedChallengeClass(tm, 1);
            s=GWEN_DB_GetCharValue(dbT, "needChallengeAmount", 0, NULL);
            if (s && strcasecmp(s, "j")==0)
              AH_TanMethod_SetNeedChallengeAmount(tm, 1);
            AH_TanMethod_SetInitMode(tm, GWEN_DB_GetIntValue(dbT, "initMode", 0, 0));
            s=GWEN_DB_GetCharValue(dbT, "tanMediumIdNeeded", 0, NULL);
            if (s && strcasecmp(s, "j")==0)
              AH_TanMethod_SetNeedTanMediumId(tm, 1);
            AH_TanMethod_SetMaxActiveTanMedia(tm, GWEN_DB_GetIntValue(dbT, "maxActiveTanMedia", 0, 0));

            DBG_INFO(AQHBCI_LOGDOMAIN,
                     "Adding TAN method %d [%s] for GV version %d",
                     AH_TanMethod_GetFunction(tm),
                     AH_TanMethod_GetMethodId(tm),
                     version);
            AH_TanMethod_SetGvVersion(tm, version);
            AH_TanMethod_List_Add(tm, ue->tanMethodDescriptions);

            dbT=GWEN_DB_FindNextGroup(dbT, "tanMethod");
          }
        }

        dbV=GWEN_DB_GetNextGroup(dbV);
      }
    }
    GWEN_DB_Group_free(db);
  }
}



void AH_User_LoadSepaDescriptors(AB_USER *u)
{
  AH_USER *ue;
  GWEN_DB_NODE *db;
  int rv;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  /* read directly from BPD */

  GWEN_StringList_Clear(ue->sepaDescriptors);
  db=GWEN_DB_Group_new("bpd");
  rv=AH_Job_SampleBpdVersions("JobGetAccountSepaInfo", u, db);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No BPD for TAN job");
  }
  else {
    GWEN_DB_NODE *dbV;

    dbV=GWEN_DB_GetFirstGroup(db);
    while (dbV) {
      int version;

      version=atoi(GWEN_DB_GroupName(dbV));
      if (version>0) {
        GWEN_DB_NODE *dbT;

        /* always overwrite with latest version received */
        GWEN_StringList_Clear(ue->sepaDescriptors);
        dbT=GWEN_DB_FindFirstGroup(dbV, "SupportedSepaFormats");
        if (!dbT) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "No SEPA descriptor found");
        }
        while (dbT) {
          int i;

          for (i=0; i<100; i++) {
            const char *s;

            s=GWEN_DB_GetCharValue(dbT, "format", i, NULL);
            if (!(s && *s))
              break;
            GWEN_StringList_AppendString(ue->sepaDescriptors, s, 0, 1);
            DBG_INFO(AQHBCI_LOGDOMAIN,
                     "Adding SEPA descriptor [%s] for GV version %d",
                     s, version);
          }

          dbT=GWEN_DB_FindNextGroup(dbT, "SupportedSepaFormats");
        }
      }

      dbV=GWEN_DB_GetNextGroup(dbV);
    }
  }
  GWEN_DB_Group_free(db);
}




const AH_TAN_METHOD_LIST *AH_User_GetTanMethodDescriptions(AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  /* always reload TAN methods from BPD */
  AH_User_LoadTanMethods(u);

  return ue->tanMethodDescriptions;
}



int AH_User_GetMaxTransfersPerJob(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->maxTransfersPerJob;
}



void AH_User_SetMaxTransfersPerJob(AB_USER *u, int i)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->maxTransfersPerJob=i;
}



int AH_User_GetMaxDebitNotesPerJob(const AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->maxDebitNotesPerJob;
}



void AH_User_SetMaxDebitNotesPerJob(AB_USER *u, int i)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->maxDebitNotesPerJob=i;
}



const GWEN_STRINGLIST *AH_User_GetSepaDescriptors(AB_USER *u)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->sepaDescriptors;
}



const char *AH_User_GetSepaTransferProfile(const AB_USER *u)
{
  const AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->sepaTransferProfile;
}



void AH_User_SetSepaTransferProfile(AB_USER *u, const char *profileName)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  free(ue->sepaTransferProfile);
  if (profileName)
    ue->sepaTransferProfile=strdup(profileName);
  else
    ue->sepaTransferProfile=NULL;
}



const char *AH_User_GetSepaDebitNoteProfile(const AB_USER *u)
{
  const AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->sepaDebitNoteProfile;
}



void AH_User_SetSepaDebitNoteProfile(AB_USER *u, const char *profileName)
{
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  free(ue->sepaDebitNoteProfile);
  if (profileName)
    ue->sepaDebitNoteProfile=strdup(profileName);
  else
    ue->sepaDebitNoteProfile=NULL;
}

int AH_User_VerifyInitialKey(GWEN_CRYPT_TOKEN *ct,
                             const GWEN_CRYPT_TOKEN_CONTEXT *ctx,
                             AB_USER *user,
                             GWEN_CRYPT_KEY *key,
                             uint16_t sentModl,
                             const char *keyName)
{


  uint32_t  keyHashAlgo;
  uint32_t  keySize;
  uint32_t  keyNum;
  uint32_t  keyVer;
  uint32_t  expLen;
  uint8_t  *modulus;
  uint8_t  *exponent;
  GWEN_MDIGEST *md;
  uint8_t *mdPtr;
  unsigned int mdSize;
  uint8_t   modBuffer[1024];
  uint8_t   expBuffer[256];
  uint32_t  keyHashNum;
  uint32_t  keyHashVer;
  uint8_t   canVerifyWithHash=0;
  uint16_t  keySizeForHash=0;
  const uint8_t    *keyHash;
  uint32_t keyHashLen;
  char hashString[1024];
  int rv;
  int i;


  /* check if NOTEPAD contained a key hash */
  keyHashAlgo=GWEN_Crypt_Token_Context_GetKeyHashAlgo(ctx);

  modulus=&modBuffer[0];
  exponent=&expBuffer[0];
  keySize=1024;
  expLen=256;
  GWEN_Crypt_KeyRsa_GetModulus(key, modulus, &keySize);
  GWEN_Crypt_KeyRsa_GetExponent(key, exponent, &expLen);
  keyNum=GWEN_Crypt_Key_GetKeyNumber(key);
  keyVer=GWEN_Crypt_Key_GetKeyVersion(key);

  /* check if we got the keyHashAlgo from a card, otherwise determine it ourselves */

  if (keyHashAlgo == GWEN_Crypt_HashAlgoId_None ||  keyHashAlgo == GWEN_Crypt_HashAlgoId_Unknown) {
    uint8_t rxhVersion = AH_User_GetRdhType(user);
    if (rxhVersion < 6) {
      keyHashAlgo=GWEN_Crypt_HashAlgoId_Rmd160;
    }
    else {
      keyHashAlgo=GWEN_Crypt_HashAlgoId_Sha256;
    }
  }
  if (AH_User_GetCryptMode(user) == AH_CryptMode_Rdh && AH_User_GetRdhType(user) == 1) {
    keySizeForHash=128;
  }
  else {
    keySizeForHash=keySize;
  }


  keyHash=GWEN_Crypt_Token_Context_GetKeyHashPtr(ctx);
  keyHashLen=GWEN_Crypt_Token_Context_GetKeyHashLen(ctx);

  /* check if we got a hash from a banking card */
  if (keyHash != NULL && keyHashLen > 0) {

    keyHashNum=GWEN_Crypt_Token_Context_GetKeyHashNum(ctx);
    keyHashVer=GWEN_Crypt_Token_Context_GetKeyHashVer(ctx);
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Found bank key hash on the zka card! (Hash Algo Identifier: [%d], keyNum: [%d], keyVer: [%d]", keyHashAlgo, keyHashNum,
             keyHashVer);
    if (keyHashNum == keyNum && keyHashVer==keyVer) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Key Number and Key Version of the Hash match transmitted key, try verifying with the Hash.");
      canVerifyWithHash=1;
    }
  }

  /* build key hash */
  {
    int expPadBytes=keySizeForHash-expLen;
    int modPadBytes=keySizeForHash-keySize;

    /* pad exponent to length of modulus */
    GWEN_BUFFER *keyBuffer;
    keyBuffer=GWEN_Buffer_new(NULL, 2*keySize, 0, 0);
    GWEN_Buffer_FillWithBytes(keyBuffer, 0x0, expPadBytes);
    GWEN_Buffer_AppendBytes(keyBuffer, (const char *)exponent, expLen);
    if (modPadBytes) {
      GWEN_Buffer_FillWithBytes(keyBuffer, 0x0, modPadBytes);
    }
    GWEN_Buffer_AppendBytes(keyBuffer, (const char *)modulus, keySize);



    if (keyHashAlgo==GWEN_Crypt_HashAlgoId_Sha256) {
      /*SHA256*/
      md=GWEN_MDigest_Sha256_new();
      DBG_INFO(AQHBCI_LOGDOMAIN, "Hash Algo for key verification is SHA256.");
    }
    else if (keyHashAlgo==GWEN_Crypt_HashAlgoId_Rmd160) {
      md=GWEN_MDigest_Rmd160_new();
      DBG_INFO(AQHBCI_LOGDOMAIN, "Hash Algo for key verification is RIPMED160.");
    }
    else {
      /* ERROR, wrong hash algo */
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Hash Algorithm of Bank Public %s Key not correct! (Hash Identifier %d)", keyName,
                keyHashAlgo);
      return 0;
    }

    GWEN_MDigest_Begin(md);
    GWEN_MDigest_Update(md, (uint8_t *)GWEN_Buffer_GetStart(keyBuffer), 2*keySize);
    GWEN_MDigest_End(md);
    mdPtr=GWEN_MDigest_GetDigestPtr(md);
    mdSize=GWEN_MDigest_GetDigestSize(md);
    GWEN_Buffer_free(keyBuffer);
  }

  memset(hashString, 0, 1024);
  for (i=0; i<GWEN_MDigest_GetDigestSize(md); i++)
    sprintf(hashString+3*i, "%02x ", *(mdPtr+i));
  DBG_INFO(AQHBCI_LOGDOMAIN, "Key Hash from the Bank Public %s key: Hash Length: %d, Hash: %s", keyName, keyHashLen,
           hashString);

  if (canVerifyWithHash) {

    uint16_t matchingBytes=keySize;

    char cardHashString[1024];

    /* compare hashes */

    memset(cardHashString, 0, 1024);
    for (i=0; i<keyHashLen; i++)
      sprintf(cardHashString+3*i, "%02x ", keyHash[i]);
    DBG_INFO(AQHBCI_LOGDOMAIN, "Key Hash on the Card: Hash Length: %d, Hash: %s", keyHashLen, cardHashString);


    if (keyHashLen==mdSize) {
      for (i = 0; i < mdSize; i++) {
        if (mdPtr[i] != (uint8_t) keyHash[i]) {
          matchingBytes--;
        }
      }
    }
    else {
      matchingBytes = 0;
    }
    GWEN_MDigest_free(md);
    DBG_INFO(AQHBCI_LOGDOMAIN, "Hash comparison: of %d bytes %d matched", keyHashLen, matchingBytes);
    if (matchingBytes == keySize) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Verified the bank's public %s key with the hash from the zka card.", keyName);
      return 1;
    }
    else {
      GWEN_BUFFER *msgBuffer;
      GWEN_BUFFER *titleBuffer;
      char numBuf[32];
      /* ERROR, hash sizes not identical */


      DBG_ERROR(AQHBCI_LOGDOMAIN, "Hash Sizes of Bank Public %s Key do not match!", keyName);
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Verify new server %s key, please verify! (num: %d, version: %d, hash: %s)", keyName,
                keyNum, keyVer, hashString);
      GWEN_Gui_ProgressLog2(0,
                            GWEN_LoggerLevel_Warning,
                            I18N("Hash Sizes of Bank Public %s Key do not match!"),
                            keyName);
      GWEN_Gui_ProgressLog2(0,
                            GWEN_LoggerLevel_Warning,
                            I18N("Received new server %s key, please verify! (num: %d, version: %d, hash: %s)"),
                            keyName, keyNum, keyVer, hashString);

      titleBuffer=GWEN_Buffer_new(NULL, 256, 0, 1);
      GWEN_Buffer_AppendString(titleBuffer, "Could not verify received public ");
      GWEN_Buffer_AppendString(titleBuffer, keyName);
      GWEN_Buffer_AppendString(titleBuffer, " bank key with card hash!");

      msgBuffer=GWEN_Buffer_new(NULL, 2048, 0, 1);
      GWEN_Buffer_AppendString(msgBuffer,
                               "Could not verify the received key with the key hash stored on your banking card!\n");
      GWEN_Buffer_AppendString(msgBuffer, "Hashes did not match!\n");
      GWEN_Buffer_AppendString(msgBuffer, "Hash on the card has length ");
      snprintf(numBuf, sizeof(numBuf), "%d, ",
               keyHashLen);
      GWEN_Buffer_AppendString(msgBuffer, numBuf);
      GWEN_Buffer_AppendString(msgBuffer, "hash value ");
      if (keyHashAlgo==GWEN_Crypt_HashAlgoId_Sha256) {
        /*SHA256*/
        GWEN_Buffer_AppendString(msgBuffer, "(SHA256):\n");

      }
      else if (keyHashAlgo==GWEN_Crypt_HashAlgoId_Rmd160) {
        GWEN_Buffer_AppendString(msgBuffer, "(RIPEMD-160):\n");
      }
      GWEN_Buffer_AppendString(msgBuffer, cardHashString);
      GWEN_Buffer_AppendString(msgBuffer, "\n\nHash from transmitted key has length ");
      snprintf(numBuf, sizeof(numBuf), "%d, ",
               mdSize);
      GWEN_Buffer_AppendString(msgBuffer, numBuf);
      GWEN_Buffer_AppendString(msgBuffer, "hash value ");
      if (keyHashAlgo==GWEN_Crypt_HashAlgoId_Sha256) {
        /*SHA256*/
        GWEN_Buffer_AppendString(msgBuffer, "(SHA256):\n");

      }
      else if (keyHashAlgo==GWEN_Crypt_HashAlgoId_Rmd160) {
        GWEN_Buffer_AppendString(msgBuffer, "(RIPEMD-160):\n");
      }
      GWEN_Buffer_AppendString(msgBuffer, hashString);
      GWEN_Buffer_AppendString(msgBuffer, "\n\nIMPORTANT WARNING: you normally should only proceed if the hash matches!\n"
                               "Contact your bank immediately if the hash does not match!\n\n");
      GWEN_Buffer_AppendString(msgBuffer, "Do you really want to import this key?");
      rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_WARN | GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS,
                             I18N(GWEN_Buffer_GetStart(titleBuffer)),
                             I18N(GWEN_Buffer_GetStart(msgBuffer)),
                             I18N("Import"), I18N("Abort"), NULL, 0);
      GWEN_Buffer_free(msgBuffer);
      GWEN_Buffer_free(titleBuffer);
      if (rv!=1) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Public %s Key not accepted by user.", keyName);
        return 0;
      }
      else {
        DBG_INFO(AQHBCI_LOGDOMAIN,
                 "Bank's public %s key accepted by the user.", keyName);
        return 1;

      }
    }
  }

  {
    GWEN_BUFFER *titleBuffer;
    GWEN_BUFFER *msgBuffer;
    char numBuf[32];

    GWEN_MDigest_free(md);

    DBG_ERROR(AQHBCI_LOGDOMAIN, "Received new server %s key, please verify! (num: %d, version: %d, hash: %s)", keyName,
              keyNum, keyVer, hashString);
    GWEN_Gui_ProgressLog2(0,
                          GWEN_LoggerLevel_Warning,
                          I18N("Received new server %s key, please verify! (num: %d, version: %d, hash: %s)"),
                          keyName, keyNum, keyVer, hashString);

    titleBuffer=GWEN_Buffer_new(NULL, 256, 0, 1);
    GWEN_Buffer_AppendString(titleBuffer, "Received Public ");
    GWEN_Buffer_AppendString(titleBuffer, keyName);
    GWEN_Buffer_AppendString(titleBuffer, " Bank Key");

    msgBuffer=GWEN_Buffer_new(NULL, 2048, 0, 1);
    GWEN_Buffer_AppendString(msgBuffer, "Received a unverified public bank key!\n");
    GWEN_Buffer_AppendString(msgBuffer, "Key Number: ");
    snprintf(numBuf, sizeof(numBuf), "%d",
             keyNum);
    GWEN_Buffer_AppendString(msgBuffer, numBuf);
    GWEN_Buffer_AppendString(msgBuffer, "\nKey Version: ");
    snprintf(numBuf, sizeof(numBuf), "%d",
             keyNum);
    GWEN_Buffer_AppendString(msgBuffer, numBuf);
    GWEN_Buffer_AppendString(msgBuffer, "\n\nHash from transmitted key has length ");
    snprintf(numBuf, sizeof(numBuf), "%d",
             mdSize);
    GWEN_Buffer_AppendString(msgBuffer, numBuf);
    GWEN_Buffer_AppendString(msgBuffer, ", value ");
    if (keyHashAlgo==GWEN_Crypt_HashAlgoId_Sha256) {
      /*SHA256*/
      GWEN_Buffer_AppendString(msgBuffer, "(SHA256):\n");

    }
    else if (keyHashAlgo==GWEN_Crypt_HashAlgoId_Rmd160) {
      GWEN_Buffer_AppendString(msgBuffer, "(RIPEMD-160):\n");
    }
    GWEN_Buffer_AppendString(msgBuffer, hashString);
    GWEN_Buffer_AppendString(msgBuffer, "\n\nIMPORTANT WARNING: This hash should match the hash in an INI letter you should"
                             "have received from your bank!\n"
                             "Contact your bank immediately if the hash does not match!\n\n");
    GWEN_Buffer_AppendString(msgBuffer, "Do you really want to import this key?");
    rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_WARN | GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS,
                           I18N(GWEN_Buffer_GetStart(titleBuffer)),
                           I18N(GWEN_Buffer_GetStart(msgBuffer)),
                           I18N("Import"), I18N("Abort"), NULL, 0);
    GWEN_Buffer_free(msgBuffer);
    GWEN_Buffer_free(titleBuffer);
    if (rv!=1) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Public %s Key not accepted by user.", keyName);
      return 0;
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Bank's public %s key accepted by the user.", keyName);
      return 1;

    }


  }
  return 0;
}


