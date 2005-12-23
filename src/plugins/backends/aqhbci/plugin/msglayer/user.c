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
#include "aqhbci_l.h"
#include "customer_l.h"
#include "hbci_l.h"
#include "customer_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/misc2.h>
#include <gwenhywfar/text.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_LIST_FUNCTIONS(AH_USER, AH_User);
GWEN_LIST2_FUNCTIONS(AH_USER, AH_User);


AH_USER *AH_User_new(AH_BANK *b,
                     const char *userId,
                     AH_CRYPT_MODE cm,
                     AH_MEDIUM *m) {
  AH_USER *u;

  assert(b);
  assert(userId);
  assert(m);
  assert(!AH_HBCI_CheckStringSanity(userId));
  GWEN_NEW_OBJECT(AH_USER, u);
  u->usage=1;
  GWEN_LIST_INIT(AH_USER, u);
  u->bank=b;
  u->userId=strdup(userId);
  u->medium=m;
  AH_Medium_Attach(m);
  u->cryptMode=cm;
  u->customers=AH_Customer_List_new();

  u->contextIdx=-1;
  return u;
}



void AH_User_Attach(AH_USER *u){
  assert(u);
  u->usage++;
}



void AH_User_free(AH_USER *u){
  if (u) {
    assert(u->usage);
    if (--(u->usage)==0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Destroying AH_USER");
      GWEN_LIST_FINI(AH_USER, u);
      AH_Customer_List_free(u->customers);
      free(u->userId);
      AH_Medium_free(u->medium);
      AH_BpdAddr_free(u->serverAddress);
      GWEN_FREE_OBJECT(u);
    }
  }
}



AH_BANK *AH_User_GetBank(const AH_USER *u){
  assert(u);
  return u->bank;
}



const char *AH_User_GetUserId(const AH_USER *u){
  assert(u);
  return u->userId;
}



void AH_User_SetUserId(AH_USER *u, const char *s) {
  assert(u);
  free(u->userId);
  if (s) u->userId=strdup(s);
  else u->userId=0;
}



const char *AH_User_GetPeerId(const AH_USER *u) {
  assert(u);
  return u->peerId;
}



void AH_User_SetPeerId(AH_USER *u, const char *s) {
  assert(u);
  free(u->peerId);
  if (s) u->peerId=strdup(s);
  else u->peerId=0;
}



int AH_User_GetContextIdx(const AH_USER *u){
  assert(u);
  return u->contextIdx;
}



void AH_User_SetContextIdx(AH_USER *u, int idx){
  assert(u);
  u->contextIdx=idx;
}



AH_MEDIUM *AH_User_GetMedium(const AH_USER *u){
  assert(u);
  return u->medium;
}



AH_CUSTOMER *AH_User_FindCustomer(const AH_USER *u,
                                  const char *customerId){
  AH_CUSTOMER *cu;

  assert(u);
  assert(customerId);
  cu=AH_Customer_List_First(u->customers);
  while(cu) {
    if (-1!=GWEN_Text_ComparePattern(AH_Customer_GetCustomerId(cu),
                                     customerId, 0))
      return cu;
    cu=AH_Customer_List_Next(cu);
  } /* while */

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Customer \"%s\" not found", customerId);
  return 0;
}



AH_CUSTOMER_LIST2 *AH_User_GetCustomers(const AH_USER *u,
                                        const char *customerId){
  AH_CUSTOMER *cu;
  AH_CUSTOMER_LIST2 *cl;

  assert(u);
  assert(customerId);
  cl=AH_Customer_List2_new();
  cu=AH_Customer_List_First(u->customers);
  while(cu) {
    if (-1!=GWEN_Text_ComparePattern(AH_Customer_GetCustomerId(cu),
                                     customerId, 0))
      AH_Customer_List2_PushBack(cl, cu);
    cu=AH_Customer_List_Next(cu);
  } /* while */

  if (AH_Customer_List2_GetSize(cl)==0) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "No customers found");
    AH_Customer_List2_free(cl);
    return 0;
  }

  return cl;
}



AH_CRYPT_MODE AH_User_GetCryptMode(const AH_USER *u) {
  assert(u);
  return u->cryptMode;
}



void AH_User_SetCryptMode(AH_USER *u, AH_CRYPT_MODE m) {
  assert(u);
  u->cryptMode=m;
}






int AH_User_AddCustomer(AH_USER *u, AH_CUSTOMER *cu){
  if (AH_User_FindCustomer(u, AH_Customer_GetCustomerId(cu))) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN,
              "Customer \"%s\" already enlisted",
              AH_Customer_GetCustomerId(cu));
    return -1;
  }
  AH_Customer_List_Add(cu, u->customers);
  DBG_INFO(AQHBCI_LOGDOMAIN,
           "Customer \"%s\" added to internal list",
           AH_Customer_GetCustomerId(cu));
  return 0;
}



int AH_User_RemoveCustomer(AH_USER *u, AH_CUSTOMER *cu) {
  assert(u);
  assert(cu);
  AH_Customer_List_Del(cu);
  return 0;
}



AH_USER_STATUS AH_User_GetStatus(const AH_USER *u){
  assert(u);
  return u->status;
}



void AH_User_SetStatus(AH_USER *u, AH_USER_STATUS i){
  assert(u);
  u->status=i;
}



const char *AH_User_StatusName(AH_USER_STATUS st){
  switch(st) {
  case AH_UserStatusNew:      return "new";
  case AH_UserStatusEnabled:  return "enabled";
  case AH_UserStatusPending:  return "pending";
  case AH_UserStatusDisabled: return "disabled";
  default:                    return "unknown";
  } /* switch */
}



AH_USER_STATUS AH_User_StatusFromName(const char *s){
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



AH_HBCI *AH_User_GetHbci(const AH_USER *u){
  assert(u);
  assert(u->bank);
  return AH_Bank_GetHbci(u->bank);
}



AH_USER *AH_User_fromDb(AH_BANK *b, GWEN_DB_NODE *db) {
  AH_HBCI *hbci;
  GWEN_DB_NODE *gr;
  AH_MEDIUM *m;
  const char *mediumName;
  const char *mediumTypeName;
  const char *mediumSubTypeName;
  AH_CRYPT_MODE cm;
  AH_USER *u;
  int country;
  const char *bankId;
  const char *userId;
  const char *s;

  assert(b);
  hbci=AH_Bank_GetHbci(b);
  assert(hbci);
  userId=GWEN_DB_GetCharValue(db, "userId", 0, 0);
  assert(userId);

  country=AH_Bank_GetCountry(b);
  bankId=AH_Bank_GetBankId(b);

  s=GWEN_DB_GetCharValue(db, "cryptMode", 0, 0);
  if (!s) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "cryptmode missing for user \"%s\"", userId);
    abort();
  }
  cm=AH_CryptMode_fromString(s);

  /* check medium settings */
  gr=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "medium");
  if (!gr) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No medium settings for user \"%s\"", userId);
    abort();
  }

  /* get medium */
  mediumName=GWEN_DB_GetCharValue(gr, "mediumName", 0, 0);
  mediumTypeName=GWEN_DB_GetCharValue(gr, "mediumTypeName", 0, 0);
  mediumSubTypeName=GWEN_DB_GetCharValue(gr, "mediumSubTypeName", 0, 0);
  m=AH_HBCI_FindMedium(hbci,
                       mediumTypeName,
                       mediumName);
  if (!m) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
	      "Medium for user \"%s\" is not available", userId);
    abort();
  }

  u=AH_User_new(b, userId, cm, m);
  assert(u);

  s=GWEN_DB_GetCharValue(db, "peerId", 0, 0);
  if (s)
    u->peerId=strdup(s);
  u->contextIdx=GWEN_DB_GetIntValue(db, "contextIdx", 0, -1);
  u->status=AH_User_StatusFromName(GWEN_DB_GetCharValue(db, "status", 0,
                                                        "unknown"));

  /* load server address */
  gr=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "server");
  if (gr) {
    u->serverAddress=AH_BpdAddr_FromDb(gr);
    assert(u->serverAddress);
  }

  /* load customers */
  gr=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "customers");
  if (gr) {
    gr=GWEN_DB_FindFirstGroup(gr, "customer");
    while(gr) {
      AH_CUSTOMER *cu;

      cu=AH_Customer_fromDb(u, gr);
      assert(cu);
      AH_Customer_List_Add(cu, u->customers);
      gr=GWEN_DB_FindNextGroup(gr, "customer");
    }
  }

  return u;
}



int AH_User_toDb(const AH_USER *u, GWEN_DB_NODE *db) {
  GWEN_DB_NODE *gr;
  AH_CUSTOMER *cu;

  if (u->userId)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "userId", u->userId);
  if (u->peerId)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "peerId", u->peerId);

  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "status",
                       AH_User_StatusName(u->status));

  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "cryptMode",
                       AH_CryptMode_toString(u->cryptMode));

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "contextIdx", u->contextIdx);

  gr=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "medium");
  assert(gr);

  GWEN_DB_SetCharValue(gr, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "mediumTypeName",
                       AH_Medium_GetMediumTypeName(u->medium));
  GWEN_DB_SetCharValue(gr, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "mediumName",
                       AH_Medium_GetMediumName(u->medium));

  /* save serverAddress */
  if (u->serverAddress) {
    gr=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "server");
    assert(gr);
    if (AH_BpdAddr_ToDb(u->serverAddress, gr)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error writing server address");
      return -1;
    }
  }

  /* save customers */
  cu=AH_Customer_List_First(u->customers);
  if (cu) {
    gr=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "customers");
    assert(gr);
    while(cu) {
      GWEN_DB_NODE *dbC;

      dbC=GWEN_DB_GetGroup(gr, GWEN_PATH_FLAGS_CREATE_GROUP, "customer");
      assert(dbC);
      if (AH_Customer_toDb(cu, dbC)) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Error writing customer");
        return -1;
      }
      cu=AH_Customer_List_Next(cu);
    } /* while */
  }

  return 0;
}



const AH_BPD_ADDR *AH_User_GetAddress(const AH_USER *u){
  assert(u);
  return u->serverAddress;
}



void AH_User_SetAddress(AH_USER *u, const AH_BPD_ADDR *a){
  assert(u);
  AH_BpdAddr_free(u->serverAddress);
  if (a) u->serverAddress=AH_BpdAddr_dup(a);
  else u->serverAddress=0;
}

















