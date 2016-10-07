/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2011 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



int AH_Job__GetJobGroup(GWEN_DB_NODE *dbJob, const char *groupName, GWEN_DB_NODE **pResult) {
  GWEN_DB_NODE *dbRd;

  dbRd=GWEN_DB_GetGroup(dbJob, GWEN_PATH_FLAGS_NAMEMUSTEXIST, groupName);
  if (dbRd==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Group \"%s\" not found in response", groupName);
    return GWEN_ERROR_NOT_FOUND;
  }

  dbRd=GWEN_DB_GetGroup(dbRd, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
  if (dbRd==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing \"data\" group inside group \"%s\"", groupName);
    return GWEN_ERROR_INVALID;
  }

  dbRd=GWEN_DB_GetGroup(dbRd, GWEN_PATH_FLAGS_NAMEMUSTEXIST, groupName);
  if (dbRd==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing effective group \"%s\" inside response", groupName);
    return GWEN_ERROR_INVALID;
  }

  *pResult=dbRd;
  return 0;
}



int AH_Job__Commit_Accounts(AH_JOB *j){
  GWEN_DB_NODE *dbJob;
  GWEN_DB_NODE *dbCurr=NULL;
  int rv;
  AB_ACCOUNT_LIST *accList;
  AB_BANKING *ab;
  AH_BPD *bpd;

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  bpd=AH_User_GetBpd(j->user);

  dbJob=j->jobResponses;
  accList=AB_Account_List_new();

  dbCurr=GWEN_DB_FindFirstGroup(dbJob, "AccountData");
  while(dbCurr) {
    GWEN_DB_NODE *dbAccountData;

    dbAccountData=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data/AccountData");
    if (dbAccountData) {
      AB_ACCOUNT *acc;
      GWEN_DB_NODE *dbUpd;
      GWEN_DB_NODE *gr;
      int t;

      DBG_INFO(AQHBCI_LOGDOMAIN, "Found an account");

      /* account data found */
      acc=AB_Banking_CreateAccount(ab, AH_PROVIDER_NAME);
      assert(acc);
      /* AB_Banking_CreateAccount() already assigns a unique id, we don't want that just yet */
      AB_Account_SetUniqueId(acc, 0);

      AB_Account_SetBankCode(acc, GWEN_DB_GetCharValue(dbAccountData, "bankCode", 0, 0));
      AB_Account_SetAccountNumber(acc, GWEN_DB_GetCharValue(dbAccountData, "accountId", 0, 0));
      AB_Account_SetIBAN(acc, GWEN_DB_GetCharValue(dbAccountData, "iban", 0, 0));
      AB_Account_SetAccountName(acc, GWEN_DB_GetCharValue(dbAccountData, "account/name", 0, 0));
      AB_Account_SetSubAccountId(acc, GWEN_DB_GetCharValue(dbAccountData, "accountsubid", 0, 0));
      AB_Account_SetOwnerName(acc, GWEN_DB_GetCharValue(dbAccountData, "name1", 0, 0));

      if (GWEN_DB_GetIntValue(dbAccountData, "head/version", 0, 1)>=4)
	/* KTV in version 2 available */
	AH_Account_AddFlags(acc, AH_BANK_FLAGS_KTV2);
      else
	AH_Account_SubFlags(acc, AH_BANK_FLAGS_KTV2);

      /* account type (from FinTS_3.0_Formals) */
      t=GWEN_DB_GetIntValue(dbAccountData, "type", 0, 1);
      if (t>=1 && t<=9)          /* Kontokorrent-/Girokonto */
	AB_Account_SetAccountType(acc, AB_AccountType_Bank);
      else if (t>=10 && t<=19)   /* Sparkonto */
	AB_Account_SetAccountType(acc, AB_AccountType_Savings);
      else if (t>=20 && t<=29)   /* Festgeldkonto/Termineinlagen */
	AB_Account_SetAccountType(acc, AB_AccountType_MoneyMarket);
      else if (t>=30 && t<=39)   /* Wertpapierdepot */
	AB_Account_SetAccountType(acc, AB_AccountType_Investment);
      else if (t>=40 && t<=49)   /* Kredit-/Darlehenskonto */
	AB_Account_SetAccountType(acc, AB_AccountType_Credit);
      else if (t>=50 && t<=59)   /* Kreditkartenkonto */
	AB_Account_SetAccountType(acc, AB_AccountType_CreditCard);
      else if (t>=60 && t<=69)   /* Fonds-Depot bei einer Kapitalanlagengesellschaft */
	AB_Account_SetAccountType(acc, AB_AccountType_Investment);
      else if (t>=70 && t<=79)   /* Bausparvertrag */
	AB_Account_SetAccountType(acc, AB_AccountType_Savings);
      else if (t>=80 && t<=89)   /* Versicherungsvertrag */
	AB_Account_SetAccountType(acc, AB_AccountType_Savings);
      else if (t>=90 && t<=99)   /* sonstige */
	AB_Account_SetAccountType(acc, AB_AccountType_Unknown);
      else
	AB_Account_SetAccountType(acc, AB_AccountType_Unknown);

      /* set bank name */
      if (bpd) {
	const char *s;

	s=AH_Bpd_GetBankName(bpd);
	if (s && *s)
	  AB_Account_SetBankName(acc, s);
      }


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

#if 0
  if (AB_Account_List_GetCount(accList)<1) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Found no accounts");
    GWEN_DB_Dump(dbJob, 2);
  }
#endif

  /* only keep accounts which have at least IBAN or bankcode and account number */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Checking for empty accounts");
  if (AB_Account_List_GetCount(accList)) {
    AB_ACCOUNT *acc;
    AB_ACCOUNT_LIST *accList2;

    accList2=AB_Account_List_new();
    while( (acc=AB_Account_List_First(accList)) ) {
      const char *accountNum;
      const char *bankCode;
      const char *iban;

      AB_Account_List_Del(acc);
      accountNum=AB_Account_GetAccountNumber(acc);
      bankCode=AB_Account_GetBankCode(acc);
      iban=AB_Account_GetIBAN(acc);

      if ((iban && *iban) || (accountNum && *accountNum && bankCode && *bankCode)) {
	AB_Account_List_Add(acc, accList2);
      }
      else {
	DBG_INFO(AQHBCI_LOGDOMAIN, "Removing empty account from import list");
	AB_Account_free(acc);
      }
    }
    AB_Account_List_free(accList);
    accList=accList2;
  }

  /* find out which of the accounts are new */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Checking for existing or to be added accounts");
  if (AB_Account_List_GetCount(accList)) {
    AB_ACCOUNT *acc;

    acc=AB_Account_List_First(accList);
    while(acc) {
      AB_ACCOUNT *storedAcc=NULL;
      const char *accountNum;
      const char *bankCode;
      const char *iban;

      accountNum=AB_Account_GetAccountNumber(acc);
      bankCode=AB_Account_GetBankCode(acc);
      iban=AB_Account_GetIBAN(acc);

      DBG_INFO(AQHBCI_LOGDOMAIN, "Checking account [blz=%s, acc=%s, iban=%s]",
	       bankCode?bankCode:"<none>",
	       accountNum?accountNum:"<none>",
	       iban?iban:"<none>");

      /* first look for that specific combination of given iban / bankcode+account number */
      if ((iban && *iban) || (accountNum && *accountNum && bankCode && *bankCode)) {
	/* both spec given */
	storedAcc=AB_Banking_FindAccount2(ab, AH_PROVIDER_NAME,
					  AB_Account_GetCountry(acc),
					  AB_Account_GetBankCode(acc),
					  AB_Account_GetAccountNumber(acc),
					  AB_Account_GetSubAccountId(acc),
					  AB_Account_GetIBAN(acc),
					  AB_Account_GetAccountType(acc));
	if (storedAcc==NULL) {
	  DBG_INFO(AQHBCI_LOGDOMAIN, "Not found, trying with unspecific account type");
	  storedAcc=AB_Banking_FindAccount2(ab, AH_PROVIDER_NAME,
					    AB_Account_GetCountry(acc),
					    AB_Account_GetBankCode(acc),
					    AB_Account_GetAccountNumber(acc),
					    AB_Account_GetSubAccountId(acc),
					    AB_Account_GetIBAN(acc),
					    AB_AccountType_Unknown);
	}
      }
      else if (!(iban && *iban) || (accountNum && *accountNum && bankCode && *bankCode)) {
	storedAcc=AB_Banking_FindAccount2(ab, AH_PROVIDER_NAME,
					  AB_Account_GetCountry(acc),
					  AB_Account_GetBankCode(acc),
					  AB_Account_GetAccountNumber(acc),
					  AB_Account_GetSubAccountId(acc),
					  "", /* empty IBAN (not "*"!) */
					  AB_Account_GetAccountType(acc));
	if (storedAcc==NULL) {
	  DBG_INFO(AQHBCI_LOGDOMAIN, "Not found, trying with unspecific account type");
	  storedAcc=AB_Banking_FindAccount2(ab, AH_PROVIDER_NAME,
					    AB_Account_GetCountry(acc),
					    AB_Account_GetBankCode(acc),
					    AB_Account_GetAccountNumber(acc),
					    AB_Account_GetSubAccountId(acc),
					    "", /* empty IBAN (not "*"!) */
					    AB_AccountType_Unknown);
	}
      }
      else if ((iban && *iban) || !(accountNum && *accountNum && bankCode && *bankCode)) {
	storedAcc=AB_Banking_FindAccount2(ab, AH_PROVIDER_NAME,
					  NULL,
					  "", /* empty bank code */
					  "", /* empty account number */
					  AB_Account_GetSubAccountId(acc),
					  AB_Account_GetIBAN(acc),
					  AB_Account_GetAccountType(acc));
	if (storedAcc==NULL) {
	  DBG_INFO(AQHBCI_LOGDOMAIN, "Not found, trying with unspecific account type");
	  storedAcc=AB_Banking_FindAccount2(ab, AH_PROVIDER_NAME,
					    NULL,
					    "", /* empty bank code */
					    "", /* empty account number */
					    AB_Account_GetSubAccountId(acc),
					    AB_Account_GetIBAN(acc),
					    AB_AccountType_Unknown);
	}
      }
      else {
	/* neither iban nor bank code/account number, should not happen... */
      }


      /* if no stored account try a more generic approach */
      if (storedAcc==NULL) {
	DBG_INFO(AQHBCI_LOGDOMAIN, "Not found, trying unspecific approach");
	storedAcc=AB_Banking_FindAccount2(ab, AH_PROVIDER_NAME,
					  AB_Account_GetCountry(acc),
					  AB_Account_GetBankCode(acc),
					  AB_Account_GetAccountNumber(acc),
					  AB_Account_GetSubAccountId(acc),
					  AB_Account_GetIBAN(acc),
					  AB_Account_GetAccountType(acc));
      }
      /* if still no stored account try again with unspecific account type */
      if (storedAcc==NULL) {
	DBG_INFO(AQHBCI_LOGDOMAIN, "Not found, trying with unspecific account type");
	storedAcc=AB_Banking_FindAccount2(ab, AH_PROVIDER_NAME,
					  AB_Account_GetCountry(acc),
					  AB_Account_GetBankCode(acc),
					  AB_Account_GetAccountNumber(acc),
					  AB_Account_GetSubAccountId(acc),
					  AB_Account_GetIBAN(acc),
					  AB_AccountType_Unknown);
      }

      if (storedAcc) {
	DBG_INFO(AQHBCI_LOGDOMAIN, "Found a matching account");
	AB_Account_SetUniqueId(acc, AB_Account_GetUniqueId(storedAcc));
      }

      acc=AB_Account_List_Next(acc);
    }
  }

  /* now either add new accounts or modify existing ones */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Adding new or modifying existing accounts");
  if (AB_Account_List_GetCount(accList)) {
    AB_ACCOUNT *acc;

    while( (acc=AB_Account_List_First(accList)) ) {
      AB_ACCOUNT *storedAcc=NULL;
      GWEN_DB_NODE *dbTempUpd=NULL;

      /* remove from list. if this is a new account it will be added to AqBanking's internal
       * list, by which AqBanking takes over the object. It is is a known account, it will be
       * freed later */
      AB_Account_List_Del(acc);

      dbTempUpd=AH_Account_GetDbTempUpd(acc);
      if (dbTempUpd)
	dbTempUpd=GWEN_DB_Group_dup(dbTempUpd);

      if (AB_Account_GetUniqueId(acc)) {
	/* account already exists, needs update */
	DBG_INFO(AQHBCI_LOGDOMAIN, "Account exists, modifying");

	storedAcc=AB_Banking_GetAccount(ab, AB_Account_GetUniqueId(acc));
	assert(storedAcc);

	rv=AB_Banking_BeginExclUseAccount(ab, storedAcc);
	if (rv<0) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	}
	else {
	  const char *s;

	  /* locking ok, apply changes */

	  s=AB_Account_GetCountry(acc);
	  if (s && *s)
	    AB_Account_SetCountry(storedAcc, s);

	  s=AB_Account_GetBankCode(acc);
	  if (s && *s)
	    AB_Account_SetBankCode(storedAcc, s);

	  s=AB_Account_GetAccountNumber(acc);
	  if (s && *s)
	    AB_Account_SetAccountNumber(storedAcc, s);

	  s=AB_Account_GetSubAccountId(acc);
	  if (s && *s)
	    AB_Account_SetSubAccountId(storedAcc, s);

	  s=AB_Account_GetIBAN(acc);
	  if (s && *s)
	    AB_Account_SetIBAN(storedAcc, s);

	  s=AB_Account_GetBIC(acc);
	  if (s && *s)
	    AB_Account_SetBIC(storedAcc, s);

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
	  if (AB_Account_HasUser(storedAcc, AB_User_GetUniqueId(j->user))==GWEN_ERROR_NOT_FOUND) {
	    DBG_INFO(AQHBCI_LOGDOMAIN, "Adding this job's owber as user to account");
	    AB_Account_SetUser(storedAcc, j->user);
	  }

	  /* handle selected users */
	  if (AB_Account_HasSelectedUser(storedAcc, AB_User_GetUniqueId(j->user))==GWEN_ERROR_NOT_FOUND) {
	    DBG_INFO(AQHBCI_LOGDOMAIN, "Adding this job's owber as selected user to account");
	    AB_Account_SetSelectedUser(storedAcc, j->user);
	  }

	  /* unlock account */
	  rv=AB_Banking_EndExclUseAccount(ab, storedAcc, 0);
	  if (rv<0) {
	    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	    AB_Banking_EndExclUseAccount(ab, acc, 1); /* abort */
	  }
	}
	/* free local representation */
	AB_Account_free(acc);
      }
      else {
	/* account is new, add it (CAVEAT: takes over account!) */
	DBG_INFO(AQHBCI_LOGDOMAIN, "Account is new, adding");
	AB_Account_SetUser(acc, j->user);
	AB_Account_SetSelectedUser(acc, j->user);
	rv=AB_Banking_AddAccount(ab, acc);
	if (rv<0) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN, "Coud not add account (%d)", rv);
	}
	else {
	  DBG_INFO(AQHBCI_LOGDOMAIN, "Reading back added account");
	  storedAcc=AB_Banking_GetAccount(ab, AB_Account_GetUniqueId(acc));
	  assert(storedAcc);
	}
      }

      /* replace UPD jobs for this account inside user */
      if (storedAcc && dbTempUpd) {
	GWEN_DB_NODE *dbUpd;
	GWEN_DB_NODE *gr;
	char numbuf[32];

	DBG_INFO(AQHBCI_LOGDOMAIN, "Setting UPD jobs for this account in user");

	/* get UPD jobs */
	dbUpd=AH_User_GetUpd(j->user);
	assert(dbUpd);
  
	/* create and clear group for each account (check subaccount id) */
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
    } /* while */
  } /* if accounts */
  AB_Account_List_free(accList);

  /* done */
  return 0;
}



int AH_Job__Commit_Bpd(AH_JOB *j){
  GWEN_DB_NODE *dbJob    ;
  GWEN_DB_NODE *dbRd=NULL;
  AH_BPD *bpd;
  GWEN_DB_NODE *n;
  const char *p;
  int i;
  int rv;
  GWEN_MSGENGINE *e;

  dbJob=GWEN_DB_GetFirstGroup(j->jobResponses);

  rv=AH_Job__GetJobGroup(dbJob, "bpd", &dbRd);
  if (rv<0) {
    if (rv!=GWEN_ERROR_NOT_FOUND) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    DBG_INFO(AQHBCI_LOGDOMAIN, "No BPD in response");
    return 0;
  }

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Found BPD, replacing existing");

  e=AH_User_GetMsgEngine(j->user);
  assert(e);

  /* create new BPD */
  bpd=AH_Bpd_new();

  /* read version */
  AH_Bpd_SetBpdVersion(bpd, GWEN_DB_GetIntValue(dbRd, "version", 0, 0));

  /* read bank name */
  p=GWEN_DB_GetCharValue(dbRd, "name", 0, 0);
  if (p)
    AH_Bpd_SetBankName(bpd, p);

  /* read message and job limits */
  AH_Bpd_SetJobTypesPerMsg(bpd, GWEN_DB_GetIntValue(dbRd, "jobtypespermsg", 0, 0));
  AH_Bpd_SetMaxMsgSize(bpd, GWEN_DB_GetIntValue(dbRd, "maxmsgsize", 0, 0));

  /* read languages */
  n=GWEN_DB_GetGroup(dbRd, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "languages");
  if (n) {
    for (i=0;;i++) {
      int k;

      k=GWEN_DB_GetIntValue(n, "language", i, 0);
      if (k) {
	if (AH_Bpd_AddLanguage(bpd, k)) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN, "Too many languages (%d)", i);
	  break;
	}
      }
      else
	break;
    } /* for */
  } /* if languages */

  /* read supported version */
  n=GWEN_DB_GetGroup(dbRd, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "versions");
  if (n) {
    for (i=0;;i++) {
      int k;

      k=GWEN_DB_GetIntValue(n, "version", i, 0);
      if (k) {
	if (AH_Bpd_AddHbciVersion(bpd, k)) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN, "Too many versions (%d)", i);
	  break;
	}
      }
      else
	break;
    } /* for */
  } /* if versions */


  /* communication parameters */
  rv=AH_Job__GetJobGroup(dbJob, "ComData", &dbRd);
  if (rv==0) {
    GWEN_DB_NODE *currService;

    DBG_INFO(AQHBCI_LOGDOMAIN, "Found communication infos");

    currService=GWEN_DB_FindFirstGroup(dbRd, "service");
    while(currService) {
      AH_BPD_ADDR *ba;

      ba=AH_BpdAddr_FromDb(currService);
      if (ba) {
	DBG_INFO(AQHBCI_LOGDOMAIN, "Adding service");
	AH_Bpd_AddAddr(bpd, ba);
      }
      currService=GWEN_DB_FindNextGroup(currService, "service");
    }
  } /* if ComData found */


  /* special extension of BPD for PIN/TAN mode */
  rv=AH_Job__GetJobGroup(dbJob, "PinTanBPD", &dbRd);
  if (rv==0) {
    GWEN_DB_NODE *bn;
    GWEN_DB_NODE *currJob;

    bn=AH_Bpd_GetBpdJobs(bpd, GWEN_MsgEngine_GetProtocolVersion(e));
    assert(bn);

    currJob=GWEN_DB_FindFirstGroup(dbRd, "job");
    while(currJob) {
      const char *jobName;
      int needTAN;
      GWEN_DB_NODE *dbJob;

      jobName=GWEN_DB_GetCharValue(currJob, "job", 0, 0);
      assert(jobName);
      dbJob=GWEN_DB_GetGroup(bn, GWEN_DB_FLAGS_DEFAULT, jobName);
      assert(dbJob);
      needTAN=strcasecmp(GWEN_DB_GetCharValue(currJob, "needTan", 0, "N"), "J")==0;
      GWEN_DB_SetIntValue(dbJob, GWEN_DB_FLAGS_OVERWRITE_VARS, "needTan", needTAN);
      currJob=GWEN_DB_FindNextGroup(currJob, "job");
    } /* while */
  } /* if PIN/TAN extension found */


  /* check for BPD jobs */
  n=GWEN_DB_GetFirstGroup(dbJob);
  while(n) {
    dbRd=GWEN_DB_GetGroup(n, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
    if (dbRd)
      dbRd=GWEN_DB_GetFirstGroup(dbRd);
    if (dbRd) {
      GWEN_XMLNODE *bpdn;
      int segver;
      /* check for BPD job */

      DBG_INFO(AQHBCI_LOGDOMAIN, "Checking whether \"%s\" is a BPD job", GWEN_DB_GroupName(dbRd));
      segver=GWEN_DB_GetIntValue(dbRd, "head/version", 0, 0);
      /* get segment description (first try id, then code) */
      bpdn=GWEN_MsgEngine_FindNodeByProperty(e, "SEG", "id", segver, GWEN_DB_GroupName(dbRd));
      if (!bpdn)
	bpdn=GWEN_MsgEngine_FindNodeByProperty(e, "SEG", "code", segver, GWEN_DB_GroupName(dbRd));
      if (bpdn) {
	DBG_DEBUG(AQHBCI_LOGDOMAIN, "Found a candidate");
	if (atoi(GWEN_XMLNode_GetProperty(bpdn, "isbpdjob", "0"))) {
	  /* segment contains a BPD job, move contents */
	  GWEN_DB_NODE *bn;
	  char numbuffer[32];

	  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Found BPD job \"%s\"", GWEN_DB_GroupName(dbRd));
	  bn=AH_Bpd_GetBpdJobs(bpd, GWEN_MsgEngine_GetProtocolVersion(e));
	  assert(bn);
	  bn=GWEN_DB_GetGroup(bn, GWEN_DB_FLAGS_DEFAULT,
			      GWEN_DB_GroupName(dbRd));
	  assert(bn);

	  if (GWEN_Text_NumToString(segver, numbuffer, sizeof(numbuffer)-1, 0)<1) {
	    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Buffer too small");
	    abort();
	  }
	  bn=GWEN_DB_GetGroup(bn, GWEN_DB_FLAGS_OVERWRITE_GROUPS, numbuffer);
	  assert(bn);

	  GWEN_DB_AddGroupChildren(bn, dbRd);
	  /* remove "head" and "segment" group */
	  GWEN_DB_DeleteGroup(bn, "head");
	  GWEN_DB_DeleteGroup(bn, "segment");
	  DBG_INFO(AQHBCI_LOGDOMAIN, "Added BPD Job %s:%d", GWEN_DB_GroupName(dbRd), segver);
	} /* if isbpdjob */
	else {
	  DBG_INFO(AQHBCI_LOGDOMAIN,
		   "Segment \"%s\" is known but not as a BPD job",
		   GWEN_DB_GroupName(dbRd));
	}
      } /* if segment found */
      else {
	DBG_WARN(AQHBCI_LOGDOMAIN, "Did not find segment \"%s\" (%d) ignoring",
		 GWEN_DB_GroupName(dbRd), segver);
      }
    }
    n=GWEN_DB_GetNextGroup(n);
  } /* while */


  /* set BPD */
  AH_User_SetBpd(j->user, bpd);
  return 0;
}





int AH_Job__CommitSystemData(AH_JOB *j, int doLock) {
  GWEN_DB_NODE *dbCurr;
  AB_USER *u;
  AB_BANKING *ab;
  AH_HBCI *h;
  GWEN_MSGENGINE *e;
  int rv;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Committing data");
  assert(j);
  assert(j->usage);

  u=j->user;
  assert(u);
  h=AH_Job_GetHbci(j);
  assert(h);
  ab=AH_Job_GetBankingApi(j);
  assert(ab);
  e=AH_User_GetMsgEngine(j->user);
  assert(e);

  /* GWEN_DB_Dump(j->jobResponses, 2); */

  /* try to extract bank parameter data */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Committing BPD");
  rv=AH_Job__Commit_Bpd(j);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* try to extract accounts */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Committing accounts");
  rv=AH_Job__Commit_Accounts(j);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  dbCurr=GWEN_DB_GetFirstGroup(j->jobResponses);
  while(dbCurr) {
    GWEN_DB_NODE *dbRd;

    dbRd=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
    if (dbRd) {
      dbRd=GWEN_DB_GetFirstGroup(dbRd);
    }
    if (dbRd) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Checking group \"%s\"", GWEN_DB_GroupName(dbRd));
      if (strcasecmp(GWEN_DB_GroupName(dbRd), "SegResult")==0){
        GWEN_DB_NODE *dbRes;

        dbRes=GWEN_DB_GetFirstGroup(dbRd);
        while(dbRes) {
          if (strcasecmp(GWEN_DB_GroupName(dbRes), "result")==0) {
            int code;
//            const char *text;
  
            code=GWEN_DB_GetIntValue(dbRes, "resultcode", 0, 0);
//            text=GWEN_DB_GetCharValue(dbRes, "text", 0, 0);
            if (code==3920) {
              int i;

              AH_User_ClearTanMethodList(u);
              for (i=0; ; i++) {
                int j;

                j=GWEN_DB_GetIntValue(dbRes, "param", i, 0);
		if (j==0)
		  break;
		DBG_NOTICE(AQHBCI_LOGDOMAIN, "Adding allowed TAN method %d", j);
		AH_User_AddTanMethod(u, j);
	      } /* for */
	      if (i==0) {
                /* add single step if empty list */
		DBG_INFO(AQHBCI_LOGDOMAIN, "No allowed TAN method reported, assuming 999");
		AH_User_AddTanMethod(u, 999);
	      }
	    }
          } /* if result */
          dbRes=GWEN_DB_GetNextGroup(dbRes);
        } /* while */
      }
      else if (strcasecmp(GWEN_DB_GroupName(dbRd), "GetKeyResponse")==0){
	/* TODO: Read the key received and ask the user to accept it */
      }

      else if (strcasecmp(GWEN_DB_GroupName(dbRd), "SecurityMethods")==0){
        GWEN_DB_NODE *dbT;

        dbT=GWEN_DB_FindFirstGroup(dbRd, "SecProfile");
	while(dbT) {
	  const char *code;
          int version;
  
	  code=GWEN_DB_GetCharValue(dbT, "code", 0, NULL);
	  version=GWEN_DB_GetIntValue(dbT, "version", 0, -1);
	  if (code && (version>0)) {
	    DBG_ERROR(AQHBCI_LOGDOMAIN,
		      "Bank supports mode %s %d",
		      code, version);
	    /* TODO: store possible modes */
	  }
	  dbT=GWEN_DB_FindNextGroup(dbT, "SecProfile");
	} /* while */
      }

      else if (strcasecmp(GWEN_DB_GroupName(dbRd), "UserData")==0){
        /* UserData found */
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Found UserData");
        AH_User_SetUpdVersion(j->user, GWEN_DB_GetIntValue(dbRd, "version", 0, 0));
      }

      if (strcasecmp(GWEN_DB_GroupName(dbRd), "BankMsg")==0){
        const char *subject;
        const char *text;

        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Found a bank message");
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Notice,
			     I18N("Bank message received"));
        subject=GWEN_DB_GetCharValue(dbRd, "subject", 0, "(Kein Betreff)");
        text=GWEN_DB_GetCharValue(dbRd, "text", 0, 0);
        if (subject && text) {
	  AB_MESSAGE *amsg;
          GWEN_TIME *ti;

          ti=GWEN_CurrentTime();
	  amsg=AB_Message_new();
	  AB_Message_SetSubject(amsg, subject);
	  AB_Message_SetText(amsg, text);
	  AB_Message_SetDateReceived(amsg, ti);
	  GWEN_Time_free(ti);
	  AB_Message_SetUserId(amsg, AB_User_GetUniqueId(u));
	  AB_Message_List_Add(amsg, j->messages);

	  if (1) {
	    GWEN_DB_NODE *dbTmp;

            /* save message, later this will no longer be necessary */
	    dbTmp=GWEN_DB_Group_new("bank message");
	    GWEN_DB_SetCharValue(dbTmp, GWEN_DB_FLAGS_OVERWRITE_VARS,
				 "subject", subject);
	    GWEN_DB_SetCharValue(dbTmp, GWEN_DB_FLAGS_OVERWRITE_VARS,
				 "text", text);
	    if (AH_HBCI_SaveMessage(h, j->user, dbTmp)) {
	      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not save this message:");
	      GWEN_DB_Dump(dbTmp, 2);
	    }
	    GWEN_DB_Group_free(dbTmp);
	  }

        } /* if subject and text given */
      } /* if bank msg */


    } /* if response data found */
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Finished.");
  return 0;
}



int AH_Job_CommitSystemData(AH_JOB *j, int doLock) {
  AB_USER *u;
  AB_BANKING *ab;
  int rv, rv2;

  u=j->user;
  assert(u);

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  /* lock user */
  if (doLock) {
    rv=AB_Banking_BeginExclUseUser(ab, u);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  /* commit data */
  rv2=AH_Job__CommitSystemData(j, doLock);

  if (doLock) {
    /* unlock user */
    rv=AB_Banking_EndExclUseUser(ab, u, 0);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AB_Banking_EndExclUseUser(ab, u, 1); /* abandon */
      return rv;
    }
  }

  return rv2;
}
