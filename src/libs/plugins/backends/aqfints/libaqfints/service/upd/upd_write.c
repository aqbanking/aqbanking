/***************************************************************************
 begin       : Sun Jul 28 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "upd_write.h"

#include "libaqfints/aqfints.h"
#include "libaqfints/service/upd/updjob.h"
#include "libaqfints/service/upd/accountdata.h"
#include "libaqfints/service/upd/userdata.h"

#include <gwenhywfar/debug.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void _writeUserData(const AQFINTS_USERDATA *userData, GWEN_DB_NODE *db);
static void _writeAccountData(const AQFINTS_ACCOUNTDATA *accountData, GWEN_DB_NODE *db);
static void _writeUpdJob(const AQFINTS_UPDJOB *updJob, GWEN_DB_NODE *db);
static const char *_limitTypeToChar(AQFINTS_LIMIT_TYPE t);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AQFINTS_Upd_Write(const AQFINTS_USERDATA *userData,
                      AQFINTS_PARSER *parser,
                      int refSegNum,
                      AQFINTS_SEGMENT_LIST *segmentList)
{
  AQFINTS_SEGMENT *segment;
  GWEN_DB_NODE *dbSegment;
  AQFINTS_ACCOUNTDATA_LIST *accountDataList;

  segment=AQFINTS_Parser_CreateSegmentByCode(parser, "HIUPA", 0);
  if (segment==NULL) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "Segment definition not found");
    return GWEN_ERROR_GENERIC;
  }

  dbSegment=AQFINTS_Segment_GetDbData(segment);
  _writeUserData(userData, dbSegment);
  AQFINTS_Segment_SetRefSegmentNumber(segment, refSegNum);
  AQFINTS_Segment_List_Add(segment, segmentList);

  accountDataList=AQFINTS_UserData_GetAccountDataList(userData);
  if (accountDataList && AQFINTS_AccountData_List_GetCount(accountDataList)) {
    AQFINTS_ACCOUNTDATA *accountData;

    accountData=AQFINTS_AccountData_List_First(accountDataList);
    while (accountData) {
      segment=AQFINTS_Parser_CreateSegmentByCode(parser, "HIUPD", 0);
      if (segment==NULL) {
        DBG_INFO(AQFINTS_LOGDOMAIN, "Segment definition not found");
        return GWEN_ERROR_GENERIC;
      }
      dbSegment=AQFINTS_Segment_GetDbData(segment);
      _writeAccountData(accountData, dbSegment);
      AQFINTS_Segment_SetRefSegmentNumber(segment, refSegNum);
      AQFINTS_Segment_List_Add(segment, segmentList);
      accountData=AQFINTS_AccountData_List_Next(accountData);
    }
  }

  return 0;
}






void _writeUserData(const AQFINTS_USERDATA *userData, GWEN_DB_NODE *db)
{
  const char *s;
  int i;

  s=AQFINTS_UserData_GetUserId(userData);
  if (s && *s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "userId", s);

  i=AQFINTS_UserData_GetVersion(userData);
  if (i>=0)
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "version", i);

  i=AQFINTS_UserData_GetIgnoreUpdJobs(userData);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "ignoreUPDJobs", i);

  s=AQFINTS_UserData_GetUserName(userData);
  if (s && *s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "userName", s);

  s=AQFINTS_UserData_GetGenericExtension(userData);
  if (s && *s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "genericExt", s);
}



void _writeAccountData(const AQFINTS_ACCOUNTDATA *accountData, GWEN_DB_NODE *db)
{
  const char *s;
  int i;
  AQFINTS_UPDJOB_LIST *updJobList;

  s=AQFINTS_AccountData_GetAccountNumber(accountData);
  if (s && *s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "accountId", s);

  s=AQFINTS_AccountData_GetAccountSuffix(accountData);
  if (s && *s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "accountSubId", s);

  s=AQFINTS_AccountData_GetBankCode(accountData);
  if (s && *s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "bankCode", s);

  i=AQFINTS_AccountData_GetCountry(accountData);
  if (i>=0)
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "country", i);

  s=AQFINTS_AccountData_GetIban(accountData);
  if (s && *s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "iban", s);

  s=AQFINTS_AccountData_GetAccountName(accountData);
  if (s && *s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "accountName", s);

  s=AQFINTS_AccountData_GetCustomerId(accountData);
  if (s && *s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "customer", s);

  i=AQFINTS_AccountData_GetAccountType(accountData);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "type", i);

  s=AQFINTS_AccountData_GetCurrency(accountData);
  if (s && *s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "currency", s);

  s=AQFINTS_AccountData_GetName1(accountData);
  if (s && *s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "name1", s);

  s=AQFINTS_AccountData_GetName2(accountData);
  if (s && *s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "name2", s);


  s=_limitTypeToChar(AQFINTS_AccountData_GetLimitType(accountData));
  if (s && *s) {
    const AB_VALUE *v;

    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "limitType", s);

    v=AQFINTS_AccountData_GetLimitValue(accountData);
    if (v) {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 32, 0, 1);
      AB_Value_toHbciString(v, tbuf);
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "limitValue", GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
    }

    i=AQFINTS_AccountData_GetLimitDays(accountData);
    if (i>0)
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "limitDays", i);
  }

  updJobList=AQFINTS_AccountData_GetUpdJobs(accountData);
  if (updJobList && AQFINTS_UpdJob_List_GetCount(updJobList)) {
    AQFINTS_UPDJOB *updJob;

    updJob=AQFINTS_UpdJob_List_First(updJobList);
    while (updJob) {
      GWEN_DB_NODE *dbUpdJob;

      dbUpdJob=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_CREATE_GROUP, "updjob");
      _writeUpdJob(updJob, dbUpdJob);
      updJob=AQFINTS_UpdJob_List_Next(updJob);
    }
  }

  s=AQFINTS_AccountData_GetGenericExtension(accountData);
  if (s && *s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "genericExt", s);

}



void _writeUpdJob(const AQFINTS_UPDJOB *updJob, GWEN_DB_NODE *db)
{
  const char *s;
  int i;

  s=AQFINTS_UpdJob_GetCode(updJob);
  if (s && *s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "job", s);

  i=AQFINTS_UpdJob_GetMinSigs(updJob);
  if (i>0)
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "minsign", i);

  s=_limitTypeToChar(AQFINTS_UpdJob_GetLimitType(updJob));
  if (s && *s) {
    const AB_VALUE *v;

    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "limitType", s);

    v=AQFINTS_UpdJob_GetLimitValue(updJob);
    if (v) {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 32, 0, 1);
      AB_Value_toHbciString(v, tbuf);
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "limitValue", GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
    }

    s=AQFINTS_UpdJob_GetLimitCurrency(updJob);
    if (s && *s)
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "limitCurrency", s);

    i=AQFINTS_UpdJob_GetLimitDays(updJob);
    if (i>0)
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "limitDays", i);
  }
}



const char *_limitTypeToChar(AQFINTS_LIMIT_TYPE t)
{
  switch (t) {
  case AQFINTS_LimitType_JobLimit:
    return "E";
  case AQFINTS_LimitType_DayLimit:
    return "T";
  case AQFINTS_LimitType_WeekLimit:
    return "W";
  case AQFINTS_LimitType_MonthLimit:
    return "M";
  case AQFINTS_LimitType_TimeLimit:
    return "Z";
  default:
    break;
  }

  return NULL;
}




