/***************************************************************************
    begin       : Wed May 07 2008
    copyright   : (C) 2018 by Martin Preuss
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




AB_ACCOUNT *EBC_Account_new(AB_PROVIDER *pro) {
  AB_ACCOUNT *a;
  EBC_ACCOUNT *ae;

  a=AB_Account_new();
  assert(a);
  AB_Account_SetProvider(a, pro);
  AB_Account_SetBackendName(a, "aqebics");

  GWEN_NEW_OBJECT(EBC_ACCOUNT, ae);
  GWEN_INHERIT_SETDATA(AB_ACCOUNT, EBC_ACCOUNT, a, ae, EBC_Account_freeData);

  ae->readFromDbFn=AB_Account_SetReadFromDbFn(a, EBC_Account_ReadFromDb);
  ae->writeToDbFn=AB_Account_SetWriteToDbFn(a, EBC_Account_WriteToDb);

  return a;
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



int EBC_Account_ReadFromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db) {
  EBC_ACCOUNT *ae;
  GWEN_DB_NODE *dbP;
  int rv;
  AB_PROVIDER *pro;
  const char *s;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, EBC_ACCOUNT, a);
  assert(ae);

  /* save provider, because AB_Account_ReadFromDb clears it */
  pro=AB_Account_GetProvider(a);

  /* read data for base class */
  rv=(ae->readFromDbFn)(a, db);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* set provider again */
  AB_Account_SetProvider(a, pro);

  dbP=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "data/backend");

  /* read data for provider */
  ae->flags=EBC_Account_Flags_fromDb(dbP, "accountFlags");

  free(ae->ebicsId);
  s=GWEN_DB_GetCharValue(dbP, "ebicsId", 0, 0);
  if (s) ae->ebicsId=strdup(s);
  else ae->ebicsId=NULL;

  return 0;
}



int EBC_Account_WriteToDb(const AB_ACCOUNT *a, GWEN_DB_NODE *db) {
  EBC_ACCOUNT *ae;
  GWEN_DB_NODE *dbP;
  int rv;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, EBC_ACCOUNT, a);
  assert(ae);

  rv=(ae->writeToDbFn)(a, db);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* write data for provider */
  dbP=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "data/backend");

  EBC_Account_Flags_toDb(dbP, "accountFlags", ae->flags);
  if (ae->ebicsId)
    GWEN_DB_SetCharValue(dbP, GWEN_DB_FLAGS_OVERWRITE_VARS, "ebicsId", ae->ebicsId);

  return 0;
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









