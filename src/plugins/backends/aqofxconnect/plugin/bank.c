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

#include "bank_p.h"


#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_LIST_FUNCTIONS(AO_BANK, AO_Bank)


AO_BANK *AO_Bank_new(AB_PROVIDER *pro,
                     const char *country, const char *bankId){
  AO_BANK *b;

  assert(pro);
  assert(country);
  assert(bankId);
  GWEN_NEW_OBJECT(AO_BANK, b);
  GWEN_LIST_INIT(AO_BANK, b);
  b->provider=pro;
  b->country=strdup(country);
  b->bankId=strdup(bankId);
  b->accounts=AB_Account_List_new();
  b->users=AO_User_List_new();

  return b;
}



void AO_Bank_free(AO_BANK *b){
  if (b) {
    GWEN_LIST_FINI(AO_BANK, b);
    AO_User_List_free(b->users);
    AB_Account_List_free(b->accounts);
    free(b->httpHost);
    free(b->serverAddr);
    free(b->fid);
    free(b->org);
    free(b->brokerId);
    free(b->bankName);
    free(b->bankId);
    free(b->country);
    GWEN_FREE_OBJECT(b);
  }
}



AB_PROVIDER *AO_Bank_GetProvider(const AO_BANK *b) {
  assert(b);
  return b->provider;
}



const char *AO_Bank_GetCountry(const AO_BANK *b){
  assert(b);
  return b->country;
}



const char *AO_Bank_GetBankId(const AO_BANK *b){
  assert(b);
  return b->bankId;
}



const char *AO_Bank_GetBankName(const AO_BANK *b){
  assert(b);
  return b->bankName;
}



void AO_Bank_SetBankName(AO_BANK *b, const char *s){
  assert(b);
  free(b->bankName);
  if (s) b->bankName=strdup(s);
  else b->bankName=0;
}



const char *AO_Bank_GetServerAddr(const AO_BANK *b){
  assert(b);
  return b->serverAddr;
}



void AO_Bank_SetServerAddr(AO_BANK *b, const char *s){
  assert(b);
  free(b->serverAddr);
  if (s) b->serverAddr=strdup(s);
  else b->serverAddr=0;
}



int AO_Bank_GetServerPort(const AO_BANK *b){
  assert(b);
  return b->serverPort;
}



void AO_Bank_SetServerPort(AO_BANK *b, int i){
  assert(b);
  b->serverPort=i;
}



AO_BANK *AO_Bank_fromDb(AB_PROVIDER *pro, GWEN_DB_NODE *db){
  AO_BANK *b;
  const char *s;
  GWEN_DB_NODE *dbT;

  s=GWEN_DB_GetCharValue(db, "bankId", 0, 0);
  assert(s);
  b=AO_Bank_new(pro, GWEN_DB_GetCharValue(db, "country", 0, "us"), s);
  AO_Bank_SetBankName(b, GWEN_DB_GetCharValue(db, "bankName", 0, 0));

  s=GWEN_DB_GetCharValue(db, "serverType", 0, 0);
  if (s) {
    if (strcasecmp(s, "http")==0)
      b->serverType=AO_Bank_ServerTypeHTTP;
    else if (strcasecmp(s, "https")==0)
      b->serverType=AO_Bank_ServerTypeHTTPS;
    else {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Unknown server type \"%s\"", s);
      abort();
    }
  }
  else
    b->serverType=AO_Bank_ServerTypeUnknown;
  AO_Bank_SetServerAddr(b, GWEN_DB_GetCharValue(db, "serverAddr", 0, 0));
  b->serverPort=GWEN_DB_GetIntValue(db, "serverPort", 0, 443);
  if (b->serverType==AO_Bank_ServerTypeUnknown) {
    if (b->serverPort==80)
      b->serverType=AO_Bank_ServerTypeHTTP;
    else
      b->serverType=AO_Bank_ServerTypeHTTPS;
  }

  s=GWEN_DB_GetCharValue(db, "brokerId", 0, 0);
  if (s)
    b->brokerId=strdup(s);

  s=GWEN_DB_GetCharValue(db, "fid", 0, 0);
  if (s)
    b->fid=strdup(s);

  s=GWEN_DB_GetCharValue(db, "org", 0, 0);
  if (s)
    b->org=strdup(s);

  s=GWEN_DB_GetCharValue(db, "httpHost", 0, 0);
  if (s)
    b->httpHost=strdup(s);
  b->httpVMajor=GWEN_DB_GetIntValue(db, "httpVMajor", 0, 1);
  b->httpVMinor=GWEN_DB_GetIntValue(db, "httpVMinor", 0, 0);

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "users");
  if (dbT) {
    dbT=GWEN_DB_FindFirstGroup(dbT, "user");
    while(dbT) {
      AO_USER *u;

      u=AO_User_fromDb(b, dbT);
      assert(u);
      AO_User_List_Add(u, b->users);
      dbT=GWEN_DB_FindNextGroup(dbT, "user");
    }
  }

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "accounts");
  if (dbT) {
    dbT=GWEN_DB_FindFirstGroup(dbT, "account");
    while(dbT) {
      AB_ACCOUNT *a;

      a=AO_Account_fromDb(AB_Provider_GetBanking(b->provider), dbT);
      assert(a);
      AB_Account_List_Add(a, b->accounts);
      dbT=GWEN_DB_FindNextGroup(dbT, "account");
    }
  }

  return b;
}



int AO_Bank_toDb(const AO_BANK *b, GWEN_DB_NODE *db) {
  assert(b);
  assert(db);
  if (b->country)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "country", b->country);
  if (b->bankId)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "bankId", b->bankId);
  if (b->bankName)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "bankName", b->bankName);

  if (b->brokerId)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "brokerId", b->brokerId);

  if (b->org)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "org", b->org);

  if (b->fid)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "fid", b->fid);

  switch(b->serverType) {
  case AO_Bank_ServerTypeHTTP:
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "serverType", "http");
    break;
  case AO_Bank_ServerTypeHTTPS:
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "serverType", "https");
    break;
  default:
    break;
  }

  if (b->serverAddr)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "serverAddr", b->serverAddr);
  if (b->serverPort!=0)
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "serverPort", b->serverPort);
  if (b->httpHost)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "httpHost", b->httpHost);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "httpVMajor", b->httpVMajor);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "httpVMinor", b->httpVMinor);

  if (AO_User_List_GetCount(b->users)) {
    AO_USER *u;
    GWEN_DB_NODE *dbG;

    dbG=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "users");
    assert(dbG);
    u=AO_User_List_First(b->users);
    assert(u);
    while(u) {
      GWEN_DB_NODE *dbT;

      dbT=GWEN_DB_GetGroup(dbG, GWEN_PATH_FLAGS_CREATE_GROUP, "user");
      if (AO_User_toDb(u, dbT)) {
        DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
                  "Could not store user \"%s\"",
                  AO_User_GetUserId(u));
        abort();
      }
      u=AO_User_List_Next(u);
    }
  }

  if (AB_Account_List_GetCount(b->accounts)) {
    AB_ACCOUNT *a;
    GWEN_DB_NODE *dbG;

    dbG=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "accounts");
    assert(dbG);
    a=AB_Account_List_First(b->accounts);
    assert(a);
    while(a) {
      GWEN_DB_NODE *dbT;

      dbT=GWEN_DB_GetGroup(dbG, GWEN_PATH_FLAGS_CREATE_GROUP, "account");
      if (AO_Account_toDb(a, dbT)) {
        DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
                  "Could not store account \"%s\"",
                  AB_Account_GetAccountNumber(a));
        abort();
      }
      a=AB_Account_List_Next(a);
    }
  }

  return 0;
}



AB_ACCOUNT *AO_Bank_FindAccount(AO_BANK *b, const char *id) {
  AB_ACCOUNT *a;

  assert(b);
  assert(id);
  a=AB_Account_List_First(b->accounts);
  while(a) {
    const char *s;

    s=AB_Account_GetAccountNumber(a);
    if (s && -1!=GWEN_Text_ComparePattern(s, id, 0))
      break;
    a=AB_Account_List_Next(a);
  }

  return a;
}



int AO_Bank_AddAccount(AO_BANK *b, AB_ACCOUNT *a) {
  assert(b);
  assert(a);
  if (AO_Bank_FindAccount(b, AB_Account_GetAccountNumber(a))) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
             "Account \"%s\" already exists",
             AB_Account_GetAccountNumber(a));
    return -1;
  }
  AB_Account_List_Add(a, b->accounts);
  return 0;
}



AO_USER *AO_Bank_FindUser(AO_BANK *b, const char *id) {
  AO_USER *u;

  assert(b);
  assert(id);
  u=AO_User_List_First(b->users);
  while(u) {
    const char *s;

    s=AO_User_GetUserId(u);
    if (s && -1!=GWEN_Text_ComparePattern(s, id, 0))
      break;
    u=AO_User_List_Next(u);
  }

  return u;
}



int AO_Bank_AddUser(AO_BANK *b, AO_USER *u) {
  assert(b);
  assert(u);
  if (AO_Bank_FindUser(b, AO_User_GetUserId(u))) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
             "User \"%s\" already exists",
             AO_User_GetUserId(u));
    return -1;
  }
  AO_User_List_Add(u, b->users);
  return 0;
}



AB_ACCOUNT_LIST *AO_Bank_GetAccounts(const AO_BANK *b){
  assert(b);
  return b->accounts;
}



AO_BANK_SERVERTYPE AO_Bank_GetServerType(const AO_BANK *b){
  assert(b);
  return b->serverType;
}



void AO_Bank_SetServerType(AO_BANK *b, AO_BANK_SERVERTYPE t){
  assert(b);
  b->serverType=t;
}



const char *AO_Bank_GetBrokerId(const AO_BANK *b){
  assert(b);
  return b->brokerId;
}



void AO_Bank_SetBrokerId(AO_BANK *b, const char *s){
  assert(b);
  free(b->brokerId);
  if (s) b->brokerId=strdup(s);
  else b->brokerId=0;
}



const char *AO_Bank_GetOrg(const AO_BANK *b){
  assert(b);
  return b->org;
}



void AO_Bank_SetOrg(AO_BANK *b, const char *s){
  assert(b);
  free(b->org);
  if (s) b->org=strdup(s);
  else b->org=0;
}



const char *AO_Bank_GetFid(const AO_BANK *b){
  assert(b);
  return b->fid;
}



void AO_Bank_SetFid(AO_BANK *b, const char *s){
  assert(b);
  free(b->fid);
  if (s) b->fid=strdup(s);
  else b->fid=0;
}



const char *AO_Bank_GetHttpHost(const AO_BANK *b){
  assert(b);
  return b->httpHost;
}



void AO_Bank_SetHttpHost(AO_BANK *b, const char *s){
  assert(b);
  free(b->httpHost);
  if (s) b->httpHost=strdup(s);
  else b->httpHost=0;
}



int AO_Bank_GetHttpVMajor(const AO_BANK *b){
  assert(b);
  return b->httpVMajor;
}



void AO_Bank_SetHttpVMajor(AO_BANK *b, int i){
  assert(b);
  b->httpVMajor=i;
}



int AO_Bank_GetHttpVMinor(const AO_BANK *b){
  assert(b);
  return b->httpVMinor;
}



void AO_Bank_SetHttpVMinor(AO_BANK *b, int i){
  assert(b);
  b->httpVMinor=i;
}



AO_USER_LIST *AO_Bank_GetUsers(const AO_BANK *b){
  assert(b);
  return b->users;
}

















