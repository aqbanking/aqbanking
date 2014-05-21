/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2011 by Martin Preuss
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
#include "tanmethod_l.h"
#include <aqhbci/provider.h>
#include "adminjobs_l.h"

#include "hhd_l.h"

#include <aqbanking/banking_be.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>


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
  if (flags & AH_USER_FLAGS_KEEP_MULTIPLE_BLANKS)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name,
			 "keepMultipleBlanks");
  if (flags & AH_USER_FLAGS_TAN_OMIT_SMS_ACCOUNT)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name,
                         "omitSmsAccount");
  if (flags & AH_USER_FLAGS_TLS_ONLY_SAFE_CIPHERS)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name,
                         "tlsOnlySafeCiphers");
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
    else if (strcasecmp(s, "forceSsl3")==0 ||
	     strcasecmp(s, "forceSslv3")==0)
      fl|=AH_USER_FLAGS_FORCE_SSL3;
    else if (strcasecmp(s, "noBase64")==0)
      fl|=AH_USER_FLAGS_NO_BASE64;
    else if (strcasecmp(s, "keepMultipleBlanks")==0)
      fl|=AH_USER_FLAGS_KEEP_MULTIPLE_BLANKS;
    else if (strcasecmp(s, "omitSmsAccount")==0)
      fl|=AH_USER_FLAGS_TAN_OMIT_SMS_ACCOUNT;
    else if (strcasecmp(s, "tlsOnlySafeCiphers")==0)
      fl|=AH_USER_FLAGS_TLS_ONLY_SAFE_CIPHERS;
    else {
      DBG_WARN(AQHBCI_LOGDOMAIN, "Unknown user flag \"%s\"", s);
    }
  }

  return fl;
}





int AH_User_Extend(AB_USER *u, AB_PROVIDER *pro,
                   AB_PROVIDER_EXTEND_MODE em,
                   GWEN_DB_NODE *db) {
  DBG_INFO(AQHBCI_LOGDOMAIN, "Extending user with mode %d", em);
  if (em==AB_ProviderExtendMode_Create ||
      em==AB_ProviderExtendMode_Extend) {
    AH_USER *ue;
    const char *s;
    int rv;

    GWEN_NEW_OBJECT(AH_USER, ue);
    GWEN_INHERIT_SETDATA(AB_USER, AH_USER, u, ue, AH_User_freeData);

    ue->tanMethodList[0]=-1;
    ue->tanMethodCount=0;

    ue->hbci=AH_Provider_GetHbci(pro);
    ue->tanMethodDescriptions=AH_TanMethod_List_new();
    ue->sepaDescriptors=GWEN_StringList_new();

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

    if (em==AB_ProviderExtendMode_Create) {
      ue->hbciVersion=210;
      ue->bpd=AH_Bpd_new();
      ue->dbUpd=GWEN_DB_Group_new("upd");
      ue->maxTransfersPerJob=AH_USER_MAX_TRANSFERS_PER_JOB;
      ue->maxDebitNotesPerJob=AH_USER_MAX_DEBITNOTES_PER_JOB;
    }
    else {
      /* update db to latest version */
      rv=AH_HBCI_UpdateDbUser(ue->hbci, db);
      if (rv<0) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not update user db (%d)", rv);
        GWEN_Gui_ShowError(I18N("AqBanking Settings Database Error"),
                           I18N("Your settings database might be in an inconsistent state!"));
        return rv;
      }
      AH_User_ReadDb(u, db);
      AH_User_LoadTanMethods(u);
      AH_User_LoadSepaDescriptors(u);
      if (rv==1) {
	/* updated config, write it now */
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Writing back updated HBCI user %d", AB_User_GetUniqueId(u));
	rv=AB_Banking_SaveUserConfig(AB_Provider_GetBanking(pro), u, 1);
	if (rv<0) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not save user db (%d)", rv);
          GWEN_Gui_ShowError(I18N("AqBanking Settings Database Error"),
                             I18N("Your settings database might be in an inconsistent state!"));
          return rv;
	}
      }
    }
  }
  else if (em==AB_ProviderExtendMode_Reload) {
    /* just reload user */
    AH_User_ReadDb(u, db);
    AH_User_LoadTanMethods(u);
    AH_User_LoadSepaDescriptors(u);
  }
  else {
    AH_USER *ue;

    ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
    assert(ue);

    if (em==AB_ProviderExtendMode_Add) {
    }
    else if (em==AB_ProviderExtendMode_Save) {
      AH_User_toDb(u, db);
    } /* if save */
  }

  return 0;
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
  GWEN_DB_Group_free(ue->dbUpd);
  AH_Bpd_free(ue->bpd);
  GWEN_MsgEngine_free(ue->msgEngine);
  AH_TanMethod_List_free(ue->tanMethodDescriptions);
  GWEN_StringList_free(ue->sepaDescriptors);
  GWEN_FREE_OBJECT(ue);
}



void AH_User_ReadDb(AB_USER *u, GWEN_DB_NODE *db) {
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
  if (s) ue->peerId=strdup(s);
  else ue->peerId=NULL;
  
  /* get system id */
  free(ue->systemId);
  s=GWEN_DB_GetCharValue(db, "systemId", 0, 0);
  if (s) ue->systemId=strdup(s);
  else ue->systemId=NULL;
  
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
  if (s) ue->httpContentType=strdup(s);
  else ue->httpContentType=NULL;
  
  /* read user flags */
  ue->flags=AH_User_Flags_fromDb(db, "userFlags");
  
  /* setup medium stuff */
  free(ue->tokenType);
  s=GWEN_DB_GetCharValue(db, "tokenType", 0, 0);
  if (s) ue->tokenType=strdup(s);
  else ue->tokenType=NULL;
  
  free(ue->tokenName);
  s=GWEN_DB_GetCharValue(db, "tokenName", 0, 0);
  if (s) ue->tokenName=strdup(s);
  else ue->tokenName=NULL;
  
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
  if (s) ue->sepaTransferProfile=strdup(s);
  else ue->sepaTransferProfile=NULL;
  free(ue->sepaDebitNoteProfile);
  s=GWEN_DB_GetCharValue(db, "sepaDebitNoteProfile", 0, NULL);
  if (s) ue->sepaDebitNoteProfile=strdup(s);
  else ue->sepaDebitNoteProfile=NULL;

  free(ue->tanMediumId);
  s=GWEN_DB_GetCharValue(db, "tanMediumId", 0, NULL);
  if (s) ue->tanMediumId=strdup(s);
  else ue->tanMediumId=NULL;
}



void AH_User_toDb(AB_USER *u, GWEN_DB_NODE *db) {
  AH_USER *ue;
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

  if (ue->dbUpd)
    GWEN_DB_Group_free(ue->dbUpd);
  if (n) {
    ue->dbUpd=GWEN_DB_Group_dup(n);
  }
  else
    ue->dbUpd=GWEN_DB_Group_new("upd");
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
                     int flags){
  GWEN_BUFFER *nbuf;
  int rv;
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

  nbuf=GWEN_Buffer_new(0, 256 ,0 ,1);
  AH_User_MkPinName(u, nbuf);

  rv=GWEN_Gui_GetPassword(flags,
			  GWEN_Buffer_GetStart(nbuf),
			  I18N("Enter PIN"),
                          buffer,
			  pwbuffer,
			  minLen,
			  maxLen,
			  0);
  GWEN_Buffer_free(nbuf);

  return rv;
}



int AH_User_InputPasswd(AB_USER *u,
			char *pwbuffer,
			int minLen, int maxLen,
                        int flags){
  GWEN_BUFFER *nbuf;
  int rv;
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
	     I18N("Please enter a new password for \n"
		  "user %s at %s\n"
		  "The input must be at least %d characters long.%s"
		  "<html>"
		  "<p>"
		  "Please enter a new password for user <i>%s</i> at "
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
	     I18N("Please enter the password for \n"
		  "user %s at %s\n"
		  "%s"
		  "<html>"
		  "Please enter the password for user <i>%s</i> at"
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

  nbuf=GWEN_Buffer_new(0, 256 ,0 ,1);
  AH_User_MkPasswdName(u, nbuf);

  rv=GWEN_Gui_GetPassword(flags,
			  GWEN_Buffer_GetStart(nbuf),
			  I18N("Enter Password"),
			  buffer,
			  pwbuffer,
			  minLen,
			  maxLen,
			  0);
  GWEN_Buffer_free(nbuf);

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
			  /*GWEN_GUI_INPUT_FLAGS_NUMERIC |*/
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
  char buffer[1024];
  const char *un;
  const char *bn=NULL;
  GWEN_BUFFER *nbuf;
  GWEN_BUFFER *xbuf;
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

  xbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* text version */
  snprintf(buffer, sizeof(buffer)-1,
           I18N("Please enter the TAN\n"
                "for user %s at %s.\n"), un, bn);
  buffer[sizeof(buffer)-1]=0;
  GWEN_Buffer_AppendString(xbuf, buffer);
  if (challenge && *challenge) {
    GWEN_Buffer_AppendString(xbuf, I18N("The server provided the following challenge:"));
    GWEN_Buffer_AppendString(xbuf, "\n");
    GWEN_Buffer_AppendString(xbuf, challenge);
  }

  /* html version */
  GWEN_Buffer_AppendString(xbuf,
                           "<html>"
                           "<p>");
  snprintf(buffer, sizeof(buffer)-1,
           I18N("Please enter the TAN for user <i>%s</i> at <i>%s</i>."), un, bn);
  buffer[sizeof(buffer)-1]=0;
  GWEN_Buffer_AppendString(xbuf, buffer);
  GWEN_Buffer_AppendString(xbuf, "</p>");
  if (challenge && *challenge) {
    GWEN_Buffer_AppendString(xbuf, "<p>");
    GWEN_Buffer_AppendString(xbuf, I18N("The server provided the following challenge:"));
    GWEN_Buffer_AppendString(xbuf, "</p>"
                             "<p align=\"center\" >"
                             "<font color=\"blue\">");
    GWEN_Buffer_AppendString(xbuf, challenge);
    GWEN_Buffer_AppendString(xbuf,
                             "</font>"
                             "</p>"
                             "</html>");
  }

  nbuf=GWEN_Buffer_new(0, 256 ,0 ,1);
  AH_User_MkTanName(u, challenge, nbuf);
  rv=GWEN_Gui_GetPassword(GWEN_GUI_INPUT_FLAGS_TAN |
			  /*GWEN_GUI_INPUT_FLAGS_NUMERIC |*/
                          GWEN_GUI_INPUT_FLAGS_SHOW,
			  GWEN_Buffer_GetStart(nbuf),
                          I18N("Enter TAN"),
                          GWEN_Buffer_GetStart(xbuf),
                          pwbuffer,
			  minLen,
			  maxLen,
			  0);
  GWEN_Buffer_free(xbuf);
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

  DBG_INFO(AQHBCI_LOGDOMAIN, "TAN method %d not available", method);
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



int AH_User_GetSelectedTanMethod(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->selectedTanMethod;
}



void AH_User_SetSelectedTanMethod(AB_USER *u, int i) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->selectedTanMethod=i;
}



const char *AH_User_GetTanMediumId(const AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->tanMediumId;
}



void AH_User_SetTanMediumId(AB_USER *u, const char *s) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  free(ue->tanMediumId);
  if (s) ue->tanMediumId=strdup(s);
  else ue->tanMediumId=NULL;
}



void AH_User_LoadTanMethods(AB_USER *u) {
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
      while(dbV) {
	int version;

	version=atoi(GWEN_DB_GroupName(dbV));
	if (version>0) {
	  GWEN_DB_NODE *dbT;

	  dbT=GWEN_DB_FindFirstGroup(dbV, "tanMethod");
	  if (!dbT) {
	    DBG_INFO(AQHBCI_LOGDOMAIN, "No tan method found");
	  }
	  while(dbT) {
	    AH_TAN_METHOD *tm;
	    const char *s;

	    tm=AH_TanMethod_new();
	    AH_TanMethod_SetFunction(tm, GWEN_DB_GetIntValue(dbT, "function", 0, 0));
	    AH_TanMethod_SetProcess(tm, GWEN_DB_GetIntValue(dbT, "process", 0, 0));
	    AH_TanMethod_SetMethodId(tm, GWEN_DB_GetCharValue(dbT, "methodId", 0, 0));
	    AH_TanMethod_SetMethodName(tm, GWEN_DB_GetCharValue(dbT, "methodName", 0, 0));
	    AH_TanMethod_SetTanMaxLen(tm, GWEN_DB_GetIntValue(dbT, "tanMaxLen", 0, 0));
	    AH_TanMethod_SetFormatId(tm, GWEN_DB_GetCharValue(dbT, "formatId", 0, 0));
	    AH_TanMethod_SetPrompt(tm, GWEN_DB_GetCharValue(dbT, "prompt", 0, 0));
	    AH_TanMethod_SetReturnMaxLen(tm, GWEN_DB_GetIntValue(dbT, "returnMaxLen", 0, 0));
	    AH_TanMethod_SetMaxActiveLists(tm, GWEN_DB_GetIntValue(dbT, "maxActiveLists", 0, 0));
	    AH_TanMethod_SetGvVersion(tm, GWEN_DB_GetIntValue(dbT, "gvVersion", 0, 0));
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
	    AH_TanMethod_SetMaxActiveTanMedia(tm, GWEN_DB_GetIntValue(dbT, "maxActiveMedia", 0, 0));

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



void AH_User_LoadSepaDescriptors(AB_USER *u) {
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
    while(dbV) {
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
	while(dbT) {
	  int i;

	  for (i=0; i<100; i++) {
	    const char *s;

	    s=GWEN_DB_GetCharValue(dbT, "format", i, NULL);
	    if (! (s && *s))
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




const AH_TAN_METHOD_LIST *AH_User_GetTanMethodDescriptions(AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  /* always reload TAN methods from BPD */
  AH_User_LoadTanMethods(u);

  return ue->tanMethodDescriptions;
}



int AH_User_GetMaxTransfersPerJob(const AB_USER *u){
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->maxTransfersPerJob;
}



void AH_User_SetMaxTransfersPerJob(AB_USER *u, int i){
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->maxTransfersPerJob=i;
}



int AH_User_GetMaxDebitNotesPerJob(const AB_USER *u){
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->maxDebitNotesPerJob;
}



void AH_User_SetMaxDebitNotesPerJob(AB_USER *u, int i){
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  ue->maxDebitNotesPerJob=i;
}



int AH_User_AddTextWithoutTags(const char *s, GWEN_BUFFER *obuf) {
  while(*s) {
    if (*s=='<') {
      const char *s2;
      int l;

      s2=s;
      s2++;
      while(*s2 && *s2!='>')
	s2++;
      l=s2-s-2;
      if (l>0) {
	const char *s3;

	s3=s;
	s3++;
	if (l==2) {
	  if (strncasecmp(s3, "br", 2)==0)
	    GWEN_Buffer_AppendString(obuf, "\n");
	}
	else if (l==3) {
	  if (strncasecmp(s3, "br/", 3)==0)
	    GWEN_Buffer_AppendString(obuf, "\n");
	}
      }
      s=s2; /* set s to position of closing bracket */
    }
    else
      GWEN_Buffer_AppendByte(obuf, *s);
    /* next char */
    s++;
  }

  return 0;
}



int AH_User_InputTanWithChallenge2(AB_USER *u,
				   const char *challenge,
				   const char *challengeHhd,
				   char *pwbuffer,
				   int minLen,
				   int maxLen){
  int rv;
  char buffer[1024];
  const char *un;
  const char *bn=NULL;
  GWEN_BUFFER *nbuf;
  GWEN_BUFFER *xbuf;
  AB_BANKINFO *bi;
  uint32_t iflags=0;

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

  iflags=GWEN_GUI_INPUT_FLAGS_TAN | GWEN_GUI_INPUT_FLAGS_SHOW;

  buffer[0]=0;
  buffer[sizeof(buffer)-1]=0;

  xbuf=GWEN_Buffer_new(0, 256, 0, 1);

  if (challengeHhd && *challengeHhd) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "ChallengeHHD is [%s]", challengeHhd);
    GWEN_Buffer_AppendString(xbuf, "$OBEGIN$");
    rv=AH_HHD14_Translate(challengeHhd, xbuf);
    if (rv<0) {
      GWEN_Buffer_free(xbuf);
      AB_BankInfo_free(bi);
      return rv;
    }
    GWEN_Buffer_AppendString(xbuf, "$OEND$");
    iflags|=GWEN_GUI_INPUT_FLAGS_OPTICAL;

    /* text version */
    snprintf(buffer, sizeof(buffer)-1,
	     I18N("Please enter the TAN\n"
		  "for user %s at %s.\n"), un, bn);
    buffer[sizeof(buffer)-1]=0;
    GWEN_Buffer_AppendString(xbuf, buffer);
    if (challenge && *challenge) {
      GWEN_Buffer_AppendString(xbuf, challenge);
      GWEN_Buffer_AppendString(xbuf, "\n");
    }
    else {
      GWEN_Buffer_AppendString(xbuf, I18N("Please enter the TAN from the device."));
    }
  }
  else if (challenge && *challenge) {
    const char *s;

    /* look for "CHLGUC" */
    s=GWEN_Text_StrCaseStr(challenge, "CHLGUC");
    if (s) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Challenge contains CHLGUC");
      GWEN_Buffer_AppendString(xbuf, "$OBEGIN$");
      rv=AH_HHD14_Translate(s, xbuf);
      if (rv<0) {
	GWEN_Buffer_free(xbuf);
	AB_BankInfo_free(bi);
	return rv;
      }
      GWEN_Buffer_AppendString(xbuf, "$OEND$");
      iflags|=GWEN_GUI_INPUT_FLAGS_OPTICAL;

      /* text version */
      snprintf(buffer, sizeof(buffer)-1,
               I18N("Please enter the TAN\n"
                    "for user %s at %s.\n"), un, bn);
      buffer[sizeof(buffer)-1]=0;
      GWEN_Buffer_AppendString(xbuf, buffer);

      /* extract text */
      s=GWEN_Text_StrCaseStr(challenge, "CHLGTEXT");
      if (s) {
	/* skip "CHLGTEXT" and 4 digits */
	s+=12;
	/* add rest of the message (replace HTML tags, if any) */
	AH_User_AddTextWithoutTags(s, xbuf);
      }
      else {
	/* create own text */
	GWEN_Buffer_AppendString(xbuf, I18N("Please enter the TAN from the device."));
      }
    }
    else {
      /* no optical challenge */
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Challenge contains no optical data");
      GWEN_Buffer_AppendString(xbuf, I18N("The server provided the following challenge:"));
      GWEN_Buffer_AppendString(xbuf, "\n");
      GWEN_Buffer_AppendString(xbuf, challenge);
    }
  }

  nbuf=GWEN_Buffer_new(0, 256 ,0 ,1);
  AH_User_MkTanName(u, challenge, nbuf);
  rv=GWEN_Gui_GetPassword(iflags,
			  GWEN_Buffer_GetStart(nbuf),
			  I18N("Enter TAN"),
			  GWEN_Buffer_GetStart(xbuf),
			  pwbuffer,
			  minLen,
			  maxLen,
			  0);
  GWEN_Buffer_free(xbuf);
  GWEN_Buffer_free(nbuf);
  AB_BankInfo_free(bi);
  return rv;
}



const char *AH_User_FindSepaDescriptor(AB_USER *u, const char *tmpl) {
  AH_USER *ue;
  GWEN_STRINGLISTENTRY *se;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  if (GWEN_StringList_Count(ue->sepaDescriptors)<1)
    AH_User_LoadSepaDescriptors(u);

  se=GWEN_StringList_FirstEntry(ue->sepaDescriptors);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    if (s && *s && -1!=GWEN_Text_ComparePattern(s, tmpl, 1))
      return s;

    se=GWEN_StringListEntry_Next(se);
  }

  return NULL;
}



const GWEN_STRINGLIST *AH_User_GetSepaDescriptors(AB_USER *u) {
  AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->sepaDescriptors;
}



const char *AH_User_GetSepaTransferProfile(const AB_USER *u) {
  const AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->sepaTransferProfile;
}



void AH_User_SetSepaTransferProfile(AB_USER *u, const char *profileName) {
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



const char *AH_User_GetSepaDebitNoteProfile(const AB_USER *u) {
  const AH_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, AH_USER, u);
  assert(ue);

  return ue->sepaDebitNoteProfile;
}



void AH_User_SetSepaDebitNoteProfile(AB_USER *u, const char *profileName) {
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
