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

#include "upd_read.h"

#include "aqfints/aqfints.h"
#include "servicelayer/upd/updjob.h"
#include "servicelayer/upd/accountdata.h"
#include "servicelayer/upd/userdata.h"

#include <gwenhywfar/debug.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static AQFINTS_UPDJOB *readUpdJob(GWEN_DB_NODE *db);
static void readAccountDataLimit(AQFINTS_ACCOUNTDATA *accountData, GWEN_DB_NODE *db);
static AQFINTS_LIMIT_TYPE limitTypeFromChar(const char *s);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



AQFINTS_USERDATA_LIST *AQFINTS_Upd_SampleUpdFromSegmentList(AQFINTS_SEGMENT_LIST *segmentList,
                                                            int removeFromSegList)
{
  AQFINTS_SEGMENT *segment;
  AQFINTS_USERDATA_LIST *userDataList;

  userDataList=AQFINTS_UserData_List_new();

  segment=AQFINTS_Segment_List_First(segmentList);
  while (segment) {
    AQFINTS_SEGMENT *nextSegment;
    const char *sCode;
    int doRemoveSegment=0;

    nextSegment=AQFINTS_Segment_List_Next(segment);

    sCode=AQFINTS_Segment_GetCode(segment);
    if (sCode && *sCode && strcasecmp(sCode, "HIUPA")==0) { /* read userData */
      GWEN_DB_NODE *db;

      db=AQFINTS_Segment_GetDbData(segment);
      if (db) {
        AQFINTS_USERDATA *userData;

        userData=AQFINTS_Upd_ReadUserData(db);
        if (userData) {
          DBG_ERROR(AQFINTS_LOGDOMAIN, "Adding user data");
          AQFINTS_UserData_List_Add(userData, userDataList);
          if (removeFromSegList)
            doRemoveSegment=1;
        }
      }
    }
    else if (sCode && *sCode && strcasecmp(sCode, "HIUPD")==0) { /* read accountData */
      GWEN_DB_NODE *db;

      db=AQFINTS_Segment_GetDbData(segment);
      if (db) {
        AQFINTS_ACCOUNTDATA *accountData;

        accountData=AQFINTS_Upd_ReadAccountData(db);
        if (accountData) {
          AQFINTS_USERDATA *userData;

          userData=AQFINTS_UserData_List_Last(userDataList);
          if (userData) {
            DBG_ERROR(AQFINTS_LOGDOMAIN, "Adding account data");
            AQFINTS_UserData_AddAccountData(userData, accountData);
          }
          else {
            DBG_ERROR(AQFINTS_LOGDOMAIN, "Got account data wihtout prior userData, ignoring accountData");
            AQFINTS_AccountData_free(accountData);
          }
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

  if (AQFINTS_UserData_List_GetCount(userDataList)==0) {
    AQFINTS_UserData_List_free(userDataList);
    return NULL;
  }

  return userDataList;
}



AQFINTS_USERDATA *AQFINTS_Upd_ReadUserData(GWEN_DB_NODE *db)
{
  AQFINTS_USERDATA *userData;
  const char *s;
  int i;

  userData=AQFINTS_UserData_new();

  s=GWEN_DB_GetCharValue(db, "userId", 0, NULL);
  if (s && *s)
    AQFINTS_UserData_SetUserId(userData, s);

  i=GWEN_DB_GetIntValue(db, "version", 0, 0);
  AQFINTS_UserData_SetVersion(userData, i);

  i=GWEN_DB_GetIntValue(db, "ignoreUpdJobs", 0, 0);
  AQFINTS_UserData_SetIgnoreUpdJobs(userData, i);

  s=GWEN_DB_GetCharValue(db, "userName", 0, NULL);
  if (s && *s)
    AQFINTS_UserData_SetUserName(userData, s);

  s=GWEN_DB_GetCharValue(db, "genericExt", 0, NULL);
  if (s && *s)
    AQFINTS_UserData_SetGenericExtension(userData, s);

  return userData;
}



AQFINTS_ACCOUNTDATA *AQFINTS_Upd_ReadAccountData(GWEN_DB_NODE *db)
{
  AQFINTS_ACCOUNTDATA *accountData;
  const char *s;
  int i;
  GWEN_DB_NODE *dbGroup;

  accountData=AQFINTS_AccountData_new();

  s=GWEN_DB_GetCharValue(db, "accountId", 0, NULL);
  if (s && *s)
    AQFINTS_AccountData_SetAccountNumber(accountData, s);

  i=GWEN_DB_GetIntValue(db, "country", 0, 280);
  AQFINTS_AccountData_SetCountry(accountData, i);

  s=GWEN_DB_GetCharValue(db, "bankCode", 0, NULL);
  if (s && *s)
    AQFINTS_AccountData_SetBankCode(accountData, s);

  s=GWEN_DB_GetCharValue(db, "iban", 0, NULL);
  if (s && *s)
    AQFINTS_AccountData_SetIban(accountData, s);

  s=GWEN_DB_GetCharValue(db, "customer", 0, NULL);
  if (s && *s)
    AQFINTS_AccountData_SetCustomerId(accountData, s);

  i=GWEN_DB_GetIntValue(db, "type", 0, 280);
  AQFINTS_AccountData_SetAccountType(accountData, i);

  s=GWEN_DB_GetCharValue(db, "currency", 0, NULL);
  if (s && *s)
    AQFINTS_AccountData_SetCurrency(accountData, s);

  s=GWEN_DB_GetCharValue(db, "name1", 0, NULL);
  if (s && *s)
    AQFINTS_AccountData_SetName1(accountData, s);

  s=GWEN_DB_GetCharValue(db, "name2", 0, NULL);
  if (s && *s)
    AQFINTS_AccountData_SetName2(accountData, s);

  s=GWEN_DB_GetCharValue(db, "accountName", 0, NULL);
  if (s && *s)
    AQFINTS_AccountData_SetAccountName(accountData, s);

  s=GWEN_DB_GetCharValue(db, "genericExt", 0, NULL);
  if (s && *s)
    AQFINTS_AccountData_SetGenericExtension(accountData, s);

  readAccountDataLimit(accountData, db);

  dbGroup=GWEN_DB_FindFirstGroup(db, "updJob");
  while (dbGroup) {
    AQFINTS_UPDJOB *j;

    j=readUpdJob(dbGroup);
    assert(j);
    AQFINTS_AccountData_AddUpdJob(accountData, j);
    dbGroup=GWEN_DB_FindNextGroup(dbGroup, "updJob");
  }

  return accountData;
}



void readAccountDataLimit(AQFINTS_ACCOUNTDATA *accountData, GWEN_DB_NODE *db)
{
  GWEN_DB_NODE *dbLimit;

  dbLimit=GWEN_DB_FindFirstGroup(db, "limit");
  if (dbLimit) {
    GWEN_DB_NODE *dbBtg;
    const char *s;
    int i;

    s=GWEN_DB_GetCharValue(dbLimit, "type", 0, NULL);
    if (s && *s) {
      AQFINTS_LIMIT_TYPE limitType;

      limitType=limitTypeFromChar(s);
      if (limitType>AQFINTS_LimitType_None)
        AQFINTS_AccountData_SetLimitType(accountData, limitType);
    }

    dbBtg=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "btg");
    if (dbBtg) {
      s=GWEN_DB_GetCharValue(dbBtg, "value", 0, NULL);
      if (s && *s) {
        const char *sCurrency;
        AB_VALUE *val;

        sCurrency=GWEN_DB_GetCharValue(dbBtg, "currency", 0, NULL);
        val=AB_Value_fromString(s);
        assert(val);
        if (sCurrency && *sCurrency)
          AB_Value_SetCurrency(val, sCurrency);
        AQFINTS_AccountData_SetLimitValue(accountData, val);
        AB_Value_free(val);
      }

      i=GWEN_DB_GetIntValue(dbLimit, "days", 0, 0);
      AQFINTS_AccountData_SetLimitDays(accountData, i);
    }
  }
}


AQFINTS_UPDJOB *readUpdJob(GWEN_DB_NODE *db)
{
  AQFINTS_UPDJOB *j;
  const char *s;
  int i;

  j=AQFINTS_UpdJob_new();
  s=GWEN_DB_GetCharValue(db, "job", 0, NULL);
  if (s && *s)
    AQFINTS_UpdJob_SetCode(j, s);

  i=GWEN_DB_GetIntValue(db, "minsign", 0, 0);
  AQFINTS_UpdJob_SetMinSigs(j, i);

  s=GWEN_DB_GetCharValue(db, "limitType", 0, NULL);
  if (s && *s) {
    AQFINTS_LIMIT_TYPE limitType;

    limitType=limitTypeFromChar(s);
    if (limitType>AQFINTS_LimitType_None)
      AQFINTS_UpdJob_SetLimitType(j, limitType);
  }

  s=GWEN_DB_GetCharValue(db, "limitValue", 0, NULL);
  if (s && *s) {
    const char *sCurrency;
    AB_VALUE *val;

    sCurrency=GWEN_DB_GetCharValue(db, "limitCurrency", 0, NULL);
    val=AB_Value_fromString(s);
    assert(val);
    if (sCurrency && *sCurrency) {
      AB_Value_SetCurrency(val, sCurrency);
      AQFINTS_UpdJob_SetLimitCurrency(j, sCurrency);
    }
    AQFINTS_UpdJob_SetLimitValue(j, val);
    AB_Value_free(val);
  }

  return j;
}



AQFINTS_LIMIT_TYPE limitTypeFromChar(const char *s)
{
  if (strcasecmp(s, "E")==0)
    return AQFINTS_LimitType_JobLimit;
  else if (strcasecmp(s, "T")==0)
    return AQFINTS_LimitType_DayLimit;
  else if (strcasecmp(s, "W")==0)
    return AQFINTS_LimitType_WeekLimit;
  else if (strcasecmp(s, "M")==0)
    return AQFINTS_LimitType_MonthLimit;
  else if (strcasecmp(s, "Z")==0)
    return AQFINTS_LimitType_TimeLimit;
  else {
    DBG_ERROR(0, "Unknown limit type \"%s\"", s);
    return AQFINTS_LimitType_Unknown;
  }
}



