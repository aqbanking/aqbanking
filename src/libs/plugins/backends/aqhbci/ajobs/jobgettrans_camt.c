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

static int _jobApi_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int _jobApi_GetLimits(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits);
static int _jobApi_HandleCommand(AH_JOB *j, const AB_TRANSACTION *t);

static int _addSupportedCamtFormats(AH_JOB *j, GWEN_DB_NODE *dbArgs);
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
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Trying to create CAMT job");
  j=AH_AccountJob_new("JobGetTransactionsCAMT", pro, u, account);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "CAMT Job not found");
    return NULL;
  }

  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandGetTransactions);

  /* set virtual functions */
  AH_Job_SetProcessFn(j, _jobApi_Process);
  AH_Job_SetGetLimitsFn(j, _jobApi_GetLimits);
  AH_Job_SetHandleCommandFn(j, _jobApi_HandleCommand);
  AH_Job_SetHandleResultsFn(j, AH_Job_HandleResults_Empty);

  /* get arguments DB */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  /*GWEN_DB_Dump(dbParams, 2); */

  rv=_addSupportedCamtFormats(j, dbArgs);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(j);
    return NULL;
  }

  /* set some known arguments */
  AH_AccountJob_WriteNationalAccountInfoToArgs(j);
  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "allAccounts", "N");
  return j;
}



int _jobApi_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  AB_ACCOUNT *a;
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobGetTransactionsCAMT");

  a=AH_AccountJob_GetAccount(j);
  dbResponses=AH_Job_GetResponses(j);

  ai=AB_Provider_GetOrAddAccountInfoForAccount(ctx, a);

  /* search for "Transactions" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while (dbCurr) {
    int rv;
    GWEN_DB_NODE *dbTransactions;

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
    dbTransactions=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data/transactionsCAMT");
    if (dbTransactions) {
      rv=_readTransactionsFromResponse(j, ai, dbTransactions);
      if (rv) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        AH_Job_SetStatus(j, AH_JobStatusError);
        return rv;
      }
    }

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }
  AB_Provider_DumpTransactionsIfDebug(ai, AQHBCI_LOGDOMAIN);
  return 0;
}



int _jobApi_GetLimits(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits)
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



int _jobApi_HandleCommand(AH_JOB *j, const AB_TRANSACTION *t)
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



int _addSupportedCamtFormats(AH_JOB *j, GWEN_DB_NODE *dbArgs)
{
  AB_SWIFT_DESCR_LIST *descrList;
  int foundAtLeastOneDescr=0;

  descrList=AH_Job_GetSwiftDescriptorsSupportedByJob(j, NULL, "supportedFormat", "camt", 52);
  if (descrList) {
    AB_SWIFT_DESCR *descr;

    descr=AB_SwiftDescr_List_First(descrList);
    while (descr) {
      const char *s;

      s=AB_SwiftDescr_GetAlias2(descr);
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Adding supported CAMT format [%s]", s);
      GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "SupportedFormats/format", s);
      foundAtLeastOneDescr=1;
      descr=AB_SwiftDescr_List_Next(descr);
    }
    AB_SwiftDescr_List_free(descrList);
  }

  if (!foundAtLeastOneDescr) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No supported CAMT profile found, job not supported");
    return GWEN_ERROR_GENERIC;
  }

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



int _readTransactions(AH_JOB *j,
                      AB_IMEXPORTER_ACCOUNTINFO *ai,
                      const char *docType,
                      int ty,
                      const uint8_t *ptr,
                      uint32_t len)
{
  AB_PROVIDER *pro;
  AB_IMEXPORTER_CONTEXT *tempContext;
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

  AB_Provider_MergeContextsSetTypeAndFreeSrc(ai, tempContext, ty);

  return 0;
}



