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

  i=0;
  while(jobList[i]!=AB_Transaction_CommandUnknown) {
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
    i++;
  } /* while i */

  return 0;
}



int AH_Provider_UpdateAccountSpec(AB_PROVIDER *pro, AB_ACCOUNT_SPEC *as, int doLock) {
  int rv;
  uint32_t aid=0;
  AB_ACCOUNT *a=NULL;
  uint32_t uid=0;
  AB_USER *u=NULL;
  AB_TRANSACTION_LIMITS_LIST *tll;

  /* get unique account id */
  aid=AB_AccountSpec_GetUniqueId(as);
  if (aid==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Account has no unique id assigned, SNH!");
    return GWEN_ERROR_INTERNAL;
  }

  /* get corresponding account */
  rv=AB_Provider_GetAccount(pro, aid, doLock, doLock, &a);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_Account_free(a);
    return rv;
  }
  assert(a);

  /* get user id */
  uid=AB_Account_GetUserId(a);
  if (uid==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Account has no user id assigned, SNH!");
    return GWEN_ERROR_INTERNAL;
  }

  /* get user */
  rv=AB_Provider_GetUser(pro, uid, doLock, doLock, &u);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_Account_free(a);
    return rv;
  }

  /* create and set transaction limits per command */
  tll=AB_TransactionLimits_List_new();
  rv=AH_Provider__CreateTransactionLimitsForAccount(pro, u, a, tll);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_TransactionLimits_List_free(tll);
    AB_User_free(u);
    AB_Account_free(a);
    return rv;
  }
  AB_AccountSpec_SetTransactionLimitsList(as, tll);

  AB_User_free(u);
  AB_Account_free(a);

  return 0;
}











