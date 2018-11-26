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
#include "aqhbci_l.h"
#include "provider_l.h"
#include "hbci-updates_l.h"
#include <aqhbci/provider.h>

#include <aqbanking/banking_be.h>
#include <aqbanking/account.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT(AB_ACCOUNT, AH_ACCOUNT)


AB_ACCOUNT *AH_Account_new(AB_PROVIDER *pro) {
  AB_ACCOUNT *a;
  AH_ACCOUNT *ae;

  a=AB_Account_new();
  assert(a);
  AB_Account_SetProvider(a, pro);
  GWEN_NEW_OBJECT(AH_ACCOUNT, ae);
  GWEN_INHERIT_SETDATA(AB_ACCOUNT, AH_ACCOUNT, a, ae, AH_Account_freeData);
  ae->flags=AH_BANK_FLAGS_DEFAULT;
  ae->hbci=AH_Provider_GetHbci(pro);

  ae->readFromDbFn=AB_Account_SetReadFromDbFn(a, AH_Account_ReadFromDb);
  ae->writeToDbFn=AB_Account_SetWriteToDbFn(a, AH_Account_WriteToDb);

  return a;
}



int AH_Account_ReadFromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db) {
  AH_ACCOUNT *ae;
  GWEN_DB_NODE *dbP;
  int rv;
  const char *s;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  rv=(ae->readFromDbFn)(a, db);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* read data for provider */
  dbP=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "data/backend");

  ae->flags=AH_Account_Flags_fromDb(dbP, "accountFlags");

  s=GWEN_DB_GetCharValue(dbP, "suffix", 0, NULL);
  if (s && *s) {
    ae->flags|=AH_BANK_FLAGS_KTV2;
    if (strcasecmp(s, "<empty>")!=0)
      AB_Account_SetSubAccountId(a, s);
  }

  return 0;
}



int AH_Account_WriteToDb(const AB_ACCOUNT *a, GWEN_DB_NODE *db) {
  AH_ACCOUNT *ae;
  GWEN_DB_NODE *dbP;
  int rv;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  rv=(ae->writeToDbFn)(a, db);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* write data for provider */
  dbP=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "data/backend");

  AH_Account_Flags_toDb(dbP, "accountFlags", ae->flags);
  GWEN_DB_DeleteVar(dbP, "suffix");

  return 0;
}



void GWENHYWFAR_CB AH_Account_freeData(void *bp, void *p) {
  AH_ACCOUNT *ae;

  ae=(AH_ACCOUNT*) p;

  if (ae->dbTempUpd)
    GWEN_DB_Group_free(ae->dbTempUpd);

  GWEN_FREE_OBJECT(ae);
}



AH_HBCI *AH_Account_GetHbci(const AB_ACCOUNT *a) {
  AH_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);
  return ae->hbci;
}



void AH_Account_Flags_toDb(GWEN_DB_NODE *db, const char *name,
                           uint32_t flags) {
  GWEN_DB_DeleteVar(db, name);
  if (flags & AH_BANK_FLAGS_PREFER_SINGLE_TRANSFER)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name,
                         "preferSingleTransfer");
  if (flags & AH_BANK_FLAGS_PREFER_SINGLE_DEBITNOTE)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name,
                         "preferSingleDebitNote");
  if (flags & AH_BANK_FLAGS_KTV2)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name,
                         "ktv2");
  if (flags & AH_BANK_FLAGS_SEPA)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name,
                         "sepa");
  if (flags & AH_BANK_FLAGS_SEPA_PREFER_SINGLE_TRANSFER)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name,
                         "sepaPreferSingleTransfer");
  if (flags & AH_BANK_FLAGS_SEPA_PREFER_SINGLE_DEBITNOTE)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, name,
                         "sepaPreferSingleDebitNote");
}



uint32_t AH_Account_Flags_fromDb(GWEN_DB_NODE *db, const char *name) {
  uint32_t fl=0;
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
    else if (strcasecmp(s, "ktv2")==0)
      fl|=AH_BANK_FLAGS_KTV2;
    else if (strcasecmp(s, "sepa")==0)
      fl|=AH_BANK_FLAGS_SEPA;
    else if (strcasecmp(s, "sepaPreferSingleTransfer")==0)
      fl|=AH_BANK_FLAGS_SEPA_PREFER_SINGLE_TRANSFER;
    else if (strcasecmp(s, "sepaPreferSingleDebitNote")==0)
      fl|=AH_BANK_FLAGS_SEPA_PREFER_SINGLE_DEBITNOTE;
    else {
      DBG_WARN(AQHBCI_LOGDOMAIN, "Unknown account flag \"%s\"", s);
    }
  }

  return fl;
}



uint32_t AH_Account_GetFlags(const AB_ACCOUNT *a) {
  AH_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  return ae->flags;
}



void AH_Account_SetFlags(AB_ACCOUNT *a, uint32_t flags) {
  AH_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  ae->flags=flags;
}



void AH_Account_AddFlags(AB_ACCOUNT *a, uint32_t flags) {
  AH_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  ae->flags|=flags;
}



void AH_Account_SubFlags(AB_ACCOUNT *a, uint32_t flags) {
  AH_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  ae->flags&=~flags;
}



GWEN_DB_NODE *AH_Account_GetDbTempUpd(const AB_ACCOUNT *a) {
  AH_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  return ae->dbTempUpd;
}



void AH_Account_SetDbTempUpd(AB_ACCOUNT *a, GWEN_DB_NODE *db) {
  AH_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  if (ae->dbTempUpd)
    GWEN_DB_Group_free(ae->dbTempUpd);
  ae->dbTempUpd=db;
}




