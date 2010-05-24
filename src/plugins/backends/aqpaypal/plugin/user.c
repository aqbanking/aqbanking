/***************************************************************************
    begin       : Sat May 08 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "user_p.h"
#include "provider_l.h"

#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT(AB_USER, APY_USER)



void APY_User_Extend(AB_USER *u, AB_PROVIDER *pro,
		     AB_PROVIDER_EXTEND_MODE em,
		     GWEN_DB_NODE *db) {
  if (em==AB_ProviderExtendMode_Create ||
      em==AB_ProviderExtendMode_Extend) {
    APY_USER *ue;
    const char *s;

    GWEN_NEW_OBJECT(APY_USER, ue);
    GWEN_INHERIT_SETDATA(AB_USER, APY_USER, u, ue, APY_User_freeData);

    if (em==AB_ProviderExtendMode_Create) {
      s=AB_User_GetCountry(u);
      if (!s || !*s)
	AB_User_SetCountry(u, "de");
    }
    else {
      APY_User_ReadDb(u, db);
    }
  }
  else {
    if (em==AB_ProviderExtendMode_Add) {
    }
    else if (em==AB_ProviderExtendMode_Save)
      APY_User_toDb(u, db);
  }
}



void GWENHYWFAR_CB APY_User_freeData(void *bp, void *p) {
  APY_USER *ue;

  ue=(APY_USER*)p;
  free(ue->serverUrl);
  free(ue->apiPassword);
  free(ue->apiSignature);
  GWEN_FREE_OBJECT(ue);
}



void APY_User_ReadDb(AB_USER *u, GWEN_DB_NODE *db) {
  APY_USER *ue;
  const char *s;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  /* get server address */
  free(ue->serverUrl);
  s=GWEN_DB_GetCharValue(db, "server", 0, 0);
  if (s && *s) ue->serverUrl=strdup(s);
  else ue->serverUrl=NULL;

  /* setup HTTP version */
  ue->httpVMajor=GWEN_DB_GetIntValue(db, "httpVMajor", 0, -1);
  ue->httpVMinor=GWEN_DB_GetIntValue(db, "httpVMinor", 0, -1);
  if (ue->httpVMajor==-1 || ue->httpVMinor==-1) {
    ue->httpVMajor=1;
    ue->httpVMinor=1;
  }
}



void APY_User_toDb(AB_USER *u, GWEN_DB_NODE *db) {
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  if (ue->serverUrl)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "server", ue->serverUrl);

  /* save http settings */
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "httpVMajor", ue->httpVMajor);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "httpVMinor", ue->httpVMinor);
}



const char *APY_User_GetServerUrl(const AB_USER *u) {
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  return ue->serverUrl;
}



void APY_User_SetServerUrl(AB_USER *u, const char *s) {
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  free(ue->serverUrl);
  if (s) ue->serverUrl=strdup(s);
  else ue->serverUrl=NULL;
}



const char *APY_User_GetApiPassword(const AB_USER *u) {
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  return ue->apiPassword;
}



const char *APY_User_GetApiSignature(const AB_USER *u) {
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  return ue->apiSignature;
}



void APY_User_SetApiSecrets_l(AB_USER *u, const char *password, const char *signature) {
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  free(ue->apiPassword);
  if (password) ue->apiPassword=strdup(password);
  else ue->apiPassword=NULL;

  free(ue->apiSignature);
  if (signature) ue->apiSignature=strdup(signature);
  else ue->apiSignature=NULL;
}



int APY_User_SetApiSecrets(AB_USER *u, const char *password, const char *signature) {
  GWEN_BUFFER *tbuf;
  int rv;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Text_EscapeToBuffer(password, tbuf);
  GWEN_Buffer_AppendByte(tbuf, ':');
  GWEN_Text_EscapeToBuffer(signature, tbuf);
  rv=APY_Provider_WriteUserApiSecrets(AB_User_GetProvider(u), u, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return 0;
}



int APY_User_GetHttpVMajor(const AB_USER *u) {
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  return ue->httpVMajor;
}



void APY_User_SetHttpVMajor(const AB_USER *u, int i) {
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  ue->httpVMajor=i;
}



int APY_User_GetHttpVMinor(const AB_USER *u) {
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  return ue->httpVMinor;
}



void APY_User_SetHttpVMinor(const AB_USER *u, int i) {
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  ue->httpVMinor=i;
}






