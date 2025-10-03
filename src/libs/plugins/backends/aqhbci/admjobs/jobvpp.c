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



/* ------------------------------------------------------------------------------------------------
 * code
 * ------------------------------------------------------------------------------------------------
 */

GWEN_INHERIT(AH_JOB, AH_JOB_VPP);



AH_JOB *AH_Job_VPP_new(AB_PROVIDER *pro, AB_USER *u, int jobVersion)
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
  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, _cbProcess);

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
  free(aj->pollingId);
  free(aj->vopId);
  free(aj->paymentStatusFormat);
  free(aj->vopMsg);
  GWEN_FREE_OBJECT(aj);
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
      GWEN_DB_Dump(dbVppResponse, 2);

      s=GWEN_DB_GetCharValue(dbVppResponse, "pollingId", 0, 0);
      free(aj->pollingId);
      aj->pollingId=(s && *s)?strdup(s):NULL;
      if (s && *s) {
        /* write polling id for next message */
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Received polling id: %s", s);
        GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "pollingId", s);
      }

      s=GWEN_DB_GetCharValue(dbVppResponse, "vopId", 0, NULL);
      free(aj->vopId);
      aj->vopId=(s && *s)?strdup(s):NULL;

      s=GWEN_DB_GetCharValue(dbVppResponse, "paymentStatusFormat", 0, NULL);
      free(aj->paymentStatusFormat);
      aj->paymentStatusFormat=(s && *s)?strdup(s):NULL;

      s=GWEN_DB_GetCharValue(dbVppResponse, "vopMsg", 0, NULL);
      free(aj->vopMsg);
      aj->vopMsg=(s && *s)?strdup(s):NULL;

      break; /* break loop, we found the vppResponse */
    } /* if "VppResponse" */
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */

  return 0;
}



const char *AH_Job_VPP_GetVopId(const AH_JOB *j)
{
  AH_JOB_VPP *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_VPP, j);
  assert(aj);

  return aj->vopId;
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

      s=GWEN_DB_GetCharValue(dbParams, "", 0, NULL);
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




