/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "user_p.h"
#include "provider_l.h"

#include <gwenhywfar/debug.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT(AB_USER, EBC_USER)


const char *EBC_User_Status_toString(EBC_USER_STATUS st){
  switch(st) {
  case EBC_UserStatus_New:      return "new";
  case EBC_UserStatus_Enabled:  return "enabled";
  case EBC_UserStatus_Init1:    return "init1";
  case EBC_UserStatus_Init2:    return "init2";
  case EBC_UserStatus_Disabled: return "disabled";
  default:                      return "unknown";
  } /* switch */
}



EBC_USER_STATUS EBC_User_Status_fromString(const char *s){
  assert(s);
  if (strcasecmp(s, "new")==0)
    return EBC_UserStatus_New;
  else if (strcasecmp(s, "enabled")==0)
    return EBC_UserStatus_Enabled;
  else if (strcasecmp(s, "init1")==0)
    return EBC_UserStatus_Init1;
  else if (strcasecmp(s, "init2")==0)
    return EBC_UserStatus_Init2;
  else if (strcasecmp(s, "disabled")==0)
    return EBC_UserStatus_Disabled;
  else
    return EBC_UserStatus_Unknown;
}



void EBC_User_Flags_toDb(GWEN_DB_NODE *db, const char *name,
                         uint32_t flags) {
  GWEN_DB_DeleteVar(db, name);
  if (flags & EBC_USER_FLAGS_BANK_DOESNT_SIGN)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "bankDoesntSign");
  if (flags & EBC_USER_FLAGS_FORCE_SSLV3)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "forceSslv3");
  if (flags & EBC_USER_FLAGS_INI)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "INI");
  if (flags & EBC_USER_FLAGS_HIA)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "HIA");
  if (flags & EBC_USER_FLAGS_CLIENT_DATA_DOWNLOAD_SPP)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "clientDataDownloadSpp");
  if (flags & EBC_USER_FLAGS_PREVALIDATION_SPP)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "prevalidationSpp");
  if (flags & EBC_USER_FLAGS_RECOVERY_SPP)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "recoverySpp");
  if (flags & EBC_USER_FLAGS_STA_SPP)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "staSpp");
  if (flags & EBC_USER_FLAGS_IZV_SPP)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "izvSpp");
  if (flags & EBC_USER_FLAGS_USE_IZL)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "useIZL");
  if (flags & EBC_USER_FLAGS_TIMESTAMP_FIX1)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "timestampFix1");
  if (flags & EBC_USER_FLAGS_NO_EU)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "noEu");
}



uint32_t EBC_User_Flags_fromDb(GWEN_DB_NODE *db, const char *name) {
  uint32_t fl=0;
  int i;

  for (i=0; ; i++) {
    const char *s;

    s=GWEN_DB_GetCharValue(db, name, i, 0);
    if (!s)
      break;
    if (strcasecmp(s, "bankDoesntSign")==0)
      fl|=EBC_USER_FLAGS_BANK_DOESNT_SIGN;
    else if (strcasecmp(s, "forceSslv3")==0)
      fl|=EBC_USER_FLAGS_FORCE_SSLV3;
    else if (strcasecmp(s, "ini")==0)
      fl|=EBC_USER_FLAGS_INI;
    else if (strcasecmp(s, "hia")==0)
      fl|=EBC_USER_FLAGS_HIA;
    else if (strcasecmp(s, "clientDataDownloadSpp")==0)
      fl|=EBC_USER_FLAGS_CLIENT_DATA_DOWNLOAD_SPP;
    else if (strcasecmp(s, "prevalidationSpp")==0)
      fl|=EBC_USER_FLAGS_PREVALIDATION_SPP;
    else if (strcasecmp(s, "recoverySpp")==0)
      fl|=EBC_USER_FLAGS_RECOVERY_SPP;
    else if (strcasecmp(s, "staSpp")==0)
      fl|=EBC_USER_FLAGS_STA_SPP;
    else if (strcasecmp(s, "izvSpp")==0)
      fl|=EBC_USER_FLAGS_IZV_SPP;
    else if (strcasecmp(s, "useIZL")==0)
      fl|=EBC_USER_FLAGS_USE_IZL;
    else if (strcasecmp(s, "timestampFix1")==0)
      fl|=EBC_USER_FLAGS_TIMESTAMP_FIX1;
    else if (strcasecmp(s, "noEu")==0)
      fl|=EBC_USER_FLAGS_NO_EU;
    else {
      DBG_WARN(AQEBICS_LOGDOMAIN, "Unknown user flag \"%s\"", s);
    }
  }

  return fl;
}








void EBC_User_Extend(AB_USER *u, GWEN_UNUSED AB_PROVIDER *pro,
		     AB_PROVIDER_EXTEND_MODE em,
		     GWEN_DB_NODE *db) {
  DBG_INFO(AQEBICS_LOGDOMAIN, "Extending user with mode %d", em);
  if (em==AB_ProviderExtendMode_Create ||
      em==AB_ProviderExtendMode_Extend) {
    EBC_USER *ue;
    const char *s;

    GWEN_NEW_OBJECT(EBC_USER, ue);
    GWEN_INHERIT_SETDATA(AB_USER, EBC_USER, u, ue, EBC_User_freeData);

    if (em==AB_ProviderExtendMode_Create) {
      s=AB_User_GetCountry(u);
      if (!s || !*s)
	AB_User_SetCountry(u, "de");
      /* some reasonable presets */
      ue->protoVersion=strdup("H003");
      ue->signVersion=strdup("A005");
      ue->cryptVersion=strdup("E002");
      ue->authVersion=strdup("X002");
    }
    else {
      EBC_User_ReadDb(u, db);
    }
  }
  else {
    if (em==AB_ProviderExtendMode_Add) {
    }
    else if (em==AB_ProviderExtendMode_Save)
      EBC_User_toDb(u, db);
  }
}



void GWENHYWFAR_CB EBC_User_freeData(GWEN_UNUSED void *bp, void *p) {
  EBC_USER *ue;

  ue=(EBC_USER*)p;
  free(ue->peerId);
  free(ue->tokenType);
  free(ue->tokenName);
  free(ue->protoVersion);
  free(ue->signVersion);
  free(ue->cryptVersion);
  free(ue->authVersion);
  free(ue->systemId);
  free(ue->httpUserAgent);
  free(ue->httpContentType);
  free(ue->serverUrl);
  GWEN_FREE_OBJECT(ue);
}



void EBC_User_ReadDb(AB_USER *u, GWEN_DB_NODE *db) {
  EBC_USER *ue;
  const char *s;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  ue->flags=EBC_User_Flags_fromDb(db, "userFlags");
  
  s=GWEN_DB_GetCharValue(db, "status", 0, "new");
  ue->status=EBC_User_Status_fromString(s);
  
  /* load server address */
  free(ue->serverUrl);
  s=GWEN_DB_GetCharValue(db, "server", 0, 0);
  if (s) ue->serverUrl=strdup(s);
  else ue->serverUrl=NULL;

  /* get peer id */
  free(ue->peerId);
  s=GWEN_DB_GetCharValue(db, "peerId", 0, 0);
  if (s) ue->peerId=strdup(s);
  else ue->peerId=NULL;

  free(ue->systemId);
  s=GWEN_DB_GetCharValue(db, "systemId", 0, 0);
  if (s) ue->systemId=strdup(s);
  else ue->systemId=NULL;
  
  /* setup HTTP version */
  ue->httpVMajor=GWEN_DB_GetIntValue(db, "httpVMajor", 0, -1);
  ue->httpVMinor=GWEN_DB_GetIntValue(db, "httpVMinor", 0, -1);
  if (ue->httpVMajor==-1 || ue->httpVMinor==-1) {
    ue->httpVMajor=1;
    ue->httpVMinor=1;
  }

  free(ue->httpUserAgent);
  s=GWEN_DB_GetCharValue(db, "httpUserAgent", 0, 0);
  if (s) ue->httpUserAgent=strdup(s);
  else ue->httpUserAgent=NULL;

  free(ue->httpContentType);
  s=GWEN_DB_GetCharValue(db, "httpContentType", 0,
			 "text/xml; charset=UTF-8");
  if (s) ue->httpContentType=strdup(s);
  else ue->httpContentType=NULL;
  
  /* setup medium stuff */
  free(ue->tokenType);
  s=GWEN_DB_GetCharValue(db, "tokenType", 0, 0);
  if (s) ue->tokenType=strdup(s);
  else ue->tokenType=NULL;

  free(ue->tokenName);
  s=GWEN_DB_GetCharValue(db, "tokenName", 0, 0);
  if (s) ue->tokenName=strdup(s);
  else ue->tokenName=NULL;

  free(ue->protoVersion);
  s=GWEN_DB_GetCharValue(db, "protoVersion", 0, "H002");
  if (s) ue->protoVersion=strdup(s);
  else ue->protoVersion=NULL;

  free(ue->signVersion);
  s=GWEN_DB_GetCharValue(db, "signVersion", 0, "A004");
  if (s) ue->signVersion=strdup(s);
  else ue->signVersion=NULL;

  free(ue->cryptVersion);
  s=GWEN_DB_GetCharValue(db, "cryptVersion", 0, "E001");
  if (s) ue->cryptVersion=strdup(s);
  else ue->cryptVersion=NULL;

  free(ue->authVersion);
  s=GWEN_DB_GetCharValue(db, "authVersion", 0, "X001");
  if (s) ue->authVersion=strdup(s);
  else ue->authVersion=NULL;

  ue->tokenContextId=GWEN_DB_GetIntValue(db, "tokenContextId", 0, 1);
}



void EBC_User_toDb(AB_USER *u, GWEN_DB_NODE *db) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  EBC_User_Flags_toDb(db, "userFlags", ue->flags);
  
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "status",
		       EBC_User_Status_toString(ue->status));
  
  if (ue->peerId)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "peerId", ue->peerId);
  if (ue->systemId)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "systemId", ue->systemId);
  
  /* save crypt token settings */
  if (ue->tokenType)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "tokenType", ue->tokenType);
  if (ue->tokenName)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "tokenName", ue->tokenName);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "tokenContextId", ue->tokenContextId);
  if (ue->protoVersion)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "protoVersion", ue->protoVersion);
  if (ue->signVersion)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "signVersion", ue->signVersion);
  if (ue->cryptVersion)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "cryptVersion", ue->cryptVersion);
  if (ue->authVersion)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "authVersion", ue->authVersion);
  
  /* save http settings */
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "httpVMajor", ue->httpVMajor);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "httpVMinor", ue->httpVMinor);
  if (ue->httpUserAgent)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "httpUserAgent", ue->httpUserAgent);
  if (ue->httpContentType)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "httpContentType", ue->httpContentType);

  /* save URL */
  if (ue->serverUrl)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "server", ue->serverUrl);
}



const char *EBC_User_GetPeerId(const AB_USER *u) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  return ue->peerId;
}



void EBC_User_SetPeerId(AB_USER *u, const char *s) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  free(ue->peerId);
  if (s)
    ue->peerId=strdup(s);
  else
    ue->peerId=NULL;
}



uint32_t EBC_User_GetTokenContextId(const AB_USER *u){
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  return ue->tokenContextId;
}



void EBC_User_SetTokenContextId(AB_USER *u, uint32_t id){
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  ue->tokenContextId=id;
}



const char *EBC_User_GetTokenType(const AB_USER *u) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  return ue->tokenType;
}



void EBC_User_SetTokenType(AB_USER *u, const char *s) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  free(ue->tokenType);
  if (s)
    ue->tokenType=strdup(s);
  else
    ue->tokenType=NULL;
}



const char *EBC_User_GetTokenName(const AB_USER *u) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  return ue->tokenName;
}



void EBC_User_SetTokenName(AB_USER *u, const char *s) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  free(ue->tokenName);
  if (s)
    ue->tokenName=strdup(s);
  else
    ue->tokenName=NULL;
}




EBC_USER_STATUS EBC_User_GetStatus(const AB_USER *u){
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  return ue->status;
}



void EBC_User_SetStatus(AB_USER *u, EBC_USER_STATUS i){
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  ue->status=i;
}



const char *EBC_User_GetServerUrl(const AB_USER *u) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  return ue->serverUrl;
}



void EBC_User_SetServerUrl(AB_USER *u, const char *s) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  free(ue->serverUrl);
  if (s) ue->serverUrl=strdup(s);
  else ue->serverUrl=NULL;
}



const char *EBC_User_GetSystemId(const AB_USER *u) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  return ue->systemId;
}



void EBC_User_SetSystemId(AB_USER *u, const char *s) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  free(ue->systemId);
  if (s)
    ue->systemId=strdup(s);
  else
    ue->systemId=NULL;
}



uint32_t EBC_User_GetFlags(const AB_USER *u) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  return ue->flags;
}



void EBC_User_SetFlags(AB_USER *u, uint32_t flags) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  ue->flags=flags;
}



void EBC_User_AddFlags(AB_USER *u, uint32_t flags) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  ue->flags|=flags;
}



void EBC_User_SubFlags(AB_USER *u, uint32_t flags) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  ue->flags&=~flags;
}



const char *EBC_User_GetHttpUserAgent(const AB_USER *u) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  return ue->httpUserAgent;
}



void EBC_User_SetHttpUserAgent(AB_USER *u, const char *s) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  free(ue->httpUserAgent);
  if (s) ue->httpUserAgent=strdup(s);
  else ue->httpUserAgent=NULL;
}



const char *EBC_User_GetHttpContentType(const AB_USER *u) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  return ue->httpContentType;
}



void EBC_User_SetHttpContentType(AB_USER *u, const char *s) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  free(ue->httpContentType);
  if (s) ue->httpContentType=strdup(s);
  else ue->httpContentType=NULL;
}



int EBC_User_GetHttpVMajor(const AB_USER *u) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  return ue->httpVMajor;
}



void EBC_User_SetHttpVMajor(AB_USER *u, int i) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  ue->httpVMajor=i;
}



int EBC_User_GetHttpVMinor(const AB_USER *u) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  return ue->httpVMinor;
}



void EBC_User_SetHttpVMinor(AB_USER *u, int i) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  ue->httpVMinor=i;
}



const char *EBC_User_GetProtoVersion(const AB_USER *u) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  return ue->protoVersion;
}



void EBC_User_SetProtoVersion(AB_USER *u, const char *s) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  free(ue->protoVersion);
  if (s) ue->protoVersion=strdup(s);
  else ue->protoVersion=NULL;
}



const char *EBC_User_GetSignVersion(const AB_USER *u) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  return ue->signVersion;
}



void EBC_User_SetSignVersion(AB_USER *u, const char *s) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  free(ue->signVersion);
  if (s) ue->signVersion=strdup(s);
  else ue->signVersion=NULL;
}



const char *EBC_User_GetCryptVersion(const AB_USER *u) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  return ue->cryptVersion;
}



void EBC_User_SetCryptVersion(AB_USER *u, const char *s) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  free(ue->cryptVersion);
  if (s) ue->cryptVersion=strdup(s);
  else ue->cryptVersion=NULL;
}



const char *EBC_User_GetAuthVersion(const AB_USER *u) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  return ue->authVersion;
}



void EBC_User_SetAuthVersion(AB_USER *u, const char *s) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  free(ue->authVersion);
  if (s) ue->authVersion=strdup(s);
  else ue->authVersion=NULL;
}



int EBC_User_MkPasswdName(const AB_USER *u, GWEN_BUFFER *buf) {
  EBC_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, EBC_USER, u);
  assert(ue);

  if (ue->tokenType==NULL) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Missing tokenType or tokenName");
    return GWEN_ERROR_NO_DATA;
  }

  if (ue->tokenName) {
    GWEN_Buffer_AppendString(buf, "PASSWORD_");
    GWEN_Buffer_AppendString(buf, ue->tokenType);
    GWEN_Buffer_AppendString(buf, "_");
    GWEN_Buffer_AppendString(buf, ue->tokenName);
    return 0;
  }
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Missing tokenName");
    return GWEN_ERROR_NO_DATA;
  }
}












