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

#include "job_commit.h"
#include "job_commit_key.h"
#include "job_commit_bpd.h"
#include "job_commit_account.h"
#include "job_commit_key.h"
#include "aqhbci/banking/user_l.h"
#include "aqhbci/banking/account_l.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _commitSystemData(AH_JOB *j, int doLock);
static void _readSegmentResult(AH_JOB *j, GWEN_DB_NODE *dbRd);
static void _readSecurityProfile(AH_JOB *j, GWEN_DB_NODE *dbRd);
static void _readBankMessage(AH_JOB *j, GWEN_DB_NODE *dbRd);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AH_Job_CommitSystemData(AH_JOB *j, int doLock)
{
  AB_USER *user;
  AB_PROVIDER *pro;
  int rv, rv2;

  user=AH_Job_GetUser(j);

  pro=AH_Job_GetProvider(j);
  assert(pro);

  /* lock user */
  if (doLock) {
    rv=AB_Provider_BeginExclUseUser(pro, user);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  /* commit data */
  rv2=_commitSystemData(j, doLock);

  if (doLock) {
    /* unlock user */
    rv=AB_Provider_EndExclUseUser(pro, user, 0);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AB_Provider_EndExclUseUser(pro, user, 1); /* abandon */
      return rv;
    }
  }

  return rv2;
}



void AH_Job_ReadAccountDataSeg(AB_ACCOUNT *acc, GWEN_DB_NODE *dbAccountData)
{
  int t;

  assert(acc);

  AB_Account_SetBankCode(acc, GWEN_DB_GetCharValue(dbAccountData, "bankCode", 0, 0));
  AB_Account_SetAccountNumber(acc, GWEN_DB_GetCharValue(dbAccountData, "accountId", 0, 0));
  AB_Account_SetIban(acc, GWEN_DB_GetCharValue(dbAccountData, "iban", 0, 0));
  AB_Account_SetAccountName(acc, GWEN_DB_GetCharValue(dbAccountData, "account/name", 0, 0));
  AB_Account_SetSubAccountId(acc, GWEN_DB_GetCharValue(dbAccountData, "accountsubid", 0, 0));
  AB_Account_SetOwnerName(acc, GWEN_DB_GetCharValue(dbAccountData, "name1", 0, 0));

  if (GWEN_DB_GetIntValue(dbAccountData, "head/version", 0, 1)>=4)
    /* KTV in version 2 available */
    AH_Account_AddFlags(acc, AH_BANK_FLAGS_KTV2);
  else
    AH_Account_SubFlags(acc, AH_BANK_FLAGS_KTV2);

  /* account type (from FinTS_3.0_Formals) */
  t=GWEN_DB_GetIntValue(dbAccountData, "type", 0, 1);
  if (t>=1 && t<=9)          /* Kontokorrent-/Girokonto */
    AB_Account_SetAccountType(acc, AB_AccountType_Bank);
  else if (t>=10 && t<=19)   /* Sparkonto */
    AB_Account_SetAccountType(acc, AB_AccountType_Savings);
  else if (t>=20 && t<=29)   /* Festgeldkonto/Termineinlagen */
    AB_Account_SetAccountType(acc, AB_AccountType_MoneyMarket);
  else if (t>=30 && t<=39)   /* Wertpapierdepot */
    AB_Account_SetAccountType(acc, AB_AccountType_Investment);
  else if (t>=40 && t<=49)   /* Kredit-/Darlehenskonto */
    AB_Account_SetAccountType(acc, AB_AccountType_Credit);
  else if (t>=50 && t<=59)   /* Kreditkartenkonto */
    AB_Account_SetAccountType(acc, AB_AccountType_CreditCard);
  else if (t>=60 && t<=69)   /* Fonds-Depot bei einer Kapitalanlagengesellschaft */
    AB_Account_SetAccountType(acc, AB_AccountType_Investment);
  else if (t>=70 && t<=79)   /* Bausparvertrag */
    AB_Account_SetAccountType(acc, AB_AccountType_Savings);
  else if (t>=80 && t<=89)   /* Versicherungsvertrag */
    AB_Account_SetAccountType(acc, AB_AccountType_Savings);
  else if (t>=90 && t<=99)   /* sonstige */
    AB_Account_SetAccountType(acc, AB_AccountType_Unspecified);
  else
    AB_Account_SetAccountType(acc, AB_AccountType_Unspecified);
}



int _commitSystemData(AH_JOB *j, int doLock)
{
  AB_USER *user;
  GWEN_DB_NODE *dbCurr;
  int rv;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Committing data");
  user=AH_Job_GetUser(j);
  /* GWEN_DB_Dump(j->jobResponses, 2); */

  dbCurr=GWEN_DB_GetFirstGroup(AH_Job_GetResponses(j));
  while (dbCurr) {
    GWEN_DB_NODE *dbRd;

    dbRd=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
    if (dbRd) {
      dbRd=GWEN_DB_GetFirstGroup(dbRd);
    }
    if (dbRd) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Checking group \"%s\"", GWEN_DB_GroupName(dbRd));

      if (strcasecmp(GWEN_DB_GroupName(dbRd), "SegResult")==0) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Reading segment result");
        _readSegmentResult(j, dbRd);
      }
      else if (strcasecmp(GWEN_DB_GroupName(dbRd), "GetKeyResponse")==0) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Reading key response");
        AH_Job_Commit_Key(j, dbRd);
      }

      else if (strcasecmp(GWEN_DB_GroupName(dbRd), "SecurityMethods")==0) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Reading security profiles");
        _readSecurityProfile(j, dbRd);
      }

      else if (strcasecmp(GWEN_DB_GroupName(dbRd), "UserData")==0) {
        /* UserData found */
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Found UserData");
        AH_User_SetUpdVersion(user, GWEN_DB_GetIntValue(dbRd, "version", 0, 0));
      }

      else if (strcasecmp(GWEN_DB_GroupName(dbRd), "BankMsg")==0) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Reading bank message");
        _readBankMessage(j, dbRd);
      } /* if bank msg */


    } /* if response data found */
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */

  /* try to extract bank parameter data */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Committing BPD");
  rv=AH_Job_Commit_Bpd(j);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* try to extract accounts */
  if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_IGNOREACCOUNTS) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Ignoring possibly received accounts");
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Committing accounts");
    rv=AH_Job_Commit_Accounts(j);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Finished.");
  return 0;
}



void _readSegmentResult(AH_JOB *j, GWEN_DB_NODE *dbRd)
{
  AB_USER *user;
  GWEN_DB_NODE *dbRes;

  user=AH_Job_GetUser(j);

  dbRes=GWEN_DB_GetFirstGroup(dbRd);
  while (dbRes) {
    if (strcasecmp(GWEN_DB_GroupName(dbRes), "result")==0) {
      int code;
      const char *text;

      code=GWEN_DB_GetIntValue(dbRes, "resultcode", 0, 0);
      text=GWEN_DB_GetCharValue(dbRes, "text", 0, 0);
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Segment result: %d (%s)", code, text?text:"<none>");
      if (code==3920) {
        int i;

        AH_User_ClearTanMethodList(user);
        for (i=0; ; i++) {
          int j;

          j=GWEN_DB_GetIntValue(dbRes, "param", i, 0);
          if (j==0)
            break;
          DBG_NOTICE(AQHBCI_LOGDOMAIN, "Adding allowed TAN method %d", j);
          AH_User_AddTanMethod(user, j);
        } /* for */
        if (i==0) {
          /* add single step if empty list */
          DBG_INFO(AQHBCI_LOGDOMAIN, "No allowed TAN method reported, assuming 999");
          AH_User_AddTanMethod(user, 999);
        }
      }
    } /* if result */
    dbRes=GWEN_DB_GetNextGroup(dbRes);
  } /* while */
}



void _readSecurityProfile(AH_JOB *j, GWEN_DB_NODE *dbRd)
{
  GWEN_DB_NODE *dbT;

  dbT=GWEN_DB_FindFirstGroup(dbRd, "SecProfile");
  while (dbT) {
    const char *code;
    int version;

    code=GWEN_DB_GetCharValue(dbT, "code", 0, NULL);
    version=GWEN_DB_GetIntValue(dbT, "version", 0, -1);
    if (code && (version>0)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Bank supports mode %s %d", code, version);
      /* TODO: store possible modes */
    }
    dbT=GWEN_DB_FindNextGroup(dbT, "SecProfile");
  } /* while */
}



void _readBankMessage(AH_JOB *j, GWEN_DB_NODE *dbRd)
{
  AB_USER *user;
  AH_HBCI *hbci;
  AB_MESSAGE_LIST *messageList;
  const char *subject;
  const char *text;

  user=AH_Job_GetUser(j);
  hbci=AH_Job_GetHbci(j);
  messageList=AH_Job_GetMessages(j);

  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice, I18N("Bank message received"));

  subject=GWEN_DB_GetCharValue(dbRd, "subject", 0, "(Kein Betreff)");
  text=GWEN_DB_GetCharValue(dbRd, "text", 0, 0);
  if (subject && text) {
    AB_MESSAGE *amsg;
    GWEN_TIME *ti;

    ti=GWEN_CurrentTime();
    amsg=AB_Message_new();
    AB_Message_SetSource(amsg, AB_Message_SourceBank);
    AB_Message_SetSubject(amsg, subject);
    AB_Message_SetText(amsg, text);
    AB_Message_SetDateReceived(amsg, ti);
    GWEN_Time_free(ti);
    AB_Message_SetUserId(amsg, AB_User_GetUniqueId(user));
    AB_Message_List_Add(amsg, messageList);

    if (1) {
      GWEN_DB_NODE *dbTmp;

      /* save message, later this will no longer be necessary */
      dbTmp=GWEN_DB_Group_new("bank message");
      GWEN_DB_SetCharValue(dbTmp, GWEN_DB_FLAGS_OVERWRITE_VARS, "subject", subject);
      GWEN_DB_SetCharValue(dbTmp, GWEN_DB_FLAGS_OVERWRITE_VARS, "text", text);
      if (AH_HBCI_SaveMessage(hbci, user, dbTmp)) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not save this message:");
        GWEN_DB_Dump(dbTmp, 2);
      }
      GWEN_DB_Group_free(dbTmp);
    }
  } /* if subject and text given */
}


