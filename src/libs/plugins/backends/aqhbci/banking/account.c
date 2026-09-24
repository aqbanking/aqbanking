/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2026 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "account_p.h"

#include "aqhbci/aqhbci_l.h"
#include "aqhbci/banking/provider_l.h"
#include "aqhbci/msglayer/hbci-updates_l.h"
#include "aqhbci/banking/provider.h"

#include <aqbanking/banking_be.h>
#include <aqbanking/backendsupport/account.h>
#include <aqbanking/banking_l.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <assert.h>


/* ------------------------------------------------------------------------------------------------
 * global vars/statics
 * ------------------------------------------------------------------------------------------------
 */

GWEN_INHERIT(AB_ACCOUNT, AH_ACCOUNT)



static AB_FLAGDEF _accountFlagDefs[]={
  {AH_BANK_FLAGS_PREFER_SINGLE_TRANSFER,        "preferSingleTransfer"},
  {AH_BANK_FLAGS_PREFER_SINGLE_DEBITNOTE,       "preferSingleDebitNote"},
  {AH_BANK_FLAGS_KTV2,                          "ktv2"},
  {AH_BANK_FLAGS_SEPA,                          "sepa"},
  {AH_BANK_FLAGS_SEPA_PREFER_SINGLE_TRANSFER,   "sepaPreferSingleTransfer"},
  {AH_BANK_FLAGS_SEPA_PREFER_SINGLE_DEBITNOTE,  "sepaPreferSingleDebitNote"},
  {AH_BANK_FLAGS_PREFER_CAMT_DOWNLOAD,          "preferCamtDownload"},
  {AH_BANK_FLAGS_GETTRANS_FORCE_NTLACCOUNTINFO, "getTransForceNtlAccountInfo"},
  {0, NULL}
};



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

AB_ACCOUNT *AH_Account_new(AB_PROVIDER *pro)
{
  AB_ACCOUNT *a;
  AH_ACCOUNT *ae;

  a=AB_Account_new();
  assert(a);
  AB_Account_SetProvider(a, pro);
  AB_Account_SetBackendName(a, "aqhbci");

  GWEN_NEW_OBJECT(AH_ACCOUNT, ae);
  GWEN_INHERIT_SETDATA(AB_ACCOUNT, AH_ACCOUNT, a, ae, AH_Account_freeData);
  ae->flags=AH_BANK_FLAGS_DEFAULT;
  ae->hbci=AH_Provider_GetHbci(pro);

  ae->readFromDbFn=AB_Account_SetReadFromDbFn(a, AH_Account_ReadFromDb);
  ae->writeToDbFn=AB_Account_SetWriteToDbFn(a, AH_Account_WriteToDb);

  return a;
}



int AH_Account_ReadFromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db)
{
  AH_ACCOUNT *ae;
  GWEN_DB_NODE *dbP;
  int rv;
  const char *s;
  AB_PROVIDER *pro;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  /* save provider, because AB_Account_ReadFromDb clears it */
  pro=AB_Account_GetProvider(a);

  /* read data for base class */
  rv=(ae->readFromDbFn)(a, db);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* set provider again */
  AB_Account_SetProvider(a, pro);

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



int AH_Account_WriteToDb(const AB_ACCOUNT *a, GWEN_DB_NODE *db)
{
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



void GWENHYWFAR_CB AH_Account_freeData(void *bp, void *p)
{
  AH_ACCOUNT *ae;

  ae=(AH_ACCOUNT *) p;

  if (ae->dbTempUpd)
    GWEN_DB_Group_free(ae->dbTempUpd);

  GWEN_FREE_OBJECT(ae);
}



AH_HBCI *AH_Account_GetHbci(const AB_ACCOUNT *a)
{
  AH_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);
  return ae->hbci;
}



void AH_Account_Flags_toDb(GWEN_DB_NODE *db, const char *name, uint32_t flags)
{
  AB_Banking_FlagsToDb(db, _accountFlagDefs, name, flags);
}



uint32_t AH_Account_Flags_fromDb(GWEN_DB_NODE *db, const char *name)
{
  return AB_Banking_FlagsFromDb(db, _accountFlagDefs, name);
}



uint32_t AH_Account_GetFlags(const AB_ACCOUNT *a)
{
  AH_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  return ae->flags;
}



void AH_Account_SetFlags(AB_ACCOUNT *a, uint32_t flags)
{
  AH_ACCOUNT *ae;
  uint32_t aid;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  aid=AB_Account_GetUniqueId(a);
  if (ae->flags!=flags) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Changing flags of account %d (%08x) to %08x, was %08x",
             aid, aid, flags, ae->flags);
  }

  ae->flags=flags;
}



void AH_Account_AddFlags(AB_ACCOUNT *a, uint32_t flags)
{
  AH_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  AH_Account_SetFlags(a, ae->flags|flags);
}



void AH_Account_SubFlags(AB_ACCOUNT *a, uint32_t flags)
{
  AH_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  ae->flags&=~flags;
  AH_Account_SetFlags(a, (ae->flags&~flags));
}



GWEN_DB_NODE *AH_Account_GetDbTempUpd(const AB_ACCOUNT *a)
{
  AH_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  return ae->dbTempUpd;
}



void AH_Account_SetDbTempUpd(AB_ACCOUNT *a, GWEN_DB_NODE *db)
{
  AH_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AH_ACCOUNT, a);
  assert(ae);

  if (ae->dbTempUpd)
    GWEN_DB_Group_free(ae->dbTempUpd);
  ae->dbTempUpd=db;
}




