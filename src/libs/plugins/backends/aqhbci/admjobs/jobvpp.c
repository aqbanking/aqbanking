/***************************************************************************
    begin       : Fri Oct 3 2025
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobvpp_p.h"

#include "aqhbci/joblayer/job_crypt.h"



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void GWENHYWFAR_CB _freeData(void *bp, void *p);
static int _cbProcess(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int _cbPrepare(AH_JOB *j);
static void _readVopResultGroup(AH_JOB *j, AH_JOB_VPP *aj, GWEN_DB_NODE *dbVppResponse);
static void _readVopId(AH_JOB_VPP *aj, GWEN_DB_NODE *dbArgs, GWEN_DB_NODE *dbVppResponse);
static void _readPollingId(AH_JOB_VPP *aj, GWEN_DB_NODE *dbArgs, GWEN_DB_NODE *dbVppResponse);
static void _readPmtStatusReport(AH_JOB *j, AH_JOB_VPP *aj, AB_IMEXPORTER_CONTEXT *ctx, GWEN_DB_NODE *dbVppResponse);
static void _addResultsFromTransactionList(AH_JOB *j, AH_JOB_VPP *aj, AB_TRANSACTION_LIST *transactionList);
static AB_TRANSACTION_LIST *_pmtStatusReportToTransactionList(AH_JOB *j,  const uint8_t *dataPtr, uint32_t dataLen);
static AB_IMEXPORTER_CONTEXT *_dataToContext(AH_JOB *j,  const uint8_t *dataPtr, uint32_t dataLen);



/* ------------------------------------------------------------------------------------------------
 * code
 * ------------------------------------------------------------------------------------------------
 */

GWEN_INHERIT(AH_JOB, AH_JOB_VPP);



AH_JOB *AH_Job_VPP_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account, int jobVersion)
{
  AH_JOB *j;
  AH_JOB_VPP *aj;
  GWEN_DB_NODE *dbArgs;
  GWEN_DB_NODE *dbParams;
  const char *s;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Looking for VPP job in version %d", jobVersion);
  j=AH_Job_new("JobVpp", pro, u, 0, jobVersion);
  if (!j) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "VPP job in version %d not found", jobVersion);
    return NULL;
  }

  GWEN_NEW_OBJECT(AH_JOB_VPP, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_VPP, j, aj, _freeData);

  aj->account=account;
  aj->resultList=AH_VopResult_List_new();

  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, _cbProcess);
  AH_Job_SetPrepareFn(j, _cbPrepare);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);
  dbParams=AH_Job_GetParams(j);
  assert(dbParams);

  s=GWEN_DB_GetCharValue(dbParams, "paymentStatusFormat", 0, NULL);
  if (s && *s) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Setting supported payment status report: %s", s);
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "SupportedFormats/format", s);
  }

  return j;
}



void GWENHYWFAR_CB _freeData(void *bp, void *p)
{
  AH_JOB_VPP *aj;

  aj=(AH_JOB_VPP *)p;
  AH_VopResult_List_free(aj->resultList);
  free(aj->ptrVopId);
  free(aj->ptrPollingId);
  free(aj->paymentStatusFormat);
  free(aj->vopMsg);
  GWEN_FREE_OBJECT(aj);
}



int _cbPrepare(AH_JOB *j)
{
  GWEN_DB_NODE *dbResponses;

  DBG_ERROR(AQHBCI_LOGDOMAIN, "Prepare function called");
  dbResponses=AH_Job_GetResponses(j);
  if (dbResponses) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Clearing responses");
    GWEN_DB_ClearGroup(dbResponses, NULL);
  }

  return 0;
}



int _cbProcess(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  AH_JOB_VPP *aj;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbArgs;
  GWEN_DB_NODE *dbCurr;
  int rv;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Processing JobVpp");
  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_VPP, j);
  assert(aj);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  /* search for "VppResponse" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while (dbCurr) {
    GWEN_DB_NODE *dbVppResponse;

    rv=AH_Job_CheckEncryption(j, dbCurr);
    if (rv) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Compromised security (encryption)");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }
    rv=AH_Job_CheckSignature(j, dbCurr);
    if (rv) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Compromised security (signature)");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }

    dbVppResponse=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data/vppResponse");
    if (dbVppResponse) {
      const char *s;

      DBG_ERROR(AQHBCI_LOGDOMAIN, "Got a VPP response");
      //      if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevel_Debug)
      fprintf(stderr, "Response: \n"); // DEBUG
      GWEN_DB_Dump(dbVppResponse, 2);

      _readVopId(aj, dbArgs, dbVppResponse);
      _readPollingId(aj, dbArgs, dbVppResponse);

      s=GWEN_DB_GetCharValue(dbVppResponse, "paymentStatusFormat", 0, NULL);
      free(aj->paymentStatusFormat);
      aj->paymentStatusFormat=(s && *s)?strdup(s):NULL;

      s=GWEN_DB_GetCharValue(dbVppResponse, "vopMsg", 0, NULL);
      free(aj->vopMsg);
      aj->vopMsg=(s && *s)?strdup(s):NULL;

      _readVopResultGroup(j, aj, dbVppResponse);
      _readPmtStatusReport(j, aj, ctx, dbVppResponse);

      break; /* break loop, we found the vppResponse */
    } /* if "VppResponse" */
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */

  return 0;
}



const uint8_t *AH_Job_VPP_GetPtrVopId(const AH_JOB *j)
{
  AH_JOB_VPP *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_VPP, j);
  assert(aj);

  return aj->ptrVopId;
}



unsigned int AH_Job_VPP_GetLenVopId(const AH_JOB *j)
{
  AH_JOB_VPP *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_VPP, j);
  assert(aj);

  return aj->lenVopId;
}



const char *AH_Job_VPP_GetVopMsg(const AH_JOB *j)
{
  AH_JOB_VPP *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_VPP, j);
  assert(aj);

  return aj->vopMsg;
}



const AH_VOP_RESULT_LIST *AH_Job_VPP_GetResultList(const AH_JOB *j)
{
  AH_JOB_VPP *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_VPP, j);
  assert(aj);

  return aj->resultList;
}



int AH_Job_VPP_IsNeededForCode(const AH_JOB *j, const char *code)
{
  GWEN_DB_NODE *dbParams;
  GWEN_DB_NODE *dbRequiredForJobs;

  dbParams=AH_Job_GetParams(j);
  assert(dbParams);
  dbRequiredForJobs=GWEN_DB_GetGroup(dbParams, GWEN_PATH_FLAGS_PATHMUSTEXIST, "VopRequiredForJobs");
  if (dbRequiredForJobs) {
    int i;

    for(i=0; i<99; i++) {
      const char *s;

      s=GWEN_DB_GetCharValue(dbRequiredForJobs, "Job", i, NULL);
      if (!(s && *s))
        break;
      else {
        if (strcasecmp(s, code)==0)
          return 1;
      }
    }
  }

  return 0;
}



void _readVopId(AH_JOB_VPP *aj, GWEN_DB_NODE *dbArgs, GWEN_DB_NODE *dbVppResponse)
{
  const uint8_t *ptr;
  unsigned int len=0;
  
  /* get VOP id */
  ptr=GWEN_DB_GetBinValue(dbVppResponse, "vopId", 0, NULL, 0, &len);
  if (ptr && len) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Received VOP id");
    GWEN_DB_SetBinValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "vopId", ptr, len);
    free(aj->ptrVopId);
    aj->ptrVopId=(uint8_t*) malloc(len);
    memmove(aj->ptrVopId, ptr, len);
    aj->lenVopId=len;
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No VOP id received");
  }
}



void _readPollingId(AH_JOB_VPP *aj, GWEN_DB_NODE *dbArgs, GWEN_DB_NODE *dbVppResponse)
{
  const uint8_t *ptr;
  unsigned int len=0;
  
  /* get polling id */
  ptr=GWEN_DB_GetBinValue(dbVppResponse, "pollingId", 0, NULL, 0, &len);
  if (ptr && len) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Received polling id");
    GWEN_DB_SetBinValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "pollingId", ptr, len);
    free(aj->ptrPollingId);
    aj->ptrPollingId=(uint8_t*) malloc(len);
    memmove(aj->ptrPollingId, ptr, len);
    aj->lenPollingId=len;
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No polling id received");
  }
}



void _readVopResultGroup(AH_JOB *j, AH_JOB_VPP *aj, GWEN_DB_NODE *dbVppResponse)
{
  GWEN_DB_NODE *dbVppResult;

  dbVppResult=GWEN_DB_FindFirstGroup(dbVppResponse, "vopResult");
  while (dbVppResult) {
    const char *sIban;
    const char *sResult;
    const char *sAltName;
    int vopResultCode;

    sIban=GWEN_DB_GetCharValue(dbVppResult, "iban", 0, NULL);
    sResult=GWEN_DB_GetCharValue(dbVppResult, "result", 0, NULL);
    vopResultCode=sResult?AH_VopResultCode_fromString(sResult):AH_VopResultCodeNone;
    sAltName=GWEN_DB_GetCharValue(dbVppResult, "alternativeRecipientName", 0, NULL);
    if (vopResultCode!=AH_VopResultCodePending) {
      AH_VOP_RESULT *vr;

      vr=AH_VopResult_new();
      AH_VopResult_SetRemoteIban(vr, sIban);
      AH_VopResult_SetAltRemoteName(vr, sAltName);
      AH_VopResult_SetResult(vr, vopResultCode);
      AH_VopResult_List_Add(vr, aj->resultList);
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Added VOP result");
      AH_VopResult_Log(vr, AQHBCI_LOGDOMAIN, GWEN_LoggerLevel_Error); /* use ERROR for now */
    }

    dbVppResult=GWEN_DB_FindNextGroup(dbVppResult, "vopResult");
  }
}



void _readPmtStatusReport(AH_JOB *j, AH_JOB_VPP *aj, AB_IMEXPORTER_CONTEXT *ctx, GWEN_DB_NODE *dbVppResponse)
{
  const uint8_t *ptr;
  unsigned int len=0;

  ptr=GWEN_DB_GetBinValue(dbVppResponse, "paymentStatusReport", 0, NULL, 0, &len);
  if (ptr && len) {
    AB_TRANSACTION_LIST *transactionList;

    DBG_ERROR(AQHBCI_LOGDOMAIN, "Received statement status report");

    transactionList=_pmtStatusReportToTransactionList(j,  ptr, len);
    if (transactionList) {
      _addResultsFromTransactionList(j, aj, transactionList);
      AB_Transaction_List_free(transactionList);
    }
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No polling id received");
  }
}



void _addResultsFromTransactionList(AH_JOB *j, AH_JOB_VPP *aj, AB_TRANSACTION_LIST *transactionList)
{
  AB_TRANSACTION *t;

  t=AB_Transaction_List_First(transactionList);
  while(t) {
    const char *sResultCode;

    sResultCode=AB_Transaction_GetTransactionText(t);
    if (sResultCode && *sResultCode) {
      int vopResultCode;

      vopResultCode=AH_VopResultCode_fromString(sResultCode);
      if (vopResultCode!=AH_VopResultCodePending) {
	AH_VOP_RESULT *vr;

	vr=AH_VopResult_new();
	AH_VopResult_SetLocalBic(vr, AB_Transaction_GetLocalBic(t));
	AH_VopResult_SetRemoteIban(vr, AB_Transaction_GetRemoteIban(t));
	AH_VopResult_SetRemoteName(vr, AB_Transaction_GetRemoteName(t));
	AH_VopResult_SetAltRemoteName(vr, AB_Transaction_GetUltimateCreditor(t));
	AH_VopResult_SetResult(vr, vopResultCode);
	AH_VopResult_List_Add(vr, aj->resultList);
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Added VOP result");
        AH_VopResult_Log(vr, AQHBCI_LOGDOMAIN, GWEN_LoggerLevel_Error); /* use ERROR for now */
      }
      else {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Not adding incomplete VOP result (pending)");
      }
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing result code");
    }
    t=AB_Transaction_List_Next(t);
  }
}



AB_TRANSACTION_LIST *_pmtStatusReportToTransactionList(AH_JOB *j, const uint8_t *dataPtr, uint32_t dataLen)
{
  AB_IMEXPORTER_CONTEXT *ioc;

  ioc=_dataToContext(j, dataPtr, dataLen);
  if (ioc) {
    AB_TRANSACTION_LIST *transactionList;
    AB_IMEXPORTER_ACCOUNTINFO *ai;

    transactionList=AB_Transaction_List_new();
    ai=AB_ImExporterContext_GetFirstAccountInfo(ioc);
    while(ai) {
      AB_TRANSACTION *t;

      t=AB_ImExporterAccountInfo_GetFirstTransaction(ai, 0, 0);
      while(t) {
        AB_TRANSACTION *tNext;

        tNext=AB_Transaction_List_Next(t);
        AB_Transaction_List_Del(t);
        AB_Transaction_List_Add(t, transactionList);
        t=tNext;
      }

      ai=AB_ImExporterAccountInfo_List_Next(ai);
    }

    AB_ImExporterContext_free(ioc);
    if (AB_Transaction_List_GetCount(transactionList)<1) {
      AB_Transaction_List_free(transactionList);
      return NULL;
    }
    return transactionList;
  }

  return NULL;
}



AB_IMEXPORTER_CONTEXT *_dataToContext(AH_JOB *j,  const uint8_t *dataPtr, uint32_t dataLen)
{
  AB_BANKING *ab;
  GWEN_DB_NODE *dbProfile;
  AB_IMEXPORTER_CONTEXT *ioc;
  int rv;

  ioc=AB_ImExporterContext_new();
  ab=AH_Job_GetBankingApi(j);
  dbProfile=AB_Banking_GetImExporterProfile(ab, "xml", "default");
  rv=AB_Banking_ImportFromBuffer(ab, "xml", ioc, dataPtr, dataLen, dbProfile);
  if (rv<0) {
    AB_ImExporterContext_free(ioc);
    GWEN_DB_Group_free(dbProfile);
    return NULL;
  }

  GWEN_DB_Group_free(dbProfile);
  return ioc;
}



