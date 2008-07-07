/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "account_p.h"
#include "banking_l.h"
#include "provider_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT_FUNCTIONS(AB_ACCOUNT)
GWEN_LIST_FUNCTIONS(AB_ACCOUNT, AB_Account)
GWEN_LIST2_FUNCTIONS(AB_ACCOUNT, AB_Account)



AB_ACCOUNT *AB_Account_new(AB_BANKING *ab,
                           AB_PROVIDER *pro){
  AB_ACCOUNT *a;

  assert(ab);
  assert(pro);
  GWEN_NEW_OBJECT(AB_ACCOUNT, a);
  a->usage=1;
  GWEN_INHERIT_INIT(AB_ACCOUNT, a);
  GWEN_LIST_INIT(AB_ACCOUNT, a);
  a->banking=ab;
  a->provider=pro;
  a->backendName=strdup(AB_Provider_GetName(pro));
  a->data=GWEN_DB_Group_new("Data");

  return a;
}



AB_ACCOUNT *AB_Account_fromDb(AB_BANKING *ab, GWEN_DB_NODE *db){
  AB_ACCOUNT *a;
  GWEN_DB_NODE *dbT;
  const char *pname;
  AB_PROVIDER *pro;
  const char *s;
  int i;

  assert(ab);
  pname=GWEN_DB_GetCharValue(db, "provider", 0, 0);
  assert(pname);
  pro=AB_Banking_GetProvider(ab, pname);
  if (!pro) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Provider \"%s\" is not available, ignoring account",
              pname);
    return 0;
  }

  GWEN_NEW_OBJECT(AB_ACCOUNT, a);
  a->usage=1;
  GWEN_INHERIT_INIT(AB_ACCOUNT, a);
  GWEN_LIST_INIT(AB_ACCOUNT, a);
  a->banking=ab;
  a->provider=pro;
  a->backendName=strdup(pname);
  a->data=GWEN_DB_Group_new("Data");
  dbT=GWEN_DB_GetGroup(a->data, GWEN_DB_FLAGS_DEFAULT, "static");
  assert(dbT);
  GWEN_DB_AddGroupChildren(dbT, db);

  /* preset */
  if (AB_Account_GetCountry(a)==0)
    AB_Account_SetCountry(a, "de");

  /* mark DB not-dirty */
  GWEN_DB_ModifyBranchFlagsDown(a->data, 0, GWEN_DB_NODE_FLAGS_DIRTY);

  i=GWEN_DB_GetIntValue(db, "accountType", 0, 1);
  a->accountType=i;

  a->uniqueId=GWEN_DB_GetIntValue(db, "uniqueId", 0, 1);

  free(a->accountNumber);
  s=GWEN_DB_GetCharValue(db, "accountNumber", 0, NULL);
  if (s)
    a->accountNumber=strdup(s);
  else
    a->accountNumber=NULL;

  free(a->bankCode);
  s=GWEN_DB_GetCharValue(db, "bankCode", 0, NULL);
  if (s)
    a->bankCode=strdup(s);
  else
    a->bankCode=NULL;

  free(a->accountName);
  s=GWEN_DB_GetCharValue(db, "accountName", 0, NULL);
  if (s)
    a->accountName=strdup(s);
  else
    a->accountName=NULL;

  free(a->bankName);
  s=GWEN_DB_GetCharValue(db, "bankName", 0, NULL);
  if (s)
    a->bankName=strdup(s);
  else
    a->bankName=NULL;

  free(a->iban);
  s=GWEN_DB_GetCharValue(db, "iban", 0, NULL);
  if (s)
    a->iban=strdup(s);
  else
    a->iban=NULL;

  free(a->bic);
  s=GWEN_DB_GetCharValue(db, "bic", 0, NULL);
  if (s)
    a->bic=strdup(s);
  else
    a->bic=NULL;

  free(a->ownerName);
  s=GWEN_DB_GetCharValue(db, "ownerName", 0, NULL);
  if (s)
    a->ownerName=strdup(s);
  else
    a->ownerName=NULL;

  free(a->currency);
  s=GWEN_DB_GetCharValue(db, "currency", 0, NULL);
  if (s)
    a->currency=strdup(s);
  else
    a->currency=NULL;

  free(a->country);
  s=GWEN_DB_GetCharValue(db, "country", 0, NULL);
  if (s)
    a->country=strdup(s);
  else
    a->country=NULL;

  GWEN_StringList_Clear(a->userIds);
  for (i=0; i<100; i++) {
    s=GWEN_DB_GetCharValue(db, "user", i, NULL);
    if (!s)
      break;
    GWEN_StringList_AppendString(a->userIds, s, 0, 1);
  }

  free(a->selectedUser);
  s=GWEN_DB_GetCharValue(db, "selectedUser", 0, NULL);
  if (s)
    a->selectedUser=strdup(s);
  else
    a->selectedUser=NULL;

  GWEN_DB_Group_free(a->appData);
  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "apps");
  if (dbT)
    a->appData=GWEN_DB_Group_dup(dbT);
  else
    a->appData=GWEN_DB_Group_new("apps");

  GWEN_DB_Group_free(a->providerData);
  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "provider");
  if (dbT)
    a->providerData=GWEN_DB_Group_dup(dbT);
  else
    a->providerData=GWEN_DB_Group_new("provider");

  return a;
}



const char *AB_Account_GetBackendName(const AB_ACCOUNT *a) {
  assert(a);
  assert(a->usage);
  return a->backendName;
}



AB_USER_LIST2 *AB_Account_GetUsers(const AB_ACCOUNT *a) {
  AB_USER_LIST2 *ul;
  int i;

  assert(a);
  assert(a->usage);

  ul=AB_User_List2_new();
  for (i=0; ; i++) {
    uint32_t id;
    AB_USER *u;

    id=GWEN_DB_GetIntValue(a->data, "static/user", i, 0);
    if (id==0)
      break;
    u=AB_Banking_GetUser(a->banking, id);
    if (u) {
      AB_User_List2_PushBack(ul, u);
    }
    else {
      DBG_WARN(AQBANKING_LOGDOMAIN, "User with id \"%08x\" not found", id);
    }
  }

  if (AB_User_List2_GetSize(ul)==0) {
    AB_User_List2_free(ul);
    return 0;
  }

  return ul;
}



AB_USER *AB_Account_GetFirstUser(const AB_ACCOUNT *a) {
  uint32_t id;
  AB_USER *u;

  assert(a);
  assert(a->usage);

  id=GWEN_DB_GetIntValue(a->data, "static/user", 0, 0);
  if (id==0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No user");
    return 0;
  }
  u=AB_Banking_GetUser(a->banking, id);
  if (u)
    return u;
  DBG_WARN(AQBANKING_LOGDOMAIN, "User with id \"%08x\" not found", id);
  return 0;
}



void AB_Account_SetUsers(AB_ACCOUNT *a, const AB_USER_LIST2 *ul) {
  assert(a);
  assert(a->usage);

  GWEN_DB_DeleteVar(a->data, "static/user");
  if (ul) {
    AB_USER_LIST2_ITERATOR *it;

    it=AB_User_List2_First(ul);
    if (it) {
      AB_USER *u;

      u=AB_User_List2Iterator_Data(it);
      while(u) {
        GWEN_DB_SetIntValue(a->data, GWEN_DB_FLAGS_DEFAULT,
                            "static/user", AB_User_GetUniqueId(u));
        u=AB_User_List2Iterator_Next(it);
      }
      AB_User_List2Iterator_free(it);
    }
  }
}



void AB_Account_SetUser(AB_ACCOUNT *a, const AB_USER *u) {
  assert(a);
  assert(a->usage);

  GWEN_DB_DeleteVar(a->data, "static/user");
  if (u)
    GWEN_DB_SetIntValue(a->data, GWEN_DB_FLAGS_DEFAULT,
                        "static/user", AB_User_GetUniqueId(u));
}



AB_USER_LIST2 *AB_Account_GetSelectedUsers(const AB_ACCOUNT *a) {
  AB_USER_LIST2 *ul;
  int i;

  assert(a);
  assert(a->usage);

  ul=AB_User_List2_new();
  for (i=0; ; i++) {
    uint32_t id;
    AB_USER *u;

    id=GWEN_DB_GetIntValue(a->data, "static/selectedUser", i, 0);
    if (id==0)
      break;
    u=AB_Banking_GetUser(a->banking, id);
    if (u) {
      AB_User_List2_PushBack(ul, u);
    }
    else {
      DBG_WARN(AQBANKING_LOGDOMAIN,
               "User with id \"%08x\" not found", id);
    }
  }

  if (AB_User_List2_GetSize(ul)==0) {
    AB_User_List2_free(ul);
    return 0;
  }

  return ul;
}



AB_USER *AB_Account_GetFirstSelectedUser(const AB_ACCOUNT *a) {
  uint32_t id;
  AB_USER *u;

  assert(a);
  assert(a->usage);

  id=GWEN_DB_GetIntValue(a->data, "static/selectedUser", 0, 0);
  if (id==0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No user");
    return 0;
  }
  u=AB_Banking_GetUser(a->banking, id);
  if (u)
    return u;
  DBG_WARN(AQBANKING_LOGDOMAIN, "User with id \"%08x\" not found", id);
  return 0;
}



void AB_Account_SetSelectedUsers(AB_ACCOUNT *a, const AB_USER_LIST2 *ul) {
  assert(a);
  assert(a->usage);

  GWEN_DB_DeleteVar(a->data, "static/selectedUser");
  if (ul) {
    AB_USER_LIST2_ITERATOR *it;

    it=AB_User_List2_First(ul);
    if (it) {
      AB_USER *u;

      u=AB_User_List2Iterator_Data(it);
      while(u) {
        GWEN_DB_SetIntValue(a->data, GWEN_DB_FLAGS_DEFAULT,
                            "static/selectedUser", AB_User_GetUniqueId(u));
        u=AB_User_List2Iterator_Next(it);
      }
      AB_User_List2Iterator_free(it);
    }
  }
}



void AB_Account_SetSelectedUser(AB_ACCOUNT *a, const AB_USER *u) {
  assert(a);
  assert(a->usage);

  GWEN_DB_DeleteVar(a->data, "static/selectedUser");
  if (u)
    GWEN_DB_SetIntValue(a->data, GWEN_DB_FLAGS_DEFAULT,
                        "static/selectedUser", AB_User_GetUniqueId(u));
}



int AB_Account_toDb(const AB_ACCOUNT *a, GWEN_DB_NODE *db){
  GWEN_DB_NODE *dbT;

  assert(a);
  assert(a->usage);
  dbT=GWEN_DB_GetGroup(a->data, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "static");
  if (dbT)
    GWEN_DB_AddGroupChildren(db, dbT);
  if (a->provider)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "provider",
                         AB_Provider_GetName(a->provider));
  return 0;
}



void AB_Account_free(AB_ACCOUNT *a){
  if (a) {
    assert(a->usage);
    if (a->usage==1) {
      DBG_VERBOUS(AQBANKING_LOGDOMAIN, "Destroying AB_ACCOUNT");
      GWEN_LIST_FINI(AB_ACCOUNT, a);
      GWEN_INHERIT_FINI(AB_ACCOUNT, a);
      free(a->backendName);
      GWEN_DB_Group_free(a->data);

      free(a->accountNumber);
      free(a->bankCode);
      free(a->accountName);
      free(a->bankName);
      free(a->iban);
      free(a->bic);
      free(a->ownerName);
      free(a->currency);
      free(a->country);
      GWEN_StringList_free(a->userIds);
      free(a->selectedUser);

      GWEN_DB_Group_free(a->appData);
      GWEN_DB_Group_free(a->providerData);

      a->usage=0;
      GWEN_FREE_OBJECT(a);
    }
    else
      a->usage--;
  }
}



void AB_Account_Attach(AB_ACCOUNT *a){
  assert(a);
  a->usage++;
}



uint32_t AB_Account_GetUniqueId(const AB_ACCOUNT *a){
  assert(a);
  assert(a->usage);
  return GWEN_DB_GetIntValue(a->data, "static/uniqueId", 0, 0);
}



void AB_Account_SetUniqueId(AB_ACCOUNT *a, uint32_t id){
  assert(a);
  assert(a->usage);
  GWEN_DB_SetIntValue(a->data, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "static/uniqueId", id);
}



AB_ACCOUNT_TYPE AB_Account_GetAccountType(const AB_ACCOUNT *a){
  assert(a);
  assert(a->usage);
  return GWEN_DB_GetIntValue(a->data, "static/accountType", 0, 0);
}



void AB_Account_SetAccountType(AB_ACCOUNT *a, AB_ACCOUNT_TYPE t){
  assert(a);
  assert(a->usage);
  GWEN_DB_SetIntValue(a->data, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "static/accountType", t);
}



AB_PROVIDER *AB_Account_GetProvider(const AB_ACCOUNT *a) {
  assert(a);
  assert(a->usage);
  return a->provider;
}



GWEN_DB_NODE *AB_Account_GetAppData(const AB_ACCOUNT *a){
  GWEN_DB_NODE *n;
  const char *appName;

  assert(a);
  assert(a->usage);
  appName=AB_Banking_GetEscapedAppName(a->banking);
  assert(appName);
  n=GWEN_DB_GetGroup(a->data, GWEN_DB_FLAGS_DEFAULT, "static/apps");
  assert(n);
  n=GWEN_DB_GetGroup(n, GWEN_DB_FLAGS_DEFAULT, appName);
  return n;
}



GWEN_DB_NODE *AB_Account_GetProviderData(const AB_ACCOUNT *a){
  GWEN_DB_NODE *n;

  assert(a);
  assert(a->usage);
  n=GWEN_DB_GetGroup(a->data, GWEN_DB_FLAGS_DEFAULT, "static/provider");
  return n;
}



const char *AB_Account_GetAccountNumber(const AB_ACCOUNT *a){
  assert(a);
  assert(a->usage);
  return GWEN_DB_GetCharValue(a->data, "static/accountNumber", 0, 0);
}



void AB_Account_SetAccountNumber(AB_ACCOUNT *a, const char *s){
  assert(a);
  assert(a->usage);
  if (!s)
    GWEN_DB_DeleteVar(a->data, "static/accountNumber");
  else
    GWEN_DB_SetCharValue(a->data, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "static/accountNumber", s);
}



const char *AB_Account_GetBankCode(const AB_ACCOUNT *a){
  assert(a);
  assert(a->usage);
  return GWEN_DB_GetCharValue(a->data, "static/bankCode", 0, 0);
}



void AB_Account_SetBankCode(AB_ACCOUNT *a, const char *s){
  assert(a);
  assert(a->usage);
  if (!s)
    GWEN_DB_DeleteVar(a->data, "static/bankCode");
  else
    GWEN_DB_SetCharValue(a->data, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "static/bankCode", s);
}



const char *AB_Account_GetAccountName(const AB_ACCOUNT *a){
  assert(a);
  assert(a->usage);
  return GWEN_DB_GetCharValue(a->data, "static/accountName", 0, 0);
}



void AB_Account_SetAccountName(AB_ACCOUNT *a, const char *s){
  assert(a);
  assert(a->usage);
  if (!s)
    GWEN_DB_DeleteVar(a->data, "static/accountName");
  else
    GWEN_DB_SetCharValue(a->data, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "static/accountName", s);
}



const char *AB_Account_GetBankName(const AB_ACCOUNT *a){
  assert(a);
  assert(a->usage);
  return GWEN_DB_GetCharValue(a->data, "static/bankName", 0, 0);
}



void AB_Account_SetBankName(AB_ACCOUNT *a, const char *s){
  assert(a);
  assert(a->usage);
  if (!s)
    GWEN_DB_DeleteVar(a->data, "static/bankName");
  else
    GWEN_DB_SetCharValue(a->data, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "static/bankName", s);
}



const char *AB_Account_GetIBAN(const AB_ACCOUNT *a){
  assert(a);
  assert(a->usage);
  return GWEN_DB_GetCharValue(a->data, "static/iban", 0, 0);
}



void AB_Account_SetIBAN(AB_ACCOUNT *a, const char *s){
  assert(a);
  assert(a->usage);
  if (!s)
    GWEN_DB_DeleteVar(a->data, "static/iban");
  else
    GWEN_DB_SetCharValue(a->data, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "static/iban", s);
}



const char *AB_Account_GetBIC(const AB_ACCOUNT *a){
  assert(a);
  assert(a->usage);
  return GWEN_DB_GetCharValue(a->data, "static/bic", 0, 0);
}



void AB_Account_SetBIC(AB_ACCOUNT *a, const char *s){
  assert(a);
  assert(a->usage);
  if (!s)
    GWEN_DB_DeleteVar(a->data, "static/bic");
  else
    GWEN_DB_SetCharValue(a->data, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "static/bic", s);
}



const char *AB_Account_GetOwnerName(const AB_ACCOUNT *a){
  assert(a);
  assert(a->usage);
  return GWEN_DB_GetCharValue(a->data, "static/ownerName", 0, 0);
}



void AB_Account_SetOwnerName(AB_ACCOUNT *a, const char *s){
  assert(a);
  assert(a->usage);
  if (!s)
    GWEN_DB_DeleteVar(a->data, "static/ownerName");
  else
    GWEN_DB_SetCharValue(a->data, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "static/ownerName", s);
}



AB_BANKING *AB_Account_GetBanking(const AB_ACCOUNT *a){
  assert(a);
  assert(a->usage);
  return a->banking;
}



const char *AB_Account_GetCurrency(const AB_ACCOUNT *a){
  assert(a);
  assert(a->usage);
  return GWEN_DB_GetCharValue(a->data, "static/currency", 0, 0);
}




void AB_Account_SetCurrency(AB_ACCOUNT *a, const char *s){
  assert(a);
  assert(a->usage);
  assert(s);
  GWEN_DB_SetCharValue(a->data, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "static/currency", s);
}



const char *AB_Account_GetCountry(const AB_ACCOUNT *a){
  assert(a);
  assert(a->usage);
  return GWEN_DB_GetCharValue(a->data, "static/country", 0, 0);
}



void AB_Account_SetCountry(AB_ACCOUNT *a, const char *s){
  assert(a);
  assert(a->usage);
  assert(s);
  GWEN_DB_SetCharValue(a->data, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "static/country", s);
}




void AB_Account_SetProvider(AB_ACCOUNT *a, AB_PROVIDER *pro){
  assert(a);
  assert(a->usage);
  assert(pro);
  a->provider=pro;
}



AB_ACCOUNT *AB_Account_dup(AB_ACCOUNT *acc){
  AB_ACCOUNT *a;

  assert(acc);
  assert(acc->usage);
  GWEN_NEW_OBJECT(AB_ACCOUNT, a);
  a->usage=1;
  GWEN_INHERIT_INIT(AB_ACCOUNT, a);
  GWEN_LIST_INIT(AB_ACCOUNT, a);
  a->banking=acc->banking;
  a->provider=acc->provider;
  if (acc->backendName)
    a->backendName=strdup(acc->backendName);
  a->data=GWEN_DB_Group_dup(acc->data);
  return a;
}



AB_ACCOUNT *AB_Account__freeAll_cb(AB_ACCOUNT *a, void *userData) {
  AB_Account_free(a);
  return 0;
}



void AB_Account_List2_FreeAll(AB_ACCOUNT_LIST2 *al){
  AB_Account_List2_ForEach(al, AB_Account__freeAll_cb, 0);
  AB_Account_List2_free(al);
}













