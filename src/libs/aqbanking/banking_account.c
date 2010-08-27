/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/* This file is included by banking.c */



AB_ACCOUNT_LIST2 *AB_Banking_GetAccounts(const AB_BANKING *ab){
  AB_ACCOUNT_LIST2 *al;
  AB_ACCOUNT *a;

  assert(ab);
  if (AB_Account_List_GetCount(ab->accounts)==0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No accounts");
    return 0;
  }
  al=AB_Account_List2_new();
  a=AB_Account_List_First(ab->accounts);
  assert(a);
  while(a) {
    AB_Account_List2_PushBack(al, a);
    a=AB_Account_List_Next(a);
  } /* while */

  return al;
}



AB_ACCOUNT *AB_Banking_GetAccount(const AB_BANKING *ab,
                                  uint32_t uniqueId){
  AB_ACCOUNT *a;

  assert(ab);
  if (AB_Account_List_GetCount(ab->accounts)==0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No accounts");
    return 0;
  }
  a=AB_Account_List_First(ab->accounts);
  assert(a);
  while(a) {
    if (AB_Account_GetUniqueId(a)==uniqueId)
      break;
    a=AB_Account_List_Next(a);
  } /* while */

  return a;
}



AB_ACCOUNT *AB_Banking_FindAccount(const AB_BANKING *ab,
                                   const char *backendName,
                                   const char *country,
                                   const char *bankId,
                                   const char *accountId,
                                   const char *subAccountId) {
  AB_ACCOUNT *a;

  assert(ab);
  if (AB_Account_List_GetCount(ab->accounts)==0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No accounts");
    return 0;
  }
  a=AB_Account_List_First(ab->accounts);
  assert(a);

  if (!backendName) backendName="*";
  if (!country) country="*";
  if (!bankId) bankId="*";
  if (!accountId) accountId="*";
  if (!subAccountId) subAccountId="*";

  while(a) {
    const char *lbackendName;
    const char *lcountry;
    const char *lbankId;
    const char *laccountId;
    const char *lsubAccountId;

    lbackendName=AB_Account_GetBackendName(a);
    if (!lbackendName) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Account: %s/%s/%s: No backend\n",
                AB_Account_GetCountry(a),
                AB_Account_GetBankCode(a),
                AB_Account_GetAccountNumber(a));
      abort();
    }

    lcountry=AB_Account_GetCountry(a);
    lbankId=AB_Account_GetBankCode(a);
    laccountId=AB_Account_GetAccountNumber(a);
    lsubAccountId=AB_Account_GetSubAccountId(a);

    if (!lcountry) lcountry="";
    if (!lbankId) lbankId="";
    if (!laccountId) laccountId="";
    if (!lsubAccountId) lsubAccountId="";

    if ((-1!=GWEN_Text_ComparePattern(lbackendName, backendName, 0)) &&
        (-1!=GWEN_Text_ComparePattern(lcountry, country, 0)) &&
        (-1!=GWEN_Text_ComparePattern(lbankId, bankId, 0)) &&
        (-1!=GWEN_Text_ComparePattern(laccountId, accountId, 0)) &&
        (-1!=GWEN_Text_ComparePattern(lsubAccountId, subAccountId, 0)))
      break;
    a=AB_Account_List_Next(a);
  } /* while */

  return a;
}



AB_ACCOUNT_LIST2 *AB_Banking_FindAccounts(const AB_BANKING *ab,
                                          const char *backendName,
                                          const char *country,
                                          const char *bankId,
                                          const char *accountId,
                                          const char *subAccountId) {
  AB_ACCOUNT_LIST2 *al;
  AB_ACCOUNT *a;

  assert(ab);
  if (AB_Account_List_GetCount(ab->accounts)==0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No accounts");
    return 0;
  }
  al=AB_Account_List2_new();
  a=AB_Account_List_First(ab->accounts);
  assert(a);

  if (!backendName) backendName="*";
  if (!country) country="*";
  if (!bankId) bankId="*";
  if (!accountId) accountId="*";
  if (!subAccountId) subAccountId="*";

  while(a) {
    const char *lbackendName;
    const char *lcountry;
    const char *lbankId;
    const char *laccountId;
    const char *lsubAccountId;

    lbackendName=AB_Account_GetBackendName(a);
    if (!lbackendName) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Account: %s/%s/%s: No backend\n",
                AB_Account_GetCountry(a),
                AB_Account_GetBankCode(a),
                AB_Account_GetAccountNumber(a));
      abort();
    }

    lcountry=AB_Account_GetCountry(a);
    lbankId=AB_Account_GetBankCode(a);
    laccountId=AB_Account_GetAccountNumber(a);
    lsubAccountId=AB_Account_GetSubAccountId(a);

    if (!lcountry) lcountry="";
    if (!lbankId) lbankId="";
    if (!laccountId) laccountId="";
    if (!lsubAccountId) lsubAccountId="";

    if ((-1!=GWEN_Text_ComparePattern(lbackendName, backendName, 0)) &&
        (-1!=GWEN_Text_ComparePattern(lcountry, country, 0)) &&
        (-1!=GWEN_Text_ComparePattern(lbankId, bankId, 0)) &&
        (-1!=GWEN_Text_ComparePattern(laccountId, accountId, 0)) &&
        (-1!=GWEN_Text_ComparePattern(lsubAccountId, subAccountId, 0)))
      AB_Account_List2_PushBack(al, a);
    a=AB_Account_List_Next(a);
  } /* while */

  if (AB_Account_List2_GetSize(al)==0) {
    AB_Account_List2_free(al);
    return 0;
  }

  return al;
}



AB_ACCOUNT *AB_Banking_CreateAccount(AB_BANKING *ab, const char *backendName){
  AB_ACCOUNT *a;
  AB_PROVIDER *pro;
  int rv;
  uint32_t uid;

  assert(ab);
  pro=AB_Banking_GetProvider(ab, backendName);
  if (!pro) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Backend \"%s\" not found", backendName);
    return 0;
  }

  uid=AB_Banking_GetUniqueId(ab);
  assert(uid);

  a=AB_Account_new(ab, pro);
  AB_Account_SetUniqueId(a, uid);

  rv=AB_Provider_ExtendAccount(pro, a, AB_ProviderExtendMode_Create, NULL);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error extending account (%d)", rv);
    AB_Account_free(a);
    return 0;
  }

  return a;
}



int AB_Banking_AddAccount(AB_BANKING *ab, AB_ACCOUNT *a) {
  int rv;
  char groupName[32];
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbP;

  assert(ab);
  assert(a);
  rv=AB_Provider_ExtendAccount(AB_Account_GetProvider(a), a,
			       AB_ProviderExtendMode_Add,
			       NULL);
  if (rv)
    return rv;

  db=GWEN_DB_Group_new("account");
  AB_Account_toDb(a, db);
  dbP=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT,
		       "data/backend");
  rv=AB_Provider_ExtendAccount(AB_Account_GetProvider(a), a,
			       AB_ProviderExtendMode_Save,
			       dbP);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(db);
    return rv;
  }

  rv=GWEN_ConfigMgr_GetUniqueId(ab->configMgr,
				AB_CFG_GROUP_ACCOUNTS,
				groupName, sizeof(groupName)-1);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Unable to create a unique id for account [%08x] (%d)",
	      AB_Account_GetUniqueId(a), rv);
    GWEN_DB_Group_free(db);
    return rv;
  }
  groupName[sizeof(groupName)-1]=0;

  rv=GWEN_ConfigMgr_LockGroup(ab->configMgr,
			      AB_CFG_GROUP_ACCOUNTS,
			      groupName);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Unable to lock account config [%08x] (%d)",
	      AB_Account_GetUniqueId(a), rv);
    GWEN_DB_Group_free(db);
    return rv;
  }

  rv=GWEN_ConfigMgr_SetGroup(ab->configMgr,
			     AB_CFG_GROUP_ACCOUNTS,
			     groupName,
			     db);
  GWEN_DB_Group_free(db);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Unable to save account config [%08x] (%d)",
	      AB_Account_GetUniqueId(a), rv);
    GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
			       AB_CFG_GROUP_ACCOUNTS,
			       groupName);
    return rv;
  }

  /* unlock */
  rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
				AB_CFG_GROUP_ACCOUNTS,
				groupName);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Unable to unlock account config [%08x] (%d)",
	      AB_Account_GetUniqueId(a), rv);
    return rv;
  }

  AB_Account_SetDbId(a, groupName);
  AB_Account_List_Add(a, ab->accounts);
  return 0;
}



int AB_Banking_DeleteAccount(AB_BANKING *ab, AB_ACCOUNT *a) {
  int rv;
  const char *groupName;

  assert(ab);
  assert(a);

  rv=AB_Account_List_Del(a);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error on removing account from list (%d)", rv);
    return rv;
  }

  rv=AB_Provider_ExtendAccount(AB_Account_GetProvider(a), a,
			       AB_ProviderExtendMode_Remove,
			       NULL);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error on remove extension of account (%d)", rv);
    return rv;
  }

  groupName=AB_Account_GetDbId(a);
  if (groupName) {
    rv=GWEN_ConfigMgr_DeleteGroup(ab->configMgr,
				  AB_CFG_GROUP_ACCOUNTS,
				  groupName);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Unable to delete account config [%08x] (%d)",
		AB_Account_GetUniqueId(a), rv);
      return rv;
    }
  }

  AB_Account_free(a);
  return 0;
}


uint64_t AB_Banking__char2uint64(const char *accountId) {
  uint64_t res=0;
  const char *s;

  s=accountId;
  while(*s) {
    if (*s<'0' || *s>'9')
      return 0;
    res*=10;
    res+=(*s-'0');
    s++;
  }

  return res;
}



AB_ACCOUNT *AB_Banking_GetAccountByCodeAndNumber(const AB_BANKING *ab,
                                                 const char *bankCode,
                                                 const char *accountId){
  AB_ACCOUNT *a;
  const char *our_bankCode, *our_accountId;

  if (accountId==NULL)
    return NULL;
  assert(ab);
  if (AB_Account_List_GetCount(ab->accounts)==0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No accounts");
    return 0;
  }
  a=AB_Account_List_First(ab->accounts);
  assert(a);
  while(a) {
    if (bankCode) {
      our_bankCode = AB_Account_GetBankCode(a);
      our_accountId = AB_Account_GetAccountNumber(a);
      if (our_bankCode && strcasecmp(our_bankCode, bankCode)==0 &&
          our_accountId && strcasecmp(our_accountId, accountId)==0)
        break;
    }
    else {
      our_accountId = AB_Account_GetAccountNumber(a);
      if (our_accountId && strcasecmp(our_accountId, accountId)==0)
        break;
    }
    a=AB_Account_List_Next(a);
  } /* while */

  if (!a) {
    uint64_t an;

    an=AB_Banking__char2uint64(accountId);
    if (an) {
      a=AB_Account_List_First(ab->accounts);
      assert(a);
      while(a) {
        uint64_t lan;
    
        lan=AB_Banking__char2uint64(AB_Account_GetAccountNumber(a));
        if (lan) {
          if (bankCode) {
	    our_bankCode = AB_Account_GetBankCode(a);
            if (our_bankCode && strcasecmp(our_bankCode, bankCode)==0 &&
                an==lan)
              break;
          }
          else {
            if (an==lan)
              break;
          }
        }
        a=AB_Account_List_Next(a);
      } /* while */
    }
  }

  return a;
}



AB_ACCOUNT *AB_Banking_GetAccountByIban(const AB_BANKING *ab,
					const char *iban){
  AB_ACCOUNT *a;
  const char *our_iban;

  if (iban==NULL)
    return NULL;
  assert(ab);
  if (AB_Account_List_GetCount(ab->accounts)==0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No accounts");
    return 0;
  }
  a=AB_Account_List_First(ab->accounts);
  assert(a);
  while(a) {
    if (iban) {
      our_iban=AB_Account_GetIBAN(a);
      if (our_iban && strcasecmp(our_iban, iban)==0)
	break;
    }
    a=AB_Account_List_Next(a);
  } /* while */

  return a;
}




