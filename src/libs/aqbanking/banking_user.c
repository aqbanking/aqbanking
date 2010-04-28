/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: banking_simple.h 1106 2007-01-09 21:14:59Z martin $
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


AB_USER_LIST2 *AB_Banking_GetUsers(const AB_BANKING *ab){
  AB_USER_LIST2 *ul;
  AB_USER *u;

  assert(ab);
  if (AB_User_List_GetCount(ab->users)==0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No users");
    return 0;
  }
  ul=AB_User_List2_new();
  u=AB_User_List_First(ab->users);
  assert(u);
  while(u) {
    AB_User_List2_PushBack(ul, u);
    u=AB_User_List_Next(u);
  } /* while */

  return ul;
}



AB_USER *AB_Banking_GetUser(const AB_BANKING *ab, uint32_t uniqueId){
  AB_USER *u;

  assert(ab);
  if (AB_User_List_GetCount(ab->users)==0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No users");
    return 0;
  }
  u=AB_User_List_First(ab->users);
  assert(u);
  while(u) {
    if (AB_User_GetUniqueId(u)==uniqueId)
      break;
    u=AB_User_List_Next(u);
  } /* while */

  return u;
}



AB_USER *AB_Banking_FindUser(const AB_BANKING *ab,
                             const char *backendName,
                             const char *country,
                             const char *bankId,
                             const char *userId,
                             const char *customerId) {
  AB_USER *u;

  assert(ab);
  if (AB_User_List_GetCount(ab->users)==0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No users");
    return 0;
  }
  u=AB_User_List_First(ab->users);
  assert(u);

  if (!backendName) backendName="*";
  if (!country) country="*";
  if (!bankId) bankId="*";
  if (!userId) userId="*";
  if (!customerId) customerId="*";

  while(u) {
    const char *lCountry;
    const char *lBankCode;
    const char *lUserId;
    const char *lCustomerId;

    lCountry=AB_User_GetCountry(u);
    if (!lCountry)
      lCountry="";
    lBankCode=AB_User_GetBankCode(u);
    if (!lBankCode)
      lBankCode="";
    lUserId=AB_User_GetUserId(u);
    if (!lUserId)
      lUserId="";
    lCustomerId=AB_User_GetCustomerId(u);
    if (!lCustomerId)
      lCustomerId="";

    if ((-1!=GWEN_Text_ComparePattern(AB_User_GetBackendName(u),
                                      backendName, 0)) &&
        (-1!=GWEN_Text_ComparePattern(lCountry,
                                      country, 0)) &&
        (-1!=GWEN_Text_ComparePattern(lBankCode,
                                      bankId, 0)) &&
        (-1!=GWEN_Text_ComparePattern(lUserId,
                                      userId, 0)) &&
        (-1!=GWEN_Text_ComparePattern(lCustomerId,
                                      customerId, 0)))
      break;
    u=AB_User_List_Next(u);
  } /* while */

  return u;
}



AB_USER_LIST2 *AB_Banking_FindUsers(const AB_BANKING *ab,
                                    const char *backendName,
                                    const char *country,
				    const char *bankId,
				    const char *userId,
				    const char *customerId) {
  AB_USER_LIST2 *ul;
  AB_USER *u;

  assert(ab);
  if (AB_User_List_GetCount(ab->users)==0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No users");
    return 0;
  }
  ul=AB_User_List2_new();
  u=AB_User_List_First(ab->users);
  assert(u);

  if (!backendName) backendName="*";
  if (!country) country="*";
  if (!bankId) bankId="*";
  if (!userId) userId="*";
  if (!customerId) customerId="*";

  while(u) {
    const char *lCountry;
    const char *lBankCode;
    const char *lUserId;
    const char *lCustomerId;

    lCountry=AB_User_GetCountry(u);
    if (!lCountry)
      lCountry="";
    lBankCode=AB_User_GetBankCode(u);
    if (!lBankCode)
      lBankCode="";
    lUserId=AB_User_GetUserId(u);
    if (!lUserId)
      lUserId="";
    lCustomerId=AB_User_GetCustomerId(u);
    if (!lCustomerId)
      lCustomerId="";

    if ((-1!=GWEN_Text_ComparePattern(AB_User_GetBackendName(u),
                                      backendName, 0)) &&
        (-1!=GWEN_Text_ComparePattern(lCountry,
                                      country, 0)) &&
        (-1!=GWEN_Text_ComparePattern(lBankCode,
                                      bankId, 0)) &&
        (-1!=GWEN_Text_ComparePattern(lUserId,
                                      userId, 0)) &&
        (-1!=GWEN_Text_ComparePattern(lCustomerId,
				      customerId, 0))) {
      AB_User_List2_PushBack(ul, u);
    }
    u=AB_User_List_Next(u);
  } /* while */

  if (AB_User_List2_GetSize(ul)==0) {
    AB_User_List2_free(ul);
    return 0;
  }

  return ul;
}



AB_USER *AB_Banking_CreateUser(AB_BANKING *ab, const char *backendName) {
  AB_USER *u;
  AB_PROVIDER *pro;
  uint32_t uid;
  int rv;

  assert(ab);
  pro=AB_Banking_GetProvider(ab, backendName);
  if (!pro) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Backend \"%s\" not found", backendName);
    return 0;
  }

  u=AB_User_new(ab);
  AB_User_SetBackendName(u, AB_Provider_GetName(pro));
  uid=AB_Banking_GetUniqueId(ab);
  assert(uid);
  AB_User_SetUniqueId(u, uid);
  rv=AB_Provider_ExtendUser(pro, u, AB_ProviderExtendMode_Create, NULL);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error extending user (%d)", rv);
    AB_User_free(u);
    return 0;
  }

  return u;
}



int AB_Banking_AddUser(AB_BANKING *ab, AB_USER *u) {
  int rv;
  AB_USER *uTmp;
  char groupName[32];
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbP;
  const char *s;

  assert(ab);
  assert(u);

  s=AB_User_GetUserId(u);
  if (!s || *s==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Missing user id");
    return GWEN_ERROR_INVALID;
  }

  uTmp=AB_User_List_First(ab->users);
  while(uTmp) {
    if (uTmp==u) {
      DBG_INFO(AQBANKING_LOGDOMAIN,
	       "User already added!");
      return GWEN_ERROR_INVALID;
    }
    uTmp=AB_User_List_Next(uTmp);
  }

  rv=AB_Provider_ExtendUser(AB_User_GetProvider(u), u,
			    AB_ProviderExtendMode_Add,
			    NULL);
  if (rv)
    return rv;

  db=GWEN_DB_Group_new("user");
  AB_User_toDb(u, db);
  dbP=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT,
		       "data/backend");
  rv=AB_Provider_ExtendUser(AB_User_GetProvider(u), u,
			    AB_ProviderExtendMode_Save,
			    dbP);
  if (rv) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(db);
    return rv;
  }

  rv=GWEN_ConfigMgr_GetUniqueId(ab->configMgr,
				AB_CFG_GROUP_USERS,
				groupName, sizeof(groupName)-1);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Unable to create a unique id for user [%08x] (%d)",
	      AB_User_GetUniqueId(u), rv);
    GWEN_DB_Group_free(db);
    return rv;
  }
  groupName[sizeof(groupName)-1]=0;

  rv=GWEN_ConfigMgr_LockGroup(ab->configMgr,
			      AB_CFG_GROUP_USERS,
			      groupName);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Unable to lock user config [%08x] (%d)",
	      AB_User_GetUniqueId(u), rv);
    GWEN_DB_Group_free(db);
    return rv;
  }

  rv=GWEN_ConfigMgr_SetGroup(ab->configMgr,
			     AB_CFG_GROUP_USERS,
			     groupName,
			     db);
  GWEN_DB_Group_free(db);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Unable to save user config [%08x] (%d)",
	      AB_User_GetUniqueId(u), rv);
    GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
			       AB_CFG_GROUP_USERS,
			       groupName);
    return rv;
  }

  /* unlock */
  rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
				AB_CFG_GROUP_USERS,
				groupName);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Unable to unlock user config [%08x] (%d)",
	      AB_User_GetUniqueId(u), rv);
    return rv;
  }

  AB_User_SetDbId(u, groupName);
  AB_User_List_Add(u, ab->users);
  return 0;
}



static AB_USER *checkusers_fn(AB_USER *item, void *user_data) {
  AB_USER *u = user_data;
  return (item == u) ? item : NULL;
}



static AB_ACCOUNT *checkaccounts_fn(AB_ACCOUNT *item, void *user_data) {
  AB_USER_LIST2 *userlist = AB_Account_GetUsers(item);
  AB_USER *u = AB_User_List2_ForEach(userlist, checkusers_fn, user_data);
  AB_User_List2_free(userlist);
  return u ? item : NULL;
}



AB_ACCOUNT *AB_Banking_FindFirstAccountOfUser(AB_BANKING *ab, AB_USER *u) {
  AB_ACCOUNT_LIST2 *acclist;
  AB_ACCOUNT *result;

  assert(ab);
  assert(u);

  acclist = AB_Banking_GetAccounts(ab);
  result = AB_Account_List2_ForEach(acclist, checkaccounts_fn, u);
  AB_Account_List2_free(acclist);
  return result;
}



int AB_Banking_DeleteUser(AB_BANKING *ab, AB_USER *u) {
  int rv;
  AB_ACCOUNT *acc_rv;
  const char *groupName;

  assert(ab);
  assert(u);

  acc_rv=AB_Banking_FindFirstAccountOfUser(ab, u);
  if (acc_rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Error on removing user: Still belongs to an account (bankcode %s, accountnumber %s). Delete the account first.",
	      AB_Account_GetBankCode(acc_rv),
	      AB_Account_GetAccountNumber(acc_rv));
    return GWEN_ERROR_INVALID;
  }

  rv=AB_User_List_Del(u);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error on removing user from list (%d)", rv);
    return rv;
  }

  rv=AB_Provider_ExtendUser(AB_User_GetProvider(u), u,
			    AB_ProviderExtendMode_Remove,
			    NULL);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error on remove extension of user (%d)", rv);
    return rv;
  }

  groupName=AB_User_GetDbId(u);
  if (groupName) {
    rv=GWEN_ConfigMgr_DeleteGroup(ab->configMgr,
				  AB_CFG_GROUP_USERS,
				  groupName);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Unable to delete user config [%08x] (%d)",
		AB_User_GetUniqueId(u), rv);
      return rv;
    }
  }

  AB_User_free(u);
  return 0;
}
