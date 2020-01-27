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


int AB_Provider_AccountToAccountSpec(AB_PROVIDER *pro, const AB_ACCOUNT *acc, AB_ACCOUNT_SPEC *as, int doLock)
{
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
  AB_AccountSpec_SetBankName(as, AB_Account_GetBankName(acc));
  AB_AccountSpec_SetAccountNumber(as, AB_Account_GetAccountNumber(acc));
  AB_AccountSpec_SetSubAccountNumber(as, AB_Account_GetSubAccountId(acc));

  rv=AB_Provider_UpdateAccountSpec(pro, as, doLock);
  if (rv<0 && rv!=GWEN_ERROR_NOT_IMPLEMENTED) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Provider_WriteAccountSpecForAccount(AB_PROVIDER *pro, const AB_ACCOUNT *acc, int doLock)
{
  AB_ACCOUNT_SPEC *as;
  int rv;

  DBG_NOTICE(AQBANKING_LOGDOMAIN, "Writing account spec for account %u", (unsigned int) AB_Account_GetUniqueId(acc));

  as=AB_AccountSpec_new();
  rv=AB_Provider_AccountToAccountSpec(pro, acc, as, doLock);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Banking_WriteAccountSpec(AB_Provider_GetBanking(pro), as);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    AB_AccountSpec_free(as);
    return rv;
  }
  AB_AccountSpec_free(as);
  return 0;
}



int AB_Provider_CreateInitialAccountSpecs(AB_PROVIDER *pro)
{
  int rv;
  GWEN_DB_NODE *dbAll=NULL;
  GWEN_DB_NODE *db;

  /* read all config groups for accounts which have a unique id and which belong to this provider
   */
  rv=AB_Banking_ReadConfigGroups(AB_Provider_GetBanking(pro), "accounts", "uniqueId", "backendName",
                                 AB_Provider_GetName(pro), &dbAll);
  if (rv<0) {
    if (rv==GWEN_ERROR_NOT_FOUND) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "No accounts for backend [%s]", AB_Provider_GetName(pro));
      return 0;
    }
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  db=GWEN_DB_GetFirstGroup(dbAll);
  while (db) {
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
    rv=AB_Provider_AccountToAccountSpec(pro, acc, as, 1); /* doLock */
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      AB_AccountSpec_free(as);
      AB_Account_free(acc);
      return rv;
    }

    /* account object no longer needed */
    AB_Account_free(acc);

    /* write account spec */
    rv=AB_Banking_WriteAccountSpec(AB_Provider_GetBanking(pro), as);
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





AB_ACCOUNT_SPEC *AB_Provider_FindMatchingAccountSpec(AB_PROVIDER *pro, const AB_ACCOUNT *acc, AB_ACCOUNT_SPEC_LIST *asl)
{
  const char *accountNum;
  const char *bankCode;
  const char *iban;
  AB_ACCOUNT_SPEC *as=NULL;

  accountNum=AB_Account_GetAccountNumber(acc);
  bankCode=AB_Account_GetBankCode(acc);
  iban=AB_Account_GetIban(acc);

  DBG_INFO(AQBANKING_LOGDOMAIN, "Checking account [blz=%s, acc=%s, iban=%s, type=%d]",
           bankCode?bankCode:"<none>",
           accountNum?accountNum:"<none>",
           iban?iban:"<none>",
           AB_Account_GetAccountType(acc));

  /* first look for that specific combination of given iban / bankcode+account number */
  if ((iban && *iban) || (accountNum && *accountNum && bankCode && *bankCode)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Comparing IBAN and old account specs");
    /* both spec given */
    as=AB_AccountSpec_List_FindFirst(asl,
                                     pro->name,
                                     AB_Account_GetCountry(acc),
                                     AB_Account_GetBankCode(acc),
                                     AB_Account_GetAccountNumber(acc),
                                     AB_Account_GetSubAccountId(acc),
                                     AB_Account_GetIban(acc),
                                     "*", /* any currency */
                                     AB_Account_GetAccountType(acc));
    if (as==NULL) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Not found, trying with unspecific account type");
      as=AB_AccountSpec_List_FindFirst(asl,
                                       pro->name,
                                       AB_Account_GetCountry(acc),
                                       AB_Account_GetBankCode(acc),
                                       AB_Account_GetAccountNumber(acc),
                                       AB_Account_GetSubAccountId(acc),
                                       AB_Account_GetIban(acc),
                                       "*", /* any currency */
                                       AB_AccountType_Unknown);
    }
  }
  /* then look for old account specs with empty IBAN */
  else if (!(iban && *iban) || (accountNum && *accountNum && bankCode && *bankCode)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Comparing old account specs only");
    as=AB_AccountSpec_List_FindFirst(asl,
                                     pro->name,
                                     AB_Account_GetCountry(acc),
                                     AB_Account_GetBankCode(acc),
                                     AB_Account_GetAccountNumber(acc),
                                     AB_Account_GetSubAccountId(acc),
                                     "", /* empty IBAN (not "*"!) */
                                     "*", /* any currency */
                                     AB_Account_GetAccountType(acc));
    if (as==NULL) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Not found, trying with unspecific account type");
      as=AB_AccountSpec_List_FindFirst(asl,
                                       pro->name,
                                       AB_Account_GetCountry(acc),
                                       AB_Account_GetBankCode(acc),
                                       AB_Account_GetAccountNumber(acc),
                                       AB_Account_GetSubAccountId(acc),
                                       "", /* empty IBAN (not "*"!) */
                                       "*", /* any currency */
                                       AB_AccountType_Unknown);
    }
  }
  /* then look for IBAN with empty old account specs */
  else if ((iban && *iban) || !(accountNum && *accountNum && bankCode && *bankCode)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Comparing IBAN only");
    as=AB_AccountSpec_List_FindFirst(asl,
                                     pro->name,
                                     NULL,
                                     "", /* empty bank code */
                                     "", /* empty account number */
                                     AB_Account_GetSubAccountId(acc),
                                     AB_Account_GetIban(acc),
                                     "*", /* any currency */
                                     AB_Account_GetAccountType(acc));
    if (as==NULL) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Not found, trying with unspecific account type");
      as=AB_AccountSpec_List_FindFirst(asl,
                                       pro->name,
                                       NULL,
                                       "", /* empty bank code */
                                       "", /* empty account number */
                                       AB_Account_GetSubAccountId(acc),
                                       AB_Account_GetIban(acc),
                                       "*", /* any currency */
                                       AB_AccountType_Unknown);
    }
  }
  else {
    /* neither iban nor bank code/account number, should not happen... */
    DBG_INFO(AQBANKING_LOGDOMAIN, "Account not found, neither IBAN nor account number given, SNH!");
  }


  /* if no stored account try a more generic approach */
  if (as==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Not found, trying unspecific approach");
    as=AB_AccountSpec_List_FindFirst(asl,
                                     pro->name,
                                     AB_Account_GetCountry(acc),
                                     AB_Account_GetBankCode(acc),
                                     AB_Account_GetAccountNumber(acc),
                                     AB_Account_GetSubAccountId(acc),
                                     AB_Account_GetIban(acc),
                                     "*", /* any currency */
                                     AB_Account_GetAccountType(acc));
  }
  /* if still no stored account try again with unspecific account type */
  if (as==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Not found, trying with unspecific account type");
    as=AB_AccountSpec_List_FindFirst(asl,
                                     pro->name,
                                     AB_Account_GetCountry(acc),
                                     AB_Account_GetBankCode(acc),
                                     AB_Account_GetAccountNumber(acc),
                                     AB_Account_GetSubAccountId(acc),
                                     AB_Account_GetIban(acc),
                                     "*", /* any currency */
                                     AB_AccountType_Unknown);
  }

  if (as) {
    uint32_t uniqueId;

    uniqueId=AB_AccountSpec_GetUniqueId(as);
    DBG_INFO(AQBANKING_LOGDOMAIN, "Found a matching account (%x)", uniqueId);
    return as;
  }

  return NULL;
}



