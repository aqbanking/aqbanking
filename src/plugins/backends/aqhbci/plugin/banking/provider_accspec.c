/***************************************************************************
    begin       : Tue Jun 03 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/*
 * This file is included by provider.c
 */



int AH_Provider__CreateTransactionLimitsForAccount(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *acc, AB_TRANSACTION_LIMITS_LIST *tll) {
  int rv;
  int i;
  int jobList[]={
    AB_Transaction_CommandGetBalance,
    AB_Transaction_CommandGetTransactions,
    /*AB_Transaction_CommandLoadCellPhone, */
    AB_Transaction_CommandSepaTransfer,
    AB_Transaction_CommandSepaDebitNote,
    AB_Transaction_CommandSepaFlashDebitNote,
    AB_Transaction_CommandSepaCreateStandingOrder,
    AB_Transaction_CommandSepaModifyStandingOrder,
    AB_Transaction_CommandSepaDeleteStandingOrder,
    AB_Transaction_CommandSepaGetStandingOrders,
    AB_Transaction_CommandGetEStatements,
    AB_Transaction_CommandUnknown};

  for (i=0; (i<100) && (jobList[i]!=AB_Transaction_CommandUnknown); i++) {
    AH_JOB *j=NULL;
    AB_TRANSACTION_LIMITS *limits=NULL;

    DBG_INFO(AQHBCI_LOGDOMAIN, "Handling job \"%s\"", AB_Transaction_Command_toString(jobList[i]));
    DBG_INFO(AQHBCI_LOGDOMAIN, "- creating job");
    rv=AH_Provider__CreateHbciJob(pro, u, acc, jobList[i], &j);
    if (rv<0) {
      if (rv==GWEN_ERROR_NOT_AVAILABLE) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" is not available", AB_Transaction_Command_toString(jobList[i]));
      }
      else {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "- getting limits");
      rv=AH_Job_GetLimits(j, &limits);
      if (rv<0) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Error getting limits for job \"%s\": %d", AB_Transaction_Command_toString(jobList[i]), rv);
        AH_Job_free(j);
        return rv;
      }
      DBG_INFO(AQHBCI_LOGDOMAIN, "- adding limits");
      AB_TransactionLimits_List_Add(limits, tll);
      AH_Job_free(j);
    }
  } /* for i */

  return 0;
}




int AH_Provider_AccountToAccountSpecWithUser(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *acc, AB_ACCOUNT_SPEC *as) {
  AB_TRANSACTION_LIMITS_LIST *tll;
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

  /* create and set limits */
  tll=AB_TransactionLimits_List_new();
  rv=AH_Provider__CreateTransactionLimitsForAccount(pro, u, acc, tll);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_TransactionLimits_List_free(tll);
    return rv;
  }
  AB_AccountSpec_SetTransactionLimitsList(as, tll);

  return 0;
}



int AH_Provider_AccountToAccountSpec(AB_PROVIDER *pro, AB_ACCOUNT *acc, AB_ACCOUNT_SPEC *as) {
  uint32_t uid;
  AB_USER *u=NULL;
  int rv;

  /* determine user */
  uid=AB_Account_GetUserId(acc);
  if (uid==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No user in account %lu, SNH!", (unsigned long int) AB_Account_GetUniqueId(acc));
    return GWEN_ERROR_INTERNAL;
  }

  /* get user */
  rv=AH_Provider_GetUser(pro, uid, 1, 1, &u);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* read account spec */
  rv=AH_Provider_AccountToAccountSpecWithUser(pro, u, acc, as);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_User_free(u);
    return rv;
  }

  AB_User_free(u);
  return 0;
}



int AH_Provider_WriteAccountSpecForAccount(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *acc) {
  AB_ACCOUNT_SPEC *as;
  int rv;

  as=AB_AccountSpec_new();
  rv=AH_Provider_AccountToAccountSpecWithUser(pro, u, acc, as);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_AccountSpec_free(as);
    return rv;
  }

  rv=AB_Banking_WriteAccountSpec(AB_Provider_GetBanking(pro), as);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_AccountSpec_free(as);
    return rv;
  }
  AB_AccountSpec_free(as);
  return 0;
}




int AH_Provider_CreateInitialAccountSpecs(AB_PROVIDER *pro) {
  int rv;
  GWEN_DB_NODE *dbAll=NULL;
  GWEN_DB_NODE *db;

  /* read all config groups for accounts which have a unique id and which belong to AqHBCI
   * NOTE: we use the fixed value "accounts" for the group name here, since that is where all
   * previous versions of AqBanking store the account settings. This might change in later versions, though.
   */
  rv=AB_Banking_ReadConfigGroups(AB_Provider_GetBanking(pro), "accounts", "uniqueId", "provider", "AQHBCI", &dbAll);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  db=GWEN_DB_GetFirstGroup(dbAll);
  while(db) {
    AB_ACCOUNT *acc=NULL;
    AB_ACCOUNT_SPEC *as;

    acc=AB_Provider_CreateAccountObject(pro);
    rv=AB_Account_ReadFromDb(acc, db);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_DB_Group_free(dbAll);
      return rv;
    }

    /* create account spec */
    as=AB_AccountSpec_new();
    rv=AH_Provider_AccountToAccountSpec(pro, acc, as);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AB_AccountSpec_free(as);
      AB_Account_free(acc);
      return rv;
    }
  
    /* account object no longer needed */
    AB_Account_free(acc);
  
    /* write account spec */
    rv=AB_Banking_WriteAccountSpec(AB_Provider_GetBanking(pro), as);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AB_AccountSpec_free(as);
      return rv;
    }
    AB_AccountSpec_free(as);

    /* next */
    db=GWEN_DB_GetNextGroup(db);
  }

  return 0;
}






