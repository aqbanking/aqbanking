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


#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT(AB_ACCOUNT, AO_ACCOUNT)



AB_ACCOUNT *AO_Account_new(AB_BANKING *ab,
                           AB_PROVIDER *pro,
                           const char *idForProvider){
  AB_ACCOUNT *a;
  AO_ACCOUNT *ad;

  a=AB_Account_new(ab, pro, idForProvider);
  GWEN_NEW_OBJECT(AO_ACCOUNT, ad);
  GWEN_INHERIT_SETDATA(AB_ACCOUNT, AO_ACCOUNT, a, ad, AO_Account_FreeData);

  return a;
}



void AO_Account_FreeData(void *bp, void *p) {
  AO_ACCOUNT *ad;

  ad=(AO_ACCOUNT*)p;
  free(ad->userId);
  GWEN_FREE_OBJECT(ad);
}



AB_ACCOUNT *AO_Account_fromDb(AB_BANKING *ab, GWEN_DB_NODE *db){
  AB_ACCOUNT *acc;
  AO_ACCOUNT *ad;
  GWEN_DB_NODE *dbBase;
  const char *s;

  dbBase=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "base");
  assert(dbBase);
  acc=AB_Account_fromDb(ab, dbBase);
  assert(acc);

  GWEN_NEW_OBJECT(AO_ACCOUNT, ad);
  GWEN_INHERIT_SETDATA(AB_ACCOUNT, AO_ACCOUNT, acc, ad, AO_Account_FreeData);

  /* read own data */
  ad->maxPurposeLines=GWEN_DB_GetIntValue(db, "maxPurposeLines", 0, 4);
  ad->debitAllowed=GWEN_DB_GetIntValue(db, "debitAllowed", 0, 0);
  s=GWEN_DB_GetCharValue(db, "userId", 0, 0);
  assert(s);
  ad->userId=strdup(s);

  return acc;
}



int AO_Account_toDb(const AB_ACCOUNT *acc, GWEN_DB_NODE *db){
  AO_ACCOUNT *ad;
  GWEN_DB_NODE *dbBase;
  int rv;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AO_ACCOUNT, acc);
  assert(ad);

  dbBase=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "base");
  assert(dbBase);
  rv=AB_Account_toDb(acc, dbBase);
  if (rv) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here");
    return rv;
  }

  /* write own data */
  if (ad->userId)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "userId", ad->userId);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "maxPurposeLines", ad->maxPurposeLines);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "debitAllowed", ad->debitAllowed);

  return 0;
}



int AO_Account_GetMaxPurposeLines(const AB_ACCOUNT *acc){
  AO_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AO_ACCOUNT, acc);
  assert(ad);

  return ad->maxPurposeLines;
}



void AO_Account_SetMaxPurposeLines(AB_ACCOUNT *acc, int i){
  AO_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AO_ACCOUNT, acc);
  assert(ad);

  ad->maxPurposeLines=i;
}



int AO_Account_GetDebitAllowed(const AB_ACCOUNT *acc){
  AO_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AO_ACCOUNT, acc);
  assert(ad);

  return ad->debitAllowed;
}



void AO_Account_SetDebitAllowed(AB_ACCOUNT *acc, int i){
  AO_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AO_ACCOUNT, acc);
  assert(ad);

  ad->debitAllowed=i;
}



const char *AO_Account_GetUserId(const AB_ACCOUNT *acc){
  AO_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AO_ACCOUNT, acc);
  assert(ad);

  return ad->userId;
}



void AO_Account_SetUserId(AB_ACCOUNT *acc, const char *s){
  AO_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AO_ACCOUNT, acc);
  assert(ad);

  free(ad->userId);
  if (s) ad->userId=strdup(s);
  else ad->userId=0;
}










