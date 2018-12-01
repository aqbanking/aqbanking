/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "account_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT(AB_ACCOUNT, AO_ACCOUNT)


AB_ACCOUNT *AO_Account_new(AB_PROVIDER *pro) {
  AB_ACCOUNT *a;
  AO_ACCOUNT *ae;

  a=AB_Account_new();
  assert(a);
  AB_Account_SetProvider(a, pro);
  AB_Account_SetBackendName(a, "aqofxconnect");

  GWEN_NEW_OBJECT(AO_ACCOUNT, ae);
  GWEN_INHERIT_SETDATA(AB_ACCOUNT, AO_ACCOUNT, a, ae, AO_Account_freeData);

  ae->maxPurposeLines=1;
  ae->debitAllowed=0;

  ae->readFromDbFn=AB_Account_SetReadFromDbFn(a, AO_Account_ReadFromDb);
  ae->writeToDbFn=AB_Account_SetWriteToDbFn(a, AO_Account_WriteToDb);

  return a;
}



int AO_Account_ReadFromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db) {
  AO_ACCOUNT *ae;
  GWEN_DB_NODE *dbP;
  int rv;
  AB_PROVIDER *pro;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AO_ACCOUNT, a);
  assert(ae);

  /* save provider, because AB_Account_ReadFromDb clears it */
  pro=AB_Account_GetProvider(a);

  /* read data for base class */
  rv=(ae->readFromDbFn)(a, db);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* set provider again */
  AB_Account_SetProvider(a, pro);

  /* read data for provider */
  dbP=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "data/backend");

  ae->maxPurposeLines=GWEN_DB_GetIntValue(dbP, "maxPurposeLines", 0, 1);
  ae->debitAllowed=GWEN_DB_GetIntValue(dbP, "debitAllowed", 0, 1);

  return 0;
}



int AO_Account_WriteToDb(const AB_ACCOUNT *a, GWEN_DB_NODE *db) {
  AO_ACCOUNT *ae;
  GWEN_DB_NODE *dbP;
  int rv;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AO_ACCOUNT, a);
  assert(ae);

  rv=(ae->writeToDbFn)(a, db);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* write data for provider */
  dbP=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "data/backend");

  GWEN_DB_SetIntValue(dbP, GWEN_DB_FLAGS_OVERWRITE_VARS, "maxPurposeLines", ae->maxPurposeLines);
  GWEN_DB_SetIntValue(dbP, GWEN_DB_FLAGS_OVERWRITE_VARS, "debitAllowed", ae->debitAllowed);

  return 0;
}




void GWENHYWFAR_CB AO_Account_freeData(void *bp, void *p) {
  AO_ACCOUNT *ae;

  ae=(AO_ACCOUNT*) p;

  GWEN_FREE_OBJECT(ae);
}



int AO_Account_GetMaxPurposeLines(const AB_ACCOUNT *a){
  AO_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AO_ACCOUNT, a);
  assert(ae);

  return ae->maxPurposeLines;
}



void AO_Account_SetMaxPurposeLines(AB_ACCOUNT *a, int i){
  AO_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AO_ACCOUNT, a);
  assert(ae);

  ae->maxPurposeLines=i;
}



int AO_Account_GetDebitAllowed(const AB_ACCOUNT *a){
  AO_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AO_ACCOUNT, a);
  assert(ae);

  return ae->debitAllowed;
}



void AO_Account_SetDebitAllowed(AB_ACCOUNT *a, int i){
  AO_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AO_ACCOUNT, a);
  assert(ae);

  ae->debitAllowed=i;
}




