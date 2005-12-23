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


#include "customer_p.h"
#include "aqhbci_l.h"
#include "hbci_l.h"
#include "user_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_LIST_FUNCTIONS(AH_CUSTOMER, AH_Customer);

GWEN_LIST2_FUNCTIONS(AH_CUSTOMER, AH_Customer);


AH_CUSTOMER *AH_Customer_new(AH_USER *u, const char *customerId){
  AH_CUSTOMER *cu;
  GWEN_XMLNODE *defs;
  AH_BANK *b;

  assert(u);
  assert(customerId);
  assert(!AH_HBCI_CheckStringSanity(customerId));

  GWEN_NEW_OBJECT(AH_CUSTOMER, cu);
  cu->usage=1;
  GWEN_LIST_INIT(AH_CUSTOMER, cu);

  cu->user=u;

  b=AH_User_GetBank(u);
  assert(b);

  cu->customerId=strdup(customerId);
  cu->hbciVersion=210;
  cu->ignoreUPD=1;
  cu->bankUsesSignSeq=1;

  cu->httpVMajor=1;
  cu->httpVMinor=0;

  cu->bpd=AH_Bpd_new();
  cu->upd=GWEN_DB_Group_new("upd");

  /* create message engine */
  cu->msgEngine=AH_MsgEngine_new();
  AH_MsgEngine_SetCustomer(cu->msgEngine, cu);
  defs=AH_HBCI_GetDefinitions(AH_Bank_GetHbci(b));
  if (defs) {
    GWEN_MsgEngine_SetDefinitions(cu->msgEngine, defs, 0);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No definitions available, should not happen");
  }
  AH_MsgEngine_SetCustomer(cu->msgEngine, cu);

  return cu;
}



AH_CUSTOMER *AH_Customer_fromDb(AH_USER *u, GWEN_DB_NODE *db) {
  AH_CUSTOMER *cu;
  const char *p;
  GWEN_DB_NODE *gr;
  AH_BANK *b;
  GWEN_XMLNODE *defs;

  assert(u);
  assert(db);

  GWEN_NEW_OBJECT(AH_CUSTOMER, cu);
  cu->usage=1;
  GWEN_LIST_INIT(AH_CUSTOMER, cu);
  cu->user=u;
  /* AH_User_Attach(u); */
  b=AH_User_GetBank(u);
  assert(b);

  /* create message engine */
  cu->msgEngine=AH_MsgEngine_new();
  defs=AH_HBCI_GetDefinitions(AH_Bank_GetHbci(b));
  if (defs) {
    GWEN_MsgEngine_SetDefinitions(cu->msgEngine, defs, 0);
  }
  AH_MsgEngine_SetCustomer(cu->msgEngine, cu);

  cu->hbciVersion=GWEN_DB_GetIntValue(db, "hbciversion", 0, 210);
  cu->ignoreUPD=GWEN_DB_GetIntValue(db, "ignoreUPD", 0, 0);
  cu->bankDoesntSign=GWEN_DB_GetIntValue(db, "bankDoesntSign", 0, 0);
  cu->bankUsesSignSeq=GWEN_DB_GetIntValue(db, "bankUsesSignSeq", 0, 0);
  cu->httpVMajor=GWEN_DB_GetIntValue(db, "httpVMajor", 0, 1);
  cu->httpVMinor=GWEN_DB_GetIntValue(db, "httpVMinor", 0, 0);
  cu->preferSingleTransfer=GWEN_DB_GetIntValue(db,"preferSingleTransfer",0,0);
  cu->preferSingleDebitNote=GWEN_DB_GetIntValue(db,"preferSingleDebitNote",0,
                                                /* correct! */
                                                cu->preferSingleTransfer);
  cu->keepAlive=GWEN_DB_GetIntValue(db,"keepAlive",0,0);

  p=GWEN_DB_GetCharValue(db, "httpUserAgent", 0, 0);
  if (p)
    cu->httpUserAgent=strdup(p);

  p=GWEN_DB_GetCharValue(db, "customerid", 0, 0);
  if (p)
    cu->customerId=strdup(p);

  p=GWEN_DB_GetCharValue(db, "systemid", 0, 0);
  if (p)
    cu->systemId=strdup(p);

  p=GWEN_DB_GetCharValue(db, "fullname", 0, 0);
  if (p)
    cu->fullName=strdup(p);

  cu->updVersion=GWEN_DB_GetIntValue(db, "updversion", 0, 1);
  gr=GWEN_DB_GetGroup(db,
                      GWEN_PATH_FLAGS_PATHMUSTEXIST,
                      "upd");
  if (gr)
    cu->upd=GWEN_DB_Group_dup(gr);
  else
    cu->upd=GWEN_DB_Group_new("upd");

  gr=GWEN_DB_GetGroup(db,
                      GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                      "bpd");
  if (gr) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Loading BPD");
    cu->bpd=AH_Bpd_FromDb(gr);
  }
  else {
    cu->bpd=AH_Bpd_new();
  }

  return cu;
}



int AH_Customer_toDb(const AH_CUSTOMER *cu, GWEN_DB_NODE *db) {
  GWEN_DB_NODE *gr;

  assert(cu);
  assert(db);

  if (cu->customerId)
    GWEN_DB_SetCharValue(db,
                         GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "customerId", cu->customerId);
  if (cu->systemId)
    GWEN_DB_SetCharValue(db,
                         GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "systemId", cu->systemId);
  if (cu->fullName)
    GWEN_DB_SetCharValue(db,
                         GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "fullName", cu->fullName);
  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "updversion", cu->updVersion);

  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "hbciversion", cu->hbciVersion);

  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "ignoreUPD", cu->ignoreUPD);

  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "bankDoesntSign", cu->bankDoesntSign);

  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "bankUsesSignSeq", cu->bankUsesSignSeq);

  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "preferSingleTransfer", cu->preferSingleTransfer);

  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "preferSingleDebitNote", cu->preferSingleDebitNote);

  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "keepAlive", cu->keepAlive);

  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "httpVMajor", cu->httpVMajor);
  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "httpVMinor", cu->httpVMinor);
  if (cu->httpUserAgent)
    GWEN_DB_SetCharValue(db,
                         GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "httpUserAgent", cu->httpUserAgent);

  if (cu->upd) {
    gr=GWEN_DB_GetGroup(db,
                        GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                        "upd");
    assert(gr);
    GWEN_DB_AddGroupChildren(gr, cu->upd);
  }

  /* store BPD */
  if (cu->bpd) {
    gr=GWEN_DB_GetGroup(db,
                        GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                        "bpd");
    assert(gr);
    if (AH_Bpd_ToDb(cu->bpd, gr)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not save BPD");
      return -1;
    }
  }

  return 0;
}



void AH_Customer_Attach(AH_CUSTOMER *cu){
  assert(cu);
  cu->usage++;
}



void AH_Customer_free(AH_CUSTOMER *cu){
  if (cu) {
    assert(cu->usage);
    cu->usage--;
    if (cu->usage==0) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Destroying AH_CUSTOMER");
      GWEN_LIST_FINI(AH_CUSTOMER, cu);
      AH_Bpd_free(cu->bpd);
      GWEN_DB_Group_free(cu->upd);
      free(cu->httpUserAgent);
      free(cu->customerId);
      free(cu->systemId);
      free(cu->fullName);
      GWEN_MsgEngine_free(cu->msgEngine);
      /* AH_User_free(cu->user); */
      GWEN_FREE_OBJECT(cu);
    }
  }
}



AH_USER *AH_Customer_GetUser(const AH_CUSTOMER *cu){
  assert(cu);
  return cu->user;
}



int AH_Customer_GetHbciVersion(const AH_CUSTOMER *cu){
  assert(cu);
  return cu->hbciVersion;
}



void AH_Customer_SetHbciVersion(AH_CUSTOMER *cu, int i){
  assert(cu);
  cu->hbciVersion=i;
}



const char *AH_Customer_GetCustomerId(const AH_CUSTOMER *cu){
  assert(cu);
  return cu->customerId;
}



const char *AH_Customer_GetFullName(const AH_CUSTOMER *cu){
  assert(cu);
  return cu->fullName;
}



void AH_Customer_SetFullName(AH_CUSTOMER *cu, const char *s){
  assert(cu);
  free(cu->fullName);
  if (s) cu->fullName=strdup(s);
  else cu->fullName=0;
}



int AH_Customer_GetUpdVersion(const AH_CUSTOMER *cu){
  assert(cu);
  return cu->updVersion;
}



void AH_Customer_SetUpdVersion(AH_CUSTOMER *cu, int i){
  assert(cu);
  cu->updVersion=i;
}



GWEN_DB_NODE *AH_Customer_GetUpd(const AH_CUSTOMER *cu){
  assert(cu);
  return cu->upd;
}



void AH_Customer_SetUpd(AH_CUSTOMER *cu,
                             GWEN_DB_NODE *n){
  assert(cu);
  GWEN_DB_Group_free(cu->upd);
  cu->upd=n;
}



AH_BPD *AH_Customer_GetBpd(const AH_CUSTOMER *cu){
  assert(cu);
  return cu->bpd;
}



void AH_Customer_SetBpd(AH_CUSTOMER *cu, AH_BPD *bpd){
  assert(cu);
  AH_Bpd_free(cu->bpd);
  cu->bpd=bpd;
}



int AH_Customer_GetBpdVersion(const AH_CUSTOMER *cu){
  assert(cu);
  assert(cu->bpd);
  return AH_Bpd_GetBpdVersion(cu->bpd);
}



void AH_Customer_SetBpdVersion(AH_CUSTOMER *cu, int i){
  assert(cu);
  assert(cu->bpd);
  AH_Bpd_SetBpdVersion(cu->bpd, i);
}



int AH_Customer_IgnoreUPD(const AH_CUSTOMER *cu){
  assert(cu);
  return cu->ignoreUPD;
}



void AH_Customer_SetIgnoreUPD(AH_CUSTOMER *cu, int i){
  assert(cu);
  cu->ignoreUPD=i;
}



GWEN_MSGENGINE *AH_Customer_GetMsgEngine(const AH_CUSTOMER *cu){
  assert(cu);
  return cu->msgEngine;
}



void AH_Customer_SetMsgEngine(AH_CUSTOMER *cu, GWEN_MSGENGINE *e){
  assert(cu);
  GWEN_MsgEngine_free(cu->msgEngine);
  cu->msgEngine=e;
}



const char *AH_Customer_GetSystemId(const AH_CUSTOMER *cu){
  assert(cu);
  return cu->systemId;
}



void AH_Customer_SetSystemId(AH_CUSTOMER *cu, const char *s){
  assert(cu);
  free(cu->systemId);
  if (s) cu->systemId=strdup(s);
  else cu->systemId=0;
}



int AH_Customer_GetBankSigns(const AH_CUSTOMER *cu){
  assert(cu);
  return !cu->bankDoesntSign;
}



void AH_Customer_SetBankSigns(AH_CUSTOMER *cu, int b){
  assert(cu);
  cu->bankDoesntSign=!b;
}



int AH_Customer_GetBankUsesSignSeq(const AH_CUSTOMER *cu){
  assert(cu);
  return cu->bankUsesSignSeq;
}



void AH_Customer_SetBankUsesSignSeq(AH_CUSTOMER *cu, int b){
  assert(cu);
  cu->bankUsesSignSeq=b;
}



int AH_Customer_GetHttpVMajor(const AH_CUSTOMER *cu){
  assert(cu);
  return cu->httpVMajor;
}



void AH_Customer_SetHttpVMajor(AH_CUSTOMER *cu, int i){
  assert(cu);
  cu->httpVMajor=i;
}



int AH_Customer_GetHttpVMinor(const AH_CUSTOMER *cu){
  assert(cu);
  return cu->httpVMinor;
}



void AH_Customer_SetHttpVMinor(AH_CUSTOMER *cu, int i){
  assert(cu);
  cu->httpVMinor=i;
}



const char *AH_Customer_GetHttpUserAgent(const AH_CUSTOMER *cu){
  assert(cu);
  return cu->httpUserAgent;
}



void AH_Customer_SetHttpUserAgent(AH_CUSTOMER *cu, const char *s){
  assert(cu);
  free(cu->httpUserAgent);
  if (s)
    cu->httpUserAgent=strdup(s);
  else
    cu->httpUserAgent=0;
}



int AH_Customer_GetPreferSingleTransfer(const AH_CUSTOMER *cu) {
  assert(cu);
  return cu->preferSingleTransfer;
}



void AH_Customer_SetPreferSingleTransfer(AH_CUSTOMER *cu, int i) {
  assert(cu);
  cu->preferSingleTransfer=i;
}



int AH_Customer_GetPreferSingleDebitNote(const AH_CUSTOMER *cu) {
  assert(cu);
  return cu->preferSingleDebitNote;
}



void AH_Customer_SetPreferSingleDebitNote(AH_CUSTOMER *cu, int i) {
  assert(cu);
  cu->preferSingleDebitNote=i;
}



int AH_Customer_GetKeepAlive(const AH_CUSTOMER *cu) {
  assert(cu);
  return cu->keepAlive;
}



void AH_Customer_SetKeepAlive(AH_CUSTOMER *cu, int i) {
  assert(cu);
  cu->keepAlive=i;
}









