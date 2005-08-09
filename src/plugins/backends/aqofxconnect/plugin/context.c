/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "context_p.h"
#include "aqofxconnect_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>

#include <aqbanking/job_be.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>



AO_CONTEXT *AO_Context_new(AO_BANK *b, AO_USER *u, AB_JOB *job,
                           AB_IMEXPORTER_CONTEXT *ictx) {
  AO_CONTEXT *ctx;

  assert(b);
  assert(u);
  GWEN_NEW_OBJECT(AO_CONTEXT, ctx);
  ctx->bank=b;
  ctx->user=u;
  ctx->job=job;
  ctx->ictx=ictx;

  ctx->fi=(struct OfxFiLogin*)malloc(sizeof(struct OfxFiLogin));
  ctx->ai=(struct OfxAccountInfo*)malloc(sizeof(struct OfxAccountInfo));

  memset(ctx->fi, 0, sizeof(struct OfxFiLogin));
  memset(ctx->ai, 0, sizeof(struct OfxAccountInfo));
  ctx->ofxctx=libofx_get_new_context();

  ofx_set_status_cb(ctx->ofxctx,
                    AO_Context_StatusCallback,
                    ctx);
  ofx_set_account_cb(ctx->ofxctx,
                     AO_Context_AccountCallback,
                     ctx);
  ofx_set_statement_cb(ctx->ofxctx,
                       AO_Context_StatementCallback,
                       ctx);
  ofx_set_transaction_cb(ctx->ofxctx,
                         AO_Context_TransactionCallback,
                         ctx);

  return ctx;
}



void AO_Context_free(AO_CONTEXT *ctx) {
  if (ctx) {
    libofx_free_context(ctx->ofxctx);
    free(ctx->ai);
    free(ctx->fi);
    GWEN_FREE_OBJECT(ctx);
  }
}



struct OfxFiLogin *AO_Context_GetFi(const AO_CONTEXT *ctx){
  assert(ctx);
  return ctx->fi;
}



struct OfxAccountInfo *AO_Context_GetAi(const AO_CONTEXT *ctx){
  assert(ctx);
  return ctx->ai;
}



int AO_Context_Update(AO_CONTEXT *ctx){
  const char *s;

  memset(ctx->ai, 0, sizeof(struct OfxAccountInfo));
  memset(ctx->fi, 0, sizeof(struct OfxFiLogin));

  if (ctx->bank) {
    s=AO_Bank_GetBankId(ctx->bank);
    if (s && isdigit(*s))
      /* only copy bank id if it is a number (-> routing number)
       * otherwise it serves only identification purposes for this backend
       * and doesn't represent a routing number */
      strncpy(ctx->ai->bankid, s, OFX_BANKID_LENGTH-1);

    s=AO_Bank_GetBrokerId(ctx->bank);
    if (s)
      strncpy(ctx->ai->brokerid, s, OFX_BROKERID_LENGTH-1);

    s=AO_Bank_GetOrg(ctx->bank);
    if (s)
      strncpy(ctx->fi->org, s, OFX_ORG_LENGTH-1);

    s=AO_Bank_GetFid(ctx->bank);
    if (s)
      strncpy(ctx->fi->fid, s, OFX_FID_LENGTH-1);
  }

  if (ctx->user) {
    AO_BANK *b;
    AB_PROVIDER *pro;

    b=AO_User_GetBank(ctx->user);
    assert(b);
    pro=AO_Bank_GetProvider(b);
    assert(pro);
    s=AO_User_GetUserId(ctx->user);
    if (s) {
      strncpy(ctx->fi->userid, s, OFX_USERID_LENGTH-1);
      while (strlen(ctx->fi->userpass)<4) {
        GWEN_BUFFER *nbuf;
        int rv;
        char msg[]=I18N_NOOP("Please enter the password for user %s"
                             "<html>"
                             "Please enter the password for user <b>%s</b>"
                             "</html>");
        char msgbuf[512];

        nbuf=GWEN_Buffer_new(0, 64, 0, 1);
        GWEN_Buffer_AppendString(nbuf, "OFX::userpass::");
        GWEN_Buffer_AppendString(nbuf, s);
        snprintf(msgbuf, sizeof(msgbuf), I18N(msg), s, s);
        rv=AB_Banking_GetPin(AB_Provider_GetBanking(pro),
                             0,
                             GWEN_Buffer_GetStart(nbuf),
                             I18N("Enter Password"),
                             msgbuf,
                             ctx->fi->userpass,
                             4,
                             OFX_USERPASS_LENGTH);
        GWEN_Buffer_free(nbuf);
        if (rv) {
          memset(ctx->fi->userpass, 0, OFX_USERPASS_LENGTH);
          return rv;
        }
      } /* while */
    } /* if userId */
    else {
      memset(ctx->fi->userpass, 0, OFX_USERPASS_LENGTH);
    }
  } /* if user */
  else {
    memset(ctx->fi->userpass, 0, OFX_USERPASS_LENGTH);
  }

  if (ctx->job) {
    AB_ACCOUNT *a;
    AccountType t;

    a=AB_Job_GetAccount(ctx->job);
    assert(a);

    s=AB_Account_GetBankCode(a);
    if (s)
      /* use bank id from account */
      strncpy(ctx->ai->bankid, s, OFX_BANKID_LENGTH-1);

    s=AB_Account_GetAccountNumber(a);
    if (s)
      strncpy(ctx->ai->accountid, s, OFX_ACCOUNT_ID_LENGTH-1);
    switch(AB_Account_GetAccountType(a)) {
    case AB_AccountType_CreditCard:
      t=OFX_CREDITCARD_ACCOUNT;
      break;
    case AB_AccountType_Investment:
      t=OFX_INVEST_ACCOUNT;
      break;

    case AB_AccountType_Checking:
    case AB_AccountType_Savings:
    case AB_AccountType_Cash:
    case AB_AccountType_Bank:
    case AB_AccountType_Unknown:
    default:
      t=OFX_BANK_ACCOUNT;
      break;
    }
    ctx->ai->type=t;
  }

  return 0;
}



AO_BANK *AO_Context_GetBank(const AO_CONTEXT *ctx){
  assert(ctx);
  return ctx->bank;
}



AO_USER *AO_Context_GetUser(const AO_CONTEXT *ctx){
  assert(ctx);
  return ctx->user;
}



AB_JOB *AO_Context_GetJob(const AO_CONTEXT *ctx){
  assert(ctx);
  return ctx->job;
}



AB_IMEXPORTER_CONTEXT *AO_Context_GetImExContext(const AO_CONTEXT *ctx){
  assert(ctx);
  return ctx->ictx;
}



AB_IMEXPORTER_ACCOUNTINFO*
AO_Context_GetLastAccountInfo(const AO_CONTEXT *ctx){
  assert(ctx);
  return ctx->lastAccountInfo;
}



void AO_Context_SetLastAccountInfo(AO_CONTEXT *ctx,
                                   AB_IMEXPORTER_ACCOUNTINFO *ai){
  assert(ctx);
  ctx->lastAccountInfo=ai;
}



LibofxContextPtr AO_Context_GetOfxContext(const AO_CONTEXT *ctx){
  assert(ctx);
  return ctx->ofxctx;
}



int AO_Context_GetAbort(const AO_CONTEXT *ctx){
  assert(ctx);
  return ctx->abort;
}











/* OFX callbacks */
int AO_Context_StatusCallback(const struct OfxStatusData data,
                              void *user_data) {
  AO_CONTEXT *ctx;
  int isError=0;
  int isWarning=0;
  AB_PROVIDER *pro;
  GWEN_BUFFER *logbuf;

  ctx=(AO_CONTEXT*)user_data;
  assert(ctx->bank);
  pro=AO_Bank_GetProvider(ctx->bank);
  assert(pro);

  DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
            "StatusCallback");

  if (data.code_valid) {
    if (data.ofx_element_name_valid) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
                "%s: %d (%s, %s)",
                data.ofx_element_name,
                data.code,
                data.name,
                data.description);
    }
    else {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
                "OFX: %d (%s, %s)",
                data.code,
                data.name,
                data.description);
    }
    if (data.code!=0) {
      if (data.severity_valid) {
        if (data.severity==ERROR)
          isError=1;
        else if (data.severity==WARN)
          isWarning=1;
      }
      else {
        isError=1;
      }
    } /* if code is not 0 */

    /* log server response */
    logbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(logbuf, "OFX: ");
    GWEN_Buffer_AppendString(logbuf, data.name);
    GWEN_Buffer_AppendString(logbuf, " (");
    if (data.server_message_valid) {
      GWEN_Buffer_AppendString(logbuf, data.server_message);
    }
    else {
      GWEN_Buffer_AppendString(logbuf, data.description);
    }
    GWEN_Buffer_AppendString(logbuf, ")");

    if (isError) {
      AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                             0,
                             AB_Banking_LogLevelError,
                             GWEN_Buffer_GetStart(logbuf));
    }
    else if (isWarning) {
      AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                             0,
                             AB_Banking_LogLevelWarn,
                             GWEN_Buffer_GetStart(logbuf));
    }
    else {
      AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                             0,
                             AB_Banking_LogLevelNotice,
                             GWEN_Buffer_GetStart(logbuf));
    }
    GWEN_Buffer_free(logbuf);

    if (ctx->job) {
      if (isError) {
        const char *s;

        if (data.server_message_valid)
          s=data.server_message;
        else
          s=data.description;
        if (AB_Job_GetStatus(ctx->job)!=AB_Job_StatusError) {
          /* only set error code and result text for first error */
          AB_Job_SetStatus(ctx->job, AB_Job_StatusError);
          AB_Job_SetResultText(ctx->job, s);
        }
      }
    }
    if (isError) {
      ctx->lastErrorCode=data.code;
      if (!(data.ofx_element_name_valid &&
            strcasecmp(data.ofx_element_name, "SONRS")!=0)) {
        DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
                  "Will abort user queue");
        ctx->abort=1;
      }
    } /* if this is an error */

  } /* if code is valid */
  else {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "Status with invalid code");
  }
  return 0;
}



int AO_Context_AccountCallback(const struct OfxAccountData data,
                                void *user_data){
  AO_CONTEXT *ctx;
  AB_IMEXPORTER_ACCOUNTINFO *ai;

  ctx=(AO_CONTEXT*)user_data;

  DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
            "AccountCallback");

  ai=AB_ImExporterAccountInfo_new();
  if (data.account_number_valid) {
    AB_ImExporterAccountInfo_SetAccountNumber(ai, data.account_number);
    if (*(data.account_name))
      AB_ImExporterAccountInfo_SetAccountName(ai, data.account_name);
  }
  else {
    AB_ImExporterAccountInfo_SetAccountNumber(ai, "----");
  }

  if (data.bank_id_valid)
    AB_ImExporterAccountInfo_SetBankCode(ai, data.bank_id);

  if (data.account_type_valid) {
    AB_ACCOUNT_TYPE at;

    switch(data.account_type) {
    case OFX_CHECKING:   at=AB_AccountType_Checking; break;
    case OFX_SAVINGS:    at=AB_AccountType_Savings; break;
    case OFX_MONEYMRKT:  at=AB_AccountType_Investment; break;
    case OFX_CREDITLINE: at=AB_AccountType_Bank; break;
    case OFX_CMA:        at=AB_AccountType_Cash; break;
    case OFX_CREDITCARD: at=AB_AccountType_CreditCard; break;
    case OFX_INVESTMENT: at=AB_AccountType_Investment; break;
    default:
      at=AB_AccountType_Bank;
      break;
    }
    AB_ImExporterAccountInfo_SetType(ai, at);
  }
  else {
    AB_ImExporterAccountInfo_SetType(ai, AB_AccountType_Bank);
  }
  AB_ImExporterContext_AddAccountInfo(ctx->ictx, ai);
  AO_Context_SetLastAccountInfo(ctx, ai);
  return 0;
}



int AO_Context_SecurityCallback(const struct OfxSecurityData data,
                                 void *user_data) {
  AO_CONTEXT *ctx;

  ctx=(AO_CONTEXT*)user_data;
  DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
            "SecurityCallback");

  return 0;
}



int AO_Context_TransactionCallback(const struct OfxTransactionData data,
                                    void *user_data) {
  AO_CONTEXT *ctx;
  AB_IMEXPORTER_ACCOUNTINFO *ai;

  ctx=(AO_CONTEXT*)user_data;
  DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
            "TransactionCallback");

  ai=AO_Context_GetLastAccountInfo(ctx);
  if (!ai) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Transaction but no account. Ignoring");
    return -1;
  }
  else {
    AB_TRANSACTION *t;

    t=AB_Transaction_new();

    if (data.account_ptr->account_number_valid)
      AB_Transaction_SetLocalAccountNumber(t,
                                           data.account_ptr->account_number);
    else
      AB_Transaction_SetLocalAccountNumber(t, "----");

    if (data.date_posted_valid) {
      GWEN_TIME *ti;

      ti=GWEN_Time_fromSeconds(data.date_posted);
      AB_Transaction_SetValutaDate(t, ti);
      GWEN_Time_free(ti);
    }

    if (data.date_initiated_valid) {
      GWEN_TIME *ti;

      ti=GWEN_Time_fromSeconds(data.date_initiated);
      AB_Transaction_SetDate(t, ti);
      GWEN_Time_free(ti);
    }

    if (data.fi_id_valid)
      AB_Transaction_SetFiId(t, data.fi_id);

    if (data.name_valid)
      AB_Transaction_AddRemoteName(t, data.name, 0);
    if (data.memo_valid)
      AB_Transaction_AddPurpose(t, data.memo, 0);

    if (data.amount_valid) {
      AB_VALUE *val;
      const char *cur;

      cur=0;
      if (data.account_ptr)
        if (data.account_ptr->currency_valid)
          cur=data.account_ptr->currency;
      val=AB_Value_new(data.amount, cur);
      AB_Transaction_SetValue(t, val);
      AB_Value_free(val);
    }

    if (data.transactiontype_valid){
      switch(data.transactiontype){
      case OFX_CHECK:
        AB_Transaction_SetTransactionKey(t, "CHK");
        AB_Transaction_SetTransactionText(t, "Check");
        break;
      case OFX_INT:
        AB_Transaction_SetTransactionKey(t, "INT");
        AB_Transaction_SetTransactionText(t, "Interest");
        break;
      case OFX_DIV:
        AB_Transaction_SetTransactionKey(t, "DIV");
        AB_Transaction_SetTransactionText(t, "Dividend");
        break;
      case OFX_SRVCHG:
        AB_Transaction_SetTransactionKey(t, "CHG");
        AB_Transaction_SetTransactionText(t, "Service charge");
        break;
      case OFX_FEE:
        AB_Transaction_SetTransactionKey(t, "BRF");
        AB_Transaction_SetTransactionText(t, "Fee");
        break;
      case OFX_DEP:
        AB_Transaction_SetTransactionKey(t, "LDP"); /* FIXME: not sure */
        AB_Transaction_SetTransactionText(t, "Deposit");
        break;
      case OFX_ATM:
        AB_Transaction_SetTransactionKey(t, "MSC"); /* misc */
        AB_Transaction_SetTransactionText(t, "Cash dispenser");
        break;
      case OFX_POS:
        AB_Transaction_SetTransactionKey(t, "MSC"); /* misc */
        AB_Transaction_SetTransactionText(t, "Point of sale");
        break;
      case OFX_XFER:
        AB_Transaction_SetTransactionKey(t, "TRF");
        AB_Transaction_SetTransactionText(t, "Transfer");
        break;
      case OFX_PAYMENT:
        AB_Transaction_SetTransactionKey(t, "TRF"); /* FIXME: not sure */
        AB_Transaction_SetTransactionText(t, "Electronic payment");
        break;
      case OFX_CASH:
        AB_Transaction_SetTransactionKey(t, "MSC"); /* FIXME: not sure */
        AB_Transaction_SetTransactionText(t, "Cash");
        break;
      case OFX_DIRECTDEP:
        AB_Transaction_SetTransactionKey(t, "LDP"); /* FIXME: not sure */
        AB_Transaction_SetTransactionText(t, "Direct deposit");
        break;
      case OFX_DIRECTDEBIT:
        AB_Transaction_SetTransactionKey(t, "MSC"); /* FIXME: not sure */
        AB_Transaction_SetTransactionText(t, "Merchant initiated debit");
        break;
      case OFX_REPEATPMT:
        AB_Transaction_SetTransactionKey(t, "STO");
        AB_Transaction_SetTransactionText(t, "Standing order");
        break;
      case OFX_DEBIT:
      case OFX_CREDIT:
      case OFX_OTHER:
        AB_Transaction_SetTransactionKey(t, "MSC"); /* FIXME: not sure */
        break;
      }

    } /* if transaction type is valid */
    else {
      DBG_NOTICE(AQOFXCONNECT_LOGDOMAIN, "No transaction type");
    }

    if (data.server_transaction_id_valid)
      AB_Transaction_SetBankReference(t, data.server_transaction_id);

    if (data.check_number_valid)
      AB_Transaction_SetCustomerReference(t, data.check_number);
    else if (data.reference_number_valid)
      AB_Transaction_SetCustomerReference(t, data.reference_number);

    DBG_INFO(0, "Adding transaction");
    AB_ImExporterAccountInfo_AddTransaction(ai, t);
  }

  return 0;
}



int AO_Context_StatementCallback(const struct OfxStatementData data,
                                 void *user_data){
  AO_CONTEXT *ctx;

  ctx=(AO_CONTEXT*)user_data;
  DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
            "StatementCallback");

  if (ctx->lastAccountInfo==0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "Account statement but no last account info, ignoring");
    return 0;
  }

  if (data.ledger_balance_valid ||
      data.available_balance_valid) {
    AB_ACCOUNT_STATUS *ast;
    const char *currency=0;

    ast=AB_AccountStatus_new();
    if (data.currency_valid)
      currency=data.currency;
    if (data.ledger_balance_valid) {
      AB_BALANCE *bal;
      AB_VALUE *v=0;
      GWEN_TIME *ti=0;

      v=AB_Value_new(data.ledger_balance, currency);
      if (data.ledger_balance_date_valid) {
        ti=GWEN_Time_fromSeconds(data.ledger_balance_date);
        if (AB_AccountStatus_GetTime(ast)==0)
          AB_AccountStatus_SetTime(ast, ti);
      }
      bal=AB_Balance_new(v, ti);
      AB_AccountStatus_SetBookedBalance(ast, bal);
      AB_Balance_free(bal);
      GWEN_Time_free(ti);
      AB_Value_free(v);
    }

    if (data.available_balance_valid) {
      AB_VALUE *v=0;

      v=AB_Value_new(data.available_balance, currency);
      if (data.available_balance_date_valid &&
          AB_AccountStatus_GetTime(ast)==0) {
        GWEN_TIME *ti;

        ti=GWEN_Time_fromSeconds(data.available_balance_date);
        AB_AccountStatus_SetTime(ast, ti);
        GWEN_Time_free(ti);
      }
      AB_AccountStatus_SetDisposable(ast, v);
      AB_Value_free(v);
    }
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Adding account status");
    AB_ImExporterAccountInfo_AddAccountStatus(ctx->lastAccountInfo, ast);
  }

  return 0;
}





int AO_Context_ProcessImporterContext(AO_CONTEXT *ctx){
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  AB_PROVIDER *pro;

  assert(ctx);
  assert(ctx->bank);
  assert(ctx->user);
  pro=AO_Bank_GetProvider(ctx->bank);
  assert(pro);

  ai=AB_ImExporterContext_GetFirstAccountInfo(ctx->ictx);
  if (!ai) {
    DBG_INFO(0, "No accounts");
  }
  while(ai) {
    const char *country;
    const char *bankCode;
    const char *accountNumber;

    country=AO_Bank_GetCountry(ctx->bank);
    bankCode=AB_ImExporterAccountInfo_GetBankCode(ai);
    if (!bankCode || !*bankCode)
      bankCode=AO_Bank_GetBankId(ctx->bank);
    accountNumber=AB_ImExporterAccountInfo_GetAccountNumber(ai);
    if (bankCode && accountNumber) {
      AB_ACCOUNT *a;
      const char *s;

      a=AO_Bank_FindAccount(ctx->bank, accountNumber);
      if (!a) {
        char msg[]=I18N_NOOP("Adding account %s to bank %s");
        char msgbuf[512];

        DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Adding account %s to bank %s",
                  accountNumber, bankCode);

        /* account does not exist, add it */
        a=AO_Account_new(AB_Provider_GetBanking(pro),
                         pro,
                         accountNumber);
        assert(a);
        AO_Account_SetUserId(a, AO_User_GetUserId(ctx->user));
        AB_Account_SetOwnerName(a, AO_User_GetUserName(ctx->user));
        AB_Account_SetCountry(a, country);
        AB_Account_SetBankCode(a, bankCode);
        s=AO_Bank_GetBankId(ctx->bank);
        if (!s)
          s=AB_ImExporterAccountInfo_GetBankName(ai);
        AB_Account_SetBankName(a, s);
        AB_Account_SetAccountNumber(a, accountNumber);
        AB_Account_SetAccountType(a, AB_ImExporterAccountInfo_GetType(ai));

        snprintf(msgbuf, sizeof(msgbuf), I18N(msg),
                 accountNumber, bankCode);
        AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                               0,
                               AB_Banking_LogLevelNotice,
                               msgbuf);
        AO_Bank_AddAccount(ctx->bank, a);
      }
      else {
        DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
                  "Account %s at bank %s already exists",
                  accountNumber, bankCode);
      }
      /* update existing account */
      s=AO_Bank_GetBankId(ctx->bank);
      if (!s)
        s=AB_ImExporterAccountInfo_GetBankName(ai);
      if (s) {
        AB_Account_SetBankName(a, s);
        AO_Bank_SetBankName(ctx->bank, s);
      }
      s=AB_ImExporterAccountInfo_GetAccountName(ai);
      if (s)
        AB_Account_SetAccountName(a, s);
    }
    else {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
                "BankCode or AccountNumber missing (%s/%s)",
                bankCode, accountNumber);
    }
    ai=AB_ImExporterContext_GetNextAccountInfo(ctx->ictx);
  } /* while accounts */

  return 0;
}










