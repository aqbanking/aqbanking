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
#include "provider_l.h"
#include "hbci-updates_l.h"
#include <aqhbci/provider.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT(AB_ACCOUNT, AH_ACCOUNT)


void AH_Account_Extend(AB_ACCOUNT *a, AB_PROVIDER *pro,
                       AB_PROVIDER_EXTEND_MODE em) {
  AH_ACCOUNT *ae;
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);

  if (em==AB_ProviderExtendMode_Create ||
      em==AB_ProviderExtendMode_Extend) {
    int rv;

    GWEN_NEW_OBJECT(AH_ACCOUNT, ae);
    GWEN_INHERIT_SETDATA(AB_ACCOUNT, AH_ACCOUNT, a, ae,
                         AH_Account_freeData);
    ae->hbci=AH_Provider_GetHbci(pro);

    /* update db to latest version */
    rv=AH_HBCI_UpdateDbAccount(ae->hbci, db);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not update account db (%d)", rv);
      assert(0);
    }

    if (em==AB_ProviderExtendMode_Create)
      ae->flags=AH_BANK_FLAGS_DEFAULT;
    else
      ae->flags=AH_Account_Flags_fromDb(db, "accountFlags");
  }
}



void GWENHYWFAR_CB AH_Account_freeData(void *bp, void *p) {
  AH_ACCOUNT *ae;

  ae=(AH_ACCOUNT*) p;
  GWEN_FREE_OBJECT(ae);
}



AH_HBCI *AH_Account_GetHbci(const AB_ACCOUNT *a) {
  AH_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);
  return ae->hbci;
}



const char *AH_Account_GetSuffix(const AB_ACCOUNT *a){
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  return GWEN_DB_GetCharValue(db, "suffix", 0, 0);
}



void AH_Account_SetSuffix(AB_ACCOUNT *a, const char *s){
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "suffix", s);
  else
    GWEN_DB_DeleteVar(db, "suffix");
}



void AH_Account_Flags_toDb(GWEN_DB_NODE *db, const char *name,
                           GWEN_TYPE_UINT32 flags) {
  GWEN_DB_DeleteVar(db, name);
  if (flags & AH_BANK_FLAGS_PREFER_SINGLE_TRANSFER)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name,
                         "preferSingleTransfer");
  if (flags & AH_BANK_FLAGS_PREFER_SINGLE_DEBITNOTE)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name,
                         "preferSingleDebitNote");
}



GWEN_TYPE_UINT32 AH_Account_Flags_fromDb(GWEN_DB_NODE *db, const char *name) {
  GWEN_TYPE_UINT32 fl=0;
  int i;

  for (i=0; ; i++) {
    const char *s;

    s=GWEN_DB_GetCharValue(db, name, i, 0);
    if (!s)
      break;
    if (strcasecmp(s, "preferSingleTransfer")==0)
      fl|=AH_BANK_FLAGS_PREFER_SINGLE_TRANSFER;
    else if (strcasecmp(s, "preferSingleDebitNote")==0)
      fl|=AH_BANK_FLAGS_PREFER_SINGLE_DEBITNOTE;
    else {
      DBG_WARN(AQHBCI_LOGDOMAIN, "Unknown account flag \"%s\"", s);
    }
  }

  return fl;
}



GWEN_TYPE_UINT32 AH_Account_GetFlags(const AB_ACCOUNT *a) {
  AH_ACCOUNT *ae;
  GWEN_DB_NODE *db;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  db=AB_Account_GetProviderData(a);
  assert(db);

  ae->flags=AH_Account_Flags_fromDb(db, "accountFlags");
  return ae->flags;
}



void AH_Account_SetFlags(AB_ACCOUNT *a, GWEN_TYPE_UINT32 flags) {
  AH_ACCOUNT *ae;
  GWEN_DB_NODE *db;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  db=AB_Account_GetProviderData(a);
  assert(db);

  ae->flags=flags;
  AH_Account_Flags_toDb(db, "accountFlags", ae->flags);
}



void AH_Account_AddFlags(AB_ACCOUNT *a, GWEN_TYPE_UINT32 flags) {
  AH_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  AH_Account_SetFlags(a, ae->flags | flags);
}



void AH_Account_SubFlags(AB_ACCOUNT *a, GWEN_TYPE_UINT32 flags) {
  AH_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  AH_Account_SetFlags(a, ae->flags & ~flags);
}





