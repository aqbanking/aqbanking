/***************************************************************************
    begin       : Fri Nov 30 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/* included by provider.c */


int AO_Provider_AddAccount(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a) {
  uint32_t uid;
  int rv;

  /* add account */
  uid=AB_Banking_GetNamedUniqueId(AB_Provider_GetBanking(pro), "account", 1); /* startAtStdUniqueId=1 */
  rv=AB_Provider_AddAccount(pro, a);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  uid=AB_Account_GetUniqueId(a);
  assert(uid);

  /* write account spec */
  rv=AO_Provider_WriteAccountSpecForAccount(pro, a);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AO_Provider_DeleteAccount(AB_PROVIDER *pro, uint32_t uid) {
  int rv1;
  int rv2;

  rv1=AB_Banking_DeleteAccountSpec(AB_Provider_GetBanking(pro), uid);
  if (rv1<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv1);
  }

  rv2=AB_Provider_DeleteAccount(pro, uid);
  if (rv2<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv2);
  }

  if (rv1>0)
    return rv1;
  if (rv2>0)
    return rv2;
  return 0;
}




