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

AQFINTS_SEGMENT *_mkSegBankData(AQFINTS_PARSER *parser, int hbciVersion, const AQFINTS_BANKDATA *bankData);
AQFINTS_SEGMENT *_mkSegBankAddr(AQFINTS_PARSER *parser, int hbciVersion, const AQFINTS_BPDADDR *bpdAddr);
AQFINTS_SEGMENT *_mkSegTanInfo(AQFINTS_PARSER *parser, int hbciVersion, const AQFINTS_TANINFO *tanInfo);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AQFINTS_Bpd_Write(const AQFINTS_BPD *bpd, AQFINTS_PARSER *parser, int hbciVersion, int refSegNum,
                      AQFINTS_SEGMENT_LIST *segmentList)
{
  AQFINTS_BANKDATA *bankData;
  AQFINTS_BPDADDR_LIST *addrList;
  AQFINTS_TANINFO *tanInfo;

  bankData=AQFINTS_Bpd_GetBankData(bpd);
  if (bankData) {
    AQFINTS_SEGMENT *segment;

    segment=_mkSegBankData(parser, hbciVersion, bankData);
    if (segment==NULL) {
      DBG_ERROR(AQFINTS_LOGDOMAIN, "here");
      return GWEN_ERROR_GENERIC;
    }
    AQFINTS_Segment_SetRefSegmentNumber(segment, refSegNum);
    AQFINTS_Segment_List_Add(segment, segmentList);
  }

  addrList=AQFINTS_Bpd_GetAddresses(bpd);
  if (addrList) {
    AQFINTS_BPDADDR *bpdAddr;

    bpdAddr=AQFINTS_BpdAddr_List_First(addrList);
    while(bpdAddr) {
      AQFINTS_SEGMENT *segment;
  
      segment=_mkSegBankAddr(parser, hbciVersion, bpdAddr);
      if (segment==NULL) {
        DBG_ERROR(AQFINTS_LOGDOMAIN, "here");
        return GWEN_ERROR_GENERIC;
      }
      AQFINTS_Segment_SetRefSegmentNumber(segment, refSegNum);
      AQFINTS_Segment_List_Add(segment, segmentList);

      bpdAddr=AQFINTS_BpdAddr_List_Next(bpdAddr);
    }
  }

  tanInfo=AQFINTS_Bpd_GetTanInfo(bpd);
  if (tanInfo) {
    AQFINTS_SEGMENT *segment;

    segment=_mkSegTanInfo(parser, hbciVersion, tanInfo);
    if (segment==NULL) {
      DBG_ERROR(AQFINTS_LOGDOMAIN, "here");
      return GWEN_ERROR_GENERIC;
    }
    AQFINTS_Segment_SetRefSegmentNumber(segment, refSegNum);
    AQFINTS_Segment_List_Add(segment, segmentList);
  }

  return 0;
}





AQFINTS_SEGMENT *_mkSegBankData(AQFINTS_PARSER *parser, int hbciVersion, const AQFINTS_BANKDATA *bankData)
{
  AQFINTS_SEGMENT *defSegment;
  AQFINTS_SEGMENT *segment;
  GWEN_DB_NODE *dbSegment;
  GWEN_DB_NODE *dbGroup;
  const char *s;
  int i;

  defSegment=AQFINTS_Parser_FindSegmentHighestVersionForProto(parser, "HIBPA", hbciVersion);
  if (defSegment==NULL) {
    DBG_ERROR(0, "No matching definition segment found for HIBPA (proto=%d)", hbciVersion);
    return NULL;
  }

  segment=AQFINTS_Segment_new();
  AQFINTS_Segment_copy(segment, defSegment);

  dbSegment=GWEN_DB_Group_new("bankData");
  AQFINTS_Segment_SetDbData(segment, dbSegment);

  i=AQFINTS_BankData_GetVersion(bankData);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "version", i);

  i=AQFINTS_BankData_GetCountry(bankData);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "country", i);

  s=AQFINTS_BankData_GetBankCode(bankData);
  if (s)
    GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "bankCode", s);

  s=AQFINTS_BankData_GetBankName(bankData);
  if (s)
    GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "name", s);

  i=AQFINTS_BankData_GetJobTypesPerMsg(bankData);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "jobTypesPerMsg", i);

  i=AQFINTS_BankData_GetMaxMsgSize(bankData);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "maxMsgSize", i);

  i=AQFINTS_BankData_GetMinTimeout(bankData);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "minTimeout", i);

  i=AQFINTS_BankData_GetMaxTimeout(bankData);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "maxTimeout", i);

  dbGroup=GWEN_DB_GetGroup(dbSegment, GWEN_PATH_FLAGS_CREATE_GROUP, "languages");
  assert(dbGroup);
  for (i=0; i<9; i++){
    int v;

    v=AQFINTS_BankData_GetLanguagesAt(bankData, i);
    if (v<1)
      break;
    GWEN_DB_SetIntValue(dbGroup, 0, "language", v);
  }
  if (i==0)
    GWEN_DB_SetIntValue(dbGroup, GWEN_DB_FLAGS_OVERWRITE_VARS, "language", 1);

  dbGroup=GWEN_DB_GetGroup(dbSegment, GWEN_PATH_FLAGS_CREATE_GROUP, "versions");
  assert(dbGroup);
  for (i=0; i<9; i++){
    int v;

    v=AQFINTS_BankData_GetHbciVersionsAt(bankData, i);
    if (v<1)
      break;
    GWEN_DB_SetIntValue(dbGroup, 0, "version", v);
  }
  if (i==0)
    GWEN_DB_SetIntValue(dbGroup, GWEN_DB_FLAGS_OVERWRITE_VARS, "version", 300);


  return segment;
}



AQFINTS_SEGMENT *_mkSegBankAddr(AQFINTS_PARSER *parser, int hbciVersion, const AQFINTS_BPDADDR *bpdAddr)
{
  AQFINTS_SEGMENT *defSegment;
  AQFINTS_SEGMENT *segment;
  GWEN_DB_NODE *dbSegment;
  AQFINTS_BPDADDR_SERVICE_LIST *serviceList;
  const char *s;
  int i;

  defSegment=AQFINTS_Parser_FindSegmentHighestVersionForProto(parser, "HIKOM", hbciVersion);
  if (defSegment==NULL) {
    DBG_ERROR(0, "No matching definition segment found for HIKOM (proto=%d)", hbciVersion);
    return NULL;
  }

  segment=AQFINTS_Segment_new();
  AQFINTS_Segment_copy(segment, defSegment);

  dbSegment=GWEN_DB_Group_new("bpdAddr");
  AQFINTS_Segment_SetDbData(segment, dbSegment);

  i=AQFINTS_BpdAddr_GetCountry(bpdAddr);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "country", i);

  s=AQFINTS_BpdAddr_GetBankCode(bpdAddr);
  if (s)
    GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "bankCode", s);

  i=AQFINTS_BpdAddr_GetLanguage(bpdAddr);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "language", i);

  serviceList=AQFINTS_BpdAddr_GetServices(bpdAddr);
  if (serviceList) {
    AQFINTS_BPDADDR_SERVICE *service;

    service=AQFINTS_BpdAddrService_List_First(serviceList);
    while(service) {
      GWEN_DB_NODE *dbService;

      dbService=GWEN_DB_GetGroup(dbSegment, GWEN_PATH_FLAGS_CREATE_GROUP, "service");

      i=AQFINTS_BpdAddrService_GetType(service);
      GWEN_DB_SetIntValue(dbService, GWEN_DB_FLAGS_OVERWRITE_VARS, "type", i);

      s=AQFINTS_BpdAddrService_GetAddress(service);
      if (s)
        GWEN_DB_SetCharValue(dbService, GWEN_DB_FLAGS_OVERWRITE_VARS, "address", s);

      s=AQFINTS_BpdAddrService_GetSuffix(service);
      if (s)
        GWEN_DB_SetCharValue(dbService, GWEN_DB_FLAGS_OVERWRITE_VARS, "suffix", s);

      s=AQFINTS_BpdAddrService_GetFilter(service);
      if (s) {
        GWEN_DB_SetCharValue(dbService, GWEN_DB_FLAGS_OVERWRITE_VARS, "filter", s);

        i=AQFINTS_BpdAddrService_GetFilterVersion(service);
        GWEN_DB_SetIntValue(dbService, GWEN_DB_FLAGS_OVERWRITE_VARS, "filterVersion", i);
      }
      service=AQFINTS_BpdAddrService_List_Next(service);
    } /* while */
  }

  return segment;
}



AQFINTS_SEGMENT *_mkSegTanInfo(AQFINTS_PARSER *parser, int hbciVersion, const AQFINTS_TANINFO *tanInfo)
{
  AQFINTS_SEGMENT *defSegment;
  AQFINTS_SEGMENT *segment;
  GWEN_DB_NODE *dbSegment;
  AQFINTS_TANJOBINFO_LIST *tanJobList;
  const char *s;
  int i;

  defSegment=AQFINTS_Parser_FindSegmentHighestVersionForProto(parser, "HIPINS", hbciVersion);
  if (defSegment==NULL) {
    DBG_ERROR(0, "No matching definition segment found for HIPINS (proto=%d)", hbciVersion);
    return NULL;
  }

  segment=AQFINTS_Segment_new();
  AQFINTS_Segment_copy(segment, defSegment);

  dbSegment=GWEN_DB_Group_new("tanInfo");
  AQFINTS_Segment_SetDbData(segment, dbSegment);

  i=AQFINTS_TanInfo_GetJobsPerMsg(tanInfo);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "jobsPerMsg", i);

  i=AQFINTS_TanInfo_GetMinSigs(tanInfo);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "minSigs", i);

  i=AQFINTS_TanInfo_GetSecurityClass(tanInfo);
  if (i)
    GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "securityClass", i);

  i=AQFINTS_TanInfo_GetMinPinLen(tanInfo);
  if (i)
    GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "minPinLen", i);

  i=AQFINTS_TanInfo_GetMaxPinLen(tanInfo);
  if (i)
    GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "maxPinLen", i);

  i=AQFINTS_TanInfo_GetMaxTanLen(tanInfo);
  if (i)
    GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "maxTanLen", i);

  s=AQFINTS_TanInfo_GetUserIdText(tanInfo);
  if (s)
    GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "userIdText", s);

  s=AQFINTS_TanInfo_GetCustomerIdText(tanInfo);
  if (s)
    GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "customerIdText", s);

  tanJobList=AQFINTS_TanInfo_GetTanJobInfoList(tanInfo);
  if (tanJobList) {
    AQFINTS_TANJOBINFO *tanJob;

    tanJob=AQFINTS_TanJobInfo_List_First(tanJobList);
    while(tanJob) {
      GWEN_DB_NODE *dbTanJob;

      dbTanJob=GWEN_DB_GetGroup(dbSegment, GWEN_PATH_FLAGS_CREATE_GROUP, "job");

      s=AQFINTS_TanJobInfo_GetCode(tanJob);
      if (s) {
        GWEN_DB_SetCharValue(dbTanJob, GWEN_DB_FLAGS_OVERWRITE_VARS, "code", s);

        GWEN_DB_SetCharValue(dbTanJob, GWEN_DB_FLAGS_OVERWRITE_VARS, "needTan",
                             (AQFINTS_TanJobInfo_GetFlags(tanJob) & AQFINTS_TANJOBINFO_FLAGS_NEEDTAN)?"J":"N");
      }

      tanJob=AQFINTS_TanJobInfo_List_Next(tanJob);
    } /* while */
  }

  return segment;
}






