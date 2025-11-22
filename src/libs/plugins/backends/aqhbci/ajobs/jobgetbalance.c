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


#include "jobgetbalance_l.h"
#include "aqhbci/aqhbci_l.h"
#include "accountjob_l.h"
#include "aqhbci/joblayer/job_l.h"
#include "aqhbci/joblayer/job_crypt.h"
#include "aqhbci/banking/user_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/syncio_memory.h>

#include <assert.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _jobApi_ProcessBankAccount(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int _jobApi_ProcessInvestmentAccount(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);

static AB_BALANCE *_readBalance(GWEN_DB_NODE *dbBalance);
static int _readSecurities(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx, const char *docType, int noted, GWEN_BUFFER *buf);

static AB_VALUE *_readAmountFromResponseDb(GWEN_DB_NODE *dbBalance);
static GWEN_DATE *_readDateFromResponseDb(GWEN_DB_NODE *dbBalance);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



AH_JOB *AH_Job_GetBalance_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account)
{
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;
  int useRegularAccountJob=0;
  int useCreditCardJob=0;
  int useInvestmentJob=0;
  GWEN_DB_NODE *updgroup;

  //Check if we should use DKKKS
  updgroup=AH_User_GetUpdForAccount(u, account);
  if (updgroup) {
    GWEN_DB_NODE *n;

    n=GWEN_DB_GetFirstGroup(updgroup);
    while (n) {
      if (strcasecmp(GWEN_DB_GetCharValue(n, "job", 0, ""), "HKSAL")==0) {
        useRegularAccountJob = 1;
        break;
      }

      if (strcasecmp(GWEN_DB_GetCharValue(n, "job", 0, ""), "DKKKS")==0) {
        useCreditCardJob = 1;
        break;
      }

      if (strcasecmp(GWEN_DB_GetCharValue(n, "job", 0, ""), "HKKKS")==0) {
        useCreditCardJob = 2;
        break;
      }
      
      if (strcasecmp(GWEN_DB_GetCharValue(n, "job", 0, ""), "HKWPD")==0) {
        useInvestmentJob = 1;
        break;
      }
      n=GWEN_DB_GetNextGroup(n);
    } /* while */
  } /* if updgroup for the given account found */

  if (useCreditCardJob == 1)
    j=AH_AccountJob_new("JobGetBalanceCreditCard", pro, u, account);
  else if (useCreditCardJob == 2)
    j=AH_AccountJob_new("JobGetBalanceCreditCard2", pro, u, account);
  else if (useInvestmentJob)
    j=AH_AccountJob_new("JobGetBalanceInvestment", pro, u, account);
  else if (useRegularAccountJob)
    j=AH_AccountJob_new("JobGetBalance", pro, u, account);
  else {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "No UPD available for JobGetBalance");
    return 0;
  }

  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandGetBalance);

  /* overwrite some virtual functions */
  if (useInvestmentJob)
    AH_Job_SetProcessFn(j, _jobApi_ProcessInvestmentAccount);
  else
    AH_Job_SetProcessFn(j, _jobApi_ProcessBankAccount);
  AH_Job_SetGetLimitsFn(j, AH_Job_GetLimits_EmptyLimits);
  AH_Job_SetHandleCommandFn(j, AH_Job_HandleCommand_Accept);
  AH_Job_SetHandleResultsFn(j, AH_Job_HandleResults_Empty);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);
  if (useCreditCardJob || useInvestmentJob)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "accountNumber", AB_Account_GetAccountNumber(account));
  else {
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "allAccounts", "N");
    if (AH_Job_GetSegmentVersion(j)<7) {
      const char *s;

      /* HKSAL ver 7 and higher use IBAN and BIC, older segments use accountId and bankCode  */
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Adding national account specs for HKSAL version <7");
      s=AB_Account_GetAccountNumber(account);
      if (s && *s)
	GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "accountId", s);
      s=AB_Account_GetSubAccountId(account);
      if (s && *s)
	GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "accountSubId", s);
      s=AB_Account_GetBankCode(account);
      if (s && *s)
	GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "bankCode", s);
      GWEN_DB_SetIntValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "country", 280);
    }
  }

  return j;
}



AB_BALANCE *_readBalance(GWEN_DB_NODE *dbBalance)
{
  AB_BALANCE *bal;
  AB_VALUE *value;
  GWEN_DATE *dt;

  bal=AB_Balance_new();

  value=_readAmountFromResponseDb(dbBalance);
  AB_Balance_SetValue(bal, value);
  AB_Value_free(value);

  dt=_readDateFromResponseDb(dbBalance);
  AB_Balance_SetDate(bal, dt);
  GWEN_Date_free(dt);

  return bal;
}



AB_VALUE *_readAmountFromResponseDb(GWEN_DB_NODE *dbBalance)
{
  AB_VALUE *value;
  const char *p;

  /* get value */
  value=AB_Value_fromDb(dbBalance);
  if (!value) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error parsing value from DB");
    return NULL;
  }

  /* get isCredit */
  p=GWEN_DB_GetCharValue(dbBalance, "debitMark", 0, 0);
  if (p) {
    if (strcasecmp(p, "D")==0 || strcasecmp(p, "RC")==0) {
      AB_Value_Negate(value);
    }
    else if (strcasecmp(p, "C")==0 || strcasecmp(p, "RD")==0) {
      /* nothing to do */
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad debit mark \"%s\"", p);
      AB_Value_free(value);
      return NULL;
    }
  }

  return value;
}



GWEN_DATE *_readDateFromResponseDb(GWEN_DB_NODE *dbBalance)
{
  GWEN_DATE *dt=NULL;
  const char *p;

  /* read date */
  p=GWEN_DB_GetCharValue(dbBalance, "date", 0, 0);
  if (p) {
    dt=GWEN_Date_fromStringWithTemplate(p, "YYYYMMDD");
    if (dt==NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad date \"%s\"", p);
      return NULL;
    }
  }
  else {
    DBG_WARN(AQHBCI_LOGDOMAIN, "No date, using current date");
    dt=GWEN_Date_CurrentDate();
    assert(dt);
  }

  return dt;
}



int _jobApi_ProcessBankAccount(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobGetBalance");

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  /* search for "Balance" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while (dbCurr) {
    GWEN_DB_NODE *dbBalance;

    rv=AH_Job_CheckEncryption(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (encryption)");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }
    rv=AH_Job_CheckSignature(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (signature)");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }

    dbBalance=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data/balance");
    if (!dbBalance)
      dbBalance=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data/balancecreditcard");
    if (!dbBalance)
      dbBalance=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data/balancecreditcard2");
    if (dbBalance) {
      GWEN_DB_NODE *dbT;
      AB_ACCOUNT *a;
      AB_IMEXPORTER_ACCOUNTINFO *ai;

      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Got a balance");
      if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevel_Debug)
        GWEN_DB_Dump(dbBalance, 2);

      a=AH_AccountJob_GetAccount(j);
      assert(a);
      ai=AB_Provider_GetOrAddAccountInfoForAccount(ctx, a);

      /* read booked balance */
      dbT=GWEN_DB_GetGroup(dbBalance, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "booked");
      if (dbT) {
        AB_BALANCE *bal;

        bal=_readBalance(dbT);
        if (bal) {
          AB_Balance_SetType(bal, AB_Balance_TypeBooked);
          AB_ImExporterAccountInfo_AddBalance(ai, bal);
        }
      }

      /* read noted balance */
      dbT=GWEN_DB_GetGroup(dbBalance, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "noted");
      if (dbT) {
        AB_BALANCE *bal;

        bal=_readBalance(dbT);
        if (bal) {
          AB_Balance_SetType(bal, AB_Balance_TypeNoted);
          AB_ImExporterAccountInfo_AddBalance(ai, bal);
        }
      }

#if 0
      /* read credit Line */
      dbT=GWEN_DB_GetGroup(dbBalance, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "creditLine");
      if (dbT) {
        AB_VALUE *v;

        v=AB_Value_fromDb(dbT);
        if (!v) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Error parsing value from DB");
        }
        else {
          AB_AccountStatus_SetBankLine(acst, v);
        }
        AB_Value_free(v);
      }
#endif

      break; /* break loop, we found the balance */
    } /* if "Balance" */

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  return 0;
}



int _jobApi_ProcessInvestmentAccount(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  GWEN_BUFFER *tbooked;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobGetBalance");

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  tbooked=GWEN_Buffer_new(0, 8192, 0, 1);

  /* search for "Balance" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while (dbCurr) {
    GWEN_DB_NODE *dbBalance;

    //GWEN_DB_Dump(dbCurr, 8);

    rv=AH_Job_CheckEncryption(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (encryption)");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }
    rv=AH_Job_CheckSignature(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (signature)");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }

    dbBalance=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data/BalanceInvestment");

    if (dbBalance) {
      AB_ACCOUNT *a;
      AB_IMEXPORTER_ACCOUNTINFO *ai;
      const void *p;
      unsigned int bs;

      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Got a balance");
      if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevel_Debug)
        GWEN_DB_Dump(dbBalance, 2);

      a=AH_AccountJob_GetAccount(j);
      assert(a);
      ai=AB_Provider_GetOrAddAccountInfoForAccount(ctx, a);

      p=GWEN_DB_GetBinValue(dbBalance, "booked", 0, 0, 0, &bs);
      if (p && bs)
        GWEN_Buffer_AppendBytes(tbooked, p, bs);

      break; /* break loop, we found the balance */
    } /* if(dbBalance) */

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while(dbCurr) */

  GWEN_Buffer_Rewind(tbooked);
  /* now the buffers contain data to be parsed by DBIOs */

  /* read received securities */
  if (GWEN_Buffer_GetUsedBytes(tbooked)) {

    if (_readSecurities(j, ctx, "mt535", 0, tbooked)) {
      GWEN_Buffer_free(tbooked);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error parsing received securities");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return -1;
    }
  }

  if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Debug) {
    GWEN_DB_NODE *gn;
    AB_SECURITY *stmp;

    DBG_INFO(AQHBCI_LOGDOMAIN, "*** Dumping securities *********************");
    stmp=AB_ImExporterContext_GetFirstSecurity(ctx);
    while (stmp) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "*** --------------------------------------");
      gn=GWEN_DB_Group_new("security");
      AB_Security_toDb(stmp, gn);
      GWEN_DB_Dump(gn, 2);
      if (gn)
        GWEN_DB_Group_free(gn);
      stmp=AB_Security_List_Next(stmp);
    }
    AB_Security_free(stmp);

    DBG_INFO(AQHBCI_LOGDOMAIN, "*** End dumping securities *****************");
  }

  GWEN_Buffer_free(tbooked);
  return 0;
}


/* the following is ancient code. We should probably switch to using AqBanking's import functions like in
 * JobGetBalance, but I have no way of testing changes (I don't have a credit card for which HBCI is available)
 * and this code seems to work, so I don't want to touch it ATM.
 */
int _readSecurities(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx, const char *docType, int noted, GWEN_BUFFER *buf)
{
  GWEN_DBIO *dbio;
  GWEN_SYNCIO *sio;
  int rv;
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbSecurity;
  GWEN_DB_NODE *dbParams;
  AB_ACCOUNT *a;
  AB_USER *u;
  uint32_t progressId;
  uint64_t cnt=0;

  a=AH_AccountJob_GetAccount(j);
  assert(a);
  u=AH_Job_GetUser(j);
  assert(u);

  dbio=GWEN_DBIO_GetPlugin("swift");
  if (!dbio) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Plugin SWIFT is not found");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("Plugin \"SWIFT\" not found."));
    return AB_ERROR_PLUGIN_MISSING;
  }

  GWEN_Buffer_Rewind(buf);
  sio=GWEN_SyncIo_Memory_new(buf, 0);


  db=GWEN_DB_Group_new("transactions");
  dbParams=GWEN_DB_Group_new("params");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_OVERWRITE_VARS, "type", docType);
  if (AH_User_GetFlags(u) & AH_USER_FLAGS_KEEP_MULTIPLE_BLANKS)
    GWEN_DB_SetIntValue(dbParams, GWEN_DB_FLAGS_OVERWRITE_VARS, "keepMultipleBlanks", 1);
  else
    GWEN_DB_SetIntValue(dbParams, GWEN_DB_FLAGS_OVERWRITE_VARS, "keepMultipleBlanks", 0);

  rv=GWEN_DBIO_Import(dbio, sio, db, dbParams, GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error parsing SWIFT %s (%d)", docType, rv);
    GWEN_DB_Group_free(dbParams);
    GWEN_DB_Group_free(db);
    GWEN_SyncIo_free(sio);
    GWEN_DBIO_free(dbio);
    return rv;
  }
  GWEN_DB_Group_free(dbParams);
  GWEN_SyncIo_free(sio);
  GWEN_DBIO_free(dbio);

  /* first count the securities */
  dbSecurity=GWEN_DB_FindFirstGroup(db, "security");
  while (dbSecurity) {
    cnt++;
    dbSecurity=GWEN_DB_FindNextGroup(dbSecurity, "security");
  } /* while */

  progressId=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
                                    GWEN_GUI_PROGRESS_ALLOW_EMBED |
                                    GWEN_GUI_PROGRESS_SHOW_PROGRESS |
                                    GWEN_GUI_PROGRESS_SHOW_ABORT,
                                    I18N("Importing transactions..."),
                                    NULL,
                                    cnt,
                                    0);

  /* add security to list */
  dbSecurity=GWEN_DB_FindFirstGroup(db, "security");
  while (dbSecurity) {
    AB_SECURITY *asec;
    AB_VALUE *aval;
    GWEN_TIME *gt = NULL;
    const char *p;

    asec=AB_Security_new();

    p=GWEN_DB_GetCharValue(dbSecurity, "name", 0, NULL);
    if (p) {
      AB_Security_SetName(asec, p);
    }

    p=GWEN_DB_GetCharValue(dbSecurity, "nameSpace", 0, NULL);
    if (p) {
      AB_Security_SetNameSpace(asec, p);
    }

    p=GWEN_DB_GetCharValue(dbSecurity, "uniqueId", 0, NULL);
    if (p) {
      AB_Security_SetUniqueId(asec, p);
    }

    p=GWEN_DB_GetCharValue(dbSecurity, "units", 0, NULL);
    if (p) {
      aval=AB_Value_fromString(p);
      AB_Security_SetUnits(asec, aval);
      AB_Value_free(aval);
    }

    p=GWEN_DB_GetCharValue(dbSecurity, "unitPriceValue/value", 0, NULL);
    if (p) {
      aval=AB_Value_fromString(p);
      p=GWEN_DB_GetCharValue(dbSecurity, "unitPriceValue/currency", 0, NULL);
      if (p)
        AB_Value_SetCurrency(aval, p);
      AB_Security_SetUnitPriceValue(asec, aval);
      AB_Value_free(aval);
    }

    p=GWEN_DB_GetCharValue(dbSecurity, "unitPriceDate", 0, NULL);
    if (p) {
      gt=GWEN_Time_fromString(p, "YYYYMMDD");
      if (gt)
        AB_Security_SetUnitPriceDate(asec, gt);
    }

    p=GWEN_DB_GetCharValue(dbSecurity, "unitPriceDateTime", 0, NULL);
    if (p) {
      gt=GWEN_Time_fromString(p, "YYYYMMDDhhmmss");
      if (gt)
        AB_Security_SetUnitPriceDate(asec, gt);
    }

    AB_ImExporterContext_AddSecurity(ctx, asec);

    GWEN_Time_free(gt);

    dbSecurity=GWEN_DB_FindNextGroup(dbSecurity, "security");
  } /* while */

  GWEN_Gui_ProgressEnd(progressId);

  GWEN_DB_Group_free(db);
  return 0;
}


