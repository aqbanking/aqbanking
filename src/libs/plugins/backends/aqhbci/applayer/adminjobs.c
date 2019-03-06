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

#include "aqhbci/aqhbci_l.h"

#include "adminjobs_p.h"
#include "job_l.h"
#include "jobqueue_l.h"
#include "hbci_l.h"
#include "provider_l.h"

#include <aqbanking/banking_be.h>
#include <aqbanking/backendsupport/account.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/text.h>

#include <assert.h>




/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_UpdateBank
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */



GWEN_INHERIT(AH_JOB, AH_JOB_UPDATEBANK)

/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_UpdateBank_new(AB_PROVIDER *pro, AB_USER *u)
{
  AH_JOB *j;
  GWEN_DB_NODE *args;
  AH_JOB_UPDATEBANK *jd;

  assert(u);
  j=AH_Job_new("JobUpdateBankInfo", pro, u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "JobUpdateBankInfo not supported, should not happen");
    return 0;
  }

  GWEN_NEW_OBJECT(AH_JOB_UPDATEBANK, jd);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_UPDATEBANK, j, jd,
                       AH_Job_UpdateBank_FreeData)
  AH_Job_SetProcessFn(j, AH_Job_UpdateBank_Process);

  /* set arguments */
  args=AH_Job_GetArguments(j);
  assert(args);
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "open/prepare/bpdversion", 0);
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "open/prepare/updversion", 0);

  DBG_INFO(AQHBCI_LOGDOMAIN, "JobUpdateBankInfo created");
  return j;
}



void GWENHYWFAR_CB AH_Job_UpdateBank_FreeData(void *bp, void *p)
{
  AH_JOB_UPDATEBANK *jd;

  jd=(AH_JOB_UPDATEBANK *)p;
  GWEN_FREE_OBJECT(jd);
}



int AH_Job_UpdateBank_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  AH_JOB_UPDATEBANK *jd;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  GWEN_DB_NODE *dbAccountData;
  AB_USER *u;
  AB_BANKING *ab;
  int accs;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_UPDATEBANK, j);
  assert(jd);

  /* This function is just for informational purposes, the real account handling is done in
   * AH_Job_CommitSystemData() */

  if (jd->scanned)
    return 0;

  jd->scanned=1;

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  u=AH_Job_GetUser(j);
  assert(u);

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  /* search for "AccountData" */
  accs=0;
  dbCurr=GWEN_DB_FindFirstGroup(dbResponses, "AccountData");
  while (dbCurr) {
    dbAccountData=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data/AccountData");
    if (dbAccountData) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Found an account");
      accs++;
    }
    dbCurr=GWEN_DB_FindNextGroup(dbCurr, "AccountData");
  }
  if (!accs) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "No accounts found");
  }

  return 0;
}



/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_TestVersion
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

GWEN_INHERIT(AH_JOB, AH_JOB_TESTVERSION);


AH_JOB *AH_Job_TestVersion_new(AB_PROVIDER *pro, AB_USER *u, int anon)
{
  AH_JOB *j;
  GWEN_DB_NODE *args;
  AH_JOB_TESTVERSION *jd;

  assert(u);
  if (anon)
    j=AH_Job_new("JobDialogInitAnon", pro, u, 0, 0);
  else
    j=AH_Job_new("JobDialogInit", pro, u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "JobTestVersion not supported, should not happen");
    return 0;
  }
  AH_Job_AddFlags(j, AH_JOB_FLAGS_DLGJOB);
  GWEN_NEW_OBJECT(AH_JOB_TESTVERSION, jd);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_TESTVERSION, j, jd,
                       AH_Job_TestVersion_FreeData);
  AH_Job_SetProcessFn(j, AH_Job_TestVersion_Process);

  jd->versionSupported=AH_JobTestVersion_ResultUnknown;

  /* set arguments */
  args=AH_Job_GetArguments(j);
  assert(args);
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "prepare/bpdversion", 0);
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "prepare/updversion", 0);

  DBG_INFO(AQHBCI_LOGDOMAIN, "JobTestVersion created");
  return j;
}



void GWENHYWFAR_CB AH_Job_TestVersion_FreeData(void *bp, void *p)
{
  AH_JOB_TESTVERSION *jd;

  jd=(AH_JOB_TESTVERSION *)p;
  GWEN_FREE_OBJECT(jd);
}



int AH_Job_TestVersion_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  AH_JOB_TESTVERSION *jd;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  GWEN_DB_NODE *dbMsgResponse;
  int hadAGoodResult=0;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TESTVERSION, j);
  assert(jd);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Parsing this response");
  if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Debug)
    GWEN_DB_Dump(dbResponses, 2);

  /* search for "MsgResult" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);

  while (dbCurr && jd->versionSupported==AH_JobTestVersion_ResultUnknown) {
    dbMsgResponse=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                   "data/MsgResult");
    if (dbMsgResponse) {
      GWEN_DB_NODE *dbResult;

      dbResult=GWEN_DB_FindFirstGroup(dbMsgResponse, "result");
      while (dbResult &&
             jd->versionSupported==AH_JobTestVersion_ResultUnknown) {
        int code;

        DBG_INFO(AQHBCI_LOGDOMAIN, "Found message result");
        code=GWEN_DB_GetIntValue(dbResult, "resultCode", 0, -1);
        if (code>=9000) {
          if (code==9180) {
            /* version is definately not supported */
            jd->versionSupported=AH_JobTestVersion_ResultNotSupported;
          }
          else {
            if (code>=9300 && code<9400) {
              /* error with the signature/encryption, so there was
               * no complaint about the version */
              jd->versionSupported=AH_JobTestVersion_ResultMaybeSupported;
            }
            else {
              const char *s;

              /* any other error, check for substring "version" */
              s=GWEN_DB_GetCharValue(dbResult, "text", 0, 0);
              if (s) {
                if (strstr(s, "version") || strstr(s, "Version")) {
                  /* seems to be a complaint about the version */
                  jd->versionSupported=AH_JobTestVersion_ResultNotSupported;
                }
              }
              /* still undecided ? */
              if (jd->versionSupported==AH_JobTestVersion_ResultUnknown)
                /* yes, so there was no complaint about the version */
                jd->versionSupported=AH_JobTestVersion_ResultMaybeSupported;
            }
          }
        } /* if error */
        else {
          /* not an error, so the version is definately supported */
          hadAGoodResult=1;
        }
        dbResult=GWEN_DB_FindNextGroup(dbResult, "result");
      } /* while result */
    }
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */

  /* still undecided ? */
  if (jd->versionSupported==AH_JobTestVersion_ResultUnknown)
    if (hadAGoodResult)
      /* in dubio pro reo */
      jd->versionSupported=AH_JobTestVersion_ResultSupported;

  return 0;
}



AH_JOB_TESTVERSION_RESULT AH_Job_TestVersion_GetResult(const AH_JOB *j)
{
  AH_JOB_TESTVERSION *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TESTVERSION, j);
  assert(jd);

  return jd->versionSupported;
}



/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_GetStatus
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


GWEN_INHERIT(AH_JOB, AH_JOB_GETSTATUS);


AH_JOB *AH_Job_GetStatus_new(AB_PROVIDER *pro,
                             AB_USER *u,
                             const GWEN_TIME *fromDate,
                             const GWEN_TIME *toDate)
{
  AH_JOB *j;
  AH_JOB_GETSTATUS *aj;
  GWEN_DB_NODE *dbArgs;

  j=AH_Job_new("JobGetStatus", pro, u, 0, 0);
  if (!j)
    return 0;

  GWEN_NEW_OBJECT(AH_JOB_GETSTATUS, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_GETSTATUS, j, aj,
                       AH_Job_GetStatus_FreeData);
  aj->results=AH_Result_List_new();

  if (fromDate)
    aj->fromDate=GWEN_Time_dup(fromDate);
  if (toDate)
    aj->toDate=GWEN_Time_dup(toDate);

  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, AH_Job_GetStatus_Process);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  if (fromDate) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    if (GWEN_Time_toString(fromDate, "YYYYMMDD", dbuf)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "Error in fromDate");
    }
    else {
      GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "fromDate", GWEN_Buffer_GetStart(dbuf));
    }
    GWEN_Buffer_free(dbuf);
  }

  if (toDate) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    if (GWEN_Time_toString(toDate, "YYYYMMDD", dbuf)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "Error in toDate");
    }
    else {
      GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "toDate", GWEN_Buffer_GetStart(dbuf));
    }
    GWEN_Buffer_free(dbuf);
  }

  return j;
}



void GWENHYWFAR_CB AH_Job_GetStatus_FreeData(void *bp, void *p)
{
  AH_JOB_GETSTATUS *aj;

  aj=(AH_JOB_GETSTATUS *)p;
  AH_Result_List_free(aj->results);
  GWEN_FREE_OBJECT(aj);
}



int AH_Job_GetStatus_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  AH_JOB_GETSTATUS *aj;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobGetStatus");
  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETSTATUS, j);
  assert(aj);

  /* nothing to do here (because this is not a real job and it is processed
   * by AH_Outbox) */
  return 0;
}



/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_GetItanModes
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

GWEN_INHERIT(AH_JOB, AH_JOB_GETITANMODES);


AH_JOB *AH_Job_GetItanModes_new(AB_PROVIDER *pro, AB_USER *u)
{
  AH_JOB *j;
  GWEN_DB_NODE *args;
  AH_JOB_GETITANMODES *jd;

  assert(u);
  j=AH_Job_new("JobGetItanModes", pro, u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "JobGetItanModes not supported, should not happen");
    return 0;
  }
  GWEN_NEW_OBJECT(AH_JOB_GETITANMODES, jd);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_GETITANMODES, j, jd,
                       AH_Job_GetItanModes_FreeData);
  AH_Job_SetProcessFn(j, AH_Job_GetItanModes_Process);

  /* set arguments */
  args=AH_Job_GetArguments(j);
  assert(args);
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "prepare/bpdversion", 0);
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "prepare/updversion", 0);

  jd->modeList[0]=-1;
  jd->modeCount=0;

  DBG_INFO(AQHBCI_LOGDOMAIN, "JobGetItanModes created");
  return j;
}



void GWENHYWFAR_CB AH_Job_GetItanModes_FreeData(void *bp, void *p)
{
  AH_JOB_GETITANMODES *jd;

  jd=(AH_JOB_GETITANMODES *)p;
  GWEN_FREE_OBJECT(jd);
}



int AH_Job_GetItanModes_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  AH_JOB_GETITANMODES *jd;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  GWEN_DB_NODE *dbMsgResponse;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETITANMODES, j);
  assert(jd);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Parsing this response");
  if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Debug)
    GWEN_DB_Dump(dbResponses, 2);

  /* search for "SegResult" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);

  while (dbCurr) {
    dbMsgResponse=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                   "data/SegResult");
    if (dbMsgResponse) {
      GWEN_DB_NODE *dbRes;

      dbRes=GWEN_DB_FindFirstGroup(dbMsgResponse, "result");
      while (dbRes) {
        int code;

        code=GWEN_DB_GetIntValue(dbRes, "resultCode", 0, -1);
        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Found message result (%d)", code);
        if (code==3920) {
          int i;

          jd->modeList[0]=-1;
          jd->modeCount=0;

          for (i=0; ; i++) {
            int k;

            k=GWEN_DB_GetIntValue(dbRes, "param", i, 0);
            if (k==0)
              break;
            if (jd->modeCount<AH_JOB_GETITANMODES_MAXMODES) {
              jd->modeList[jd->modeCount++]=k;
              jd->modeList[jd->modeCount]=-1;
            }
            else
              break;
          } /* for */
          if (i==0) {
            DBG_ERROR(AQHBCI_LOGDOMAIN,
                      "Bad server response: No TAN method reported");
            return -1;
          }
        } /* if correct result found */

        dbRes=GWEN_DB_FindNextGroup(dbRes, "result");
      } /* while result */
    }
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */

  return 0;
}



const int *AH_Job_GetItanModes_GetModes(const AH_JOB *j)
{
  AH_JOB_GETITANMODES *jd;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETITANMODES, j);
  assert(jd);

  return jd->modeList;
}



/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_ChangePin
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */



/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_ChangePin_new(AB_PROVIDER *pro, AB_USER *u, const char *newPin)
{
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;

  assert(u);
  j=AH_Job_new("JobChangePin", pro, u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "JobChangePin not supported, should not happen");
    return 0;
  }

  /* set challenge class */
  AH_Job_SetChallengeClass(j, 90);

  /* set arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "newPin", newPin);

  DBG_INFO(AQHBCI_LOGDOMAIN, "JobChangePin created");
  return j;
}







