/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/*
 * This file is included by job.c
 */



static int AH_Job__Commit_Accounts_ReadAccounts(AH_JOB *j, AB_ACCOUNT_LIST *accList){
  AB_BANKING *ab;
  AB_PROVIDER *pro;
  AH_BPD *bpd;
  GWEN_DB_NODE *dbJob;
  GWEN_DB_NODE *dbCurr;

  ab=AH_Job_GetBankingApi(j);
  assert(ab);
  pro=AH_Job_GetProvider(j);
  assert(pro);
  bpd=AH_User_GetBpd(j->user);

  dbJob=j->jobResponses;

  dbCurr=GWEN_DB_FindFirstGroup(dbJob, "AccountData");
  while(dbCurr) {
    GWEN_DB_NODE *dbAccountData;

    dbAccountData=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data/AccountData");
    if (dbAccountData) {
      AB_ACCOUNT *acc;
      GWEN_DB_NODE *dbUpd;
      GWEN_DB_NODE *gr;

      DBG_INFO(AQHBCI_LOGDOMAIN, "Found an account");

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

      // Fixes a bug where the bank sends an account with no bank & account name
      if(!AB_Account_GetBankName(acc))
	AB_Account_SetBankName(acc, "dummy");
      if(!AB_Account_GetAccountName(acc))
	AB_Account_SetAccountName(acc, "dummy");

      /* temporarily store UPD jobs */
      dbUpd=GWEN_DB_Group_new("tmpUpd");
      assert(dbUpd);

      gr=GWEN_DB_GetFirstGroup(dbAccountData);
      while(gr) {
	if (strcasecmp(GWEN_DB_GroupName(gr), "updjob")==0) {
	  /* found an upd job */
	  GWEN_DB_AddGroup(dbUpd, GWEN_DB_Group_dup(gr));
	}
	gr=GWEN_DB_GetNextGroup(gr);
      } /* while */
      AH_Account_SetDbTempUpd(acc, dbUpd);

      /* add to list */
      AB_Account_List_Add(acc, accList);
    }
    dbCurr=GWEN_DB_FindNextGroup(dbCurr, "AccountData");
  }

  return 0;
}



static void AH_Job__Commit_Accounts_RemoveEmpty(AH_JOB *j, AB_ACCOUNT_LIST *accList){
  /* only keep accounts which have at least IBAN or bankcode and account number */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Checking for empty accounts");
  if (AB_Account_List_GetCount(accList)) {
    AB_ACCOUNT *acc;

    acc=AB_Account_List_First(accList);
    while(acc) {
      AB_ACCOUNT *accNext;
      const char *accountNum;
      const char *bankCode;
      const char *iban;

      accNext=AB_Account_List_Next(acc);
      accountNum=AB_Account_GetAccountNumber(acc);
      bankCode=AB_Account_GetBankCode(acc);
      iban=AB_Account_GetIban(acc);

      if (!((iban && *iban) || (accountNum && *accountNum && bankCode && *bankCode))) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Removing empty account from import list");
        AB_Account_List_Del(acc);
        AB_Account_free(acc);
      }
      acc=accNext;
    } /* while(acc) */
  } /* if (AB_Account_List_GetCount(accList)) */
}



static uint32_t AH_Job__Commit_Accounts_FindStored(AH_JOB *j, const AB_ACCOUNT *acc, AB_ACCOUNT_SPEC_LIST *asl){
  AB_ACCOUNT_SPEC *as=NULL;
  AB_PROVIDER *pro;

  pro=AH_Job_GetProvider(j);
  assert(pro);

  as=AB_Provider_FindMatchingAccountSpec(pro, acc, asl);
  if (as) {
    uint32_t uniqueId;

    uniqueId=AB_AccountSpec_GetUniqueId(as);
    DBG_INFO(AQHBCI_LOGDOMAIN, "Found a matching account (%x)", uniqueId);
    return uniqueId;
  }

  return 0;
}


static void AH_Job__Commit_Accounts_AddOrModify(AH_JOB *j, AB_ACCOUNT *acc){
  AB_BANKING *ab;
  AB_PROVIDER *pro;
  AB_ACCOUNT *storedAcc=NULL;
  GWEN_DB_NODE *dbTempUpd=NULL;

  ab=AH_Job_GetBankingApi(j);
  assert(ab);
  pro=AH_Job_GetProvider(j);
  assert(pro);

  dbTempUpd=AH_Account_GetDbTempUpd(acc);
  if (dbTempUpd)
    dbTempUpd=GWEN_DB_Group_dup(dbTempUpd);
  
  if (AB_Account_GetUniqueId(acc)) {
    int rv;

    /* account already exists, needs update */
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Account exists, modifying");
    rv=AB_Provider_GetAccount(pro, AB_Account_GetUniqueId(acc), 1, 0, &storedAcc); /* lock, don't unlock */
    if (rv<0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error getting referred account (%d)", rv);
    }
    else {
      const char *s;

      /* account is locked now, apply changes */
      assert(storedAcc);

      s=AB_Account_GetCountry(acc);
      if (s && *s)
        AB_Account_SetCountry(storedAcc, s);
  
      s=AB_Account_GetBankCode(acc);
      if (s && *s)
        AB_Account_SetBankCode(storedAcc, s);
  
      s=AB_Account_GetBankName(acc);
      if (s && *s)
        AB_Account_SetBankName(storedAcc, s);
  
      s=AB_Account_GetAccountNumber(acc);
      if (s && *s)
        AB_Account_SetAccountNumber(storedAcc, s);
  
      s=AB_Account_GetSubAccountId(acc);
      if (s && *s)
        AB_Account_SetSubAccountId(storedAcc, s);
  
      s=AB_Account_GetIban(acc);
      if (s && *s)
        AB_Account_SetIban(storedAcc, s);
  
      s=AB_Account_GetBic(acc);
      if (s && *s)
        AB_Account_SetBic(storedAcc, s);
  
      s=AB_Account_GetOwnerName(acc);
      if (s && *s)
        AB_Account_SetOwnerName(storedAcc, s);
  
      s=AB_Account_GetCurrency(acc);
      if (s && *s)
        AB_Account_SetCurrency(storedAcc, s);
  
      AB_Account_SetAccountType(storedAcc, AB_Account_GetAccountType(acc));
  
      /* use flags from new account */
      AH_Account_AddFlags(storedAcc, AH_Account_GetFlags(acc));
  
      /* handle users */
      AB_Account_SetUserId(storedAcc, AB_User_GetUniqueId(j->user));

      /* update and write account spec */
      rv=AH_Provider_WriteAccountSpecForAccount(pro, j->user, storedAcc);
      if (rv<0) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      }

      /* unlock account */
      rv=AB_Provider_EndExclUseAccount(pro, storedAcc, 0);
      if (rv<0) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        AB_Provider_EndExclUseAccount(pro, acc, 1); /* abort */
      }
    }
  }
  else {
    int rv;

    /* account is new, add it */
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Account is new, adding");
    AB_Account_SetUserId(acc, AB_User_GetUniqueId(j->user));
    rv=AH_Provider_AddAccount(pro, j->user, acc, 1);
    if (rv<0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Coud not write new account (%d)", rv);
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Reading back added account");
      rv=AB_Provider_GetAccount(pro, AB_Account_GetUniqueId(acc), 0, 0, &storedAcc); /* no-lock, no-unlock */
      if (rv<0) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Error getting referred account (%d)", rv);
      }
    }
  }

  /* replace UPD jobs for this account inside user (user should be locked outside this function) */
  if (storedAcc && dbTempUpd) {
    GWEN_DB_NODE *dbUpd;
    GWEN_DB_NODE *gr;
    char numbuf[32];
  
    DBG_INFO(AQHBCI_LOGDOMAIN, "Setting UPD jobs for this account in user");
  
    /* get UPD jobs */
    dbUpd=AH_User_GetUpd(j->user);
    assert(dbUpd);
  
    /* create and clear group for each account */
    snprintf(numbuf, sizeof(numbuf)-1, "uaid-%08llx",
             (unsigned long long int) AB_Account_GetUniqueId(storedAcc));
    numbuf[sizeof(numbuf)-1]=0;
  
    dbUpd=GWEN_DB_GetGroup(dbUpd, GWEN_DB_FLAGS_OVERWRITE_GROUPS, numbuf);
  
    gr=GWEN_DB_GetFirstGroup(dbTempUpd);
    while(gr) {
      if (strcasecmp(GWEN_DB_GroupName(gr), "updjob")==0) {
        /* found an upd job */
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Adding UPD job");
        GWEN_DB_AddGroup(dbUpd, GWEN_DB_Group_dup(gr));
      }
      gr=GWEN_DB_GetNextGroup(gr);
    } /* while */
  }
  
  GWEN_DB_Group_free(dbTempUpd); /* is a copy, we need to free it */
  AB_Account_free(storedAcc);
}



int AH_Job__Commit_Accounts(AH_JOB *j){
  int rv;
  AB_ACCOUNT_LIST *accList;
  AB_BANKING *ab;
  AB_PROVIDER *pro;

  ab=AH_Job_GetBankingApi(j);
  assert(ab);
  pro=AH_Job_GetProvider(j);
  assert(pro);

  accList=AB_Account_List_new();

  /* read accounts from job data */
  rv=AH_Job__Commit_Accounts_ReadAccounts(j, accList);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }

#if 0
  if (AB_Account_List_GetCount(accList)<1) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Found no accounts");
    GWEN_DB_Dump(dbJob, 2);
  }
#endif

  /* only keep accounts which have at least IBAN or bankcode and account number */
  AH_Job__Commit_Accounts_RemoveEmpty(j, accList);

  /* find out which accounts are new */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Checking for existing or to be added accounts");
  if (AB_Account_List_GetCount(accList)) {
    AB_ACCOUNT_SPEC_LIST *accountSpecList=NULL;

    accountSpecList=AB_AccountSpec_List_new();
    rv=AB_Banking_GetAccountSpecList(ab, &accountSpecList);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "No account spec list");
    }
    else {
      AB_ACCOUNT *acc;

      acc=AB_Account_List_First(accList);
      while(acc) {
        uint32_t storedUid=0;

        storedUid=AH_Job__Commit_Accounts_FindStored(j, acc, accountSpecList);
        if (storedUid) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Found a matching account (%x)", storedUid);
          AB_Account_SetUniqueId(acc, storedUid);
        }

        acc=AB_Account_List_Next(acc);
      }
    }
    AB_AccountSpec_List_free(accountSpecList);
  }

  /* now either add new accounts or modify existing ones */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Adding new or modifying existing accounts");
  if (AB_Account_List_GetCount(accList)) {
    AB_ACCOUNT *acc;

    while( (acc=AB_Account_List_First(accList)) ) {
      /* remove from list. if this is a new account it will be added to AqBanking's internal
       * list, by which AqBanking takes over the object. If it is a known account, it will be
       * freed later */
      AB_Account_List_Del(acc);
      /* add new or modify existing account */
      AH_Job__Commit_Accounts_AddOrModify(j, acc);

      /* free local representation */
      AB_Account_free(acc);

    } /* while */
  } /* if accounts */
  AB_Account_List_free(accList);

  /* done */
  return 0;
}


