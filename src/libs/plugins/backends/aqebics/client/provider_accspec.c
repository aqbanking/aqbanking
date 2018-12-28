/***************************************************************************
    begin       : Wed Dec 26 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



int EBC_Provider__CreateTransactionLimitsForAccount(AB_PROVIDER *pro, AB_ACCOUNT *acc, AB_TRANSACTION_LIMITS_LIST *tll) {
  int i;
  int jobList[]={
    /* AB_Transaction_CommandGetBalance,                */
    AB_Transaction_CommandGetTransactions,
    /* AB_Transaction_CommandLoadCellPhone,             */
    /* AB_Transaction_CommandSepaTransfer,              */
    /* AB_Transaction_CommandSepaDebitNote,             */
    /* AB_Transaction_CommandSepaFlashDebitNote,        */
    /* AB_Transaction_CommandSepaCreateStandingOrder,   */
    /* AB_Transaction_CommandSepaModifyStandingOrder,   */
    /* AB_Transaction_CommandSepaDeleteStandingOrder,   */
    /* AB_Transaction_CommandSepaGetStandingOrders,     */
    /* AB_Transaction_CommandGetEStatements,            */
    AB_Transaction_CommandUnknown};

  i=0;
  while(jobList[i]!=AB_Transaction_CommandUnknown) {
    AB_TRANSACTION_LIMITS *limits=NULL;

    DBG_INFO(AQEBICS_LOGDOMAIN, "Handling job \"%s\"", AB_Transaction_Command_toString(jobList[i]));

    limits=AB_TransactionLimits_new();
    AB_TransactionLimits_SetCommand(limits, jobList[i]);

    AB_TransactionLimits_SetMaxLenPurpose(limits, 35);
    AB_TransactionLimits_SetMaxLinesPurpose(limits, 4);
    AB_TransactionLimits_SetMaxLenRemoteName(limits, 70);

    DBG_INFO(AQEBICS_LOGDOMAIN, "- adding limits");
    AB_TransactionLimits_List_Add(limits, tll);
    i++;
  } /* while */

  return 0;
}



int EBC_Provider_UpdateAccountSpec(AB_PROVIDER *pro, AB_ACCOUNT_SPEC *as, int doLock) {
  int rv;
  uint32_t aid=0;
  AB_ACCOUNT *a=NULL;
  uint32_t uid=0;
  AB_TRANSACTION_LIMITS_LIST *tll;

  DBG_INFO(AQEBICS_LOGDOMAIN, "Updating account spec for account %u", (unsigned int) AB_AccountSpec_GetUniqueId(as));

  /* get unique account id */
  aid=AB_AccountSpec_GetUniqueId(as);
  if (aid==0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Account has no unique id assigned, SNH!");
    return GWEN_ERROR_INTERNAL;
  }

  /* get corresponding account */
  rv=AB_Provider_GetAccount(pro, aid, doLock, doLock, &a);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    AB_Account_free(a);
    return rv;
  }
  assert(a);

  /* get user id */
  uid=AB_Account_GetUserId(a);
  if (uid==0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Account has no user id assigned, SNH!");
    return GWEN_ERROR_INTERNAL;
  }

  /* create and set transaction limits per command */
  tll=AB_TransactionLimits_List_new();
  rv=EBC_Provider__CreateTransactionLimitsForAccount(pro, a, tll);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    AB_TransactionLimits_List_free(tll);
    AB_Account_free(a);
    return rv;
  }
  AB_AccountSpec_SetTransactionLimitsList(as, tll);

  AB_Account_free(a);

  return 0;
}
