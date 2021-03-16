/***************************************************************************
    begin       : Tue Jun 03 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "provider_accspec.h"

#include <aqbanking/banking_be.h>

#include "aqhbci/banking/provider_l.h"
#include "aqhbci/banking/provider_job.h"
#include "aqhbci/joblayer/job_l.h"
#include "aqhbci/tan/tanmethod.h"
#include "hbci_l.h"



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _createTransactionLimitsForAccount(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *acc,
                                              AB_TRANSACTION_LIMITS_LIST *tll);
static AB_ACCOUNT_SPEC *_createAccountSpecWithUserAndAccount(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a);
static int _updateAccountSpecWithUserAndAccount(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_ACCOUNT_SPEC *as);
static void _copyAccountToAccountSpec(const AB_ACCOUNT *acc, AB_ACCOUNT_SPEC *as);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AH_Provider_UpdateAccountSpec(AB_PROVIDER *pro, AB_ACCOUNT_SPEC *as, int doLock)
{
  int rv;
  uint32_t aid=0;
  AB_ACCOUNT *a=NULL;
  uint32_t uid=0;
  AB_USER *u=NULL;
  AB_TRANSACTION_LIMITS_LIST *tll;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Updating account spec for account %u", (unsigned int) AB_AccountSpec_GetUniqueId(as));

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
  rv=_createTransactionLimitsForAccount(pro, u, a, tll);
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



int AH_Provider_CreateAndWriteAccountSpecWithUserAndAccount(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a)
{
  AB_ACCOUNT_SPEC *as;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Creating account spec for user %u / account %u",
           (unsigned int) AB_User_GetUniqueId(u),
           (unsigned int) AB_Account_GetUniqueId(a));

  /* create account spec from given account using given user */
  as=_createAccountSpecWithUserAndAccount(pro, u, a);
  if (as==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return GWEN_ERROR_GENERIC;
  }

  /* write account spec to disk */
  rv=AB_Banking_WriteAccountSpec(AB_Provider_GetBanking(pro), as);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    AB_AccountSpec_free(as);
    return rv;
  }
  AB_AccountSpec_free(as);


  return 0;
}



AB_ACCOUNT_SPEC *_createAccountSpecWithUserAndAccount(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a)
{
  AB_ACCOUNT_SPEC *as;
  int rv;

  as=AB_AccountSpec_new();
  _copyAccountToAccountSpec(a, as);
  rv=_updateAccountSpecWithUserAndAccount(pro, u, a, as);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_AccountSpec_free(as);
    return NULL;
  }

  return as;
}



int _updateAccountSpecWithUserAndAccount(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_ACCOUNT_SPEC *as)
{
  int rv;
  AB_TRANSACTION_LIMITS_LIST *tll;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Updating account spec for account %u", (unsigned int) AB_Account_GetUniqueId(a));

  /* create and set transaction limits per command */
  tll=AB_TransactionLimits_List_new();
  rv=_createTransactionLimitsForAccount(pro, u, a, tll);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_TransactionLimits_List_free(tll);
    return rv;
  }
  AB_AccountSpec_SetTransactionLimitsList(as, tll);

  return 0;
}




void _copyAccountToAccountSpec(const AB_ACCOUNT *acc, AB_ACCOUNT_SPEC *as)
{
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
  AB_AccountSpec_SetBankName(as, AB_Account_GetBankName(acc));
  AB_AccountSpec_SetAccountNumber(as, AB_Account_GetAccountNumber(acc));
  AB_AccountSpec_SetSubAccountNumber(as, AB_Account_GetSubAccountId(acc));

}



int _createTransactionLimitsForAccount(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *acc,
                                       AB_TRANSACTION_LIMITS_LIST *tll)
{
  int rv;
  int i;
  int jobList[]= {
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
    AB_Transaction_CommandGetEStatements2,
    AB_Transaction_CommandGetDepot,
    AB_Transaction_CommandUnknown
  };

  DBG_INFO(AQHBCI_LOGDOMAIN, "Creating transaction limits for account \"%u\"", (unsigned int)AB_Account_GetUniqueId(acc));
  DBG_INFO(AQHBCI_LOGDOMAIN,
           "You may see some messages like \"Job not supported with this account\" below, those are okay, please ignore");


  i=0;
  while (jobList[i]!=AB_Transaction_CommandUnknown) {
    AH_JOB *j=NULL;
    AB_TRANSACTION_LIMITS *limits=NULL;

    DBG_INFO(AQHBCI_LOGDOMAIN, "Creating transaction limits for job \"%s\"", AB_Transaction_Command_toString(jobList[i]));
    DBG_INFO(AQHBCI_LOGDOMAIN, "- creating job");
    rv=AH_Provider_CreateHbciJob(pro, u, acc, jobList[i], &j);
    if (rv<0) {
      if (rv==GWEN_ERROR_NOT_AVAILABLE) {
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Job \"%s\" is not available", AB_Transaction_Command_toString(jobList[i]));
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
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "- adding limits");
      AB_TransactionLimits_List_Add(limits, tll);
      AH_Job_free(j);
    }
    i++;
  } /* while i */

  return 0;
}


