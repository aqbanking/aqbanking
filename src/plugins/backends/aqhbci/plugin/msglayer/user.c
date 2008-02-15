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

#include "i18n_l.h"
#include "user_p.h"
#include "hbci_l.h"
#include "provider_l.h"
#include "hbci-updates_l.h"
#include "msgengine_l.h"
#include <aqhbci/provider.h>

#include <gwenhywfar/debug.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT(AB_USER, AH_USER)


const char *AH_User_Status_toString(AH_USER_STATUS st){
  switch(st) {
  case AH_UserStatusNew:      return "new";
  case AH_UserStatusEnabled:  return "enabled";
  case AH_UserStatusPending:  return "pending";
  case AH_UserStatusDisabled: return "disabled";
  default:                    return "unknown";
  } /* switch */
}



AH_USER_STATUS AH_User_Status_fromString(const char *s){
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
                        uint32_t flags) {
  GWEN_DB_DeleteVar(db, name);
  if (flags & AH_USER_FLAGS_BANK_DOESNT_SIGN)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "bankDoesntSign");
  if (flags & AH_USER_FLAGS_BANK_USES_SIGNSEQ)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name,
                         "bankUsesSignSeq");
  if (flags & AH_USER_FLAGS_KEEPALIVE)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name,
                         "keepAlive");
  if (flags & AH_USER_FLAGS_IGNORE_UPD)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name,
                         "ignoreUpd");
  if (flags & AH_USER_FLAGS_FORCE_SSL3)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name,
			 "forceSsl3");
  if (flags & AH_USER_FLAGS_NO_BASE64)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name,
			 "noBase64");
}



uint32_t AH_User_Flags_fromDb(GWEN_DB_NODE *db, const char *name) {
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
    else if (strcasecmp(s, "keepAlive")==0)
      fl|=AH_USER_FLAGS_KEEPALIVE;
    else if (strcasecmp(s, "ignoreUpd")==0)
      fl|=AH_USER_FLAGS_IGNORE_UPD;
    else if (strcasecmp(s, "forceSsl3")==0)
      fl|=AH_USER_FLAGS_FORCE_SSL3;
    else if (strcasecmp(s, "noBase64")==0)
      fl|=AH_USER_FLAGS_NO_BASE64;
    else {
      DBG_WARN(AQHBCI_LOGDOMAIN, "Unknown user flag \"%s\"", s);
    }
  }

  return fl;
}





void AH_User_Extend(AB_USER *u, AB_PROVIDER *pro,
                    AB_PROVIDER_EXTEND_MODE em) {
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *gr;

  db=AB_User_GetProviderData(u);
  assert(db);

  if (em==AB_ProviderExtendMode_Create ||
      em==AB_ProviderExtendMode_Extend) {
    AH_USER *ue;
    const char *s;
    int rv;
    int i;

    GWEN_NEW_OBJECT(AH_USER, ue);
    GWEN_INHERIT_SETDATA(AB_USER, AH_USER, u, ue, AH_User_freeData);

    ue->tanMethodList[0]=-1;
    ue->tanMethodCount=0;

    ue->hbci=AH_Provider_GetHbci(pro);
    /* update db to latest version */
    rv=AH_HBCI_UpdateDbUser(ue->hbci, db);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not update user db (%d)", rv);
      assert(0);
    }

    s=AB_User_GetCountry(u);
    if (!s || !*s)
      AB_User_SetCountry(u, "de");

    ue->msgEngine=AH_MsgEngine_new();
    GWEN_MsgEngine_SetEscapeChar(ue->msgEngine, '?');
    GWEN_MsgEngine_SetCharsToEscape(ue->msgEngine, ":+\'");
    AH_MsgEngine_SetUser(ue->msgEngine, u);
    GWEN_MsgEngine_SetDefinitions(ue->msgEngine,
                                  AH_HBCI_GetDefinitions(ue->hbci),
                                  0);

    s=GWEN_DB_GetCharValue(db, "cryptMode", 0, "unknown");
    ue->cryptMode=AH_CryptMode_fromString(s);

    s=GWEN_DB_GetCharValue(db, "status", 0, "unknown");
    ue->status=AH_User_Status_fromString(s);

    ue->hbciVersion=GWEN_DB_GetIntValue(db, "hbciVersion", 0, 210);

    /* load server address */
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

    /* load BPD */
    gr=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "bpd");
    if (gr) {
      ue->bpd=AH_Bpd_FromDb(gr);
      assert(ue->bpd);
    }
    else
      ue->bpd=AH_Bpd_new();

    /* load UPD */
    gr=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "upd");
    if (gr)
      ue->dbUpd=GWEN_DB_Group_dup(gr);
    else
      ue->dbUpd=GWEN_DB_Group_new("upd");

    /* get peer id */
    s=GWEN_DB_GetCharValue(db, "peerId", 0, 0);
    if (s)
      ue->peerId=strdup(s);

    /* get system id */
    s=GWEN_DB_GetCharValue(db, "systemId", 0, 0);
    if (s)
      ue->systemId=strdup(s);

    ue->updVersion=GWEN_DB_GetIntValue(db, "updVersion", 0, 0);

    /* setup HTTP version */
    ue->httpVMajor=GWEN_DB_GetIntValue(db, "httpVMajor", 0, -1);
    ue->httpVMinor=GWEN_DB_GetIntValue(db, "httpVMinor", 0, -1);
    if (ue->httpVMajor==-1 || ue->httpVMinor==-1) {
      ue->httpVMajor=1;
      ue->httpVMinor=0;
    }

    s=GWEN_DB_GetCharValue(db, "httpContentType", 0, 0);
    if (s)
      ue->httpContentType=strdup(s);

    /* read user flags */
    ue->flags=AH_User_Flags_fromDb(db, "userFlags");

    /* setup medium stuff */
    s=GWEN_DB_GetCharValue(db, "tokenType", 0, 0);
    if (s)
      ue->tokenType=strdup(s);
    s=GWEN_DB_GetCharValue(db, "tokenName", 0, 0);
    if (s)
      ue->tokenName=strdup(s);

    ue->tokenContextId=GWEN_DB_GetIntValue(db, "tokenContextId", 0, 1);

    /* get rdh type */
    ue->rdhType=GWEN_DB_GetIntValue(db, "rdhType", 0, -1);

    /* read supported TAN methods */
    for (i=0; i<AH_USER_MAX_TANMETHODS; i++) {
      int method;

      method=GWEN_DB_GetIntValue(db, "tanMethodList", i, -1);
      if (method==-1)
	break;
      ue->tanMethodList[ue->tanMethodCount++]=method;
      ue->tanMethodList[ue->tanMethodCount]=-1;
    }
  }
  else {
    AH_USER *ue;
    const char *s;

    ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
    assert(ue);

    if (em==AB_ProviderExtendMode_Add) {
    }
    else if (em==AB_ProviderExtendMode_Save) {
      int i;

      GWEN_DB_ClearGroup(db, NULL);

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

      /* save UPD */
      assert(ue->bpd);
      gr=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "bpd");
      assert(gr);
      AH_Bpd_ToDb(ue->bpd, gr);

      /* save BPD */
      gr=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "upd");
      assert(gr);
      GWEN_DB_AddGroupChildren(gr, ue->dbUpd);

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

    } /* if save */
  }
}


void GWENHYWFAR_CB AH_User_freeData(void *bp, void *p) {
  AH_USER *ue;

  ue=(AH_USER*)p;
  free(ue->peerId);
  free(ue->systemId);
  free(ue->httpContentType);
  free(ue->httpUserAgent);
  free(ue->tokenType);
  free(ue->tokenName);
  free(ue->prompt);
  GWEN_Url_free(ue->serverUrl);
  AH_Bpd_free(ue->bpd);
  GWEN_MsgEngine_free(ue->msgEngine);
  GWEN_FREE_OBJECT(ue);
}



const char *AH_User_GetPeerId(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->peerId;
}



void AH_User_SetPeerId(AB_USER *u, const char *s) {
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



uint32_t AH_User_GetTokenContextId(const AB_USER *u){
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->tokenContextId;
}



void AH_User_SetTokenContextId(AB_USER *u, uint32_t id){
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->tokenContextId=id;
}



AH_CRYPT_MODE AH_User_GetCryptMode(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->cryptMode;
}



void AH_User_SetCryptMode(AB_USER *u, AH_CRYPT_MODE m) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->cryptMode=m;
}



AH_USER_STATUS AH_User_GetStatus(const AB_USER *u){
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->status;
}



void AH_User_SetStatus(AB_USER *u, AH_USER_STATUS i){
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->status=i;
}



AH_HBCI *AH_User_GetHbci(const AB_USER *u){
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->hbci;
}



const GWEN_URL *AH_User_GetServerUrl(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->serverUrl;
}



void AH_User_SetServerUrl(AB_USER *u, const GWEN_URL *url) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  GWEN_Url_free(ue->serverUrl);
  if (url) ue->serverUrl=GWEN_Url_dup(url);
  else ue->serverUrl=0;
}



int AH_User_GetUpdVersion(const AB_USER *u){
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->updVersion;
}



void AH_User_SetUpdVersion(AB_USER *u, int i){
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->updVersion=i;
}



GWEN_DB_NODE *AH_User_GetUpd(const AB_USER *u){
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->dbUpd;
}



void AH_User_SetUpd(AB_USER *u, GWEN_DB_NODE *n){
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  if (n) {
    GWEN_DB_Group_free(ue->dbUpd);
    ue->dbUpd=GWEN_DB_Group_dup(n);
  }
  else
    GWEN_DB_ClearGroup(ue->dbUpd, NULL);
}



AH_BPD *AH_User_GetBpd(const AB_USER *u){
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->bpd;
}



void AH_User_SetBpd(AB_USER *u, AH_BPD *bpd){
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



int AH_User_GetBpdVersion(const AB_USER *u){
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  assert(ue->bpd);
  return AH_Bpd_GetBpdVersion(ue->bpd);
}



void AH_User_SetBpdVersion(AB_USER *u, int i){
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  assert(ue->bpd);
  AH_Bpd_SetBpdVersion(ue->bpd, i);
}



const char *AH_User_GetSystemId(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->systemId;
}



void AH_User_SetSystemId(AB_USER *u, const char *s) {
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



GWEN_MSGENGINE *AH_User_GetMsgEngine(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->msgEngine;
}



uint32_t AH_User_GetFlags(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->flags;
}



void AH_User_SetFlags(AB_USER *u, uint32_t flags) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->flags=flags;
}



void AH_User_AddFlags(AB_USER *u, uint32_t flags) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->flags|=flags;
}



void AH_User_SubFlags(AB_USER *u, uint32_t flags) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->flags&=~flags;
}



int AH_User_GetHbciVersion(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->hbciVersion;
}



void AH_User_SetHbciVersion(AB_USER *u, int i) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->hbciVersion=i;
}



const char *AH_User_GetHttpUserAgent(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->httpUserAgent;
}



void AH_User_SetHttpUserAgent(AB_USER *u, const char *s) {
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



int AH_User_GetHttpVMajor(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->httpVMajor;
}



void AH_User_SetHttpVMajor(AB_USER *u, int i) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->httpVMajor=i;
}



int AH_User_GetHttpVMinor(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->httpVMinor;
}



void AH_User_SetHttpVMinor(AB_USER *u, int i) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->httpVMinor=i;
}



uint32_t AH_User_GetTanMethods(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->tanMethods;
}



void AH_User_SetTanMethods(AB_USER *u, uint32_t m) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->tanMethods=m;
}



void AH_User_AddTanMethods(AB_USER *u, uint32_t m) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->tanMethods|=m;
}



void AH_User_SubTanMethods(AB_USER *u, uint32_t m) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->tanMethods&=~m;
}



int AH_User_GetRdhType(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->rdhType;
}



void AH_User_SetRdhType(AB_USER *u, int i) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->rdhType=i;
}



const char *AH_User_GetTokenType(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->tokenType;
}



void AH_User_SetTokenType(AB_USER *u, const char *s) {
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



const char *AH_User_GetTokenName(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->tokenName;
}



void AH_User_SetTokenName(AB_USER *u, const char *s) {
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



int AH_User_MkPasswdName(const AB_USER *u, GWEN_BUFFER *buf) {
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



int AH_User_MkPinName(const AB_USER *u, GWEN_BUFFER *buf) {
  return AH_User_MkPasswdName(u, buf);
}



int AH_User_MkTanName(const AB_USER *u,
		      const char *challenge,
		      GWEN_BUFFER *buf) {
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
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing tokenType or tokenName");
    return GWEN_ERROR_NO_DATA;
  }
}



void AH_User__MkPrompt(AB_USER *u,
		       const char *t,
		       GWEN_BUFFER *pbuf,
		       int minLen, int maxLen,
		       int flags){
  const char *numeric_warning = "";
  char buffer[512];
  const char *un;
  const char *bn=NULL;
  AB_BANKINFO *bi;

  assert(u);
  un=AB_User_GetUserId(u);

  /* find bank name */
  bi=AB_Banking_GetBankInfo(AB_User_GetBanking(u),
			    "de",
                            "*",
			    AB_User_GetBankCode(u));
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
	     I18N("Please enter a new %s for \n"
		  "user %s at %s\n"
		  "The input must be at least %d characters long.%s"
		  "<html>"
		  "<p>"
		  "Please enter a new %s for user <i>%s</i> at "
		  "<i>%s</i>."
		  "</p>"
		  "<p>"
		  "The input must be at least %d characters long.%s"
		  "</p>"
		  "</html>"),
	     t, un, bn,
	     minLen,
	     numeric_warning,
	     t, un, bn,
	     minLen,
	     numeric_warning);
  }
  else {
    snprintf(buffer, sizeof(buffer)-1,
	     I18N("Please enter the %s for \n"
		  "user %s at %s\n"
		  "%s"
		  "<html>"
		  "Please enter the %s for user <i>%s</i> at"
		  "<i>%s</i>.<br>"
		  "%s"
		  "</html>"),
	     t, un, bn,
	     numeric_warning,
	     t, un, bn,
	     numeric_warning);
  }
  buffer[sizeof(buffer)-1]=0;

  GWEN_Buffer_AppendString(pbuf, buffer);
  AB_BankInfo_free(bi);
}



int AH_User_InputPin(AB_USER *u,
		     char *pwbuffer,
		     int minLen, int maxLen,
		     int flags){
  GWEN_BUFFER *pbuf;
  GWEN_BUFFER *nbuf;
  int rv;

  pbuf=GWEN_Buffer_new(0, 256 ,0 ,1);
  AH_User__MkPrompt(u, I18N("PIN"), pbuf, minLen, maxLen, flags);

  nbuf=GWEN_Buffer_new(0, 256 ,0 ,1);
  AH_User_MkPinName(u, nbuf);

  rv=GWEN_Gui_GetPassword(flags,
			  GWEN_Buffer_GetStart(nbuf),
			  I18N("Enter PIN"),
			  GWEN_Buffer_GetStart(pbuf),
			  pwbuffer,
			  minLen,
			  maxLen,
			  0);
  GWEN_Buffer_free(nbuf);
  GWEN_Buffer_free(pbuf);
  return rv;
}



int AH_User_InputPasswd(AB_USER *u,
			char *pwbuffer,
			int minLen, int maxLen,
			int flags){
  GWEN_BUFFER *pbuf;
  GWEN_BUFFER *nbuf;
  int rv;

  pbuf=GWEN_Buffer_new(0, 256 ,0 ,1);
  AH_User__MkPrompt(u, I18N("password"), pbuf, minLen, maxLen, flags);

  nbuf=GWEN_Buffer_new(0, 256 ,0 ,1);
  AH_User_MkPasswdName(u, nbuf);

  rv=GWEN_Gui_GetPassword(flags,
			  GWEN_Buffer_GetStart(nbuf),
			  I18N("Enter Password"),
			  GWEN_Buffer_GetStart(pbuf),
			  pwbuffer,
			  minLen,
			  maxLen,
			  0);
  GWEN_Buffer_free(nbuf);
  GWEN_Buffer_free(pbuf);
  return rv;
}



int AH_User_InputTan(AB_USER *u,
		     char *pwbuffer,
		     int minLen,
		     int maxLen){
  int rv;
  char buffer[512];
  const char *un;
  const char *bn=NULL;
  GWEN_BUFFER *nbuf;
  AB_BANKINFO *bi;

  assert(u);
  un=AB_User_GetUserId(u);
  /* find bank name */
  bi=AB_Banking_GetBankInfo(AB_User_GetBanking(u),
			    "de",
                            "*",
			    AB_User_GetBankCode(u));
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

  nbuf=GWEN_Buffer_new(0, 256 ,0 ,1);
  AH_User_MkTanName(u, NULL, nbuf);

  rv=GWEN_Gui_GetPassword(GWEN_GUI_INPUT_FLAGS_TAN |
			  GWEN_GUI_INPUT_FLAGS_NUMERIC |
			  GWEN_GUI_INPUT_FLAGS_SHOW,
			  GWEN_Buffer_GetStart(nbuf),
			  I18N("Enter TAN"),
			  buffer,
			  pwbuffer,
			  minLen,
			  maxLen,
			  0);
  GWEN_Buffer_free(nbuf);
  AB_BankInfo_free(bi);
  return rv;
}



int AH_User_InputTanWithChallenge(AB_USER *u,
				  const char *challenge,
				  char *pwbuffer,
				  int minLen,
				  int maxLen){
  int rv;
  char buffer[512];
  const char *un;
  const char *bn=NULL;
  GWEN_BUFFER *nbuf;
  AB_BANKINFO *bi;

  assert(u);
  un=AB_User_GetUserId(u);
  /* find bank name */
  bi=AB_Banking_GetBankInfo(AB_User_GetBanking(u),
			    "de",
                            "*",
			    AB_User_GetBankCode(u));
  if (bi)
    bn=AB_BankInfo_GetBankName(bi);
  if (!bn)
    AB_User_GetBankCode(u);

  buffer[0]=0;
  buffer[sizeof(buffer)-1]=0;
  snprintf(buffer, sizeof(buffer)-1,
	   I18N("Please enter the TAN\n"
		"for user %s at %s.\n"
                "The server provided the following challenge:\n"
                "%s"
                "<html>"
                "<p>"
		"Please enter the TAN for user <i>%s</i> at "
		"<i>%s</i>."
		"</p>"
                "<p>"
                "The server provided the following challenge:"
                "</p>"
                "<p align=\"center\" >"
                "<font color=\"blue\">%s</font>"
                "</p>"
                "</html>"),
	   un, bn, challenge,
	   un, bn, challenge);

  nbuf=GWEN_Buffer_new(0, 256 ,0 ,1);
  AH_User_MkTanName(u, challenge, nbuf);
  rv=GWEN_Gui_GetPassword(GWEN_GUI_INPUT_FLAGS_TAN |
			  GWEN_GUI_INPUT_FLAGS_NUMERIC |
			  GWEN_GUI_INPUT_FLAGS_SHOW,
			  GWEN_Buffer_GetStart(nbuf),
			  I18N("Enter TAN"),
			  buffer,
			  pwbuffer,
			  minLen,
			  maxLen,
			  0);
  GWEN_Buffer_free(nbuf);
  AB_BankInfo_free(bi);
  return rv;
}



int AH_User_SetTanStatus(AB_USER *u,
			 const char *challenge,
			 const char *tan,
			 GWEN_GUI_PASSWORD_STATUS status){
  GWEN_BUFFER *nbuf;
  int rv;

  nbuf=GWEN_Buffer_new(0, 256 ,0 ,1);
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
			 GWEN_GUI_PASSWORD_STATUS status){
  GWEN_BUFFER *nbuf;
  int rv;

  nbuf=GWEN_Buffer_new(0, 256 ,0 ,1);
  AH_User_MkPinName(u, nbuf);
  rv=GWEN_Gui_SetPasswordStatus(GWEN_Buffer_GetStart(nbuf),
				pin,
				status,
				0);
  GWEN_Buffer_free(nbuf);
  return rv;
}



const char *AH_User_GetHttpContentType(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->httpContentType;
}



void AH_User_SetHttpContentType(AB_USER *u, const char *s) {
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



const int *AH_User_GetTanMethodList(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->tanMethodList;
}



int AH_User_HasTanMethod(const AB_USER *u, int method) {
  AH_USER *ue;
  int i;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  for (i=0; i<AH_USER_MAX_TANMETHODS; i++) {
    if (ue->tanMethodList[i]==method)
      return 1;
  }

  return 0;
}



int AH_User_HasTanMethodOtherThan(const AB_USER *u, int method) {
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



void AH_User_AddTanMethod(AB_USER *u, int method) {
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



void AH_User_ClearTanMethodList(AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->tanMethodList[0]=-1;
  ue->tanMethodCount=0;
}



int AH_User_GetTanMethodCount(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->tanMethodCount;
}













