/***************************************************************************
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

#include <aqbanking/banking_be.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT(AB_ACCOUNT, AH_ACCOUNT)


int AH_Account_Extend(AB_ACCOUNT *a, AB_PROVIDER *pro,
                      AB_PROVIDER_EXTEND_MODE em,
                      GWEN_DB_NODE *dbBackend) {
  AH_ACCOUNT *ae;

  assert(a);

  if (em==AB_ProviderExtendMode_Create ||
      em==AB_ProviderExtendMode_Extend) {
    int rv;

    GWEN_NEW_OBJECT(AH_ACCOUNT, ae);
    GWEN_INHERIT_SETDATA(AB_ACCOUNT, AH_ACCOUNT, a, ae,
                         AH_Account_freeData);
    ae->hbci=AH_Provider_GetHbci(pro);

    if (em==AB_ProviderExtendMode_Create)
      ae->flags=AH_BANK_FLAGS_DEFAULT;
    else {
      /* update db to latest version */
      rv=AH_HBCI_UpdateDbAccount(ae->hbci, dbBackend);
      if (rv<0) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not update account db (%d)", rv);
        GWEN_Gui_ShowError(I18N("AqBanking Settings Database Error"),
                           I18N("Your settings database might be in an inconsistent state!"));
        return rv;
      }
      AH_Account_ReadDb(a, dbBackend);
      if (rv==1) {
	/* updated config, write it now */
	rv=AB_Banking_SaveAccountConfig(AB_Provider_GetBanking(pro), a, 1);
	if (rv<0) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not save account db (%d)", rv);
          GWEN_Gui_ShowError(I18N("AqBanking Settings Database Error"),
                             I18N("Your settings database might be in an inconsistent state!"));
          return rv;
        }
      }
    }
  }
  else if (em==AB_ProviderExtendMode_Reload) {
    AH_Account_ReadDb(a, dbBackend);
  }
  else if (em==AB_ProviderExtendMode_Save) {
    AH_Account_toDb(a, dbBackend);
  }

  return 0;
}



void GWENHYWFAR_CB AH_Account_freeData(void *bp, void *p) {
  AH_ACCOUNT *ae;

  ae=(AH_ACCOUNT*) p;
  GWEN_FREE_OBJECT(ae);
}



void AH_Account_ReadDb(AB_ACCOUNT *a, GWEN_DB_NODE *db) {
  AH_ACCOUNT *ae;
  const char *s;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  ae->flags=AH_Account_Flags_fromDb(db, "accountFlags");
  
  s=GWEN_DB_GetCharValue(db, "suffix", 0, NULL);
  if (s && *s) {
    ae->flags|=AH_BANK_FLAGS_KTV2;
    if (strcasecmp(s, "<empty>")!=0)
      AB_Account_SetSubAccountId(a, s);
  }
}



void AH_Account_toDb(AB_ACCOUNT *a, GWEN_DB_NODE *db) {
  AH_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  AH_Account_Flags_toDb(db, "accountFlags", ae->flags);
  GWEN_DB_DeleteVar(db, "suffix");
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





