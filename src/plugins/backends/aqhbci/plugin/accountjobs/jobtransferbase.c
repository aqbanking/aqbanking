/***************************************************************************
    begin       : Tue Dec 31 2013
    copyright   : (C) 2004-2013 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobtransferbase_p.h"
#include "aqhbci_l.h"
#include "accountjob_l.h"
#include "job_l.h"
#include "provider_l.h"
#include "hhd_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/text.h>

#include <aqbanking/job_be.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_INHERIT(AH_JOB, AH_JOB_TRANSFERBASE);




/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_TransferBase_new(const char *jobName,
                                  AB_TRANSACTION_TYPE tt,
                                  AB_TRANSACTION_SUBTYPE tst,
                                  AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;
  AH_JOB_TRANSFERBASE *aj;

  j=AH_AccountJob_new(jobName, u, account);
  if (!j)
    return 0;

  GWEN_NEW_OBJECT(AH_JOB_TRANSFERBASE, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_TRANSFERBASE, j, aj,
                       AH_Job_TransferBase_FreeData);

  aj->transactionType=tt;
  aj->transactionSubType=tst;

  /* overwrite some virtual functions */
  AH_Job_SetExchangeFn(j, AH_Job_TransferBase_Exchange);
  AH_Job_SetProcessFn(j, AH_Job_TransferBase_Process);

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
void GWENHYWFAR_CB AH_Job_TransferBase_FreeData(void *bp, void *p){
  AH_JOB_TRANSFERBASE *aj;

  aj=(AH_JOB_TRANSFERBASE*)p;
  free(aj->fiid);

  GWEN_FREE_OBJECT(aj);
}



/* --------------------------------------------------------------- FUNCTION */
const char *AH_Job_TransferBase_GetFiid(const AH_JOB *j) {
  AH_JOB_TRANSFERBASE *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TRANSFERBASE, j);
  assert(aj);

  return aj->fiid;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_TransferBase_SepaExportTransactions(AH_JOB *j, GWEN_DB_NODE *profile) {
  AH_JOB_TRANSFERBASE *aj;
  GWEN_DB_NODE *dbArgs;
  AB_BANKING *ab;
  const char *descriptor;
  const AB_TRANSACTION *t;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exporting transaction");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TRANSFERBASE, j);
  assert(aj);

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  descriptor=GWEN_DB_GetCharValue(profile, "descriptor", 0, 0);
  assert(descriptor);
  DBG_INFO(AQHBCI_LOGDOMAIN, "Using SEPA descriptor %s and profile %s",
	   descriptor, GWEN_DB_GetCharValue(profile, "name", 0, 0));

  /* set data in job */
  t=AH_Job_GetFirstTransfer(j);
  if (t) {
    AB_IMEXPORTER_CONTEXT *ioc;
    AB_TRANSACTION *cpy;
    AB_IMEXPORTER *ie;

    /* add transfers as transactions for export (exporters only use transactions) */
    ioc=AB_ImExporterContext_new();
    while(t) {
      cpy=AB_Transaction_dup(t);
      AB_ImExporterContext_AddTransaction(ioc, cpy);
      t=AB_Transaction_List_Next(t);
    }

    ie=AB_Banking_GetImExporter(ab, "sepa");
    if (ie) {
      GWEN_BUFFER *dbuf;

      dbuf=GWEN_Buffer_new(0, 256, 0, 1);
      rv=AB_ImExporter_ExportToBuffer(ie, ioc, dbuf, profile);
      AB_ImExporterContext_free(ioc);
      if (rv<0) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        GWEN_Buffer_free(dbuf);
        return rv;
      }

      /* store descriptor */
      GWEN_DB_SetCharValue(dbArgs,
                           GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "descriptor",
                       descriptor);
      /* store transfer */
      GWEN_DB_SetBinValue(dbArgs,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "transfer",
                          GWEN_Buffer_GetStart(dbuf),
                          GWEN_Buffer_GetUsedBytes(dbuf));
      GWEN_Buffer_free(dbuf);
    }
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "here");
      AB_ImExporterContext_free(ioc);
      return GWEN_ERROR_NO_DATA;
    }
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction in job");
    return GWEN_ERROR_INTERNAL;
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_TransferBase_ExchangeParams_SepaUndated(AH_JOB *j, AB_JOB *bj,
                                                   AB_IMEXPORTER_CONTEXT *ctx) {
  AB_TRANSACTION_LIMITS *lim;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging params");

  /* set some default limits */
  lim=AB_TransactionLimits_new();
  AB_TransactionLimits_SetMaxLenPurpose(lim, 35);
  AB_TransactionLimits_SetMaxLinesPurpose(lim, 4);
  AB_TransactionLimits_SetMaxLenRemoteName(lim, 70);
  AB_TransactionLimits_SetMaxLinesRemoteName(lim, 1);

  AB_TransactionLimits_SetNeedDate(lim, -1);

  AB_Job_SetFieldLimits(bj, lim);
  AB_TransactionLimits_free(lim);

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_TransferBase_ExchangeArgs_SepaUndated(AH_JOB *j, AB_JOB *bj, AB_IMEXPORTER_CONTEXT *ctx) {
  const AB_TRANSACTION_LIMITS *lim=NULL;
  AB_BANKING *ab;
  const AB_TRANSACTION *t=NULL;
  AB_TRANSACTION *tCopy=NULL;
  int rv;
  AB_USER *u;
  uint32_t uflags;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging args");

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  u=AH_Job_GetUser(j);
  assert(u);

  uflags=AH_User_GetFlags(u);

  /* get limits and transaction */
  lim=AB_Job_GetFieldLimits(bj);
  t=AB_Job_GetTransaction(bj);
  if (t==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction in job");
    return GWEN_ERROR_INVALID;
  }

  /* DISABLED according to a discussion on aqbanking-user:
   * The application should do this, not the library.
  AB_Transaction_FillLocalFromAccount(t, a); */

  /* validate transaction */
  rv=AB_Transaction_CheckForSepaConformity(t, (uflags & AH_USER_FLAGS_USE_STRICT_SEPA_CHARSET)?1:0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Transaction_CheckPurposeAgainstLimits(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Transaction_CheckNamesAgainstLimits(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  tCopy=AB_Transaction_dup(t);

  /* set group id so the application can now which transfers went together in one setting */
  AB_Transaction_SetGroupId(tCopy, AH_Job_GetId(j));

  /* store validated transaction in job */
  AB_Job_SetTransaction(bj, tCopy);

  /* store copy of transaction for later */
  AH_Job_AddTransfer(j, tCopy);

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_TransferBase_ExchangeArgs_SepaDated(AH_JOB *j, AB_JOB *bj, AB_IMEXPORTER_CONTEXT *ctx) {
  const AB_TRANSACTION_LIMITS *lim=NULL;
  AB_BANKING *ab;
  const AB_TRANSACTION *t=NULL;
  AB_TRANSACTION *tCopy=NULL;
  int rv;
  AB_USER *u;
  uint32_t uflags;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging args");

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  u=AH_Job_GetUser(j);
  assert(u);

  uflags=AH_User_GetFlags(u);

  /* get limits and transaction */
  lim=AB_Job_GetFieldLimits(bj);
  t=AB_Job_GetTransaction(bj);
  if (t==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction in job");
    return GWEN_ERROR_INVALID;
  }

  /* DISABLED according to a discussion on aqbanking-user:
   * The application should do this, not the library.
  AB_Transaction_FillLocalFromAccount(t, a); */

  /* validate transaction */
  rv=AB_Transaction_CheckForSepaConformity(t, (uflags & AH_USER_FLAGS_USE_STRICT_SEPA_CHARSET)?1:0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Transaction_CheckPurposeAgainstLimits(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Transaction_CheckNamesAgainstLimits(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Transaction_CheckDateAgainstLimits(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }


  tCopy=AB_Transaction_dup(t);

  /* set group id so the application can now which transfers went together in one setting */
  AB_Transaction_SetGroupId(tCopy, AH_Job_GetId(j));

  /* store validated transaction in job */
  AB_Job_SetTransaction(bj, tCopy);

  /* store copy of transaction for later */
  AH_Job_AddTransfer(j, tCopy);

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_TransferBase_ExchangeArgs_SepaDatedDebit(AH_JOB *j, AB_JOB *bj, AB_IMEXPORTER_CONTEXT *ctx) {
  const AB_TRANSACTION_LIMITS *lim=NULL;
  AB_BANKING *ab;
  const AB_TRANSACTION *t=NULL;
  AB_TRANSACTION *tCopy=NULL;
  int rv;
  AB_USER *u;
  uint32_t uflags;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging args");

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  u=AH_Job_GetUser(j);
  assert(u);

  uflags=AH_User_GetFlags(u);

  /* get limits and transaction */
  lim=AB_Job_GetFieldLimits(bj);
  t=AB_Job_GetTransaction(bj);
  if (t==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction in job");
    return GWEN_ERROR_INVALID;
  }

  /* DISABLED according to a discussion on aqbanking-user:
   * The application should do this, not the library.
  AB_Transaction_FillLocalFromAccount(t, a); */

  /* validate transaction */
  rv=AB_Transaction_CheckForSepaConformity(t, (uflags & AH_USER_FLAGS_USE_STRICT_SEPA_CHARSET)?1:0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Transaction_CheckPurposeAgainstLimits(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Transaction_CheckNamesAgainstLimits(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Transaction_CheckDateAgainstSequenceLimits(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }


  tCopy=AB_Transaction_dup(t);

  /* set group id so the application can now which transfers went together in one setting */
  AB_Transaction_SetGroupId(tCopy, AH_Job_GetId(j));

  /* store validated transaction in job */
  AB_Job_SetTransaction(bj, tCopy);

  /* store copy of transaction for later */
  AH_Job_AddTransfer(j, tCopy);

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_TransferBase_ExchangeResults(AH_JOB *j, AB_JOB *bj, AB_IMEXPORTER_CONTEXT *ctx) {
  AH_JOB_TRANSFERBASE *aj;
  AH_RESULT_LIST *rl;
  AH_RESULT *r;
  int has10;
  int has20;
  AB_TRANSACTION_STATUS tStatus;
  const AB_TRANSACTION *t;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TRANSFERBASE, j);
  assert(aj);

  rl=AH_Job_GetSegResults(j);
  assert(rl);

  r=AH_Result_List_First(rl);
  if (!r) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No segment results");
    AB_Job_SetStatus(bj, AB_Job_StatusError);
    return GWEN_ERROR_NO_DATA;
  }
  has10=0;
  has20=0;
  while(r) {
    int rcode;

    rcode=AH_Result_GetCode(r);
    if (rcode>=10 && rcode<=19)
      has10=1;
    else if (rcode>=20 && rcode <=29)
      has20=1;
    r=AH_Result_List_Next(r);
  }

  if (has20) {
    AB_Job_SetStatus(bj, AB_Job_StatusFinished);
    DBG_INFO(AQHBCI_LOGDOMAIN, "Job finished");
  }
  else if (has10) {
    AB_Job_SetStatus(bj, AB_Job_StatusPending);
    DBG_INFO(AQHBCI_LOGDOMAIN, "Job pending");
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Error status (neither 0010 nor 0020)");
    AB_Job_SetStatus(bj, AB_Job_StatusError);
  }

  if (has20)
    tStatus=AB_Transaction_StatusAccepted;
  else if (has10)
    tStatus=AB_Transaction_StatusPending;
  else
    tStatus=AB_Transaction_StatusRejected;

  t=AB_Job_GetTransaction(bj);
  if (t) {
    AB_TRANSACTION *cpy;

    cpy=AB_Transaction_dup(t);
    AB_Transaction_SetFiId(cpy, aj->fiid);
    AB_Transaction_SetStatus(cpy, tStatus);
    AB_Transaction_SetType(cpy, aj->transactionType);
    AB_Transaction_SetSubType(cpy, aj->transactionSubType);

    switch(aj->transactionType) {
    case AB_Transaction_TypeUnknown:
    case AB_Transaction_TypeTransaction:
      AB_ImExporterContext_AddTransaction(ctx, cpy);      /* takes over cpy */
      break;

    case AB_Transaction_TypeTransfer:
    case AB_Transaction_TypeDebitNote:
    case AB_Transaction_TypeEuTransfer:
    case AB_Transaction_TypeSepaTransfer:
    case AB_Transaction_TypeSepaDebitNote:
    case AB_Transaction_TypeInternalTransfer:
      if (aj->transactionSubType==AB_Transaction_SubTypeStandingOrder)
        AB_ImExporterContext_AddStandingOrder(ctx, cpy);  /* takes over cpy */
      else
        AB_ImExporterContext_AddTransfer(ctx, cpy);       /* takes over cpy */
      break;
    }
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_TransferBase_Exchange(AH_JOB *j, AB_JOB *bj,
                                 AH_JOB_EXCHANGE_MODE m,
                                 AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_TRANSFERBASE *aj;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging (%d)", m);

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TRANSFERBASE, j);
  assert(aj);

  switch(m) {
  case AH_Job_ExchangeModeParams:
    if (aj->exchangeParamsFn)
      return aj->exchangeParamsFn(j, bj, ctx);
    break;

  case AH_Job_ExchangeModeArgs:
    if (aj->exchangeArgsFn)
      return aj->exchangeArgsFn(j, bj, ctx);
    break;

  case AH_Job_ExchangeModeResults:
    if (aj->exchangeResultsFn)
      return aj->exchangeResultsFn(j, bj, ctx);
    else
      return AH_Job_TransferBase_ExchangeResults(j, bj, ctx);
    break;

  default:
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Unsupported exchange mode");
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */

  /* just ignore */
  DBG_ERROR(AQHBCI_LOGDOMAIN, "Exchange mode %d not implemented", m);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_TransferBase_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx) {
  AH_JOB_TRANSFERBASE *aj;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  const char *responseName;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TRANSFERBASE, j);
  assert(aj);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing");
  responseName=AH_Job_GetResponseName(j);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  /* search for "TransferBaseSingleResponse" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while(dbCurr) {
    int rv;

    rv=AH_Job_CheckEncryption(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (encryption)");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }
    rv=AH_Job_CheckSignature(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (signature)");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }

    if (responseName && *responseName) {
      GWEN_DB_NODE *dbXA;

      dbXA=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
      if (dbXA)
        dbXA=GWEN_DB_GetGroup(dbXA, GWEN_PATH_FLAGS_NAMEMUSTEXIST, responseName);
      if (dbXA) {
        const char *s;

        s=GWEN_DB_GetCharValue(dbXA, "referenceId", 0, 0);
        if (s) {
          free(aj->fiid);
          aj->fiid=strdup(s);
        }
      }
    }
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }


  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Job_TransferBase_SetExchangeParamsFn(AH_JOB *j, AH_JOB_TRANSFERBASE_EXCHANGE_FN f){
  AH_JOB_TRANSFERBASE *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TRANSFERBASE, j);
  assert(aj);

  aj->exchangeParamsFn=f;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Job_TransferBase_SetExchangeArgsFn(AH_JOB *j, AH_JOB_TRANSFERBASE_EXCHANGE_FN f){
  AH_JOB_TRANSFERBASE *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TRANSFERBASE, j);
  assert(aj);

  aj->exchangeArgsFn=f;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Job_TransferBase_SetExchangeResultsFn(AH_JOB *j, AH_JOB_TRANSFERBASE_EXCHANGE_FN f){
  AH_JOB_TRANSFERBASE *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TRANSFERBASE, j);
  assert(aj);

  aj->exchangeResultsFn=f;
}
