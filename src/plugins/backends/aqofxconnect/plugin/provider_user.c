/***************************************************************************
    begin       : Fri Nov 30 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/




int AO_Provider_AddUser(AB_PROVIDER *pro, AB_USER *u) {
  uint32_t uid;
  int rv;

  uid=AB_Banking_GetNamedUniqueId(AB_Provider_GetBanking(pro), "user", 1); /* startAtStdUniqueId=1 */
  AB_User_SetUniqueId(u, uid);
  rv=AB_Provider_WriteUser(pro, uid, 1, 1, u); /* lock, unlock */
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return 0;
}



int AO_Provider_DeleteUser(AB_PROVIDER *pro, uint32_t uid) {
  int rv;
  AB_ACCOUNT_LIST *al;

  al=AB_Account_List_new();
  rv=AB_Provider_ReadAccounts(pro, al);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    AB_Account_List_free(al);
    return rv;
  }
  else {
    AB_ACCOUNT *a;
    int cnt=0;

    a=AB_Account_List_First(al);
    while(a) {
      if (AB_Account_GetUserId(a)==uid) {
        DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Account %lu still uses this user", (unsigned long int) AB_Account_GetUniqueId(a));
        cnt++;
      }
      a=AB_Account_List_Next(a);
    }
    if (cnt>0) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "%d accounts using this user",cnt);
      AB_Account_List_free(al);
      return GWEN_ERROR_INVALID;
    }
  }
  AB_Account_List_free(al);

  rv=AB_Provider_DeleteUser(pro, uid);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return 0;
}



