/***************************************************************************
 begin       : Sat Augl 03 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "bpd_read.h"

#include "libaqfints/aqfints.h"

#include "servicelayer/bpd/bpd.h"

#include <gwenhywfar/debug.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static void _readTanMethods(GWEN_DB_NODE *db, int segmentVersion, AQFINTS_TANMETHOD_LIST *tmList);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



AQFINTS_BPD *AQFINTS_Bpd_SampleBpdFromSegmentList(AQFINTS_PARSER *parser,
                                                  AQFINTS_SEGMENT_LIST *segmentList,
                                                  int removeFromSegList)
{
  AQFINTS_SEGMENT *segment;
  AQFINTS_BPD *bpd=NULL;

  bpd=AQFINTS_Bpd_new();

  segment=AQFINTS_Segment_List_First(segmentList);
  while (segment) {
    AQFINTS_SEGMENT *nextSegment;
    const char *sCode;
    int segVer;
    int doRemoveSegment=0;
    GWEN_DB_NODE *db;

    nextSegment=AQFINTS_Segment_List_Next(segment);

    db=AQFINTS_Segment_GetDbData(segment);
    segVer=AQFINTS_Segment_GetSegmentVersion(segment);
    sCode=AQFINTS_Segment_GetCode(segment);
    DBG_ERROR(0, "Handling segment %s:%d", sCode?sCode:"(unnamed)", segVer);
    if (db && sCode && *sCode) {
      if (strcasecmp(sCode, "HIBPA")==0) { /* read bankData */
        AQFINTS_BANKDATA *bankData;

        bankData=AQFINTS_Bpd_ReadBankData(db);
        if (bankData) {
          DBG_ERROR(AQFINTS_LOGDOMAIN, "Found bank data");
          AQFINTS_Bpd_SetBankData(bpd, bankData);
          if (removeFromSegList)
            doRemoveSegment=1;
        }
      }
      else if (strcasecmp(sCode, "HIKOM")==0) { /* read bpdAddr */
        AQFINTS_BPDADDR *bpdAddr;

        bpdAddr=AQFINTS_Bpd_ReadBpdAddr(db);
        if (bpdAddr) {
          AQFINTS_Bpd_AddBpdAddr(bpd, bpdAddr);
          if (removeFromSegList)
            doRemoveSegment=1;
        }
      }
      else if (strcasecmp(sCode, "HIPINS")==0) { /* read tanInfo */
        AQFINTS_TANINFO *ti;

        ti=AQFINTS_Bpd_ReadTanInfo(db);
        if (ti) {
          AQFINTS_Bpd_SetTanInfo(bpd, ti);
          if (removeFromSegList)
            doRemoveSegment=1;
        }
      }
      else {
        AQFINTS_SEGMENT *defSegment;

        defSegment=AQFINTS_Parser_FindSegmentByCode(parser, sCode, segVer, 0);
        if (defSegment==NULL) {
          DBG_ERROR(0, "Segment %s:%d not found in definitions", sCode?sCode:"(unnamed)", segVer);
        }
        if (defSegment && (AQFINTS_Segment_GetFlags(defSegment) & AQFINTS_SEGMENT_FLAGS_ISBPD)) {
          AQFINTS_BPDJOB *j;

          /* is a bpd job */
          DBG_ERROR(0, "Job %s:%d is a BPD job", sCode, segVer);
          j=AQFINTS_Bpd_ReadBpdJob(db);
          if (j) {
            AQFINTS_Bpd_AddBpdJob(bpd, j);
            if (strcasecmp(sCode, "HITANS")==0) {
              AQFINTS_TANMETHOD_LIST *tmList;

              /* special handling for HITANS (parameters for HKTAN) */
              tmList=AQFINTS_Bpd_GetTanMethodList(bpd);
              if (tmList==NULL) {
                tmList=AQFINTS_TanMethod_List_new();
                AQFINTS_Bpd_SetTanMethodList(bpd, tmList);
              }
              _readTanMethods(db, segVer, tmList);
            }
            if (removeFromSegList)
              doRemoveSegment=1;
          }
        }
      }
    }

    if (doRemoveSegment) {
      AQFINTS_Segment_List_Del(segment);
      AQFINTS_Segment_free(segment);
    }

    segment=nextSegment;
  }

  if (AQFINTS_Bpd_GetBankData(bpd)==NULL) {
    AQFINTS_Bpd_free(bpd);
    return NULL;
  }

  return bpd;
}



void _readTanMethods(GWEN_DB_NODE *db, int segmentVersion, AQFINTS_TANMETHOD_LIST *tmList)
{
  GWEN_DB_NODE *dbT;

  dbT=GWEN_DB_FindFirstGroup(db, "tanMethod");
  while (dbT) {
    AQFINTS_TANMETHOD *tm;

    tm=AQFINTS_Bpd_ReadTanMethod(dbT);
    if (tm) {
      int fn;

      /* store segment version within function code */
      fn=(segmentVersion*1000)+AQFINTS_TanMethod_GetFunction(tm);
      AQFINTS_TanMethod_SetFunction(tm, fn);
      AQFINTS_TanMethod_List_Add(tm, tmList);
    }
    dbT=GWEN_DB_FindNextGroup(dbT, "tanMethod");
  }
}





AQFINTS_BANKDATA *AQFINTS_Bpd_ReadBankData(GWEN_DB_NODE *db)
{
  AQFINTS_BANKDATA *bankData;
  const char *s;
  int i;
  GWEN_DB_NODE *dbT;

  bankData=AQFINTS_BankData_new();

  i=GWEN_DB_GetIntValue(db, "version", 0, 0);
  AQFINTS_BankData_SetVersion(bankData, i);

  i=GWEN_DB_GetIntValue(db, "country", 0, 0);
  AQFINTS_BankData_SetCountry(bankData, i);

  s=GWEN_DB_GetCharValue(db, "bankCode", 0, NULL);
  if (s && *s)
    AQFINTS_BankData_SetBankCode(bankData, s);

  s=GWEN_DB_GetCharValue(db, "name", 0, NULL);
  if (s && *s)
    AQFINTS_BankData_SetBankName(bankData, s);

  i=GWEN_DB_GetIntValue(db, "jobTypesPerMsg", 0, 0);
  AQFINTS_BankData_SetJobTypesPerMsg(bankData, i);

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "languages");
  if (dbT) {
    for (i=0; i<9; i++) {
      int v;

      v=GWEN_DB_GetIntValue(dbT, "language", i, -1);
      if (v<0)
        break;
      AQFINTS_BankData_SetLanguagesAt(bankData, i, v);
    }
  }

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "versions");
  if (dbT) {
    for (i=0; i<9; i++) {
      int v;

      v=GWEN_DB_GetIntValue(dbT, "version", i, -1);
      if (v<0)
        break;
      AQFINTS_BankData_SetHbciVersionsAt(bankData, i, v);
    }
  }

  i=GWEN_DB_GetIntValue(db, "maxMsgSize", 0, 0);
  AQFINTS_BankData_SetMaxMsgSize(bankData, i);

  i=GWEN_DB_GetIntValue(db, "minTimeout", 0, 0);
  AQFINTS_BankData_SetMinTimeout(bankData, i);

  i=GWEN_DB_GetIntValue(db, "maxTimeout", 0, 0);
  AQFINTS_BankData_SetMaxTimeout(bankData, i);

  return bankData;
}



AQFINTS_BPDADDR *AQFINTS_Bpd_ReadBpdAddr(GWEN_DB_NODE *db)
{
  AQFINTS_BPDADDR *addr;
  const char *s;
  int i;
  GWEN_DB_NODE *dbT;

  addr=AQFINTS_BpdAddr_new();

  i=GWEN_DB_GetIntValue(db, "country", 0, 0);
  AQFINTS_BpdAddr_SetCountry(addr, i);

  s=GWEN_DB_GetCharValue(db, "bankCode", 0, NULL);
  AQFINTS_BpdAddr_SetBankCode(addr, s);

  i=GWEN_DB_GetIntValue(db, "language", 0, 0);
  AQFINTS_BpdAddr_SetLanguage(addr, i);

  dbT=GWEN_DB_FindFirstGroup(db, "service");
  while (dbT) {
    AQFINTS_BPDADDR_SERVICE *srv;

    srv=AQFINTS_Bpd_ReadBpdAddrService(dbT);
    AQFINTS_BpdAddr_AddService(addr, srv);
    dbT=GWEN_DB_FindNextGroup(dbT, "service");
  }

  return addr;
}



AQFINTS_BPDADDR_SERVICE *AQFINTS_Bpd_ReadBpdAddrService(GWEN_DB_NODE *db)
{
  AQFINTS_BPDADDR_SERVICE *srv;

  srv=AQFINTS_BpdAddrService_fromDb(db);
  if (srv==NULL) {
    DBG_INFO(0, "here");
    return NULL;
  }

  return srv;
}



AQFINTS_BPDJOB *AQFINTS_Bpd_ReadBpdJob(GWEN_DB_NODE *db)
{
  GWEN_DB_NODE *dbT;

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "head");
  if (dbT) {
    AQFINTS_BPDJOB *j;
    const char *s;
    int i;

    j=AQFINTS_BpdJob_new();

    s=GWEN_DB_GetCharValue(dbT, "code", 0, NULL);
    if (s && *s)
      AQFINTS_BpdJob_SetCode(j, s);

    i=GWEN_DB_GetIntValue(dbT, "version", 0, 0);
    AQFINTS_BpdJob_SetVersion(j, i);

    i=GWEN_DB_GetIntValue(db, "jobsPerMsg", 0, 0);
    AQFINTS_BpdJob_SetJobsPerMsg(j, i);

    i=GWEN_DB_GetIntValue(db, "minSigs", 0, 0);
    AQFINTS_BpdJob_SetMinSigs(j, i);

    i=GWEN_DB_GetIntValue(db, "securityClass", 0, 0);
    AQFINTS_BpdJob_SetSecurityClass(j, i);

    AQFINTS_BpdJob_SetSettings(j, GWEN_DB_Group_dup(db));

    return j;
  }
  else {
    DBG_ERROR(0, "BpdJob has no HEAD group");
    GWEN_DB_Dump(db, 2);
  }

  return NULL;
}



AQFINTS_TANINFO *AQFINTS_Bpd_ReadTanInfo(GWEN_DB_NODE *db)
{
  AQFINTS_TANINFO *ti;
  int i;
  GWEN_DB_NODE *dbT;

  ti=AQFINTS_TanInfo_new();

  i=GWEN_DB_GetIntValue(db, "jobsPerMsg", 0, 0);
  AQFINTS_TanInfo_SetJobsPerMsg(ti, i);

  i=GWEN_DB_GetIntValue(db, "minSigs", 0, 0);
  AQFINTS_TanInfo_SetMinSigs(ti, i);

  i=GWEN_DB_GetIntValue(db, "securityClass", 0, 0);
  AQFINTS_TanInfo_SetSecurityClass(ti, i);

  dbT=GWEN_DB_FindFirstGroup(db, "job");
  while (dbT) {
    AQFINTS_TANJOBINFO *tj;

    tj=AQFINTS_Bpd_ReadTanJobInfo(dbT);
    AQFINTS_TanInfo_AddTanJobInfo(ti, tj);
    dbT=GWEN_DB_FindNextGroup(dbT, "job");
  }

  return ti;
}



AQFINTS_TANJOBINFO *AQFINTS_Bpd_ReadTanJobInfo(GWEN_DB_NODE *db)
{
  AQFINTS_TANJOBINFO *tj;
  const char *s;

  tj=AQFINTS_TanJobInfo_new();

  s=GWEN_DB_GetCharValue(db, "code", 0, NULL);
  AQFINTS_TanJobInfo_SetCode(tj, s);

  s=GWEN_DB_GetCharValue(db, "needTan", 0, NULL);
  if (s && (*s=='j' || *s=='J'))
    AQFINTS_TanJobInfo_AddFlags(tj, AQFINTS_TANJOBINFO_FLAGS_NEEDTAN);

  return tj;
}



AQFINTS_TANMETHOD *AQFINTS_Bpd_ReadTanMethod(GWEN_DB_NODE *db)
{
  AQFINTS_TANMETHOD *tm;
  int i;
  const char *s;

  tm=AQFINTS_TanMethod_new();

  i=GWEN_DB_GetIntValue(db, "function", 0, 0);
  AQFINTS_TanMethod_SetFunction(tm, i);

  i=GWEN_DB_GetIntValue(db, "process", 0, 0);
  AQFINTS_TanMethod_SetProcess(tm, i);

  s=GWEN_DB_GetCharValue(db, "methodId", 0, NULL);
  AQFINTS_TanMethod_SetMethodId(tm, s);

  s=GWEN_DB_GetCharValue(db, "zkaTanName", 0, NULL);
  AQFINTS_TanMethod_SetZkaTanName(tm, s);

  s=GWEN_DB_GetCharValue(db, "zkaTanVersion", 0, NULL);
  AQFINTS_TanMethod_SetZkaTanVersion(tm, s);

  s=GWEN_DB_GetCharValue(db, "methodName", 0, NULL);
  AQFINTS_TanMethod_SetMethodName(tm, s);

  i=GWEN_DB_GetIntValue(db, "tanMaxLen", 0, 0);
  AQFINTS_TanMethod_SetTanMaxLen(tm, i);

  i=GWEN_DB_GetIntValue(db, "formatId", 0, 0);
  AQFINTS_TanMethod_SetFormatId(tm, i);

  s=GWEN_DB_GetCharValue(db, "prompt", 0, NULL);
  AQFINTS_TanMethod_SetPrompt(tm, s);

  i=GWEN_DB_GetIntValue(db, "returnMaxLen", 0, 0);
  AQFINTS_TanMethod_SetReturnMaxLen(tm, i);

  s=GWEN_DB_GetCharValue(db, "multiTanAllowed", 0, "N");
  if (s && (*s=='J' || *s=='j'))
    AQFINTS_TanMethod_AddFlags(tm, AQFINTS_TANMETHOD_FLAGS_MULTITAN_ALLOWED);

  i=GWEN_DB_GetIntValue(db, "timeShiftAllowed", 0, 0);
  AQFINTS_TanMethod_SetTimeShiftAllowed(tm, i);

  s=GWEN_DB_GetCharValue(db, "stornoAllowed", 0, "N");
  if (s && (*s=='J' || *s=='j'))
    AQFINTS_TanMethod_AddFlags(tm, AQFINTS_TANMETHOD_FLAGS_STORNO_ALLOWED);

  i=GWEN_DB_GetIntValue(db, "needSmsAccount", 0, 0);
  AQFINTS_TanMethod_SetNeedSmsAccount(tm, i);

  i=GWEN_DB_GetIntValue(db, "needLocalAccount", 0, 0);
  AQFINTS_TanMethod_SetNeedLocalAccount(tm, i);

  s=GWEN_DB_GetCharValue(db, "needChallengeClass", 0, "N");
  if (s && (*s=='J' || *s=='j'))
    AQFINTS_TanMethod_AddFlags(tm, AQFINTS_TANMETHOD_FLAGS_NEED_CHALLENGE_CLASS);

  s=GWEN_DB_GetCharValue(db, "challengeIsStructured", 0, "N");
  if (s && (*s=='J' || *s=='j'))
    AQFINTS_TanMethod_AddFlags(tm, AQFINTS_TANMETHOD_FLAGS_CHALLENGE_IS_STRUCTURED);

  s=GWEN_DB_GetCharValue(db, "initMode", 0, 0);
  AQFINTS_TanMethod_SetInitMode(tm, s);

  i=GWEN_DB_GetIntValue(db, "needTanMediumId", 0, 0);
  AQFINTS_TanMethod_SetNeedTanMediumId(tm, i);

  s=GWEN_DB_GetCharValue(db, "needHhdUcResponse", 0, "N");
  if (s && (*s=='J' || *s=='j'))
    AQFINTS_TanMethod_AddFlags(tm, AQFINTS_TANMETHOD_FLAGS_NEED_HHDUC);

  i=GWEN_DB_GetIntValue(db, "maxActiveTanMedia", 0, 0);
  AQFINTS_TanMethod_SetMaxActiveTanMedia(tm, i);

  return tm;
}



