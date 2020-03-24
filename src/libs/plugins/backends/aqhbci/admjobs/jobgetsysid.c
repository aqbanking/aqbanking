/***************************************************************************
    begin       : Fri Feb 01 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobgetsysid_p.h"


GWEN_INHERIT(AH_JOB, AH_JOB_GETSYSID)


AH_JOB *AH_Job_GetSysId_new(AB_PROVIDER *pro, AB_USER *u)
{
  AH_JOB *j;
  GWEN_DB_NODE *args;
  AH_JOB_GETSYSID *jd;

  assert(u);
  j=AH_Job_new("JobSync", pro, u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "JobSync not supported, should not happen");
    return 0;
  }

  GWEN_NEW_OBJECT(AH_JOB_GETSYSID, jd);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_GETSYSID, j, jd,
                       AH_Job_GetSysId_FreeData);
  AH_Job_SetProcessFn(j, AH_Job_GetSysId_Process);
  AH_Job_SetNextMsgFn(j, AH_Job_GetSysId_NextMsg);

  /* set arguments */
  args=AH_Job_GetArguments(j);
  assert(args);
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS, "ident/country", 280);
  GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS, "ident/bankCode", AB_User_GetBankCode(u));
  GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS, "ident/customerId", AB_User_GetCustomerId(u));

  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS, "sync/mode", 0);
  GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS, "sync/systemId", "0");

  DBG_INFO(AQHBCI_LOGDOMAIN, "JobGetSysId created");
#if 0
  AH_Job_Dump(j, stderr, 2);
#endif
  return j;
}



void GWENHYWFAR_CB AH_Job_GetSysId_FreeData(void *bp, void *p)
{
  AH_JOB_GETSYSID *jd;

  jd=(AH_JOB_GETSYSID *)p;
  free(jd->sysId);
  GWEN_FREE_OBJECT(jd);
}



int AH_Job_GetSysId_ExtractSysId(AH_JOB *j)
{
  AH_JOB_GETSYSID *jd;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  GWEN_DB_NODE *dbSyncResponse;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETSYSID, j);
  assert(jd);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Extracting system-id from this response:");
  if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Debug)
    GWEN_DB_Dump(dbResponses, 2);

  /* search for "SyncResponse" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while (dbCurr) {
    dbSyncResponse=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data/SyncResponse");
    if (dbSyncResponse) {
      const char *s;

      DBG_INFO(AQHBCI_LOGDOMAIN, "Found a sync response");
      s=GWEN_DB_GetCharValue(dbSyncResponse, "systemId", 0, 0);
      if (s) {
        free(jd->sysId);
        jd->sysId=strdup(s);
        return 0;
      }
      else {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "No system id found");
        free(jd->sysId);
        jd->sysId=0;
        AH_Job_SetStatus(j, AH_JobStatusError);
        return -1;
      }
    }
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }
  DBG_ERROR(AQHBCI_LOGDOMAIN, "No syncresponse found");
  AH_Job_SetStatus(j, AH_JobStatusError);
  return GWEN_ERROR_GENERIC;
}



int AH_Job_GetSysId_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  AH_JOB_GETSYSID *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETSYSID, j);
  assert(jd);

  return AH_Job_GetSysId_ExtractSysId(j);
}



const char *AH_Job_GetSysId_GetSysId(AH_JOB *j)
{
  AH_JOB_GETSYSID *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETSYSID, j);
  assert(jd);
  return jd->sysId;
}




int AH_Job_GetSysId_NextMsg(AH_JOB *j)
{
  AH_JOB_GETSYSID *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETSYSID, j);
  assert(jd);

  if (AH_Job_GetSysId_ExtractSysId(j)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not extract system id");
    return 0;
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Setting system id [%s]", jd->sysId);
    AH_User_SetSystemId(AH_Job_GetUser(j), jd->sysId);
  }

  return 1;
}



