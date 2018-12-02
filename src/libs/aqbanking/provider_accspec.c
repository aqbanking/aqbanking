/***************************************************************************
 begin       : Sun Dec 02 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


/*
 * This file is included by provider.c
 */


int AB_Provider_AccountToAccountSpec(AB_PROVIDER *pro, const AB_ACCOUNT *acc, AB_ACCOUNT_SPEC *as, int doLock) {
  int rv;

  assert(acc);
  assert(as);

  AB_AccountSpec_SetType(as, AB_Account_GetAccountType(acc));
  AB_AccountSpec_SetUniqueId(as, AB_Account_GetUniqueId(acc));
  AB_AccountSpec_SetBackendName(as, AB_Account_GetBackendName(acc));
  AB_AccountSpec_SetOwnerName(as, AB_Account_GetOwnerName(acc));
  AB_AccountSpec_SetAccountName(as, AB_Account_GetAccountName(acc));
  AB_AccountSpec_SetCurrency(as, AB_Account_GetCurrency(acc));
  AB_AccountSpec_SetIban(as, AB_Account_GetIban(acc));
  AB_AccountSpec_SetBic(as, AB_Account_GetBic(acc));

  AB_AccountSpec_SetCountry(as, AB_Account_GetCountry(acc));
  AB_AccountSpec_SetBankCode(as, AB_Account_GetBankCode(acc));
  AB_AccountSpec_SetAccountNumber(as, AB_Account_GetAccountNumber(acc));
  AB_AccountSpec_SetSubAccountNumber(as, AB_Account_GetSubAccountId(acc));

  rv=AB_Provider_UpdateAccountSpec(pro, as, doLock);
  if (rv<0 && rv!=GWEN_ERROR_NOT_IMPLEMENTED) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Provider_WriteAccountSpecForAccount(AB_PROVIDER *pro, const AB_ACCOUNT *acc, int doLock) {
  AB_ACCOUNT_SPEC *as;
  int rv;

  as=AB_AccountSpec_new();
  rv=AB_Provider_AccountToAccountSpec(pro, acc, as);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Banking_WriteAccountSpec(AB_Provider_GetBanking(pro), as, doLock);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    AB_AccountSpec_free(as);
    return rv;
  }
  AB_AccountSpec_free(as);
  return 0;
}



int AB_Provider_CreateInitialAccountSpecs(AB_PROVIDER *pro) {
  int rv;
  GWEN_DB_NODE *dbAll=NULL;
  GWEN_DB_NODE *db;

  /* read all config groups for accounts which have a unique id and which belong to this provider
   */
  rv=AB_Banking_ReadConfigGroups(AB_Provider_GetBanking(pro), "accounts", "uniqueId", "provider",
                                 AB_Provider_GetName(pro), &dbAll);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  db=GWEN_DB_GetFirstGroup(dbAll);
  while(db) {
    AB_ACCOUNT *acc=NULL;
    AB_ACCOUNT_SPEC *as;

    acc=AB_Provider_CreateAccountObject(pro);
    rv=AB_Account_ReadFromDb(acc, db);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_DB_Group_free(dbAll);
      return rv;
    }

    /* create account spec */
    as=AB_AccountSpec_new();
    rv=AB_Provider_AccountToAccountSpec(pro, acc, as);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      AB_AccountSpec_free(as);
      AB_Account_free(acc);
      return rv;
    }
  
    /* account object no longer needed */
    AB_Account_free(acc);
  
    /* write account spec */
    rv=AB_Banking_WriteAccountSpec(AB_Provider_GetBanking(pro), as, 1); /* doLock */
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      AB_AccountSpec_free(as);
      return rv;
    }
    AB_AccountSpec_free(as);

    /* next */
    db=GWEN_DB_GetNextGroup(db);
  }
  GWEN_DB_Group_free(dbAll);

  return 0;
}



