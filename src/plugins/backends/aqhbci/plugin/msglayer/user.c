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
                        GWEN_TYPE_UINT32 flags) {
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
}



GWEN_TYPE_UINT32 AH_User_Flags_fromDb(GWEN_DB_NODE *db, const char *name) {
  GWEN_TYPE_UINT32 fl=0;
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
  GWEN_TYPE_UINT32 mediumId;

  db=AB_User_GetProviderData(u);
  assert(db);

  if (em==AB_ProviderExtendMode_Create ||
      em==AB_ProviderExtendMode_Extend) {
    AH_USER *ue;
    const char *s;
    int rv;

    GWEN_NEW_OBJECT(AH_USER, ue);
    GWEN_INHERIT_SETDATA(AB_USER, AH_USER, u, ue, AH_User_freeData);

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
    AH_MsgEngine_SetUser(ue->msgEngine, u);
    GWEN_MsgEngine_SetDefinitions(ue->msgEngine,
                                  AH_HBCI_GetDefinitions(ue->hbci),
                                  0);


    ue->flags=AH_User_Flags_fromDb(db, "userFlags");

    /* get medium */
    mediumId=GWEN_DB_GetIntValue(db, "medium", 0, 0);
    if (mediumId) {
      ue->medium=AH_HBCI_FindMediumById(ue->hbci, mediumId);
      if (!ue->medium) {
        DBG_ERROR(AQHBCI_LOGDOMAIN,
                  "Medium for user \"%08x\" is not available",
                  AB_User_GetUniqueId(u));
        abort();
      }
      AH_Medium_Attach(ue->medium);
    }
    else {
      if (em==AB_ProviderExtendMode_Extend) {
        DBG_ERROR(AQHBCI_LOGDOMAIN,
                  "No medium id for user \"%s\"",
                  AB_User_GetUserId(u));
        abort();
      }
    }

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
  }
  else {
    AH_USER *ue;

    ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
    assert(ue);

    if (em==AB_ProviderExtendMode_Add) {
      assert(AH_User_GetMedium(u));
    }
    else if (em==AB_ProviderExtendMode_Save) {
      assert(ue->medium);
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "medium",
                          AH_Medium_GetUniqueId(ue->medium));

      assert(ue->bpd);
      gr=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "bpd");
      assert(gr);
      AH_Bpd_ToDb(ue->bpd, gr);
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
    } /* if save */
  }
}


void AH_User_freeData(void *bp, void *p) {
  AH_USER *ue;

  ue=(AH_USER*)p;
  AH_Medium_free(ue->medium);
  GWEN_Url_free(ue->serverUrl);
  AH_Bpd_free(ue->bpd);
  GWEN_MsgEngine_free(ue->msgEngine);
  GWEN_FREE_OBJECT(ue);
}



const char *AH_User_GetPeerId(const AB_USER *u) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  return GWEN_DB_GetCharValue(db, "peerId", 0, 0);
}



void AH_User_SetPeerId(AB_USER *u, const char *s) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "peerId", s);
  else
    GWEN_DB_DeleteVar(db, "peerId");
}



int AH_User_GetContextIdx(const AB_USER *u){
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  return GWEN_DB_GetIntValue(db, "contextIdx", 0, 0);
}



void AH_User_SetContextIdx(AB_USER *u, int idx){
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "contextIdx", idx);
}



AH_CRYPT_MODE AH_User_GetCryptMode(const AB_USER *u) {
  GWEN_DB_NODE *db;
  const char *s;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  s=GWEN_DB_GetCharValue(db, "cryptMode", 0, "unknown");
  return AH_CryptMode_fromString(s);
}



void AH_User_SetCryptMode(AB_USER *u, AH_CRYPT_MODE m) {
  GWEN_DB_NODE *db;
  const char *s;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  s=AH_CryptMode_toString(m);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "cryptMode", s);
}



AH_USER_STATUS AH_User_GetStatus(const AB_USER *u){
  GWEN_DB_NODE *db;
  const char *s;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  s=GWEN_DB_GetCharValue(db, "status", 0, "unknown");
  return AH_User_Status_fromString(s);
}



void AH_User_SetStatus(AB_USER *u, AH_USER_STATUS i){
  GWEN_DB_NODE *db;
  const char *s;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  s=AH_User_Status_toString(i);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "status", s);
}



AH_HBCI *AH_User_GetHbci(const AB_USER *u){
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->hbci;
}



void AH_User_SetMedium(AB_USER *u, AH_MEDIUM *m) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  if (m)
    AH_Medium_Attach(m);
  AH_Medium_free(ue->medium);
  ue->medium=m;
}



AH_MEDIUM *AH_User_GetMedium(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->medium;
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
  GWEN_DB_NODE *db;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  db=AB_User_GetProviderData(u);
  assert(db);

  return GWEN_DB_GetIntValue(db, "updVersion", 0, 0);
}



void AH_User_SetUpdVersion(AB_USER *u, int i){
  AH_USER *ue;
  GWEN_DB_NODE *db;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  db=AB_User_GetProviderData(u);
  assert(db);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "updVersion", i);
}



GWEN_DB_NODE *AH_User_GetUpd(const AB_USER *u){
  AH_USER *ue;
  GWEN_DB_NODE *db;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  db=AB_User_GetProviderData(u);
  assert(db);

  return GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "upd");
}



void AH_User_SetUpd(AB_USER *u, GWEN_DB_NODE *n){
  AH_USER *ue;
  GWEN_DB_NODE *db;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  db=AB_User_GetProviderData(u);
  assert(db);

  db=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "upd");
  if (n) {
    GWEN_DB_AddGroupChildren(db, n);
    GWEN_DB_Group_free(n);
  }
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
  GWEN_DB_NODE *db;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  db=AB_User_GetProviderData(u);
  assert(db);
  if (bpd) {
    if (ue->bpd!=bpd)
      AH_Bpd_free(ue->bpd);
    ue->bpd=bpd;
    db=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "bpd");
    assert(db);
    AH_Bpd_ToDb(ue->bpd, db);
  }
  else {
    ue->bpd=0;
    GWEN_DB_DeleteGroup(db, "bpd");
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
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  return GWEN_DB_GetCharValue(db, "systemId", 0, 0);
}



void AH_User_SetSystemId(AB_USER *u, const char *s) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "systemId", s);
  else
    GWEN_DB_DeleteVar(db, "systemId");
}



GWEN_MSGENGINE *AH_User_GetMsgEngine(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->msgEngine;
}



GWEN_TYPE_UINT32 AH_User_GetFlags(const AB_USER *u) {
  AH_USER *ue;
  GWEN_DB_NODE *db;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  db=AB_User_GetProviderData(u);
  assert(db);

  ue->flags=AH_User_Flags_fromDb(db, "userFlags");
  return ue->flags;
}



void AH_User_SetFlags(AB_USER *u, GWEN_TYPE_UINT32 flags) {
  AH_USER *ue;
  GWEN_DB_NODE *db;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  db=AB_User_GetProviderData(u);
  assert(db);

  ue->flags=flags;
  AH_User_Flags_toDb(db, "userFlags", ue->flags);
}



void AH_User_AddFlags(AB_USER *u, GWEN_TYPE_UINT32 flags) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  AH_User_SetFlags(u, ue->flags | flags);
}



void AH_User_SubFlags(AB_USER *u, GWEN_TYPE_UINT32 flags) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  AH_User_SetFlags(u, ue->flags & ~flags);
}



int AH_User_GetHbciVersion(const AB_USER *u) {
  AH_USER *ue;
  GWEN_DB_NODE *db;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  db=AB_User_GetProviderData(u);
  assert(db);

  return GWEN_DB_GetIntValue(db, "hbciVersion", 0, 0);
}



void AH_User_SetHbciVersion(AB_USER *u, int i) {
  AH_USER *ue;
  GWEN_DB_NODE *db;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  db=AB_User_GetProviderData(u);
  assert(db);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "hbciVersion", i);
}



const char *AH_User_GetHttpUserAgent(const AB_USER *u) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  return GWEN_DB_GetCharValue(db, "httpUserAgent", 0, 0);
}



void AH_User_SetHttpUserAgent(AB_USER *u, const char *s) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "httpUserAgent", s);
  else
    GWEN_DB_DeleteVar(db, "httpUserAgent");
}



int AH_User_GetHttpVMajor(const AB_USER *u) {
  AH_USER *ue;
  GWEN_DB_NODE *db;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  db=AB_User_GetProviderData(u);
  assert(db);

  return GWEN_DB_GetIntValue(db, "httpVMajor", 0, 0);
}



void AH_User_SetHttpVMajor(AB_USER *u, int i) {
  AH_USER *ue;
  GWEN_DB_NODE *db;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  db=AB_User_GetProviderData(u);
  assert(db);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "httpVMajor", i);
}



int AH_User_GetHttpVMinor(const AB_USER *u) {
  AH_USER *ue;
  GWEN_DB_NODE *db;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  db=AB_User_GetProviderData(u);
  assert(db);

  return GWEN_DB_GetIntValue(db, "httpVMinjor", 0, 0);
}



void AH_User_SetHttpVMinor(AB_USER *u, int i) {
  AH_USER *ue;
  GWEN_DB_NODE *db;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  db=AB_User_GetProviderData(u);
  assert(db);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "httpVMinor", i);
}

















