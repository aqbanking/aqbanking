/***************************************************************************
    begin       : Sat Dec 15 2018
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobgettrans_camt_l.h"
#include "aqhbci/joblayer/job_swift.h"
#include "aqhbci/joblayer/job_crypt.h"

#include <assert.h>


GWEN_INHERIT(AH_JOB, AH_JOB_GETTRANS_CAMT);


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int _getLimits(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits);
static int _handleCommand(AH_JOB *j, const AB_TRANSACTION *t);

static int _readBooked(AH_JOB *j, AB_IMEXPORTER_ACCOUNTINFO *ai, GWEN_DB_NODE *dbBooked);
static int _readTransactionsFromResponse(AH_JOB *j, AB_IMEXPORTER_ACCOUNTINFO *ai, GWEN_DB_NODE *dbXA);
static int _readTransactions(AH_JOB *j,
			     AB_IMEXPORTER_ACCOUNTINFO *ai,
			     const char *docType,
			     int ty,
			     const uint8_t *ptr,
			     uint32_t len);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



AH_JOB *AH_Job_GetTransactionsCAMT_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account)
{
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;
  GWEN_DB_NODE *dbParams;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Trying to create CAMT job");
  j=AH_AccountJob_new("JobGetTransactionsCAMT", pro, u, account);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "CAMT Job not found");
    return NULL;
  }

  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandGetTransactions);

  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, _process);
  AH_Job_SetGetLimitsFn(j, _getLimits);
  AH_Job_SetHandleCommandFn(j, _handleCommand);
  AH_Job_SetHandleResultsFn(j, AH_Job_HandleResults_Empty);

  /* get params DB */
  dbParams=AH_Job_GetParams(j);
  assert(dbParams);

  /* get arguments DB */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  /*GWEN_DB_Dump(dbParams, 2); */

  {
    AB_SWIFT_DESCR_LIST *descrList;

    descrList=AH_Job_GetSwiftDescriptorsSupportedByJob(j, NULL, "supportedFormat", "camt", 52);
    if (descrList) {
      AB_SWIFT_DESCR *descr;

      descr=AB_SwiftDescr_List_First(descrList);
      while (descr) {
        const char *s;

        s=AB_SwiftDescr_GetAlias2(descr);
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Adding supported CAMT format [%s]", s);
        GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "SupportedFormats/format", s);
        descr=AB_SwiftDescr_List_Next(descr);
      }
      AB_SwiftDescr_List_free(descrList);
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No supported CAMT profile found, job not supported");
      AH_Job_free(j);
      return NULL;
    }
  }

  /* set some known arguments */
  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "allAccounts", "N");
  return j;
}



int _readTransactions(AH_JOB *j,
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

  rv=AB_Banking_ImportFromBufferLoadProfile(AB_Provider_GetBanking(pro),
                                            "xml",
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



int _process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  AB_ACCOUNT *a;
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobGetTransactionsCAMT");

  a=AH_AccountJob_GetAccount(j);
  assert(a);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  ai=AB_ImExporterContext_GetOrAddAccountInfo(
					      ctx,
                                              AB_Account_GetUniqueId(a),
                                              AB_Account_GetIban(a),
					      AB_Account_GetBankCode(a),
					      AB_Account_GetAccountNumber(a),
					      AB_Account_GetAccountType(a));
  assert(ai);


  /* search for "Transactions" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while (dbCurr) {
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

    rv=_readTransactionsFromResponse(j, ai, GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                                             "data/transactionsCAMT"));
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  AB_Provider_DumpTransactionsIfDebug(ai, AQHBCI_LOGDOMAIN);

  return 0;
}



int _readTransactionsFromResponse(AH_JOB *j, AB_IMEXPORTER_ACCOUNTINFO *ai, GWEN_DB_NODE *dbXA)
{
  if (dbXA) {
    int rv;
    const void *p;
    unsigned int bs;

    rv=_readBooked(j, ai, GWEN_DB_GetGroup(dbXA, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "booked"));
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    /* get noted transactions */
    p=GWEN_DB_GetBinValue(dbXA, "noted", 0, 0, 0, &bs);
    if (p && bs) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Reading noted data");
      /*rv=_readTransactions(j, ai, "camt_052_001_02", AB_Transaction_TypeNotedStatement, p, bs);*/
      /* let the importer determine the correct format */
      rv=_readTransactions(j, ai, "default", AB_Transaction_TypeNotedStatement, p, bs);
      if (rv<0) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
  }

  return 0;
}



int _readBooked(AH_JOB *j, AB_IMEXPORTER_ACCOUNTINFO *ai, GWEN_DB_NODE *dbBooked)
{
  if (dbBooked) {
    int i=0;

    /* get booked transactions (daily reports!) */
    DBG_INFO(AQHBCI_LOGDOMAIN, "Found booked transaction group");

    for (i=0; i<10000; i++) {
      const void *p;
      unsigned int bs;

      p=GWEN_DB_GetBinValue(dbBooked, "dayData", i, 0, 0, &bs);
      if (p && bs) {
        int rv;

        DBG_INFO(AQHBCI_LOGDOMAIN, "Reading booked day data (%d)", i+1);
        /* rv=_readTransactions(j, ai, "camt_052_001_02", AB_Transaction_TypeStatement, p, bs); */
        /* let the importer determine the correct format */
        rv=_readTransactions(j, ai, "default", AB_Transaction_TypeStatement, p, bs);
        if (rv<0) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
          return rv;
        }
      }
      else
        break;
    } /* for i */
  } /* if dbBooked */

  return 0;
}



int _getLimits(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits)
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



int _handleCommand(AH_JOB *j, const AB_TRANSACTION *t)
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






