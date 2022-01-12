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


#include "job_commit_account.h"
#include "aqhbci/banking/user_l.h"
#include "aqhbci/banking/account_l.h"
#include "aqhbci/banking/provider_l.h"
#include "aqhbci/banking/provider_accspec.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static AB_ACCOUNT_LIST *_readAccounts(AB_PROVIDER *pro, AB_USER *user, GWEN_DB_NODE *dbResponses);
static AB_ACCOUNT *_readAndSanitizeAccountData(AB_PROVIDER *pro, AH_BPD *bpd, GWEN_DB_NODE *dbAccountData);
static void _removeEmpty(AB_ACCOUNT_LIST *accList);
static void _matchAccountsWithStoredAccountsAndAssignStoredId(AB_PROVIDER *pro, AB_ACCOUNT_LIST *accList);
static uint32_t _findStored(AB_PROVIDER *pro, const AB_ACCOUNT *acc, AB_ACCOUNT_SPEC_LIST *asl);
static void _addOrModify(AB_PROVIDER *pro, AB_USER *user, AB_ACCOUNT *acc);
static void _possiblyReplaceUpdJobsForAccountInLockedUser(AB_USER *user, AB_ACCOUNT *storedAcc,
                                                          GWEN_DB_NODE *dbTempUpd);
static AB_ACCOUNT *_getLoadedAndUpdatedOrCreatedAccount(AB_PROVIDER *pro, AB_USER *user, AB_ACCOUNT *acc);
static void _possiblyUpdateAndWriteAccountSpec(AB_PROVIDER *pro, AB_USER *user, AB_ACCOUNT *storedAcc);
static void _updateAccountInfo(AB_ACCOUNT *targetAccount, const AB_ACCOUNT *sourceAccount);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



void AH_Job_Commit_Accounts(AH_JOB *j)
{
  AB_PROVIDER *pro;
  AB_USER *user;
  AB_ACCOUNT_LIST *accList;

  pro=AH_Job_GetProvider(j);
  user=AH_Job_GetUser(j);
  accList=_readAccounts(pro, user, AH_Job_GetResponses(j));
  if (accList) {
    _removeEmpty(accList);
    _matchAccountsWithStoredAccountsAndAssignStoredId(pro, accList);

    /* now either add new accounts or modify existing ones */
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Adding new or modifying existing accounts");
    if (AB_Account_List_GetCount(accList)) {
      AB_ACCOUNT *acc;

      while ((acc=AB_Account_List_First(accList))) {
        AB_Account_List_Del(acc);
        _addOrModify(pro, user, acc);
        AB_Account_free(acc);
      } /* while */
    } /* if accounts */
    AB_Account_List_free(accList);
  }
}



AB_ACCOUNT_LIST *_readAccounts(AB_PROVIDER *pro, AB_USER *user, GWEN_DB_NODE *dbResponses)
{
  AB_ACCOUNT_LIST *accList;
  AH_BPD *bpd;
  GWEN_DB_NODE *dbCurr;

  bpd=AH_User_GetBpd(user);

  accList=AB_Account_List_new();

  dbCurr=GWEN_DB_FindFirstGroup(dbResponses, "AccountData");
  while (dbCurr) {
    AB_ACCOUNT *acc;

    acc=_readAndSanitizeAccountData(pro, bpd, GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data/AccountData"));
    if (acc)
      AB_Account_List_Add(acc, accList);

    dbCurr=GWEN_DB_FindNextGroup(dbCurr, "AccountData");
  }

  if (AB_Account_List_GetCount(accList)<1) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No account received.");
    AB_Account_List_free(accList);
    return NULL;
  }

  return accList;
}



AB_ACCOUNT *_readAndSanitizeAccountData(AB_PROVIDER *pro, AH_BPD *bpd, GWEN_DB_NODE *dbAccountData)
{
  if (dbAccountData) {
    AB_ACCOUNT *acc;
    GWEN_DB_NODE *dbUpd;
    GWEN_DB_NODE *gr;

    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Found an account");

    /* account data found */
    acc=AB_Provider_CreateAccountObject(pro);
    assert(acc);

    /* read info from "AccountData" segment */
    AH_Job_ReadAccountDataSeg(acc, dbAccountData);

    /* set bank name */
    if (bpd) {
      const char *s;

      s=AH_Bpd_GetBankName(bpd);
      if (s && *s)
        AB_Account_SetBankName(acc, s);
    }

    /* Fixes a bug where the bank sends an account with no bank & account name */
    if (!AB_Account_GetBankName(acc))
      AB_Account_SetBankName(acc, "dummy");
    if (!AB_Account_GetAccountName(acc))
      AB_Account_SetAccountName(acc, "dummy");

    /* temporarily store UPD jobs */
    dbUpd=GWEN_DB_Group_new("tmpUpd");
    assert(dbUpd);

    gr=GWEN_DB_GetFirstGroup(dbAccountData);
    while (gr) {
      if (strcasecmp(GWEN_DB_GroupName(gr), "updjob")==0)
        GWEN_DB_AddGroup(dbUpd, GWEN_DB_Group_dup(gr));
      gr=GWEN_DB_GetNextGroup(gr);
    }
    AH_Account_SetDbTempUpd(acc, dbUpd);

    return acc;
  }
  else
    return NULL;
}



static void _removeEmpty(AB_ACCOUNT_LIST *accList)
{
  /* only keep accounts which have at least IBAN or bankcode and account number */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Checking for empty accounts");
  if (AB_Account_List_GetCount(accList)) {
    AB_ACCOUNT *acc;

    acc=AB_Account_List_First(accList);
    while (acc) {
      AB_ACCOUNT *accNext;
      const char *accountNum;
      const char *bankCode;
      const char *iban;

      accNext=AB_Account_List_Next(acc);
      accountNum=AB_Account_GetAccountNumber(acc);
      bankCode=AB_Account_GetBankCode(acc);
      iban=AB_Account_GetIban(acc);

      if (!((iban && *iban) || (accountNum && *accountNum && bankCode && *bankCode))) {
        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Removing empty account from import list");
        AB_Account_List_Del(acc);
        AB_Account_free(acc);
      }
      acc=accNext;
    } /* while(acc) */
  } /* if (AB_Account_List_GetCount(accList)) */
}



void _matchAccountsWithStoredAccountsAndAssignStoredId(AB_PROVIDER *pro, AB_ACCOUNT_LIST *accList)
{
  /* find out which accounts are new */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Checking for existing or to be added accounts");
  if (AB_Account_List_GetCount(accList)) {
    AB_BANKING *ab;
    AB_ACCOUNT_SPEC_LIST *accountSpecList=NULL;
    int rv;

    ab=AB_Provider_GetBanking(pro);
    accountSpecList=AB_AccountSpec_List_new();
    rv=AB_Banking_GetAccountSpecList(ab, &accountSpecList);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "No account spec list");
    }
    else {
      AB_ACCOUNT *acc;

      acc=AB_Account_List_First(accList);
      while (acc) {
        uint32_t storedUid;

        storedUid=_findStored(pro, acc, accountSpecList);
        if (storedUid) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Found a matching account (%x, %lu)", storedUid, (long unsigned int) storedUid);
          AB_Account_SetUniqueId(acc, storedUid);
        }

        acc=AB_Account_List_Next(acc);
      }
    }
    AB_AccountSpec_List_free(accountSpecList);
  }
}



static uint32_t _findStored(AB_PROVIDER *pro, const AB_ACCOUNT *acc, AB_ACCOUNT_SPEC_LIST *asl)
{
  const char *accountNum;
  const char *bankCode;
  const char *iban;
  int accountType;
  AB_ACCOUNT_SPEC *as=NULL;

  accountNum=AB_Account_GetAccountNumber(acc);
  bankCode=AB_Account_GetBankCode(acc);
  iban=AB_Account_GetIban(acc);
  accountType=AB_Account_GetAccountType(acc);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Checking account [blz=%s, acc=%s, iban=%s, type=%d]",
           bankCode?bankCode:"<none>",
           accountNum?accountNum:"<none>",
           iban?iban:"<none>",
           accountType);

  if (iban && *iban && accountType>AB_AccountType_Unknown) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Comparing IBAN and old account specs");
    /* IBAN given, try that first */
    as=AB_AccountSpec_List_FindFirst(asl,
                                     AB_Provider_GetName(pro),
                                     NULL,                         /* country */
                                     NULL,                         /* bank code */
                                     NULL,                         /* account number */
                                     NULL,                         /* subAccountId */
                                     AB_Account_GetIban(acc),     /* iban */
                                     NULL, /* any currency */
                                     accountType);
  }

  if (as==NULL) {
    if (accountNum && *accountNum && bankCode && *bankCode && accountType>AB_AccountType_Unknown) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Comparing old account specs");
      as=AB_AccountSpec_List_FindFirst(asl,
                                       AB_Provider_GetName(pro),
                                       AB_Account_GetCountry(acc),
                                       AB_Account_GetBankCode(acc),
                                       AB_Account_GetAccountNumber(acc),
                                       AB_Account_GetSubAccountId(acc),
                                       NULL,
                                       NULL, /* any currency */
                                       accountType);
    }
  }

  if (as) {
    uint32_t uniqueId;

    uniqueId=AB_AccountSpec_GetUniqueId(as);
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Found a matching account (%lu)", (long unsigned int) uniqueId);
    return uniqueId;
  }

  return 0;
}


static void _addOrModify(AB_PROVIDER *pro, AB_USER *user, AB_ACCOUNT *acc)
{
  AB_ACCOUNT *storedAcc;
  GWEN_DB_NODE *dbTempUpd;

  dbTempUpd=AH_Account_GetDbTempUpd(acc);
  if (dbTempUpd)
    dbTempUpd=GWEN_DB_Group_dup(dbTempUpd);
  storedAcc=_getLoadedAndUpdatedOrCreatedAccount(pro, user, acc);
  _possiblyReplaceUpdJobsForAccountInLockedUser(user, storedAcc, dbTempUpd);
  _possiblyUpdateAndWriteAccountSpec(pro, user, storedAcc);

  GWEN_DB_Group_free(dbTempUpd); /* is a copy, we need to free it */
  AB_Account_free(storedAcc);
}



void _possiblyReplaceUpdJobsForAccountInLockedUser(AB_USER *user, AB_ACCOUNT *storedAcc, GWEN_DB_NODE *dbTempUpd)
{
  /* replace UPD jobs for this account inside user (user should be locked outside this function) */
  if (storedAcc && dbTempUpd) {
    GWEN_DB_NODE *dbUpd;
    GWEN_DB_NODE *gr;
    char numbuf[32];

    DBG_INFO(AQHBCI_LOGDOMAIN, "Setting UPD jobs for account %u in user %u",
             (unsigned int) AB_Account_GetUniqueId(storedAcc),
             (unsigned int) AB_User_GetUniqueId(user));

    /* get UPD jobs */
    dbUpd=AH_User_GetUpd(user);
    assert(dbUpd);

    /* create and clear group for each account */
    snprintf(numbuf, sizeof(numbuf)-1, "uaid-%08" PRIx32,
             AB_Account_GetUniqueId(storedAcc));
    numbuf[sizeof(numbuf)-1]=0;

    dbUpd=GWEN_DB_GetGroup(dbUpd, GWEN_DB_FLAGS_OVERWRITE_GROUPS, numbuf);

    gr=GWEN_DB_GetFirstGroup(dbTempUpd);
    while (gr) {
      if (strcasecmp(GWEN_DB_GroupName(gr), "updjob")==0) {
        /* found an upd job */
        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Adding UPD job");
        GWEN_DB_AddGroup(dbUpd, GWEN_DB_Group_dup(gr));
      }
      gr=GWEN_DB_GetNextGroup(gr);
    } /* while */
  }
}



AB_ACCOUNT *_getLoadedAndUpdatedOrCreatedAccount(AB_PROVIDER *pro, AB_USER *user, AB_ACCOUNT *acc)
{
  if (AB_Account_GetUniqueId(acc)) {
    int rv;
    AB_ACCOUNT *storedAcc=NULL;

    /* account already exists, needs update */
    DBG_INFO(AQHBCI_LOGDOMAIN, "Account exists, modifying");
    rv=AB_Provider_GetAccount(pro, AB_Account_GetUniqueId(acc), 1, 0, &storedAcc); /* lock, don't unlock */
    if (rv<0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error getting referred account (%d)", rv);
      return NULL;
    }
    else {
      /* account is locked now, apply changes */
      _updateAccountInfo(storedAcc, acc);
      AB_Account_SetUserId(storedAcc, AB_User_GetUniqueId(user));

      /* unlock account */
      rv=AB_Provider_EndExclUseAccount(pro, storedAcc, 0);
      if (rv<0) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        AB_Provider_EndExclUseAccount(pro, storedAcc, 1); /* abort */
        AB_Account_free(storedAcc);
        return NULL;
      }

      return storedAcc;
    }
  }
  else {
    int rv;

    /* account is new, add it */
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Account is new, adding");
    AB_Account_SetUserId(acc, AB_User_GetUniqueId(user));
    rv=AB_Provider_AddAccount(pro, acc, 0); /* do not lock corresponding user because it already is locked! */
    if (rv<0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Coud not write new account (%d)", rv);
      return NULL;
    }
    else {
      AB_ACCOUNT *storedAcc=NULL;

      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Reading back added account");
      rv=AB_Provider_GetAccount(pro, AB_Account_GetUniqueId(acc), 0, 0, &storedAcc); /* no-lock, no-unlock */
      if (rv<0) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Error getting referred account (%d)", rv);
        return NULL;
      }
      return storedAcc;
    }
  }
}



void _possiblyUpdateAndWriteAccountSpec(AB_PROVIDER *pro, AB_USER *user, AB_ACCOUNT *storedAcc)
{
  /* update and write account spec */
  if (storedAcc) {
    int rv;

    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Updating account spec for account %u in user %u",
               (unsigned int) AB_Account_GetUniqueId(storedAcc),
               (unsigned int) AB_User_GetUniqueId(user));

    rv=AH_Provider_CreateAndWriteAccountSpecWithUserAndAccount(pro, user, storedAcc);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    }
  }
}



void _updateAccountInfo(AB_ACCOUNT *targetAccount, const AB_ACCOUNT *sourceAccount)
{
  const char *s;

  s=AB_Account_GetCountry(sourceAccount);
  if (s && *s)
    AB_Account_SetCountry(targetAccount, s);

  s=AB_Account_GetBankCode(sourceAccount);
  if (s && *s)
    AB_Account_SetBankCode(targetAccount, s);

  s=AB_Account_GetBankName(sourceAccount);
  if (s && *s)
    AB_Account_SetBankName(targetAccount, s);

  s=AB_Account_GetAccountNumber(sourceAccount);
  if (s && *s)
    AB_Account_SetAccountNumber(targetAccount, s);

  s=AB_Account_GetSubAccountId(sourceAccount);
  if (s && *s)
    AB_Account_SetSubAccountId(targetAccount, s);

  s=AB_Account_GetIban(sourceAccount);
  if (s && *s)
    AB_Account_SetIban(targetAccount, s);

  s=AB_Account_GetBic(sourceAccount);
  if (s && *s)
    AB_Account_SetBic(targetAccount, s);

  s=AB_Account_GetOwnerName(sourceAccount);
  if (s && *s)
    AB_Account_SetOwnerName(targetAccount, s);

  s=AB_Account_GetCurrency(sourceAccount);
  if (s && *s)
    AB_Account_SetCurrency(targetAccount, s);

  AB_Account_SetAccountType(targetAccount, AB_Account_GetAccountType(sourceAccount));

  /* use flags from new account */
  AH_Account_AddFlags(targetAccount, AH_Account_GetFlags(sourceAccount));
}



