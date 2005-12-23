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
#include "aqhbci_l.h"
#include "hbci_l.h"
#include "account_l.h"
#include "user_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_LIST_FUNCTIONS(AH_BANK, AH_Bank);
GWEN_LIST2_FUNCTIONS(AH_BANK, AH_Bank);
GWEN_INHERIT_FUNCTIONS(AH_BANK);


AH_BANK *AH_Bank_new(AH_HBCI *hbci,
                     int country,
                     const char *bankId) {
  AH_BANK *b;

  assert(hbci);
  assert(country);
  assert(bankId);
  assert(*bankId);
  GWEN_NEW_OBJECT(AH_BANK, b);
  GWEN_LIST_INIT(AH_BANK, b);
  GWEN_INHERIT_INIT(AH_BANK, b);
  b->hbci=hbci;
  b->country=country;

  /* check bank id */
  assert(!AH_HBCI_CheckStringSanity(bankId));
  b->bankId=strdup(bankId);
  b->users=AH_User_List_new();
  b->accounts=AH_Account_List_new();
  b->usage=1;
  return b;
}



void AH_Bank_free(AH_BANK *b){
  if (b) {
    assert(b->usage);
    b->usage--;
    if (b->usage==0) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Destroying AH_BANK");
      GWEN_INHERIT_FINI(AH_BANK, b);
      free(b->bankId);
      free(b->bankName);
      AH_Account_List_free(b->accounts);
      AH_User_List_free(b->users);

      GWEN_LIST_FINI(AH_BANK, b);
      GWEN_FREE_OBJECT(b);
    }
  }
}



AH_BANK *AH_Bank_fromDb(AH_HBCI *hbci, GWEN_DB_NODE *db){
  const char *p;
  AH_BANK *b;
  GWEN_DB_NODE *gr;

  assert(hbci);
  GWEN_NEW_OBJECT(AH_BANK, b);
  b->usage=1;
  GWEN_LIST_INIT(AH_BANK, b);
  b->users=AH_User_List_new();
  b->accounts=AH_Account_List_new();
  b->hbci=hbci;

  b->country=GWEN_DB_GetIntValue(db, "country", 0, 280);

  p=GWEN_DB_GetCharValue(db, "bankId", 0, 0);
  if (p)
    b->bankId=strdup(p);

  p=GWEN_DB_GetCharValue(db, "bankName", 0, 0);
  if (p)
    b->bankName=strdup(p);

  /* read users */
  gr=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "users");
  if (gr) {
    gr=GWEN_DB_FindFirstGroup(gr, "user");
    while(gr) {
      AH_USER *u;

      u=AH_User_fromDb(b, gr);
      assert(u);
      AH_User_List_Add(u, b->users);
      gr=GWEN_DB_FindNextGroup(gr, "user");
    }
  }

  /* read accounts */
  gr=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "accounts");
  if (gr) {
    gr=GWEN_DB_FindFirstGroup(gr, "account");
    while(gr) {
      AH_ACCOUNT *a;

      a=AH_Account_fromDb(b, gr);
      assert(a);
      AH_Account_List_Add(a, b->accounts);
      gr=GWEN_DB_FindNextGroup(gr, "account");
    }
  }

  return b;
}



int AH_Bank_toDb(const AH_BANK *b, GWEN_DB_NODE *db){
  GWEN_DB_NODE *gr;
  AH_USER *u;
  AH_ACCOUNT *a;

  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "country", b->country);

  if (b->bankId)
    GWEN_DB_SetCharValue(db,
                         GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "bankId", b->bankId);

  if (b->bankName)
    GWEN_DB_SetCharValue(db,
                         GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "bankName", b->bankName);


  /* store users */
  u=AH_User_List_First(b->users);
  if (u) {
    gr=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "users");
    assert(gr);
    while(u) {
      GWEN_DB_NODE *dbU;

      dbU=GWEN_DB_GetGroup(gr, GWEN_PATH_FLAGS_CREATE_GROUP, "user");
      if (AH_User_toDb(u, dbU)) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Error saving user");
        return -1;
      }
      u=AH_User_List_Next(u);
    } /* while */
  }

  /* store accounts */
  a=AH_Account_List_First(b->accounts);
  if (a) {
    gr=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "accounts");
    assert(gr);
    while(a) {
      GWEN_DB_NODE *dbA;

      dbA=GWEN_DB_GetGroup(gr, GWEN_PATH_FLAGS_CREATE_GROUP, "account");
      if (AH_Account_toDb(a, dbA)) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Error saving account");
        return -1;
      }
      a=AH_Account_List_Next(a);
    } /* while */
  }

  return 0;
}



void AH_Bank_Attach(AH_BANK *b){
  assert(b);
  b->usage++;
}



AH_HBCI *AH_Bank_GetHbci(const AH_BANK *b){
  assert(b);
  return b->hbci;
}



int AH_Bank_GetCountry(const AH_BANK *b){
  assert(b);
  return b->country;
}



const char *AH_Bank_GetBankId(const AH_BANK *b){
  assert(b);
  return b->bankId;
}



AH_USER *AH_Bank_FindUser(const AH_BANK *b, const char *userId) {
  AH_USER *u;

  assert(b);
  assert(userId);

  u=AH_User_List_First(b->users);

  while(u) {
    if (-1!=GWEN_Text_ComparePattern(AH_User_GetUserId(u), userId, 0))
      return u;
    u=AH_User_List_Next(u);
  } /* while */

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "User \"%s\" not found", userId);
  return 0;
}



AH_USER_LIST2 *AH_Bank_GetUsers(const AH_BANK *b, const char *userId) {
  AH_USER *u;
  AH_USER_LIST2 *ul;

  assert(b);
  assert(userId);

  ul=AH_User_List2_new();
  u=AH_User_List_First(b->users);

  while(u) {
    if (-1!=GWEN_Text_ComparePattern(AH_User_GetUserId(u), userId, 0))
      AH_User_List2_PushBack(ul, u);
    u=AH_User_List_Next(u);
  } /* while */

  if (AH_User_List2_GetSize(ul)==0) {
    AH_User_List2_free(ul);
    return 0;
  }

  return ul;
}



int AH_Bank_AddUser(AH_BANK *b, AH_USER *u) {

  if (AH_Bank_FindUser(b, AH_User_GetUserId(u))) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "User \"%s\" already enlisted", AH_User_GetUserId(u));
    return -1;
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Adding user \"%s\"", AH_User_GetUserId(u));
  AH_User_List_Add(u, b->users);
  return 0;
}



int AH_Bank_RemoveUser(AH_BANK *b, AH_USER *u){
  assert(b);
  assert(u);
  AH_User_List_Del(u);
  return 0;
}




AH_CUSTOMER *AH_Bank_FindCustomer(const AH_BANK *b,
                                  const char *userId,
                                  const char *customerId) {
  AH_USER *u;

  assert(b);
  u=AH_User_List_First(b->users);
  while(u) {
    if (-1!=GWEN_Text_ComparePattern(AH_User_GetUserId(u), userId, 0)) {
      AH_CUSTOMER *cu;

      cu=AH_User_FindCustomer(u, customerId);
      if (cu)
        return cu;
    }

    u=AH_User_List_Next(u);
  } /* while */

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Customer not found");
  return 0;
}



AH_CUSTOMER_LIST2 *AH_Bank_GetCustomers(const AH_BANK *b,
                                        const char *userId,
                                        const char *customerId){
  AH_USER *u;
  AH_CUSTOMER_LIST2 *cl;

  assert(b);
  assert(userId);
  assert(customerId);
  cl=AH_Customer_List2_new();
  u=AH_User_List_First(b->users);
  while(u) {
    if (-1!=GWEN_Text_ComparePattern(AH_User_GetUserId(u), userId, 0)) {
      AH_CUSTOMER_LIST2 *tcl;

      tcl=AH_User_GetCustomers(u, customerId);
      if (tcl) {
        AH_CUSTOMER_LIST2_ITERATOR *it;
        AH_CUSTOMER *cu;

	it=AH_Customer_List2_First(tcl);
	assert(it);
	cu=AH_Customer_List2Iterator_Data(it);
	while(cu) {
          AH_Customer_List2_PushBack(cl, cu);
	  cu=AH_Customer_List2Iterator_Next(it);
	}
        AH_Customer_List2Iterator_free(it);
	AH_Customer_List2_free(tcl);
      }
    }

    u=AH_User_List_Next(u);
  } /* while */

  if (AH_Customer_List2_GetSize(cl)==0) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "No customers found");
    AH_Customer_List2_free(cl);
    return 0;
  }

  return cl;
}






AH_ACCOUNT *AH_Bank_FindAccount(const AH_BANK *b,
                                const char *accountId){
  AH_ACCOUNT *a;

  assert(b);
  assert(accountId);

  a=AH_Account_List_First(b->accounts);

  while(a) {
    if (-1!=GWEN_Text_ComparePattern(AH_Account_GetAccountId(a),
                                     accountId, 0))
      return a;
    a=AH_Account_List_Next(a);
  } /* while */

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Account \"%s\" not found", accountId);
  return 0;
}



AH_ACCOUNT_LIST2 *AH_Bank_GetAccounts(const AH_BANK *b,
                                      const char *accountId){
  AH_ACCOUNT *a;
  AH_ACCOUNT_LIST2 *al;

  assert(b);
  assert(accountId);

  al=AH_Account_List2_new();
  a=AH_Account_List_First(b->accounts);

  while(a) {
    if (-1!=GWEN_Text_ComparePattern(AH_Account_GetAccountId(a),
                                     accountId, 0))
      AH_Account_List2_PushBack(al, a);
    a=AH_Account_List_Next(a);
  } /* while */

  if (AH_Account_List2_GetSize(al)==0) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "No accounts found");
    AH_Account_List2_free(al);
    return 0;
  }

  return al;
}



int AH_Bank_AddAccount(AH_BANK *b, AH_ACCOUNT *a){
  if (AH_Bank_FindAccount(b, AH_Account_GetAccountId(a))) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Account \"%s\" already enlisted",
              AH_Account_GetAccountId(a));
    return -1;
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Adding account \"%s\"",
            AH_Account_GetAccountId(a));
  AH_Account_List_Add(a, b->accounts);
  return 0;
}



int AH_Bank_RemoveAccount(AH_BANK *b, AH_ACCOUNT *a){
  AH_Account_List_Del(a);
  return 0;
}



const char *AH_Bank_GetBankName(const AH_BANK *b){
  assert(b);
  return b->bankName;
}



void AH_Bank_SetBankName(AH_BANK *b, const char *s){
  assert(b);
  free(b->bankName);
  if (s) b->bankName=strdup(s);
  else b->bankName=0;
}








