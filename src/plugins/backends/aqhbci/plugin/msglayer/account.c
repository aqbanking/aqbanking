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


#include "account_p.h"
#include "aqhbci_l.h"
#include "hbci_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_LIST_FUNCTIONS(AH_ACCOUNT, AH_Account);
GWEN_INHERIT_FUNCTIONS(AH_ACCOUNT);
GWEN_LIST2_FUNCTIONS(AH_ACCOUNT, AH_Account);


AH_ACCOUNT *AH_Account_new(AH_BANK *b,
                           const char *bankId,
                           const char *accountId) {
  AH_ACCOUNT *a;

  assert(b);
  assert(bankId);
  assert(accountId);
  GWEN_NEW_OBJECT(AH_ACCOUNT, a);
  GWEN_LIST_INIT(AH_ACCOUNT, a);
  GWEN_INHERIT_INIT(AH_ACCOUNT, a);

  a->bank=b;
  assert(!AH_HBCI_CheckStringSanity(bankId));
  assert(!AH_HBCI_CheckStringSanity(accountId));
  a->bankId=strdup(bankId);
  a->accountId=strdup(accountId);
  a->customers=GWEN_StringList_new();

  a->usage=1;
  return a;
}



void AH_Account_free(AH_ACCOUNT *a) {
  if (a) {
    assert(a->usage);
    if (--(a->usage)==0) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Destroying AH_Account");
      GWEN_INHERIT_FINI(AH_ACCOUNT, a);

      free(a->bankId);
      free(a->accountId);
      free(a->accountName);
      free(a->ownerName);
      free(a->suffix);
      GWEN_StringList_free(a->customers);

      GWEN_LIST_FINI(AH_ACCOUNT, a);
      GWEN_FREE_OBJECT(a);
    }
  }
}



void AH_Account_Attach(AH_ACCOUNT *a){
  assert(a);
  a->usage++;
}



AH_BANK *AH_Account_GetBank(const AH_ACCOUNT *a){
  assert(a);
  return a->bank;
}



void AH_Account_SetBank(AH_ACCOUNT *a, AH_BANK *b){
  assert(a);
  assert(b);
  if (b!=a->bank) {
    a->bank=b;
  }
}



const char *AH_Account_GetBankId(const AH_ACCOUNT *a){
  assert(a);
  return a->bankId;
}



void AH_Account_SetBankId(AH_ACCOUNT *a, const char *s){
  assert(a);
  free(a->bankId);
  if (s) a->bankId=strdup(s);
  else a->bankId=0;
}



const char *AH_Account_GetAccountId(const AH_ACCOUNT *a){
  assert(a);
  return a->accountId;
}



void AH_Account_SetAccountId(AH_ACCOUNT *a, const char *s){
  assert(a);
  free(a->accountId);
  if (s) a->accountId=strdup(s);
  else a->accountId=0;
}



const char *AH_Account_GetAccountName(const AH_ACCOUNT *a){
  assert(a);
  return a->accountName;
}



void AH_Account_SetAccountName(AH_ACCOUNT *a,
                               const char *s){
  assert(a);
  assert(s);
  free(a->accountName);
  a->accountName=strdup(s);
}



const char *AH_Account_GetOwnerName(const AH_ACCOUNT *a){
  assert(a);
  return a->ownerName;
}



void AH_Account_SetOwnerName(AH_ACCOUNT *a,
                             const char *s){
  assert(a);
  assert(s);
  free(a->ownerName);
  a->ownerName=strdup(s);
}



const GWEN_STRINGLIST *AH_Account_GetCustomers(const AH_ACCOUNT *a){
  assert(a);
  return a->customers;
}



void AH_Account_AddCustomer(AH_ACCOUNT *a, const char *cid) {

  assert(a);
  assert(cid);

  GWEN_StringList_AppendString(a->customers, cid, 0, 1);
}



void AH_Account_ClearCustomers(AH_ACCOUNT *a) {
  assert(a);

  GWEN_StringList_Clear(a->customers);
}



AH_ACCOUNT *AH_Account_fromDb(AH_BANK *b, GWEN_DB_NODE *db) {
  AH_ACCOUNT *a;
  const char *p;
  unsigned int i;

  assert(b);
  assert(db);
  GWEN_NEW_OBJECT(AH_ACCOUNT, a);
  GWEN_LIST_INIT(AH_ACCOUNT, a);
  GWEN_INHERIT_INIT(AH_ACCOUNT, a);

  a->bank=b;

  /* read variables */
  p=GWEN_DB_GetCharValue(db, "bankId", 0, 0);
  free(a->bankId);
  if (p) a->bankId=strdup(p);
  else a->bankId=strdup("");;

  p=GWEN_DB_GetCharValue(db, "accountId", 0, 0);
  free(a->accountId);
  if (p) a->accountId=strdup(p);
  else a->accountId=strdup("");;

  p=GWEN_DB_GetCharValue(db, "accountName", 0, 0);
  free(a->accountName);
  if (p) a->accountName=strdup(p);
  else a->accountName=strdup("");;

  p=GWEN_DB_GetCharValue(db, "ownerName", 0, 0);
  free(a->ownerName);
  if (p) a->ownerName=strdup(p);
  else a->ownerName=strdup("");;

  GWEN_StringList_free(a->customers);
  a->customers=GWEN_StringList_new();
  for (i=0; ; i++) {
    p=GWEN_DB_GetCharValue(db, "customer", i, 0);
    if (p) {
      GWEN_StringList_AppendString(a->customers, p, 0, 1);
    }
    else
      break;
  } /* for */

  a->usage=1;
  return a;
}



int AH_Account_toDb(const AH_ACCOUNT *a, GWEN_DB_NODE *db) {
  GWEN_STRINGLISTENTRY *se;

  assert(a);
  assert(db);

  /* write variables */
  if (a->bankId)
    GWEN_DB_SetCharValue(db,
                         GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "bankId", a->bankId);

  if (a->accountId)
    GWEN_DB_SetCharValue(db,
                         GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "accountId", a->accountId);

  if (a->accountName)
    GWEN_DB_SetCharValue(db,
                         GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "accountName", a->accountName);

  if (a->ownerName)
    GWEN_DB_SetCharValue(db,
                         GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "ownerName", a->ownerName);

  GWEN_DB_DeleteVar(db, "customer");
  se=GWEN_StringList_FirstEntry(a->customers);
  while(se) {
    GWEN_DB_SetCharValue(db,
                         GWEN_DB_FLAGS_DEFAULT,
                         "customer",
                         GWEN_StringListEntry_Data(se));
    se=GWEN_StringListEntry_Next(se);
  } /* while */

  return 0;
}



void AH_Account_CleanUp(AH_ACCOUNT *a) {
  assert(a);
  if (a->usage==1)
    AH_Account_free(a);
}



AH_ACCOUNT *AH_Account__freeAll_cb(AH_ACCOUNT *a, void *userData) {
  AH_Account_free(a);
  return 0;
}



void AH_Account_List2_freeAll(AH_ACCOUNT_LIST2 *al){
  AH_Account_List2_ForEach(al, AH_Account__freeAll_cb, 0);
  AH_Account_List2_free(al);
}



const char *AH_Account_GetSuffix(const AH_ACCOUNT *a){
  assert(a);
  return a->suffix;
}



void AH_Account_SetSuffix(AH_ACCOUNT *a, const char *s){
  assert(a);
  free(a->suffix);
  if (s) a->suffix=strdup(s);
  else a->suffix=0;
}











