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



AB_USER *APY_User_new(AB_PROVIDER *pro)
{
  AB_USER *u;
  APY_USER *ue;

  u=AB_User_new();
  assert(u);
  GWEN_NEW_OBJECT(APY_USER, ue);
  GWEN_INHERIT_SETDATA(AB_USER, APY_USER, u, ue, APY_User_freeData);

  AB_User_SetProvider(u, pro);
  AB_User_SetBackendName(u, "aqpaypal");

  ue->readFromDbFn=AB_User_SetReadFromDbFn(u, APY_User_ReadFromDb);
  ue->writeToDbFn=AB_User_SetWriteToDbFn(u, APY_User_WriteToDb);

  AB_User_SetCountry(u, "de");

  return u;
}





void GWENHYWFAR_CB APY_User_freeData(void *bp, void *p)
{
  APY_USER *ue;

  ue=(APY_USER *)p;
  free(ue->serverUrl);
  free(ue->apiPassword);
  free(ue->apiSignature);
  free(ue->apiUserId);
  GWEN_FREE_OBJECT(ue);
}



int APY_User_ReadFromDb(AB_USER *u, GWEN_DB_NODE *db)
{
  APY_USER *ue;
  AB_PROVIDER *pro;
  GWEN_DB_NODE *dbP;
  int rv;
  const char *s;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  /* save provider, because AB_User_ReadFromDb clears it */
  pro=AB_User_GetProvider(u);

  /* read data for base class */
  rv=(ue->readFromDbFn)(u, db);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* set provider again */
  AB_User_SetProvider(u, pro);

  /* read data for provider */
  dbP=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "data/backend");

  /* get server address */
  free(ue->serverUrl);
  s=GWEN_DB_GetCharValue(dbP, "server", 0, 0);
  if (s && *s)
    ue->serverUrl=strdup(s);
  else
    ue->serverUrl=NULL;

  /* setup HTTP version */
  ue->httpVMajor=GWEN_DB_GetIntValue(dbP, "httpVMajor", 0, -1);
  ue->httpVMinor=GWEN_DB_GetIntValue(dbP, "httpVMinor", 0, -1);
  if (ue->httpVMajor==-1 || ue->httpVMinor==-1) {
    ue->httpVMajor=1;
    ue->httpVMinor=1;
  }

  return 0;
}



int APY_User_WriteToDb(const AB_USER *u, GWEN_DB_NODE *db)
{
  APY_USER *ue;
  int rv;
  GWEN_DB_NODE *dbP;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  /* write data for base class */
  rv=(ue->writeToDbFn)(u, db);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* write data for provider */
  dbP=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "data/backend");

  if (ue->serverUrl)
    GWEN_DB_SetCharValue(dbP, GWEN_DB_FLAGS_OVERWRITE_VARS, "server", ue->serverUrl);

  /* save http settings */
  GWEN_DB_SetIntValue(dbP, GWEN_DB_FLAGS_OVERWRITE_VARS, "httpVMajor", ue->httpVMajor);
  GWEN_DB_SetIntValue(dbP, GWEN_DB_FLAGS_OVERWRITE_VARS, "httpVMinor", ue->httpVMinor);

  return 0;
}



const char *APY_User_GetServerUrl(const AB_USER *u)
{
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  return ue->serverUrl;
}



void APY_User_SetServerUrl(AB_USER *u, const char *s)
{
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  free(ue->serverUrl);
  if (s)
    ue->serverUrl=strdup(s);
  else
    ue->serverUrl=NULL;
}



const char *APY_User_GetApiUserId(const AB_USER *u)
{
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  return ue->apiUserId;
}



const char *APY_User_GetApiPassword(const AB_USER *u)
{
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  return ue->apiPassword;
}



const char *APY_User_GetApiSignature(const AB_USER *u)
{
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  return ue->apiSignature;
}



void APY_User_SetApiSecrets_l(AB_USER *u, const char *password, const char *signature, const char *userid)
{
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  free(ue->apiPassword);
  if (password && *password)
    ue->apiPassword=strdup(password);
  else
    ue->apiPassword=NULL;

  free(ue->apiSignature);
  if (signature && *signature)
    ue->apiSignature=strdup(signature);
  else
    ue->apiSignature=NULL;

  free(ue->apiUserId);
  if (userid && *userid)
    ue->apiUserId=strdup(userid);
  else
    ue->apiUserId=NULL;
}



int APY_User_SetApiSecrets(AB_USER *u, const char *password, const char *signature, const char *userid)
{
  GWEN_BUFFER *tbuf;
  int rv;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Text_EscapeToBuffer(password, tbuf);
  GWEN_Buffer_AppendByte(tbuf, ':');
  GWEN_Text_EscapeToBuffer(signature, tbuf);
  GWEN_Buffer_AppendByte(tbuf, ':');
  GWEN_Text_EscapeToBuffer(userid, tbuf);
  rv=APY_Provider_WriteUserApiSecrets(AB_User_GetProvider(u), u, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return 0;
}



int APY_User_GetHttpVMajor(const AB_USER *u)
{
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  return ue->httpVMajor;
}



void APY_User_SetHttpVMajor(const AB_USER *u, int i)
{
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  ue->httpVMajor=i;
}



int APY_User_GetHttpVMinor(const AB_USER *u)
{
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  return ue->httpVMinor;
}



void APY_User_SetHttpVMinor(const AB_USER *u, int i)
{
  APY_USER *ue;

  assert(u);
  ue=GWEN_INHERIT_GETDATA(AB_USER, APY_USER, u);
  assert(ue);

  ue->httpVMinor=i;
}






