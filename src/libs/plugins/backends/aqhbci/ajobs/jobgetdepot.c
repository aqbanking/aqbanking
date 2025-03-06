/***************************************************************************
    begin       : Sun Dec 27 2020
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobgetdepot_l.h"
#include "aqhbci_l.h"
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

#include <gwenhywfar/syncio_memory.h>

#include <assert.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _jobApi_prepare(AH_JOB *j);
static int _jobApi_process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int _jobApi_getLimits(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits);

static GWEN_BUFFER *_sampleDepotInfo(AH_JOB *j, GWEN_DB_NODE *dbResponses);
static int _readDepotInfo(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx, const uint8_t *ptr, uint32_t len);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



AH_JOB *AH_Job_GetDepot_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account)
{
  AH_JOB *j;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Trying to create depot job");
  j=AH_AccountJob_new("JobGetDepot", pro, u, account);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Depot job not found");
    return NULL;
  }

  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandGetDepot);

  /* overwrite some virtual functions */
  AH_Job_SetPrepareFn(j, _jobApi_prepare);
  AH_Job_SetProcessFn(j, _jobApi_process);
  AH_Job_SetGetLimitsFn(j, _jobApi_getLimits);
  AH_Job_SetHandleCommandFn(j, AH_Job_HandleCommand_Accept);
  AH_Job_SetHandleResultsFn(j, AH_Job_HandleResults_Empty);

  return j;
}



int _jobApi_prepare(AH_JOB *j)
{
  GWEN_DB_NODE *dbArgs;
  GWEN_DB_NODE *dbParams;
  const char *s;

  dbArgs=AH_Job_GetArguments(j);
  dbParams=AH_Job_GetParams(j);
  s=GWEN_DB_GetCharValue(dbParams, "CurrencySelectionAllowed", 0, 0);
  if (s && !strcmp(s, "J"))
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "currency", "EUR");
  s=GWEN_DB_GetCharValue(dbParams, "QualitySelectionAllowed", 0, 0);
  if (s && !strcmp(s, "J"))
    GWEN_DB_SetIntValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "quality", 2);

  return 0;
}



int _jobApi_process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  AB_ACCOUNT *a;
  GWEN_DB_NODE *dbResponses;
  GWEN_BUFFER *dataBuf;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobGetTransactionsDepot");

  assert(j);

  a=AH_AccountJob_GetAccount(j);
  assert(a);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  dataBuf=_sampleDepotInfo(j, dbResponses);
  if (dataBuf==NULL) {
    AH_Job_SetStatus(j, AH_JobStatusError);
    return GWEN_ERROR_GENERIC;
  }

  rv=_readDepotInfo(j, ctx, (const uint8_t *) GWEN_Buffer_GetStart(dataBuf), GWEN_Buffer_GetUsedBytes(dataBuf));
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(dataBuf);
    AH_Job_SetStatus(j, AH_JobStatusError);
    return rv;
  }

  return 0;
}



GWEN_BUFFER *_sampleDepotInfo(AH_JOB *j, GWEN_DB_NODE *dbResponses)
{
  GWEN_DB_NODE *dbCurr;
  GWEN_BUFFER *dataBuf;
  int loop;

  /* sample transactions */
  dataBuf=GWEN_Buffer_new(0, 256, 0, 1);
  loop=1;
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while (dbCurr) {
    int rv;
    GWEN_DB_NODE *dbXA;

    rv=AH_Job_CheckEncryption(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (encryption)");
      GWEN_Buffer_free(dataBuf);
      return NULL;
    }
    rv=AH_Job_CheckSignature(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (signature)");
      GWEN_Buffer_free(dataBuf);
      return NULL;
    }

    /* add transaction data from response to data buffer */
    dbXA=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data/depotResponse");
    if (dbXA) {
      const void *p;
      unsigned int bs;

      /* get noted transactions */
      p=GWEN_DB_GetBinValue(dbXA, "depotInfo", 0, 0, 0, &bs);
      if (p && bs)
        GWEN_Buffer_AppendBytes(dataBuf, p, bs);
      else {
        DBG_INFO(AQHBCI_LOGDOMAIN, "No depot info in response %d", loop);
      }
    }

    loop++;
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  if (GWEN_Buffer_GetUsedBytes(dataBuf)==0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No depot info in buffer");
    GWEN_Buffer_free(dataBuf);
    return NULL;
  }

  return dataBuf;
}



int _readDepotInfo(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx, const uint8_t *ptr, uint32_t len)
{
  AB_PROVIDER *pro;
  AB_IMEXPORTER_CONTEXT *tempContext;
  AB_SECURITY_LIST *tmpSecurityList;
  int rv;

  assert(j);
  pro=AH_Job_GetProvider(j);
  assert(pro);

  /* import data into a temporary context */
  tempContext=AB_ImExporterContext_new();

  rv=AB_Banking_ImportFromBufferLoadProfile(AB_Provider_GetBanking(pro),
                                            "swift",
                                            tempContext,
                                            "SWIFT-MT535",
                                            NULL,
                                            ptr,
                                            len);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_ImExporterContext_free(tempContext);
    return rv;
  }

  /* copy data from temporary context to real context */
  tmpSecurityList=AB_ImExporterContext_GetSecurityList(tempContext);
  if (tmpSecurityList) {
    AB_SECURITY *sec;

    while ((sec=AB_Security_List_First(tmpSecurityList))) {
      AB_Security_List_Del(sec);
      AB_ImExporterContext_AddSecurity(ctx, sec);
    }
  }

  AB_ImExporterContext_free(tempContext);

  return 0;
}



int _jobApi_getLimits(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits)
{
  AB_TRANSACTION_LIMITS *tl;

  tl=AB_TransactionLimits_new();
  AB_TransactionLimits_SetCommand(tl, AH_Job_GetSupportedCommand(j));
  /* nothing more to set for this kind of job */
  *pLimits=tl;
  return 0;
}



