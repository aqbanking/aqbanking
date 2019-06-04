/***************************************************************************
    begin       : Thu Jan 31 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobtan_p.h"


GWEN_INHERIT(AH_JOB, AH_JOB_TAN);



AH_JOB *AH_Job_Tan_new(AB_PROVIDER *pro, AB_USER *u, int process, int jobVersion)
{
  AH_JOB *j;
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbArgs;
  GWEN_DB_NODE *dbParams;
  const char *s;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Looking for TAN job in version %d", jobVersion);
  j=AH_Job_new("JobTan", pro, u, 0, jobVersion);
  if (!j) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "TAN job in version %d not found", jobVersion);
    return NULL;
  }

  GWEN_NEW_OBJECT(AH_JOB_TAN, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_TAN, j, aj, AH_Job_Tan_FreeData);
  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, AH_Job_Tan_Process);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);
  dbParams=AH_Job_GetParams(j);
  assert(dbParams);

  GWEN_DB_SetIntValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "process", process);
  if (process==1 || process==2)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "moreTans", "N");

  /* set tanmedium id (if any) */
  s=AH_User_GetTanMediumId(u);
  if (s && *s)
    AH_Job_Tan_SetTanMediumId(j, s);

  aj->tanProcess=process;

  return j;
}



void GWENHYWFAR_CB AH_Job_Tan_FreeData(void *bp, void *p)
{
  AH_JOB_TAN *aj;

  aj=(AH_JOB_TAN *)p;
  free(aj->tanMediumId);
  free(aj->reference);
  free(aj->challenge);
  free(aj->challengeHhd);
  GWEN_FREE_OBJECT(aj);
}



int AH_Job_Tan_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  int rv;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Processing JobTan");
  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  /* search for "TanResponse" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while (dbCurr) {
    GWEN_DB_NODE *dbTanResponse;

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

    dbTanResponse=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data/tanResponse");
    if (dbTanResponse) {
      const char *s;

      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Got a TAN response");
      if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevel_Debug)
        GWEN_DB_Dump(dbTanResponse, 2);

      s=GWEN_DB_GetCharValue(dbTanResponse, "challenge", 0, 0);
      if (s) {
        free(aj->challenge);
        aj->challenge=strdup(s);
      }

      /* get special HHD challenge */
      if (GWEN_DB_VariableExists(dbTanResponse, "challengeHHD")) {
        const uint8_t *p;
        unsigned int l;

        p=GWEN_DB_GetBinValue(dbTanResponse, "challengeHHD", 0, NULL, 0, &l);
        if (p && l) {
          GWEN_BUFFER *bbuf;

          DBG_INFO(AQHBCI_LOGDOMAIN, "Job has a challengeHHD string:");
          GWEN_Text_LogString((const char *)p, l, AQHBCI_LOGDOMAIN, GWEN_LoggerLevel_Info);

          bbuf=GWEN_Buffer_new(0, 256, 0, 1);
#if 1
          /* data is binary, transform to string */
          GWEN_Text_ToHexBuffer((const char *) p, l, bbuf, 0, 0, 0);
#else
          /* data is a string, no need to transform to hex */
          GWEN_Buffer_AppendBytes(bbuf, (const char *) p, l);
#endif
          free(aj->challengeHhd);
          aj->challengeHhd=strdup(GWEN_Buffer_GetStart(bbuf));
          GWEN_Buffer_free(bbuf);
        }
      }

      s=GWEN_DB_GetCharValue(dbTanResponse, "jobReference", 0, 0);
      if (s) {
        free(aj->reference);
        aj->reference=strdup(s);
      }

      break; /* break loop, we found the tanResponse */
    } /* if "TanResponse" */
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  return 0;
}



int AH_Job_Tan_GetTanMethod(const AH_JOB *j)
{
  AH_JOB_TAN *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  return aj->tanMethod;
}



void AH_Job_Tan_SetTanMethod(AH_JOB *j, int i)
{
  AH_JOB_TAN *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  aj->tanMethod=i;
}



void AH_Job_Tan_SetHash(AH_JOB *j,
                        const unsigned char *p,
                        unsigned int len)
{
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbArgs;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  GWEN_DB_SetBinValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "jobHash", p, len);
}



void AH_Job_Tan_SetReference(AH_JOB *j, const char *p)
{
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbArgs;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "jobReference", p);
}



void AH_Job_Tan_SetTanList(AH_JOB *j, const char *p)
{
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbArgs;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "tanList", p);
}



void AH_Job_Tan_SetTanInfo(AH_JOB *j, const char *p)
{
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbArgs;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "tanInfo", p);
}



void AH_Job_Tan_SetSegCode(AH_JOB *j, const char *p)
{
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbArgs;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  if (p && *p) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Setting segment id in TAN to [%s]", p);
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "segmentId", p);
  }
  else {
    GWEN_DB_DeleteVar(dbArgs, "segmentId");
  }
}



const char *AH_Job_Tan_GetChallenge(const AH_JOB *j)
{
  AH_JOB_TAN *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  return aj->challenge;
}



const char *AH_Job_Tan_GetHhdChallenge(const AH_JOB *j)
{
  AH_JOB_TAN *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  return aj->challengeHhd;
}



const char *AH_Job_Tan_GetReference(const AH_JOB *j)
{
  AH_JOB_TAN *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  return aj->reference;
}



void AH_Job_Tan_SetTanMediumId(AH_JOB *j, const char *s)
{
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbArgs;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  if (s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "tanMediumId", s);
  else
    GWEN_DB_DeleteVar(dbArgs, "tanMediumId");
}



void AH_Job_Tan_SetLocalAccountInfo(AH_JOB *j,
                                    const char *bankCode,
                                    const char *accountId,
                                    const char *accountSubId)
{
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbArgs;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  if (bankCode && *bankCode)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "localAccount/bankCode", bankCode);
  if (accountId && *accountId)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "localAccount/accountId", accountId);
  if (accountSubId && *accountSubId)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "localAccount/accountSubId", accountSubId);
}



void AH_Job_Tan_SetSmsAccountInfo(AH_JOB *j,
                                  const char *bankCode,
                                  const char *accountId,
                                  const char *accountSubId)
{
  AH_JOB_TAN *aj;
  GWEN_DB_NODE *dbArgs;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  if (bankCode && *bankCode)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "smsAccount/bankCode", bankCode);
  if (accountId && *accountId)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "smsAccount/accountId", accountId);
  if (accountSubId && *accountSubId)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "smsAccount/accountSubId", accountSubId);
}



int AH_Job_Tan_FinishSetup(AH_JOB *j, AH_JOB *accJob)
{
  AH_JOB_TAN *aj;
  int rv;
  GWEN_DB_NODE *args;
  GWEN_DB_NODE *dbParams;
  GWEN_DB_NODE *dbMethod;
  const char *s;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TAN, j);
  assert(aj);

  args=AH_Job_GetArguments(j);
  assert(args);

  dbParams=AH_Job_GetParams(j);
  assert(dbParams);

  /* get data for the selected tan method */
  dbMethod=GWEN_DB_FindFirstGroup(dbParams, "tanMethod");
  while (dbMethod) {
    int tm;

    tm=GWEN_DB_GetIntValue(dbMethod, "function", 0, -1);
    if (tm!=-1 && tm==aj->tanMethod)
      break;

    dbMethod=GWEN_DB_FindNextGroup(dbMethod, "tanMethod");
  }

  if (!dbMethod) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No info for the selected iTAN method");
    return GWEN_ERROR_GENERIC;
  }

  rv=AH_Job_AddChallengeParams(accJob, AH_Job_GetSegmentVersion(j), dbMethod);
  if (rv<0) {
    if (rv==GWEN_ERROR_NOT_SUPPORTED) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Challenge parameters not supported by job, ignoring");
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    GWEN_STRINGLIST *sl;

    /* add challenge params as provided by addChallengeParams function */
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Using result of AddChallengeParams function");
    sl=AH_Job_GetChallengeParams(accJob);
    if (sl) {
      GWEN_STRINGLISTENTRY *e;

      e=GWEN_StringList_FirstEntry(sl);
      while (e) {
        GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_DEFAULT, "challengeParams/param", GWEN_StringListEntry_Data(e));
        e=GWEN_StringListEntry_Next(e);
      }
    }
  }

  /* set challenge class */
  s=GWEN_DB_GetCharValue(dbMethod, "needChallengeClass", 0, "N");
  if (strcasecmp(s, "J")==0)
    GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS, "challengeClass", AH_Job_GetChallengeClass(accJob));

  return 0;
}






