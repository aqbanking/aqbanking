/***************************************************************************
    begin       : Sat Aug 03 2014
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobsepastandingorderget_l.h"
#include "aqhbci/aqhbci_l.h"
#include "accountjob_l.h"
#include "aqhbci/joblayer/job_l.h"
#include "aqhbci/joblayer/job_swift.h"
#include "aqhbci/joblayer/job_crypt.h"
#include "aqhbci/banking/user_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>

#include <assert.h>


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _jobApi_Prepare(AH_JOB *j);
static int _jobApi_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);

static AB_TRANSACTION *_readSto(AH_JOB *j, const char *docType, const uint8_t *ptr, uint32_t len);

static AB_TRANSACTION_PERIOD _getPeriod(const char *s);
static AB_TRANSACTION *_readTransactionFromResponse(AH_JOB *j, GWEN_DB_NODE *dbXA);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



AH_JOB *AH_Job_SepaStandingOrderGet_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account)
{
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;

  j=AH_AccountJob_new("JobSepaStandingOrderGet", pro, u, account);
  if (!j)
    return 0;

  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandSepaGetStandingOrders);

  /* overwrite some virtual functions */
  AH_Job_SetPrepareFn(j, _jobApi_Prepare);
  AH_Job_SetProcessFn(j, _jobApi_Process);
  AH_Job_SetGetLimitsFn(j, AH_Job_GetLimits_EmptyLimits);
  AH_Job_SetHandleCommandFn(j, AH_Job_HandleCommand_Accept);
  AH_Job_SetHandleResultsFn(j, AH_Job_HandleResults_Empty);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);
  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "allAccounts", "N");
  return j;
}



int _jobApi_Prepare(AH_JOB *j)
{
  GWEN_DB_NODE *dbArgs;
  AB_SWIFT_DESCR_LIST *descrList;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing job");

  /* get arguments DB */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  /* check for "pain.001.*" (dont use leading zeros because that would make it an octadecimal! */
  descrList=AH_Job_GetSwiftDescriptorsSupportedByUser(j, "pain", 1);
  if (descrList) {
    AB_SWIFT_DESCR *descr;

    descr=AB_SwiftDescr_List_First(descrList);
    while (descr) {
      const char *s;

      s=AB_SwiftDescr_GetAlias2(descr);
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Adding supported PAIN format [%s]", s);
      GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "SupportedSepaFormats/Format", s);
      descr=AB_SwiftDescr_List_Next(descr);
    }
    AB_SwiftDescr_List_free(descrList);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No supported PAIN profile found, job not supported");
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



int _jobApi_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  const char *responseName;
  int rv;
  AB_ACCOUNT *a;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobSepaStandingOrdersGet");

  assert(j);
  a=AH_AccountJob_GetAccount(j);
  assert(a);

  responseName=AH_Job_GetResponseName(j);
  if (responseName && *responseName) {
    AB_IMEXPORTER_ACCOUNTINFO *ai;
    GWEN_DB_NODE *dbResponses;
    GWEN_DB_NODE *dbCurr;

    ai=AB_Provider_GetOrAddAccountInfoForAccount(ctx, a);
    dbResponses=AH_Job_GetResponses(j);
    assert(dbResponses);

    /* search for "Transactions" */
    dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
    while (dbCurr) {
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

      /* handle job specific response data */
      dbXA=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
      if (dbXA)
        dbXA=GWEN_DB_GetGroup(dbXA, GWEN_PATH_FLAGS_NAMEMUSTEXIST, responseName);
      if (dbXA) {
        AB_TRANSACTION *t;

        t=_readTransactionFromResponse(j, dbXA);
        if (t)
          AB_ImExporterAccountInfo_AddTransaction(ai, t);
      } /* if dbXA */

      dbCurr=GWEN_DB_GetNextGroup(dbCurr);
    } /* while dbCurr */
  }

  return 0;
}



AB_TRANSACTION *_readTransactionFromResponse(AH_JOB *j, GWEN_DB_NODE *dbXA)
{
  const char *fiId;
  const void *p;
  unsigned int bs;

  fiId=GWEN_DB_GetCharValue(dbXA, "fiId", 0, NULL);

  p=GWEN_DB_GetBinValue(dbXA, "transfer", 0, 0, 0, &bs);
  if (p && bs) {
    AB_TRANSACTION *t;

    t=_readSto(j, "sepa", p, bs); /* use generic profile "sepa" */
    if (t) {
      const char *s;

      AB_Transaction_SetFiId(t, fiId);

      s=GWEN_DB_GetCharValue(dbXA, "xfirstExecutionDate", 0, NULL);
      if (s && *s) {
        GWEN_DATE *dt;

        dt=GWEN_Date_fromStringWithTemplate(s, "YYYYMMDD");
        if (dt) {
          AB_Transaction_SetFirstDate(t, dt);
          GWEN_Date_free(dt);
        }
      }

      s=GWEN_DB_GetCharValue(dbXA, "xperiod", 0, NULL);
      AB_Transaction_SetPeriod(t, _getPeriod(s));

      AB_Transaction_SetCycle(t, GWEN_DB_GetIntValue(dbXA, "cycle", 0, 0));
      AB_Transaction_SetExecutionDay(t, GWEN_DB_GetIntValue(dbXA, "executionDay", 0, 0));

      /* done */
      return t;
    } /* if t */
    else {
      DBG_WARN(AQHBCI_LOGDOMAIN, "Error reading standing order from data, ignoring");
    }
  } /* if transaction bindata */

  return NULL;
}



AB_TRANSACTION *_readSto(AH_JOB *j, const char *docType, const uint8_t *ptr, uint32_t len)
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
    return NULL;
  }


  /* return first transaction from temporary context (only contains ONE transaction) */
  tempAccountInfo=AB_ImExporterContext_GetFirstAccountInfo(tempContext);
  if (tempAccountInfo) {
    AB_TRANSACTION *t;

    t=AB_ImExporterAccountInfo_GetFirstTransaction(tempAccountInfo, 0, 0);
    if (t) {
      AB_Transaction_List_Del(t);
      AB_Transaction_SetType(t, AB_Transaction_TypeStandingOrder);
      AB_ImExporterContext_free(tempContext);
      return t;
    }
  }
  AB_ImExporterContext_free(tempContext);

  return 0;
}



AB_TRANSACTION_PERIOD _getPeriod(const char *s)
{
  if (s && *s) {
    if (strcasecmp(s, "M")==0)
      return AB_Transaction_PeriodMonthly;
    else if (strcasecmp(s, "W")==0)
      return AB_Transaction_PeriodWeekly;
  }

  return AB_Transaction_PeriodUnknown;
}

