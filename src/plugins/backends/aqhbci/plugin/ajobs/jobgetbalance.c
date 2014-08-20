/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobgetbalance_p.h"
#include "aqhbci_l.h"
#include "accountjob_l.h"
#include "job_l.h"
#include "user_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>

#include <aqbanking/jobgetbalance.h>
#include <aqbanking/jobgetbalance_be.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_INHERIT(AH_JOB, AH_JOB_GETBALANCE);




/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_GetBalance_new(AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;
  AH_JOB_GETBALANCE *aj;
  GWEN_DB_NODE *dbArgs;
  int useCreditCardJob=0;
  GWEN_DB_NODE *updgroup;

  //Check if we should use DKKKS
  updgroup=AH_User_GetUpd(u);
  assert(updgroup);
  updgroup=GWEN_DB_GetGroup(updgroup, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                            AB_Account_GetAccountNumber(account));
  if (updgroup) {
    GWEN_DB_NODE *n;

    n=GWEN_DB_GetFirstGroup(updgroup);
    while(n) {
      if (strcasecmp(GWEN_DB_GetCharValue(n, "job", 0, ""),
		     "DKKKS")==0) {
	useCreditCardJob = 1;
	break;
      }
      n=GWEN_DB_GetNextGroup(n);
    } /* while */
  } /* if updgroup for the given account found */

  if(useCreditCardJob)
    j=AH_AccountJob_new("JobGetBalanceCreditCard", u, account);
  else
    j=AH_AccountJob_new("JobGetBalance", u, account);
  if (!j)
    return 0;

  GWEN_NEW_OBJECT(AH_JOB_GETBALANCE, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_GETBALANCE, j, aj, AH_Job_GetBalance_FreeData);
  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, AH_Job_GetBalance_Process);
  AH_Job_SetExchangeFn(j, AH_Job_GetBalance_Exchange);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);
  if(useCreditCardJob)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT,
			 "accountNumber", AB_Account_GetAccountNumber(account));
  else
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT,
			 "allAccounts", "N");

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
void GWENHYWFAR_CB AH_Job_GetBalance_FreeData(void *bp, void *p){
  AH_JOB_GETBALANCE *aj;

  aj=(AH_JOB_GETBALANCE*)p;
  GWEN_FREE_OBJECT(aj);
}



/* --------------------------------------------------------------- FUNCTION */
AB_BALANCE *AH_Job_GetBalance__ReadBalance(GWEN_DB_NODE *dbT) {
  GWEN_BUFFER *buf;
  GWEN_TIME *t;
  AB_VALUE *v1, *v2;
  AB_BALANCE *bal;
  const char *p;

  bal=0;

  /* read date and time */
  buf=GWEN_Buffer_new(0, 32, 0, 1);
  p=GWEN_DB_GetCharValue(dbT, "date", 0, 0);
  if (p)
    GWEN_Buffer_AppendString(buf, p);
  else {
    AH_AccountJob_AddCurrentDate(buf);
  }
  p=GWEN_DB_GetCharValue(dbT, "time", 0, 0);
  if (p)
    GWEN_Buffer_AppendString(buf, p);
  else {
    AH_AccountJob_AddCurrentTime(buf);
  }
  t=GWEN_Time_fromString(GWEN_Buffer_GetStart(buf), "YYYYMMDDhhmmss");
  if (!t) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error parsing date and time");
  }
  GWEN_Buffer_free(buf);

  /* read value */
  v1=AB_Value_fromDb(dbT);
  v2=0;
  if (!v1) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error parsing value from DB");
  }
  else {
    p=GWEN_DB_GetCharValue(dbT, "debitMark", 0, 0);
    if (p) {
      if (strcasecmp(p, "D")==0 ||
	  strcasecmp(p, "RC")==0) {
	v2=AB_Value_dup(v1);
	AB_Value_Negate(v2);
      }
      else if (strcasecmp(p, "C")==0 ||
               strcasecmp(p, "RD")==0)
	v2=AB_Value_dup(v1);
      else {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad debit mark \"%s\"", p);
	v2=0;
      }
    }
    if (v2)
      bal=AB_Balance_new(v2, t);
    else
      bal=0;
  }

  AB_Value_free(v2);
  AB_Value_free(v1);
  GWEN_Time_free(t);

  return bal;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetBalance_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_GETBALANCE *aj;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobGetBalance");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETBALANCE, j);
  assert(aj);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  /* search for "Balance" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while(dbCurr) {
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

    dbBalance=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                               "data/balance");
    if (!dbBalance)
      dbBalance=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                         "data/balancecreditcard");
    if (dbBalance) {
      AB_ACCOUNT_STATUS *acst;
      GWEN_DB_NODE *dbT;
      AB_ACCOUNT *a;
      AB_IMEXPORTER_ACCOUNTINFO *ai;

      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Got a balance");
      if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevel_Debug)
        GWEN_DB_Dump(dbBalance, 2);

      acst=AB_AccountStatus_new();

      /* read booked balance */
      dbT=GWEN_DB_GetGroup(dbBalance, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                           "booked");
      if (dbT) {
        AB_BALANCE *bal;

        bal=AH_Job_GetBalance__ReadBalance(dbT);
        if (bal) {
	  AB_AccountStatus_SetBookedBalance(acst, bal);
	  AB_AccountStatus_SetTime(acst, AB_Balance_GetTime(bal));
          AB_Balance_free(bal);
        }
      }

      /* read noted balance */
      dbT=GWEN_DB_GetGroup(dbBalance, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                           "noted");
      if (dbT) {
        AB_BALANCE *bal;

        bal=AH_Job_GetBalance__ReadBalance(dbT);
        if (bal) {
	  AB_AccountStatus_SetNotedBalance(acst, bal);
	  if (AB_AccountStatus_GetTime(acst)==NULL)
	    AB_AccountStatus_SetTime(acst, AB_Balance_GetTime(bal));
          AB_Balance_free(bal);
        }
      }

      /* read credit Line */
      dbT=GWEN_DB_GetGroup(dbBalance, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                           "creditLine");
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

      a=AH_AccountJob_GetAccount(j);
      assert(a);
      ai=AB_ImExporterContext_GetAccountInfo(ctx,
                                             AB_Account_GetBankCode(a),
                                             AB_Account_GetAccountNumber(a));
      assert(ai);

      /* add new account status */
      AB_ImExporterAccountInfo_AddAccountStatus(ai, acst);
      break; /* break loop, we found the balance */
    } /* if "Balance" */
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetBalance_Exchange(AH_JOB *j, AB_JOB *bj,
			       AH_JOB_EXCHANGE_MODE m,
			       AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_GETBALANCE *aj;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging (%d)", m);

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETBALANCE, j);
  assert(aj);

  if (AB_Job_GetType(bj)!=AB_Job_TypeGetBalance) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Not a GetBalance job");
    return GWEN_ERROR_INVALID;
  }

  switch(m) {
  case AH_Job_ExchangeModeParams:
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "No params to exchange");
    return 0;
  case AH_Job_ExchangeModeArgs:
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "No arguments to exchange");
    return 0;
  case AH_Job_ExchangeModeResults:
    return 0;
  default:
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Unsupported exchange mode");
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */
}










