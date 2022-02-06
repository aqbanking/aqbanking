/***************************************************************************
    begin       : Sat Dec 01 2018
    copyright   : (C) 2022 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "aqpaypal/provider_accspec.h"
#include "aqpaypal/aqpaypal.h"

#include "aqbanking/backendsupport/provider_be.h"

#include <gwenhywfar/debug.h>



static int _createTransactionLimitsForAccount(AB_PROVIDER *pro, const AB_ACCOUNT *acc, AB_TRANSACTION_LIMITS_LIST *tll);




int APY_Provider_UpdateAccountSpec(AB_PROVIDER *pro, AB_ACCOUNT_SPEC *as, int doLock)
{
  int rv;
  AB_ACCOUNT *a=NULL;
  AB_TRANSACTION_LIMITS_LIST *tll;

  rv=AB_Provider_GetAccount(pro, AB_AccountSpec_GetUniqueId(as), doLock, doLock, &a);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  tll=AB_TransactionLimits_List_new();
  rv=_createTransactionLimitsForAccount(pro, a, tll);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    AB_TransactionLimits_List_free(tll);
    AB_Account_free(a);
    return rv;
  }
  AB_AccountSpec_SetTransactionLimitsList(as, tll);

  AB_Account_free(a);

  return 0;
}



int _createTransactionLimitsForAccount(AB_PROVIDER *pro, const AB_ACCOUNT *acc, AB_TRANSACTION_LIMITS_LIST *tll)
{
  int i;
  int jobList[]= {
    AB_Transaction_CommandGetBalance,
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
    AB_Transaction_CommandUnknown
  };

  i=0;
  while (jobList[i]!=AB_Transaction_CommandUnknown) {
    AB_TRANSACTION_LIMITS *limits=NULL;

    DBG_INFO(AQPAYPAL_LOGDOMAIN, "Handling job \"%s\"", AB_Transaction_Command_toString(jobList[i]));

    limits=AB_TransactionLimits_new();
    AB_TransactionLimits_SetCommand(limits, jobList[i]);
    AB_TransactionLimits_SetMaxLinesPurpose(limits, 1);

    DBG_INFO(AQPAYPAL_LOGDOMAIN, "- adding limits");
    AB_TransactionLimits_List_Add(limits, tll);
    i++;
  } /* while */

  return 0;
}



