/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobgettransactions_p.h"
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

#include <assert.h>
#include <errno.h>


GWEN_INHERIT(AH_JOB, AH_JOB_GETTRANSACTIONS);




/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_GetTransactions_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account)
{
  AH_JOB *j;
  AH_JOB_GETTRANSACTIONS *aj;
  GWEN_DB_NODE *dbArgs;
  GWEN_DB_NODE *updgroup;

  int useCreditCardJob = 0;

  //Check if we should use DKKKU
  updgroup=AH_User_GetUpdForAccount(u, account);
  if (updgroup) {
    GWEN_DB_NODE *n;
    n=GWEN_DB_GetFirstGroup(updgroup);
    while (n) {
      if (strcasecmp(GWEN_DB_GetCharValue(n, "job", 0, ""), "DKKKU")==0) {
        useCreditCardJob = 1;
        break;
      }
      n=GWEN_DB_GetNextGroup(n);
    } /* while */
  } /* if updgroup for the given account found */

  if (useCreditCardJob)
    j=AH_AccountJob_new("JobGetTransactionsCreditCard", pro, u, account);
  else
    j=AH_AccountJob_new("JobGetTransactions", pro, u, account);
  if (!j)
    return 0;

  GWEN_NEW_OBJECT(AH_JOB_GETTRANSACTIONS, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_GETTRANSACTIONS, j, aj, AH_Job_GetTransactions_FreeData);

  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandGetTransactions);

  /* overwrite some virtual functions */
  if (useCreditCardJob)
    AH_Job_SetProcessFn(j, AH_Job_GetTransactionsCreditCard_Process);
  else
    AH_Job_SetProcessFn(j, AH_Job_GetTransactions_Process);

  AH_Job_SetGetLimitsFn(j, AH_Job_GetTransactions_GetLimits);
  AH_Job_SetHandleCommandFn(j, AH_Job_GetTransactions_HandleCommand);
  AH_Job_SetHandleResultsFn(j, AH_Job_HandleResults_Empty);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);
  if (useCreditCardJob)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT,
                         "accountNumber", AB_Account_GetAccountNumber(account));
  else
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT,
                         "allAccounts", "N");
  return j;
}



/* --------------------------------------------------------------- FUNCTION */
void GWENHYWFAR_CB AH_Job_GetTransactions_FreeData(void *bp, void *p)
{
  AH_JOB_GETTRANSACTIONS *aj;

  aj=(AH_JOB_GETTRANSACTIONS *)p;

  GWEN_FREE_OBJECT(aj);
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetTransactions__ReadTransactions(AH_JOB *j,
                                             AB_IMEXPORTER_ACCOUNTINFO *ai,
                                             const char *docType,
                                             int ty,
                                             const uint8_t *ptr,
                                             uint32_t len)
{
  AB_PROVIDER *pro;
  AB_IMEXPORTER_CONTEXT *tempContext;
  AB_IMEXPORTER_ACCOUNTINFO *tempAccountInfo;
  int rv;

  assert(j);
  pro=AH_Job_GetProvider(j);
  assert(pro);

  /* import data into a temporary context */
  tempContext=AB_ImExporterContext_new();

#if 0
  DBG_ERROR(0, "About to read this SWIFT data (%s)", docType);
  GWEN_Text_DumpString((const char *) ptr, len, 2);
#endif

  rv=AB_Banking_ImportFromBufferLoadProfile(AB_Provider_GetBanking(pro),
                                            "swift",
                                            tempContext,
                                            docType,
                                            NULL,
                                            ptr,
                                            len);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_ImExporterContext_free(tempContext);
    return rv;
  }

  /* copy data from temporary context to real context */
  tempAccountInfo=AB_ImExporterContext_GetFirstAccountInfo(tempContext);
  while (tempAccountInfo) {
    AB_TRANSACTION_LIST *tl;
    AB_BALANCE_LIST *bl;

    /* move transactions, set transaction type */
    tl=AB_ImExporterAccountInfo_GetTransactionList(tempAccountInfo);
    if (tl) {
      AB_TRANSACTION *t;

      while ((t=AB_Transaction_List_First(tl))) {
        AB_Transaction_List_Del(t);
        AB_Transaction_SetType(t, ty);
        AB_ImExporterAccountInfo_AddTransaction(ai, t);
      }
    }

    /* move balances */
    bl=AB_ImExporterAccountInfo_GetBalanceList(tempAccountInfo);
    if (bl) {
      AB_BALANCE *bal;

      while ((bal=AB_Balance_List_First(bl))) {
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
int AH_Job_GetTransactions_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  AH_JOB_GETTRANSACTIONS *aj;
  AB_ACCOUNT *a;
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  GWEN_BUFFER *tbooked;
  GWEN_BUFFER *tnoted;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobGetTransactions");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETTRANSACTIONS, j);
  assert(aj);

  tbooked=GWEN_Buffer_new(0, 1024, 0, 1);
  tnoted=GWEN_Buffer_new(0, 1024, 0, 1);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  /* search for "Transactions" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while (dbCurr) {
    GWEN_DB_NODE *dbXA;

    rv=AH_Job_CheckEncryption(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (encryption)");
      GWEN_Buffer_free(tbooked);
      GWEN_Buffer_free(tnoted);
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }
    rv=AH_Job_CheckSignature(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (signature)");
      GWEN_Buffer_free(tbooked);
      GWEN_Buffer_free(tnoted);
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }

    dbXA=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                          "data/transactions");
    if (dbXA) {
      const void *p;
      unsigned int bs;

      if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevel_Debug)
        GWEN_DB_Dump(dbXA, 2);
      p=GWEN_DB_GetBinValue(dbXA, "booked", 0, 0, 0, &bs);
      if (p && bs)
        GWEN_Buffer_AppendBytes(tbooked, p, bs);
      p=GWEN_DB_GetBinValue(dbXA, "noted", 0, 0, 0, &bs);
      if (p && bs)
        GWEN_Buffer_AppendBytes(tnoted, p, bs);
    } /* if "Transactions" */
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  GWEN_Buffer_Rewind(tbooked);
  GWEN_Buffer_Rewind(tnoted);

  /* now the buffers contain data to be parsed by DBIOs */
  a=AH_AccountJob_GetAccount(j);
  assert(a);
  ai=AB_ImExporterContext_GetOrAddAccountInfo(ctx,
                                              AB_Account_GetUniqueId(a),
                                              AB_Account_GetIban(a),
                                              AB_Account_GetBankCode(a),
                                              AB_Account_GetAccountNumber(a),
                                              AB_Account_GetAccountType(a));
  assert(ai);

  /* read booked transactions */
  if (GWEN_Buffer_GetUsedBytes(tbooked)) {
    if (getenv("AQHBCI_LOGBOOKED")) {
      FILE *f;

      f=fopen("/tmp/booked.mt", "w+");
      if (f) {
        if (fwrite(GWEN_Buffer_GetStart(tbooked),
                   GWEN_Buffer_GetUsedBytes(tbooked), 1, f)!=1) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "fwrite: %s", strerror(errno));
        }
        if (fclose(f)) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "fclose: %s", strerror(errno));
        }
      }
    }

    if (AH_Job_GetTransactions__ReadTransactions(j,
                                                 ai,
                                                 "SWIFT-MT940",
                                                 AB_Transaction_TypeStatement,
                                                 (const uint8_t *) GWEN_Buffer_GetStart(tbooked),
                                                 GWEN_Buffer_GetUsedBytes(tbooked))) {
      GWEN_Buffer_free(tbooked);
      GWEN_Buffer_free(tnoted);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error parsing booked transactions");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return -1;
    }
  }

  /* read noted transactions */
  if (GWEN_Buffer_GetUsedBytes(tnoted)) {
    if (getenv("AQHBCI_LOGNOTED")) {
      FILE *f;

      f=fopen("/tmp/noted.mt", "w+");
      if (f) {
        if (fwrite(GWEN_Buffer_GetStart(tnoted),
                   GWEN_Buffer_GetUsedBytes(tnoted), 1, f)!=1) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "fwrite: %s", strerror(errno));
        }
        if (fclose(f)) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "fclose: %s", strerror(errno));
        }
      }
    }

    if (AH_Job_GetTransactions__ReadTransactions(j,
                                                 ai,
                                                 "SWIFT-MT942",
                                                 AB_Transaction_TypeNotedStatement,
                                                 (const uint8_t *) GWEN_Buffer_GetStart(tnoted),
                                                 GWEN_Buffer_GetUsedBytes(tnoted))) {
      GWEN_Buffer_free(tbooked);
      GWEN_Buffer_free(tnoted);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error parsing noted transactions");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return -1;
    }
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

  GWEN_Buffer_free(tbooked);
  GWEN_Buffer_free(tnoted);
  return 0;
}



int AH_Job_GetTransactionsCreditCard_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  AH_JOB_GETTRANSACTIONS *aj;
  AB_ACCOUNT *a;
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  AB_USER *u;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  GWEN_BUFFER *tbooked;
  GWEN_BUFFER *tnoted;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobGetTransactionsCreditCard");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETTRANSACTIONS, j);
  assert(aj);

  tbooked=GWEN_Buffer_new(0, 1024, 0, 1);
  tnoted=GWEN_Buffer_new(0, 1024, 0, 1);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);
  DBG_INFO(AQHBCI_LOGDOMAIN, "Response:");
  GWEN_DB_Dump(dbResponses, 2);
  DBG_INFO(AQHBCI_LOGDOMAIN, "Response end");


  a=AH_AccountJob_GetAccount(j);
  assert(a);
  ai=AB_ImExporterContext_GetOrAddAccountInfo(ctx,
                                              AB_Account_GetUniqueId(a),
                                              AB_Account_GetIban(a),
                                              AB_Account_GetBankCode(a),
                                              AB_Account_GetAccountNumber(a),
                                              AB_Account_GetAccountType(a));
  assert(ai);
  AB_ImExporterAccountInfo_SetAccountId(ai, AB_Account_GetUniqueId(a));

  u=AH_Job_GetUser(j);
  assert(u);

  /* search for "Transactions" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while (dbCurr) {
    GWEN_DB_NODE *dbXA;

    rv=AH_Job_CheckEncryption(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (encryption)");
      GWEN_Buffer_free(tbooked);
      GWEN_Buffer_free(tnoted);
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }
    rv=AH_Job_CheckSignature(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (signature)");
      GWEN_Buffer_free(tbooked);
      GWEN_Buffer_free(tnoted);
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }

    dbXA=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                          "data/transactionscreditcard");
    if (dbXA) {
      GWEN_DB_NODE *dbT;
      GWEN_DB_NODE *dbV;
      GWEN_DATE *date;
      GWEN_DATE *valutaDate;
      const char *p;
      const char *ref;
      int i;

      dbT=GWEN_DB_GetGroup(dbXA, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                           "entries");
      while (dbT) {
        AB_VALUE *v1;
        AB_VALUE *v2;
        GWEN_STRINGLIST *purpose;
        AB_TRANSACTION *t;

        /* read date (Buchungsdatum) */
        p=GWEN_DB_GetCharValue(dbT, "date", 0, 0);
        if (p)
          date=GWEN_Date_fromStringWithTemplate(p, "YYYYMMDD");
        else
          date=NULL;

        /* read valutaData (Umsatzdatum) */
        p=GWEN_DB_GetCharValue(dbT, "valutaDate", 0, 0);
        if (p)
          valutaDate=GWEN_Date_fromStringWithTemplate(p, "YYYYMMDD");
        else
          valutaDate=NULL;

        /* read value */
        dbV=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                             "value");
        if (dbV)
          v1=AB_Value_fromDb(dbV);
        else
          v1=NULL;
        v2=0;
        if (!v1) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Error parsing value from DB");
        }
        else {
          p=GWEN_DB_GetCharValue(dbT, "debitMark", 0, 0);
          if (p) {
            if (strcasecmp(p, "D")==0 ||
                strcasecmp(p, "RC")==0) {
              v2=AB_Value_dup(v1);
              AB_Value_Negate(v2);
            }
            else if (strcasecmp(p, "C")==0 ||
                     strcasecmp(p, "RD")==0)
              v2=AB_Value_dup(v1);
            else {
              DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad debit mark \"%s\"", p);
              v2=0;
            }
          }
          AB_Value_free(v1);
        }

        /* read purpose */
        purpose=GWEN_StringList_new();
        for (i=0; i<10; i++) {
          p=GWEN_DB_GetCharValue(dbT, "purpose", i, 0);
          if (!p)
            break;
          GWEN_StringList_AppendString(purpose, p, 0, 0);
        }

        /* read reference */
        ref=GWEN_DB_GetCharValue(dbT, "reference", 0, 0);
        if (ref)
          GWEN_StringList_AppendString(purpose, ref, 0, 0);

        t=AB_Transaction_new();
        if (ref)
          AB_Transaction_SetFiId(t, ref);
        AB_Transaction_SetUniqueAccountId(t, AB_Account_GetUniqueId(a));
        AB_Transaction_SetLocalBankCode(t, AB_User_GetBankCode(u));
        AB_Transaction_SetLocalAccountNumber(t, AB_Account_GetAccountNumber(a));
        AB_Transaction_SetValutaDate(t, valutaDate);
        AB_Transaction_SetDate(t, date);
        AB_Transaction_SetValue(t, v2);
        AB_Transaction_SetPurposeFromStringList(t, purpose);
        DBG_INFO(AQHBCI_LOGDOMAIN, "Adding transaction");
        AB_ImExporterAccountInfo_AddTransaction(ai, t);

        GWEN_StringList_free(purpose);
        AB_Value_free(v2);
        GWEN_Date_free(date);
        GWEN_Date_free(valutaDate);

        dbT = GWEN_DB_FindNextGroup(dbT, "entries");
      } //while (dbT)
    } //if (dbXA)
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetTransactions_GetLimits(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits)
{
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
int AH_Job_GetTransactions_HandleCommand(AH_JOB *j, const AB_TRANSACTION *t)
{
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






