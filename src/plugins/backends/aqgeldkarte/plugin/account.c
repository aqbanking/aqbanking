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
#include "aqgeldkarte_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT(AB_ACCOUNT, AG_ACCOUNT)



AB_ACCOUNT *AG_Account_new(AB_BANKING *ab,
                           AB_PROVIDER *pro,
                           const char *idForProvider){
  AB_ACCOUNT *a;
  AG_ACCOUNT *ad;

  a=AB_Account_new(ab, pro, idForProvider);
  GWEN_NEW_OBJECT(AG_ACCOUNT, ad);
  GWEN_INHERIT_SETDATA(AB_ACCOUNT, AG_ACCOUNT, a, ad, AG_Account_FreeData);

  return a;
}



void AG_Account_FreeData(void *bp, void *p) {
  AG_ACCOUNT *ad;

  ad=(AG_ACCOUNT*)p;
  free(ad->cardId);
  GWEN_FREE_OBJECT(ad);
}



AB_ACCOUNT *AG_Account_fromDb(AB_BANKING *ab,
                              GWEN_DB_NODE *db){
  AB_ACCOUNT *acc;
  AG_ACCOUNT *ad;
  GWEN_DB_NODE *dbBase;
  const char *s;

  dbBase=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "base");
  assert(dbBase);
  acc=AB_Account_fromDb(ab, dbBase);
  assert(acc);

  GWEN_NEW_OBJECT(AG_ACCOUNT, ad);
  GWEN_INHERIT_SETDATA(AB_ACCOUNT, AG_ACCOUNT, acc, ad, AG_Account_FreeData);

  /* read own data */
  s=GWEN_DB_GetCharValue(db, "cardId", 0, 0);
  if (s)
    ad->cardId=strdup(s);


  return acc;
}



int AG_Account_toDb(const AB_ACCOUNT *acc, GWEN_DB_NODE *db){
  AG_ACCOUNT *ad;
  GWEN_DB_NODE *dbBase;
  int rv;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AG_ACCOUNT, acc);
  assert(ad);

  dbBase=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "base");
  assert(dbBase);
  rv=AB_Account_toDb(acc, dbBase);
  if (rv) {
    DBG_INFO(AQGELDKARTE_LOGDOMAIN, "here");
    return rv;
  }


  /* write own data */
  if (ad->cardId)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "cardId", ad->cardId);

  return 0;
}



const char *AG_Account_GetCardId(const AB_ACCOUNT *acc){
  AG_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AG_ACCOUNT, acc);
  assert(ad);

  return ad->cardId;
}



void AG_Account_SetCardId(AB_ACCOUNT *acc, const char *s){
  AG_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AG_ACCOUNT, acc);
  assert(ad);

  free(ad->cardId);
  if (s) ad->cardId=strdup(s);
  else ad->cardId=0;
}












