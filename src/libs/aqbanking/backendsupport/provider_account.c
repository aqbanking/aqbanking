/***************************************************************************
 begin       : Mon Nov 26 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


/*
 * This file is included by provider.c
 */




int AB_Provider_ReadAccount(AB_PROVIDER *pro, uint32_t uid, int doLock, int doUnlock, AB_ACCOUNT *account) {
  int rv;
  GWEN_DB_NODE *db=NULL;

  rv=AB_Banking_Read_AccountConfig(AB_Provider_GetBanking(pro), uid, doLock, doUnlock, &db);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Account_ReadFromDb(account, db);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(db);
    return rv;
  }

  if (1) {
    const char *s;

    s=AB_Account_GetBackendName(account);
    if (!(s && *s)) {
      DBG_ERROR(0, "Account has no backend name!! SNH!!!");
      GWEN_DB_Dump(db, 2);
      assert(0);
    }
  }

  GWEN_DB_Group_free(db);

  return 0;
}



int AB_Provider_GetAccount(AB_PROVIDER *pro, uint32_t uid, int doLock, int doUnlock, AB_ACCOUNT **pAccount) {
  int rv;
  AB_ACCOUNT *a;

  a=AB_Provider_CreateAccountObject(pro);
  assert(a);
  rv=AB_Provider_ReadAccount(pro, uid, doLock, doUnlock, a);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    AB_Account_free(a);
    return rv;
  }
  *pAccount=a;

  return 0;
}



int AB_Provider_ReadAccounts(AB_PROVIDER *pro, AB_ACCOUNT_LIST *accountList) {
  int rv;
  GWEN_DB_NODE *dbAll=NULL;
  GWEN_DB_NODE *db;

  /* read all config groups for accounts which have a unique id and which belong to this provider */
  rv=AB_Banking_ReadConfigGroups(AB_Provider_GetBanking(pro), AB_CFG_GROUP_ACCOUNTS, "uniqueId", "provider", pro->name, &dbAll);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  db=GWEN_DB_GetFirstGroup(dbAll);
  while(db) {
    AB_ACCOUNT *a=NULL;

    a=AB_Provider_CreateAccountObject(pro);
    if (a==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error creating account for backend [%s], ignoring", pro->name);
    }
    else {
      rv=AB_Account_ReadFromDb(a, db);
      if (rv<0) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "Error reading account (%d), ignoring", rv);
        AB_Account_free(a);
      }
      else
        AB_Account_List_Add(a, accountList);
    }

    /* next */
    db=GWEN_DB_GetNextGroup(db);
  }
  GWEN_DB_Group_free(dbAll);

  return 0;
}



int AB_Provider_WriteAccount(AB_PROVIDER *pro, uint32_t uid, int doLock, int doUnlock, const AB_ACCOUNT *account) {
  int rv;
  GWEN_DB_NODE *db;

  db=GWEN_DB_Group_new("account");
  rv=AB_Account_WriteToDb(account, db);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Banking_Write_AccountConfig(AB_Provider_GetBanking(pro), uid, doLock, doUnlock, db);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(db);
    return rv;
  }
  GWEN_DB_Group_free(db);

  return 0;
}



int AB_Provider_AddAccount(AB_PROVIDER *pro, AB_ACCOUNT *a) {
  uint32_t uid;
  int rv;

  /* add account */
  uid=AB_Banking_GetNamedUniqueId(AB_Provider_GetBanking(pro), "account", 1); /* startAtStdUniqueId=1 */
  rv=AB_Provider_WriteAccount(pro, uid, 1, 1, a); /* lock, unlock */
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  /* write account spec */
  rv=AB_Provider_WriteAccountSpecForAccount(pro, a, 1); /* doLock */
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;

}



int AB_Provider_DeleteAccount(AB_PROVIDER *pro, uint32_t uid) {
  int rv1;
  int rv2;

  rv1=AB_Banking_DeleteAccountSpec(AB_Provider_GetBanking(pro), uid);
  if (rv1<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv1);
  }

  rv2=AB_Banking_Delete_AccountConfig(AB_Provider_GetBanking(pro), uid);
  if (rv2<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv2);
  }

  if (rv1>0)
    return rv1;
  if (rv2>0)
    return rv2;
  return 0;
}




int AB_Provider_BeginExclUseAccount(AB_PROVIDER *pro, AB_ACCOUNT *a) {
  int rv;
  uint32_t uid;

  uid=AB_Account_GetUniqueId(a);
  if (uid==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No unique id!");
    return GWEN_ERROR_INVALID;
  }

  rv=AB_Provider_ReadAccount(pro, uid, 1, 0, a);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return 0;
}



int AB_Provider_EndExclUseAccount(AB_PROVIDER *pro, AB_ACCOUNT *a, int abandon) {
  int rv;
  uint32_t uid;

  uid=AB_Account_GetUniqueId(a);
  if (uid==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No unique id!");
    return GWEN_ERROR_INVALID;
  }

  if (abandon) {
    rv=AB_Banking_Unlock_AccountConfig(AB_Provider_GetBanking(pro), uid);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    rv=AB_Provider_WriteAccount(pro, uid, 0, 1, a);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}




AB_ACCOUNT_SPEC *AB_Provider_FindMatchingAccountSpec(AB_PROVIDER *pro, const AB_ACCOUNT *acc, AB_ACCOUNT_SPEC_LIST *asl){
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


