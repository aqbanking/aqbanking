/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "provider_p.h"
#include <gwenhywfar/gui.h>
#include "aqbanking/dialogs/dlg_newuser_be.h"
#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/httpsession.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/dialog.h>
#include <strings.h>
#include <string.h>
#include "userdialog.h"
#include "provider_request.h"



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int AG_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int AG_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);

static int AG_Provider_SendCommands(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_IMEXPORTER_CONTEXT *ctx);
static int AG_Provider_UpdateAccountSpec(AB_PROVIDER *pro, AB_ACCOUNT_SPEC *as, int doLock);

static void AG_Provider_AddTransactionLimit(int limit, AB_TRANSACTION_LIMITS_LIST *tll);
static int AG_Provider_ExecGetBal(AB_PROVIDER *pro, AB_IMEXPORTER_ACCOUNTINFO *ai, AB_ACCOUNT *account, char *token);
static int AG_Provider_ExecGetTrans(AB_PROVIDER *pro, AB_IMEXPORTER_ACCOUNTINFO *ai, AB_ACCOUNT *account, AB_TRANSACTION *j, char *token);



/* ------------------------------------------------------------------------------------------------
 * code
 * ------------------------------------------------------------------------------------------------
 */

GWEN_INHERIT(AB_PROVIDER, AG_PROVIDER);


AB_PROVIDER *AG_Provider_new(AB_BANKING *ab)
{
  AB_PROVIDER *pro;
  AG_PROVIDER *gp;

  pro=AB_Provider_new(ab, "aqgivve");
  assert(pro);

  AB_Provider_SetInitFn(pro, AG_Provider_Init);
  AB_Provider_SetFiniFn(pro, AG_Provider_Fini);

  AB_Provider_SetGetNewUserDialogFn(pro, AG_GetNewUserDialog);
  AB_Provider_SetGetEditUserDialogFn(pro, AG_GetEditUserDialog);

  AB_Provider_SetSendCommandsFn(pro, AG_Provider_SendCommands);
  AB_Provider_SetCreateUserObjectsFn(pro, AG_Provider_CreateUserObject);

  AB_Provider_SetCreateAccountObjectsFn(pro, AG_Provider_CreateAccountObject);
  AB_Provider_SetUpdateAccountSpecFn(pro, AG_Provider_UpdateAccountSpec);

  AB_Provider_AddFlags(pro, AB_PROVIDER_FLAGS_HAS_EDITUSER_DIALOG | AB_PROVIDER_FLAGS_HAS_NEWUSER_DIALOG);

  GWEN_NEW_OBJECT(AG_PROVIDER, gp);
  GWEN_INHERIT_SETDATA(AB_PROVIDER, AG_PROVIDER, pro, gp, AG_Provider_FreeData);

  return pro;
}



int AG_Provider_SendCommands(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_IMEXPORTER_CONTEXT *ctx)
{

  AB_USERQUEUE_LIST *uql;
  AB_USERQUEUE *uq;
  AB_ACCOUNTQUEUE *aq;

  uql=AB_UserQueue_List_new();
  AB_Provider_SortProviderQueueIntoUserQueueList(pro, pq, uql);
  uq=AB_UserQueue_List_First(uql);
  while (uq) {

    AB_ACCOUNTQUEUE_LIST *aql;
    AB_USER *u;

    u=AB_UserQueue_GetUser(uq);
    DBG_INFO(AQGIVVE_LOGDOMAIN, "Handling user \"%s\"", AB_User_GetUserId(u));

    char *token;

    token = AG_Provider_Request_GetToken(u);

    if (token) {

      aql=AB_UserQueue_GetAccountQueueList(uq);


      aq=AB_AccountQueue_List_First(aql);
      while (aq) {

        AB_ACCOUNT *a=AB_AccountQueue_GetAccount(aq);

        AB_IMEXPORTER_ACCOUNTINFO *ai=AB_ImExporterContext_GetOrAddAccountInfo(ctx,
                                                                               AB_Account_GetUniqueId(a),
                                                                               AB_Account_GetIban(a),
                                                                               AB_Account_GetBankCode(a),
                                                                               AB_Account_GetAccountNumber(a),
                                                                               AB_Account_GetAccountType(a));

        AB_TRANSACTION_LIST2 *tl2 = AB_AccountQueue_GetTransactionList(aq);

        AB_TRANSACTION_LIST2_ITERATOR *it;

        it=AB_Transaction_List2_First(tl2);

        AB_TRANSACTION *t;

        t=AB_Transaction_List2Iterator_Data(it);
        while (t) {
          int command = AB_Transaction_GetCommand(t);
          DBG_INFO(AQGIVVE_LOGDOMAIN, "command: %d", command);
          switch (command) {
          case AB_Transaction_CommandGetBalance:
            AG_Provider_ExecGetBal(pro, ai, a, token);
            break;
          case AB_Transaction_CommandGetTransactions:
            AG_Provider_ExecGetTrans(pro, ai, a, t, token);
            break;
          default:
            break;
          }
          AB_Transaction_SetStatus(t, AB_Transaction_StatusAccepted);
          t=AB_Transaction_List2Iterator_Next(it);
        }

        AB_Transaction_List2Iterator_free(it);


        aq = AB_AccountQueue_List_Next(aq);
      }


      free(token);
    }
    uq=AB_UserQueue_List_Next(uq);
  }

  return 0;
}



void AG_Provider_FreeData(void *bp, void *p)
{
}



int AG_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData)
{
  if (!GWEN_Logger_IsOpen(AQGIVVE_LOGDOMAIN)) {
    GWEN_Logger_Open(AQGIVVE_LOGDOMAIN, "aqgivve", 0, GWEN_LoggerType_Console, GWEN_LoggerFacility_User);
    GWEN_Logger_SetLevel(AQGIVVE_LOGDOMAIN, GWEN_LoggerLevel_Info);
  }

  return 0;
}



int AG_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData)
{
  return 0;
}



AB_USER *AG_Provider_CreateUserObject(AB_PROVIDER *pro)
{
  AB_USER *u;
  u=AB_User_new();
  AB_User_SetProvider(u, pro);
  AB_User_SetBackendName(u, AQGIVVE_BACKENDNAME);
  AB_User_SetCountry(u, "de");
  AB_User_SetBankCode(u, "GivveCard");

  return u;
}



AB_ACCOUNT *AG_Provider_CreateAccountObject(AB_PROVIDER *pro)
{
  AB_ACCOUNT *a;
  a=AB_Account_new();
  AB_Account_SetProvider(a, pro);
  AB_Account_SetBackendName(a, AQGIVVE_BACKENDNAME);
  AB_Account_SetBankName(a, "GivveCard");
  AB_Account_SetBankCode(a, "GivveCard");
  AB_Account_SetAccountType(a, AB_AccountType_Bank);

  return a;
}



void AG_Provider_AddTransactionLimit(int limit, AB_TRANSACTION_LIMITS_LIST *tll)
{
  AB_TRANSACTION_LIMITS *balance_limits=AB_TransactionLimits_new();
  AB_TransactionLimits_SetCommand(balance_limits, limit);
  AB_TransactionLimits_SetMaxLinesPurpose(balance_limits, 1);
  AB_TransactionLimits_List_Add(balance_limits, tll);
}



int AG_Provider_UpdateAccountSpec(AB_PROVIDER *pro, AB_ACCOUNT_SPEC *as, int doLock)
{
  AB_TRANSACTION_LIMITS_LIST *tll;

  tll=AB_TransactionLimits_List_new();
  AG_Provider_AddTransactionLimit(AB_Transaction_CommandGetBalance, tll);
  AG_Provider_AddTransactionLimit(AB_Transaction_CommandGetTransactions, tll);

  AB_AccountSpec_SetTransactionLimitsList(as, tll);

  return 0;
}



int AG_Provider_ExecGetBal(AB_PROVIDER *pro, AB_IMEXPORTER_ACCOUNTINFO *ai, AB_ACCOUNT *account, char *token)
{
  AB_BALANCE *bal = AG_Provider_Request_GetBalance(account, token);
  if (bal) {
    AB_ImExporterAccountInfo_AddBalance(ai, bal);
  }

  return 0;

}



int AG_Provider_ExecGetTrans(AB_PROVIDER *pro, AB_IMEXPORTER_ACCOUNTINFO *ai, AB_ACCOUNT *account, AB_TRANSACTION *j, char *token)
{

  AB_TRANSACTION_LIST *list = AG_Provider_Request_GetTransactions(account, AB_Transaction_GetFirstDate(j),
                                                                  AB_Transaction_GetLastDate(j), token);
  AB_TRANSACTION *t;
  if (list) {
    DBG_INFO(AQGIVVE_LOGDOMAIN, "trans count: %d", AB_Transaction_List_GetCount(list));
    t = AB_Transaction_List_First(list);
    while (t) {
      DBG_INFO(AQGIVVE_LOGDOMAIN, "trans: %s", AB_Transaction_GetFiId(t));
      AB_Transaction_List_Del(t);
      AB_ImExporterAccountInfo_AddTransaction(ai, t);
      t = AB_Transaction_List_First(list);

    }
    AB_Transaction_List_free(list);
  }
  return 0;
}



