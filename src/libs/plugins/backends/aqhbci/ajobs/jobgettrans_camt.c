/***************************************************************************
    begin       : Sat Dec 15 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobgettrans_camt_p.h"
#include "aqhbci_l.h"
#include "accountjob_l.h"
#include "job_l.h"
#include "user_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/gui.h>

#include <gwenhywfar/syncio_memory.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>


GWEN_INHERIT(AH_JOB, AH_JOB_GETTRANS_CAMT);




/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_GetTransactionsCAMT_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;
  AH_JOB_GETTRANS_CAMT *aj;
  GWEN_DB_NODE *dbArgs;
  GWEN_DB_NODE *dbParams;
  int i;

  DBG_ERROR(AQHBCI_LOGDOMAIN, "Trying to create CAMT job");
  j=AH_AccountJob_new("JobGetTransactionsCAMT", pro, u, account);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "CAMT Job not found");
    return NULL;
  }

  GWEN_NEW_OBJECT(AH_JOB_GETTRANS_CAMT, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_GETTRANS_CAMT, j, aj, AH_Job_GetTransactionsCAMT_FreeData);

  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandGetTransactions);

  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, AH_Job_GetTransactionsCAMT_Process);

  AH_Job_SetGetLimitsFn(j, AH_Job_GetTransactionsCAMT_GetLimits);
  AH_Job_SetHandleCommandFn(j, AH_Job_GetTransactionsCAMT_HandleCommand);
  AH_Job_SetHandleResultsFn(j, AH_Job_HandleResults_Empty);

  /* get params DB */
  dbParams=AH_Job_GetParams(j);
  assert(dbParams);

  /* get arguments DB */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  GWEN_DB_Dump(dbParams, 2);

  for (i=0; i<10; i++) {
    const char *s;

    s=GWEN_DB_GetCharValue(dbParams, "supportedFormat", i, NULL);
    if (s==NULL)
      break;
    if (GWEN_Text_ComparePattern(s, "*camt.052.001.02", 0)!=-1) { /* TODO: widen the pattern later */
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Adding CAMT format  %s", s);
      GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "SupportedFormats/format", s);
    }
  } /* for */

  /* set some known arguments */
  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "allAccounts", "N");
  return j;
}



/* --------------------------------------------------------------- FUNCTION */
void GWENHYWFAR_CB AH_Job_GetTransactionsCAMT_FreeData(void *bp, void *p){
  AH_JOB_GETTRANS_CAMT *aj;

  aj=(AH_JOB_GETTRANS_CAMT*)p;

  GWEN_FREE_OBJECT(aj);
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetTransCAMT__ReadTransactions(AH_JOB *j,
                                          AB_IMEXPORTER_ACCOUNTINFO *ai,
                                          const char *docType,
                                          int ty,
                                          const uint8_t *ptr,
                                          uint32_t len) {
  AB_PROVIDER *pro;
  AB_IMEXPORTER_CONTEXT *tempContext;
  AB_IMEXPORTER_ACCOUNTINFO *tempAccountInfo;
  int rv;
  GWEN_BUFFER *buf;

  assert(j);
  pro=AH_Job_GetProvider(j);
  assert(pro);

  /* import data into a temporary context */
  tempContext=AB_ImExporterContext_new();
  buf=GWEN_Buffer_new(0, len, 0, 1);
  GWEN_Buffer_AppendBytes(buf, (const char*) ptr, len);
  GWEN_Buffer_Rewind(buf);

  rv=AB_Banking_ImportBuffer(AB_Provider_GetBanking(pro), tempContext, "camt", docType, buf);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(buf);
    AB_ImExporterContext_free(tempContext);
    return rv;
  }
  GWEN_Buffer_free(buf);


  /* copy data from temporary context to real context */
  tempAccountInfo=AB_ImExporterContext_GetFirstAccountInfo(tempContext);
  while(tempAccountInfo) {
    AB_TRANSACTION_LIST *tl;
    AB_BALANCE_LIST *bl;

    /* move transactions, set transaction type */
    tl=AB_ImExporterAccountInfo_GetTransactionList(tempAccountInfo);
    if (tl) {
      AB_TRANSACTION *t;

      while( (t=AB_Transaction_List_First(tl)) ) {
        AB_Transaction_List_Del(t);
        AB_Transaction_SetType(t, ty);
        AB_ImExporterAccountInfo_AddTransaction(ai, t);
      }
    }

    /* move balances */
    bl=AB_ImExporterAccountInfo_GetBalanceList(tempAccountInfo);
    if (bl) {
      AB_BALANCE *bal;

      while( (bal=AB_Balance_List_First(bl)) ) {
        AB_Balance_List_Del(bal);
        AB_ImExporterAccountInfo_AddBalance(ai, bal);
      }
    }

    tempAccountInfo=AB_ImExporterAccountInfo_List_Next(tempAccountInfo);
  }
  AB_ImExporterContext_free(tempContext);

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetTransactionsCAMT_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_GETTRANS_CAMT *aj;
  AB_ACCOUNT *a;
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobGetTransactions");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETTRANS_CAMT, j);
  assert(aj);

  a=AH_AccountJob_GetAccount(j);
  assert(a);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  ai=AB_ImExporterContext_GetOrAddAccountInfo(ctx,
                                              AB_Account_GetUniqueId(a),
                                              AB_Account_GetIban(a),
                                              AB_Account_GetBankCode(a),
                                              AB_Account_GetAccountNumber(a),
                                              AB_Account_GetAccountType(a));
  assert(ai);


  /* search for "Transactions" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while(dbCurr) {
    GWEN_DB_NODE *dbXA;

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

    dbXA=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data/transactions");
    if (dbXA) {
      const void *p;
      unsigned int bs;
      int i=0;

      if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevel_Debug)
        GWEN_DB_Dump(dbXA, 2);

      /* get booked transactions (daily reports!) */
      for (i=0; ; i++) {
        p=GWEN_DB_GetBinValue(dbXA, "booked", i, 0, 0, &bs);
        if (p && bs) {
          rv=AH_Job_GetTransCAMT__ReadTransactions(j, ai, "052.001.02", AB_Transaction_TypeStatement, p, bs);
          if (rv<0) {
            DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
            AH_Job_SetStatus(j, AH_JobStatusError);
            return rv;
          }
        }
        else
          break;
      }

      /* get noted transactions */
      p=GWEN_DB_GetBinValue(dbXA, "noted", 0, 0, 0, &bs);
      if (p && bs) {
        rv=AH_Job_GetTransCAMT__ReadTransactions(j, ai, "052.001.02", AB_Transaction_TypeNotedStatement, p, bs);
        if (rv<0) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
          AH_Job_SetStatus(j, AH_JobStatusError);
          return rv;
        }
      }
    } /* if "Transactions" */
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Debug) {
    GWEN_DB_NODE *gn;
    AB_TRANSACTION *ttmp;

    DBG_INFO(AQHBCI_LOGDOMAIN, "*** Dumping transactions *******************");
    ttmp=AB_ImExporterAccountInfo_GetFirstTransaction(ai, 0, 0);
    while (ttmp) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "*** --------------------------------------");
      gn=GWEN_DB_Group_new("transaction");
      AB_Transaction_toDb(ttmp, gn);
      GWEN_DB_Dump(gn, 2);
      GWEN_DB_Group_free(gn);
      ttmp=AB_Transaction_List_Next(ttmp);
    }

    DBG_INFO(AQHBCI_LOGDOMAIN, "*** End dumping transactions ***************");
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetTransactionsCAMT_GetLimits(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits) {
  AB_TRANSACTION_LIMITS *tl;
  GWEN_DB_NODE *dbParams;

  dbParams=AH_Job_GetParams(j);

  tl=AB_TransactionLimits_new();
  AB_TransactionLimits_SetCommand(tl, AH_Job_GetSupportedCommand(j));
  AB_TransactionLimits_SetMaxValueSetupTime(tl, GWEN_DB_GetIntValue(dbParams, "storeDays", 0, 0));
  /* nothing more to set for this kind of job */
  *pLimits=tl;
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetTransactionsCAMT_HandleCommand(AH_JOB *j, const AB_TRANSACTION *t) {
  const GWEN_DATE *da;
  
  da=AB_Transaction_GetFirstDate(t);
  if (da) {
    char dbuf[16];
    GWEN_DB_NODE *dbArgs;
  
    dbArgs=AH_Job_GetArguments(j);
    snprintf(dbuf, sizeof(dbuf), "%04d%02d%02d",
             GWEN_Date_GetYear(da),
             GWEN_Date_GetMonth(da),
             GWEN_Date_GetDay(da));
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "fromDate", dbuf);
  }
  
  da=AB_Transaction_GetLastDate(t);
  if (da) {
    char dbuf[16];
    GWEN_DB_NODE *dbArgs;
  
    dbArgs=AH_Job_GetArguments(j);
    snprintf(dbuf, sizeof(dbuf), "%04d%02d%02d",
             GWEN_Date_GetYear(da),
             GWEN_Date_GetMonth(da),
             GWEN_Date_GetDay(da));
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "toDate", dbuf);
  }
  
  return 0;
}






