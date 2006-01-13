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


void AG_Account_Extend(AB_ACCOUNT *a, AB_PROVIDER *pro,
                       AB_PROVIDER_EXTEND_MODE em){
  AG_ACCOUNT *ad;

  if (em==AB_ProviderExtendMode_Create ||
      em==AB_ProviderExtendMode_Extend) {
    GWEN_NEW_OBJECT(AG_ACCOUNT, ad);
    GWEN_INHERIT_SETDATA(AB_ACCOUNT, AG_ACCOUNT, a, ad, AG_Account_FreeData);
  }
}



void AG_Account_FreeData(void *bp, void *p) {
  AG_ACCOUNT *ad;

  ad=(AG_ACCOUNT*)p;
  GWEN_FREE_OBJECT(ad);
}



const char *AG_Account_GetCardId(const AB_ACCOUNT *a){
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);

  return GWEN_DB_GetCharValue(db, "cardId", 0, 0);
}



void AG_Account_SetCardId(AB_ACCOUNT *a, const char *s){
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);

  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "cardId", s);
  else
    GWEN_DB_DeleteVar(db, "cardId");
}












