/***************************************************************************
    begin       : Wed May 07 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "account_p.h"
#include "provider_l.h"

#include <gwenhywfar/debug.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT(AB_ACCOUNT, EBC_ACCOUNT)




void EBC_Account_Extend(AB_ACCOUNT *a, GWEN_UNUSED AB_PROVIDER *pro,
			AB_PROVIDER_EXTEND_MODE em,
			GWEN_DB_NODE *db) {
  if (em==AB_ProviderExtendMode_Create ||
      em==AB_ProviderExtendMode_Extend) {
    EBC_ACCOUNT *ae;

    GWEN_NEW_OBJECT(EBC_ACCOUNT, ae);
    GWEN_INHERIT_SETDATA(AB_ACCOUNT, EBC_ACCOUNT, a, ae, EBC_Account_freeData);

    if (em==AB_ProviderExtendMode_Create) {
    }
    else {
      EBC_Account_ReadDb(a, db);
    }
  }
  else {
    if (em==AB_ProviderExtendMode_Add) {
    }
    else if (em==AB_ProviderExtendMode_Save) {
      EBC_Account_toDb(a, db);
    } /* if save */
  }
}



void GWENHYWFAR_CB EBC_Account_freeData(GWEN_UNUSED void *bp, void *p) {
  EBC_ACCOUNT *ae;

  ae=(EBC_ACCOUNT*)p;
  free(ae->ebicsId);
  GWEN_FREE_OBJECT(ae);
}



void EBC_Account_Flags_toDb(GWEN_DB_NODE *db, const char *name,
			    uint32_t flags) {
  GWEN_DB_DeleteVar(db, name);
  if (flags & EBC_ACCOUNT_FLAGS_STA_SPP)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "staSpp");
  if (flags & EBC_ACCOUNT_FLAGS_IZV_SPP)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name, "izvSpp");
}



uint32_t EBC_Account_Flags_fromDb(GWEN_DB_NODE *db, const char *name){
  uint32_t fl=0;
  int i;

  for (i=0; ; i++) {
    const char *s;

    s=GWEN_DB_GetCharValue(db, name, i, 0);
    if (!s)
      break;
    if (strcasecmp(s, "staSpp")==0)
      fl|=EBC_ACCOUNT_FLAGS_STA_SPP;
    else if (strcasecmp(s, "izvSpp")==0)
      fl|=EBC_ACCOUNT_FLAGS_IZV_SPP;
    else {
      DBG_WARN(AQEBICS_LOGDOMAIN, "Unknown user flag \"%s\"", s);
    }
  }

  return fl;
}



void EBC_Account_ReadDb(AB_ACCOUNT *a, GWEN_DB_NODE *db) {
  EBC_ACCOUNT *ae;
  const char *s;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, EBC_ACCOUNT, a);
  assert(ae);

  ae->flags=EBC_Account_Flags_fromDb(db, "accountFlags");

  free(ae->ebicsId);
  s=GWEN_DB_GetCharValue(db, "ebicsId", 0, 0);
  if (s) ae->ebicsId=strdup(s);
  else ae->ebicsId=NULL;
}



void EBC_Account_toDb(AB_ACCOUNT *a, GWEN_DB_NODE *db) {
  EBC_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, EBC_ACCOUNT, a);
  assert(ae);

  EBC_Account_Flags_toDb(db, "accountFlags", ae->flags);

  if (ae->ebicsId)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "ebicsId", ae->ebicsId);
}



const char *EBC_Account_GetEbicsId(const AB_ACCOUNT *a) {
  EBC_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, EBC_ACCOUNT, a);
  assert(ae);

  return ae->ebicsId;
}



void EBC_Account_SetEbicsId(AB_ACCOUNT *a, const char *s) {
  EBC_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, EBC_ACCOUNT, a);
  assert(ae);

  free(ae->ebicsId);
  if (s) ae->ebicsId=strdup(s);
  else ae->ebicsId=NULL;
}



uint32_t EBC_Account_GetFlags(const AB_ACCOUNT *a) {
  EBC_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, EBC_ACCOUNT, a);
  assert(ae);

  return ae->flags;
}



void EBC_Account_SetFlags(AB_ACCOUNT *a, uint32_t flags) {
  EBC_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, EBC_ACCOUNT, a);
  assert(ae);

  ae->flags=flags;
}



void EBC_Account_AddFlags(AB_ACCOUNT *a, uint32_t flags) {
  EBC_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, EBC_ACCOUNT, a);
  assert(ae);

  ae->flags|=flags;
}



void EBC_Account_SubFlags(AB_ACCOUNT *a, uint32_t flags) {
  EBC_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, EBC_ACCOUNT, a);
  assert(ae);

  ae->flags&=~flags;
}









