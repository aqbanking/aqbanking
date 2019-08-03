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

#include "aqfints/aqfints.h"

#include "servicelayer/bpd/bpd.h"

#include <gwenhywfar/debug.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */



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
      else if (strcasecmp(sCode, "HIPINS")==0) { /* read pinTanBpd */
#if 0
        AQFINTS_BPDADDR *bpdAddr;

        bpdAddr=AQFINTS_Bpd_ReadBpdAddr(db);
        if (bpdAddr) {
          AQFINTS_Bpd_AddBpdAddr(bpd, bpdAddr);
          if (removeFromSegList)
            doRemoveSegment=1;
        }
#endif
      }
      else if (AQFINTS_Parser_FindJobDefByParams(parser, sCode, 0, 0)) {
        AQFINTS_BPDJOB *j;

        /* is a bpd job */
        DBG_ERROR(0, "Job %s:%d is a BPD job", sCode, segVer);
        j=AQFINTS_Bpd_ReadBpdJob(db);
        if (j) {
          AQFINTS_Bpd_AddBpdJob(bpd, j);
          if (removeFromSegList)
            doRemoveSegment=1;
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






AQFINTS_BANKDATA *AQFINTS_Bpd_ReadBankData(GWEN_DB_NODE *db)
{
  AQFINTS_BANKDATA *bankData;
  const char *s;
  int i;
  GWEN_DB_NODE *dbT;

  bankData=AQFINTS_BankData_new();

  s=GWEN_DB_GetCharValue(db, "name", 0, NULL);
  if (s && *s)
    AQFINTS_BankData_SetBankName(bankData, s);

  i=GWEN_DB_GetIntValue(db, "jobTypesPerMsg", 0, 0);
  AQFINTS_BankData_SetJobTypesPerMsg(bankData, i);

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTNOTEXIST, "languages");
  if (dbT) {
    for (i=0; i<9; i++) {
      int v;

      v=GWEN_DB_GetIntValue(dbT, "language", i, -1);
      if (v<0)
        break;
      AQFINTS_BankData_SetLanguagesAt(bankData, i, v);
    }
  }

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTNOTEXIST, "versions");
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
  while(dbT) {
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
  const char *s;
  int i;

  srv=AQFINTS_BpdAddrService_new();

  i=GWEN_DB_GetIntValue(db, "type", 0, 0);
  AQFINTS_BpdAddrService_SetType(srv, i);

  s=GWEN_DB_GetCharValue(db, "address", 0, NULL);
  if (s && *s)
    AQFINTS_BpdAddrService_SetAddress(srv, s);

  s=GWEN_DB_GetCharValue(db, "suffix", 0, NULL);
  if (s && *s)
    AQFINTS_BpdAddrService_SetSuffix(srv, s);

  s=GWEN_DB_GetCharValue(db, "filter", 0, NULL);
  if (s && *s)
    AQFINTS_BpdAddrService_SetFilter(srv, s);

  i=GWEN_DB_GetIntValue(db, "filterVersion", 0, 0);
  AQFINTS_BpdAddrService_SetFilterVersion(srv, i);

  return srv;
}



AQFINTS_BPDJOB *AQFINTS_Bpd_ReadBpdJob(GWEN_DB_NODE *db)
{
  GWEN_DB_NODE *dbT;

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTNOTEXIST, "head");
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

    i=GWEN_DB_GetIntValue(db, "secProfile", 0, 0);
    AQFINTS_BpdJob_SetSecProfile(j, i);

    AQFINTS_BpdJob_SetSettings(j, GWEN_DB_Group_dup(db));

    return j;
  }

  return NULL;
}




