/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Apr 05 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "transaction_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/text.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_LIST_FUNCTIONS(AB_TRANSACTION, AB_Transaction);
GWEN_LIST2_FUNCTIONS(AB_TRANSACTION, AB_Transaction);


AB_TRANSACTION *AB_Transaction_new(){
  AB_TRANSACTION *t;

  GWEN_NEW_OBJECT(AB_TRANSACTION, t);
  GWEN_LIST_INIT(AB_TRANSACTION, t);
  t->localCountryCode=280;
  t->remoteCountryCode=280;

  t->remoteOwnerName=GWEN_StringList_new();
  t->purpose=GWEN_StringList_new();

  return t;
}



void AB_Transaction_free(AB_TRANSACTION *t){
  if (t) {
    GWEN_LIST_FINI(AB_TRANSACTION, t);
    free(t->localBankCode);
    free(t->localAccountId);
    free(t->localSuffix);
    free(t->localOwnerName);
    free(t->remoteBankCode);
    free(t->remoteAccountId);
    free(t->remoteSuffix);
    GWEN_StringList_free(t->remoteOwnerName);
    GWEN_Time_free(t->valutaDate);
    GWEN_Time_free(t->date);
    AB_Value_free(t->value);
    free(t->transactionKey);
    free(t->customerReference);
    free(t->bankReference);
    free(t->transactionText);
    free(t->primanota);
    GWEN_StringList_free(t->purpose);

    GWEN_FREE_OBJECT(t);
  }
}



int AB_Transaction_GetLocalCountryCode(const AB_TRANSACTION *t){
  assert(t);
  return t->localCountryCode;
}



void AB_Transaction_SetLocalCountryCode(AB_TRANSACTION *t, int i){
  assert(t);
  t->localCountryCode=i;
}



const char *AB_Transaction_GetLocalBankCode(const AB_TRANSACTION *t){
  assert(t);
  return t->localBankCode;
}



void AB_Transaction_SetLocalBankCode(AB_TRANSACTION *t, const char *s){
  assert(t);
  free(t->localBankCode);
  if (s)
    t->localBankCode=strdup(s);
  else
    t->localBankCode=0;
}



const char *AB_Transaction_GetLocalAccountNumber(const AB_TRANSACTION *t){
  assert(t);
  return t->localAccountId;
}



void AB_Transaction_SetLocalAccountNumber(AB_TRANSACTION *t, const char *s){
  assert(t);
  free(t->localAccountId);
  if (s)
    t->localAccountId=strdup(s);
  else
    t->localAccountId=0;
}



const char *AB_Transaction_GetLocalSuffix(const AB_TRANSACTION *t){
  assert(t);
  return t->localSuffix;
}



void AB_Transaction_SetLocalSuffix(AB_TRANSACTION *t, const char *s){
  assert(t);
  free(t->localSuffix);
  if (s)
    t->localSuffix=strdup(s);
  else
    t->localSuffix=0;
}



const char *AB_Transaction_GetLocalName(const AB_TRANSACTION *t){
  assert(t);
  return t->localOwnerName;
}



void AB_Transaction_SetLocalName(AB_TRANSACTION *t, const char *s){
  assert(t);
  free(t->localOwnerName);
  if (s)
    t->localOwnerName=strdup(s);
  else
    t->localOwnerName=0;
}



int AB_Transaction_GetRemoteCountryCode(const AB_TRANSACTION *t){
  assert(t);
  return t->remoteCountryCode;
}



void AB_Transaction_SetRemoteCountryCode(AB_TRANSACTION *t, int i){
  assert(t);
  t->remoteCountryCode=i;
}



const char *AB_Transaction_GetRemoteBankCode(const AB_TRANSACTION *t){
  assert(t);
  return t->remoteBankCode;
}



void AB_Transaction_SetRemoteBankCode(AB_TRANSACTION *t, const char *s){
  assert(t);
  free(t->remoteBankCode);
  if (s)
    t->remoteBankCode=strdup(s);
  else
    t->remoteBankCode=0;
}



const char *AB_Transaction_GetRemoteAccountId(const AB_TRANSACTION *t){
  assert(t);
  return t->remoteAccountId;
}



void AB_Transaction_SetRemoteAccountNumber(AB_TRANSACTION *t, const char *s){
  assert(t);
  free(t->remoteAccountId);
  if (s)
    t->remoteAccountId=strdup(s);
  else
    t->remoteAccountId=0;
}



const char *AB_Transaction_GetRemoteSuffix(const AB_TRANSACTION *t){
  assert(t);
  return t->remoteSuffix;
}



void AB_Transaction_SetRemoteSuffix(AB_TRANSACTION *t, const char *s){
  assert(t);
  free(t->remoteSuffix);
  if (s)
    t->remoteSuffix=strdup(s);
  else
    t->remoteSuffix=0;
}



const GWEN_STRINGLIST *AB_Transaction_GetRemoteName(const AB_TRANSACTION *t){
  assert(t);
  return t->remoteOwnerName;
}



void AB_Transaction_AddRemoteName(AB_TRANSACTION *t, const char *s){
  assert(t);
  GWEN_StringList_AppendString(t->remoteOwnerName, s, 0, 0);
}



const GWEN_TIME *AB_GetTransaction_GetValutaDate(const AB_TRANSACTION *t){
  assert(t);
  return t->valutaDate;
}



void AB_Transaction_SetValutaDate(AB_TRANSACTION *t, const GWEN_TIME *d){
  assert(t);
  GWEN_Time_free(t->valutaDate);
  if (d)
    t->valutaDate=GWEN_Time_dup(d);
  else
    t->valutaDate=0;
}



const GWEN_TIME *AB_GetTransaction_GetDate(const AB_TRANSACTION *t){
  assert(t);
  return t->date;
}



void AB_Transaction_SetDate(AB_TRANSACTION *t, const GWEN_TIME *d){
  assert(t);
  GWEN_Time_free(t->date);
  if (d)
    t->date=GWEN_Time_dup(d);
  else
    t->date=0;
}



const AB_VALUE *AB_Transaction_GetValue(const AB_TRANSACTION *t){
  assert(t);
  return t->value;
}



void AB_Transaction_SetValue(AB_TRANSACTION *t, const AB_VALUE *v){
  assert(t);
  AB_Value_free(t->value);
  if (v)
    t->value=AB_Value_dup(v);
  else
    t->value=0;
}



const char *AB_Transaction_GetTransactionKey(const AB_TRANSACTION *t){
  assert(t);
  return t->transactionKey;
}



void AB_Transaction_SetTransactionKey(AB_TRANSACTION *t, const char *s){
  assert(t);
  free(t->transactionKey);
  if (s)
    t->transactionKey=strdup(s);
  else
    t->transactionKey=0;
}



const char *AB_Transaction_GetCustomerReference(const AB_TRANSACTION *t){
  assert(t);
  return t->customerReference;
}



void AB_Transaction_SetCustomerReference(AB_TRANSACTION *t, const char *s){
  assert(t);
  free(t->customerReference);
  if (s)
    t->customerReference=strdup(s);
  else
    t->customerReference=0;
}



const char *AB_Transaction_GetBankReference(const AB_TRANSACTION *t){
  assert(t);
  return t->bankReference;
}



void AB_Transaction_SetBankReference(AB_TRANSACTION *t, const char *s){
  assert(t);
  free(t->bankReference);
  if (s)
    t->bankReference=strdup(s);
  else
    t->bankReference=0;
}



int AB_Transaction_GetTransactionCode(const AB_TRANSACTION *t){
  assert(t);
  return t->transactionCode;
}



void AB_Transaction_SetTransactionCode(AB_TRANSACTION *t, int c){
}



const char *AB_Transaction_GetTransactionText(const AB_TRANSACTION *t){
  assert(t);
  return t->transactionText;
}



void AB_Transaction_SetTransactionText(AB_TRANSACTION *t, const char *s){
  assert(t);
  free(t->transactionText);
  if (s)
    t->transactionText=strdup(s);
  else
    t->transactionText=0;
}



const char *AB_Transaction_GetPrimanota(const AB_TRANSACTION *t){
  assert(t);
  return t->primanota;
}



void AB_Transaction_SetPrimanota(AB_TRANSACTION *t, const char *s){
  assert(t);
  free(t->primanota);
  if (s)
    t->primanota=strdup(s);
  else
    t->primanota=0;
}



const GWEN_STRINGLIST *AB_Transaction_GetPurpose(const AB_TRANSACTION *t){
  assert(t);
  return t->purpose;
}



void AB_Transaction_AddPurpose(AB_TRANSACTION *t, const char *s){
  assert(t);
  GWEN_StringList_AppendString(t->purpose, s, 0, 0);
}




int AB_Transaction_ToDb(const AB_TRANSACTION *t, GWEN_DB_NODE *db) {
  GWEN_STRINGLISTENTRY *se;
  const char *p;

  assert(t);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "localCountryCode",
                      t->localCountryCode);
  p=t->localBankCode;
  if (p)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "localBankCode", p);
  p=t->localAccountId;
  if (p)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "localAccountNumber", p);

  p=t->localSuffix;
  if (p)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "localSuffix", p);
  p=t->localOwnerName;
  if (p)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "localName", p);


  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "remoteCountryCode",
                      t->remoteCountryCode);
  p=t->remoteBankCode;
  if (p)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "remoteBankCode", p);
  p=t->remoteAccountId;
  if (p)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "remoteAccountNumber", p);
  p=t->remoteSuffix;
  if (p)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "remoteSuffix", p);

  se=GWEN_StringList_FirstEntry(t->remoteOwnerName);
  GWEN_DB_DeleteVar(db, "remoteName");
  while(se) {
    p=GWEN_StringListEntry_Data(se);
    if (p)
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT,
                           "remoteName", p);
    se=GWEN_StringListEntry_Next(se);
  } /* while */

  if (t->valutaDate)
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "valutaDate", GWEN_Time_Seconds(t->valutaDate));
  if (t->date)
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "date", GWEN_Time_Seconds(t->date));

  if (t->value) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "value");
    assert(dbT);
    if (AB_Value_ToDb(t->value, dbT)) {
      DBG_INFO(0, "Error storing value");
      return -1;
    }
  }

  p=t->transactionKey;
  if (p)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "transactionKey", p);
  p=t->customerReference;
  if (p)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "customerReference", p);
  p=t->bankReference;
  if (p)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "bankReference", p);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "transactionCode",
                      t->transactionCode);

  p=t->transactionText;
  if (p)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "transactionText", p);
  p=t->primanota;
  if (p)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "primanota", p);

  se=GWEN_StringList_FirstEntry(t->purpose);
  GWEN_DB_DeleteVar(db, "purpose");
  while(se) {
    p=GWEN_StringListEntry_Data(se);
    if (p)
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT,
                           "purpose", p);
    se=GWEN_StringListEntry_Next(se);
  } /* while */

  return 0;
}



AB_TRANSACTION *AB_Transaction_FromDb(GWEN_DB_NODE *db) {
  AB_TRANSACTION *t;
  const char *p;
  GWEN_DB_NODE *dbT;
  int i;
  GWEN_TYPE_UINT32 u;

  t=AB_Transaction_new();
  assert(t);

  AB_Transaction_SetLocalCountryCode(t,
                                     GWEN_DB_GetIntValue(db,
                                                         "localCountryCode",
                                                         0, 0));

  AB_Transaction_SetLocalBankCode(t,
                                  GWEN_DB_GetCharValue(db,
                                                       "localBankCode",
                                                       0, 0));
  AB_Transaction_SetLocalAccountNumber(t,
                                       GWEN_DB_GetCharValue(db,
                                                            "localAccountNumber",
                                                            0, 0));
  AB_Transaction_SetLocalSuffix(t,
                                GWEN_DB_GetCharValue(db,
                                                     "localSuffix", 0, 0));
  AB_Transaction_SetLocalName(t,
                              GWEN_DB_GetCharValue(db,
                                                   "localName",
                                                   0, 0));
  AB_Transaction_SetRemoteCountryCode(t,
                                      GWEN_DB_GetIntValue(db,
                                                          "remoteCountryCode",
                                                          0, 0));

  AB_Transaction_SetRemoteBankCode(t,
                                   GWEN_DB_GetCharValue(db,
                                                        "remoteBankCode",
                                                        0, 0));
  AB_Transaction_SetRemoteAccountNumber(t,
                                    GWEN_DB_GetCharValue(db,
                                                         "remoteAccountNumber",
                                                         0, 0));
  AB_Transaction_SetRemoteSuffix(t,
                                 GWEN_DB_GetCharValue(db,
                                                      "remoteSuffix", 0, 0));

  for (i=0; ; i++) {
    p=GWEN_DB_GetCharValue(db, "remoteName", i, 0);
    if (!p)
      break;
    AB_Transaction_AddRemoteName(t, p);
  } /* for */

  u=(unsigned int)GWEN_DB_GetIntValue(db, "valutaDate", 0, 0);
  if (u)
    AB_Transaction_SetValutaDate(t,
                                 GWEN_Time_fromSeconds(u));
  u=(unsigned int)GWEN_DB_GetIntValue(db, "date", 0, 0);
  if (u)
    AB_Transaction_SetDate(t, GWEN_Time_fromSeconds(u));

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "value");
  if (dbT)
    AB_Transaction_SetValue(t,
                            AB_Value_FromDb(dbT));

  AB_Transaction_SetTransactionKey(t,
                                   GWEN_DB_GetCharValue(db,
                                                        "transactionKey",
                                                        0, 0));
  AB_Transaction_SetCustomerReference(t,
                                      GWEN_DB_GetCharValue(db,
                                                           "customerReference",
                                                           0, 0));
  AB_Transaction_SetBankReference(t,
                                  GWEN_DB_GetCharValue(db,
                                                       "bankReference",
                                                       0, 0));

  AB_Transaction_SetTransactionCode(t,
                                    GWEN_DB_GetIntValue(db,
                                                        "transactionCode",
                                                        0, 0));


  AB_Transaction_SetTransactionText(t,
                                    GWEN_DB_GetCharValue(db,
                                                         "transactionText",
                                                         0, 0));
  AB_Transaction_SetPrimanota(t,
                              GWEN_DB_GetCharValue(db, "primanota", 0, 0));

  for (i=0; ; i++) {
    p=GWEN_DB_GetCharValue(db, "purpose", i, 0);
    if (!p)
      break;
    AB_Transaction_AddPurpose(t, p);
  } /* for */

  return t;
}














