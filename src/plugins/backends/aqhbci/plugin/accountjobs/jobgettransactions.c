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


#include "jobgettransactions_p.h"
#include "aqhbci_l.h"
#include "accountjob_l.h"
#include "job_l.h"
#include "user_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/gui.h>

#include <gwenhywfar/syncio_memory.h>

#include <aqbanking/jobgettransactions.h>
#include <aqbanking/jobgettransactions_be.h>
#include <aqbanking/job_be.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>


GWEN_INHERIT(AH_JOB, AH_JOB_GETTRANSACTIONS);




/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_GetTransactions_new(AB_USER *u,
                                   AB_ACCOUNT *account) {
  AH_JOB *j;
  AH_JOB_GETTRANSACTIONS *aj;
  GWEN_DB_NODE *dbArgs;

  int useCreditCardJob = 0;

  //Check if we should use DKKKU
  GWEN_DB_NODE *updgroup;
  updgroup=AH_User_GetUpd(u);
  assert(updgroup);
  updgroup=GWEN_DB_GetGroup(updgroup, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                            AB_Account_GetAccountNumber(account));
  if (updgroup) {
     GWEN_DB_NODE *n;
      n=GWEN_DB_GetFirstGroup(updgroup);
      while(n) {
        if (strcasecmp(GWEN_DB_GetCharValue(n, "job", 0, ""),
                       "DKKKU")==0) {
          useCreditCardJob = 1;
          break;
        }
        n=GWEN_DB_GetNextGroup(n);
      } /* while */
  } /* if updgroup for the given account found */

  if(useCreditCardJob)
    j=AH_AccountJob_new("JobGetTransactionsCreditCard", u, account);
  else
    j=AH_AccountJob_new("JobGetTransactions", u, account);
  if (!j)
    return 0;

  GWEN_NEW_OBJECT(AH_JOB_GETTRANSACTIONS, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_GETTRANSACTIONS, j, aj,
                       AH_Job_GetTransactions_FreeData);
  /* overwrite some virtual functions */
  if(useCreditCardJob)
    AH_Job_SetProcessFn(j, AH_Job_GetTransactionsCreditCard_Process);
  else
    AH_Job_SetProcessFn(j, AH_Job_GetTransactions_Process);

  AH_Job_SetExchangeFn(j, AH_Job_GetTransactions_Exchange);

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
void GWENHYWFAR_CB AH_Job_GetTransactions_FreeData(void *bp, void *p){
  AH_JOB_GETTRANSACTIONS *aj;

  aj=(AH_JOB_GETTRANSACTIONS*)p;

  GWEN_FREE_OBJECT(aj);
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetTransactions__ReadTransactions(AH_JOB *j,
                                             AB_IMEXPORTER_ACCOUNTINFO *ai,
					     const char *docType,
                                             int noted,
					     GWEN_BUFFER *buf){
  GWEN_DBIO *dbio;
  GWEN_SYNCIO *sio;
  int rv;
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbDay;
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
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Error,
			 I18N("Plugin \"SWIFT\" not found."));
    return AB_ERROR_PLUGIN_MISSING;
  }

  GWEN_Buffer_Rewind(buf);
  sio=GWEN_SyncIo_Memory_new(buf, 0);

  db=GWEN_DB_Group_new("transactions");
  dbParams=GWEN_DB_Group_new("params");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "type", docType);
  if (AH_User_GetFlags(u) & AH_USER_FLAGS_KEEP_MULTIPLE_BLANKS)
    GWEN_DB_SetIntValue(dbParams, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"keepMultipleBlanks", 1);
  else
    GWEN_DB_SetIntValue(dbParams, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"keepMultipleBlanks", 0);

  rv=GWEN_DBIO_Import(dbio, sio,
		      db, dbParams,
		      GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
	      "Error parsing SWIFT %s (%d)",
	      docType, rv);
    GWEN_DB_Group_free(dbParams);
    GWEN_DB_Group_free(db);
    GWEN_SyncIo_free(sio);
    GWEN_DBIO_free(dbio);
    return rv;
  }
  GWEN_DB_Group_free(dbParams);
  GWEN_SyncIo_free(sio);
  GWEN_DBIO_free(dbio);

  /* first count the groups */
  dbDay=GWEN_DB_FindFirstGroup(db, "day");
  while(dbDay) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_FindFirstGroup(dbDay, "transaction");
    while(dbT) {
      cnt++;
      dbT=GWEN_DB_FindNextGroup(dbT, "transaction");
    } /* while */
    dbDay=GWEN_DB_FindNextGroup(dbDay, "day");
  } /* while */

  progressId=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
				    GWEN_GUI_PROGRESS_ALLOW_EMBED |
				    GWEN_GUI_PROGRESS_SHOW_PROGRESS |
				    GWEN_GUI_PROGRESS_SHOW_ABORT,
				    I18N("Importing transactions..."),
				    NULL,
				    cnt,
				    0);

  /* add transactions to list */
  dbDay=GWEN_DB_FindFirstGroup(db, "day");
  while(dbDay) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_FindFirstGroup(dbDay, "transaction");
    while(dbT) {
      AB_TRANSACTION *t;

      t=AB_Transaction_fromDb(dbT);
      if (!t) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad transaction data:");
        GWEN_DB_Dump(dbT, 2);
      }
      else {
        AB_Transaction_SetLocalBankCode(t, AB_User_GetBankCode(u));
        AB_Transaction_SetLocalAccountNumber(t,
                                             AB_Account_GetAccountNumber(a));
        DBG_INFO(AQHBCI_LOGDOMAIN, "Adding transaction");
        if (noted)
	  AB_ImExporterAccountInfo_AddNotedTransaction(ai, t);
	else
          AB_ImExporterAccountInfo_AddTransaction(ai, t);
      }

      if (GWEN_ERROR_USER_ABORTED==
	  GWEN_Gui_ProgressAdvance(progressId, GWEN_GUI_PROGRESS_ONE)) {
	GWEN_Gui_ProgressEnd(progressId);
	return GWEN_ERROR_USER_ABORTED;
      }

      dbT=GWEN_DB_FindNextGroup(dbT, "transaction");
    } /* while */

    /* read all endsaldos */
    if (!noted) {
      dbT=GWEN_DB_FindFirstGroup(dbDay, "endSaldo");
      while (dbT) {
	GWEN_DB_NODE *dbX;
	GWEN_TIME *ti=0;
  
	dbX=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "date");
	if (dbX)
	  ti=GWEN_Time_fromDb(dbX);
	dbX=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "value");
	if (dbX) {
	  AB_VALUE *v;
  
	  v=AB_Value_fromDb(dbX);
	  if (v) {
	    AB_BALANCE *bal;
	    AB_ACCOUNT_STATUS *as;
  
	    bal=AB_Balance_new(v, ti);
	    AB_Value_free(v);
	    as=AB_AccountStatus_new();
	    if (ti)
	      AB_AccountStatus_SetTime(as, ti);
	    AB_AccountStatus_SetNotedBalance(as, bal);
	    AB_Balance_free(bal);
	    AB_ImExporterAccountInfo_AddAccountStatus(ai, as);
	  }
	}
	GWEN_Time_free(ti);
  
	dbT=GWEN_DB_FindNextGroup(dbT, "endSaldo");
      } /* while */
    }

    dbDay=GWEN_DB_FindNextGroup(dbDay, "day");
  } /* while */

  GWEN_Gui_ProgressEnd(progressId);

  GWEN_DB_Group_free(db);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetTransactions_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_GETTRANSACTIONS *aj;
  AB_ACCOUNT *a;
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  GWEN_BUFFER *tbooked;
  GWEN_BUFFER *tnoted;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobGetTransactions");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETTRANSACTIONS, j);
  assert(aj);

  tbooked=GWEN_Buffer_new(0, 1024, 0, 1);
  tnoted=GWEN_Buffer_new(0, 1024, 0, 1);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  /* search for "Transactions" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while(dbCurr) {
    GWEN_DB_NODE *dbXA;

    rv=AH_Job_CheckEncryption(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (encryption)");
      GWEN_Buffer_free(tbooked);
      GWEN_Buffer_free(tnoted);
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }
    rv=AH_Job_CheckSignature(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (signature)");
      GWEN_Buffer_free(tbooked);
      GWEN_Buffer_free(tnoted);
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }

    dbXA=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			  "data/transactions");
    if (dbXA) {
      const void *p;
      unsigned int bs;

      if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevel_Debug)
        GWEN_DB_Dump(dbXA, 2);
      p=GWEN_DB_GetBinValue(dbXA, "booked", 0, 0, 0, &bs);
      if (p && bs)
	GWEN_Buffer_AppendBytes(tbooked, p, bs);
      p=GWEN_DB_GetBinValue(dbXA, "noted", 0, 0, 0, &bs);
      if (p && bs)
	GWEN_Buffer_AppendBytes(tnoted, p, bs);
    } /* if "Transactions" */
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  GWEN_Buffer_Rewind(tbooked);
  GWEN_Buffer_Rewind(tnoted);

  /* now the buffers contain data to be parsed by DBIOs */
  a=AH_AccountJob_GetAccount(j);
  assert(a);
  ai=AB_ImExporterContext_GetAccountInfo(ctx,
                                         AB_Account_GetBankCode(a),
                                         AB_Account_GetAccountNumber(a));
  assert(ai);
  AB_ImExporterAccountInfo_SetAccountId(ai, AB_Account_GetUniqueId(a));

  /* read booked transactions */
  if (GWEN_Buffer_GetUsedBytes(tbooked)) {
    if (getenv("AQHBCI_LOGBOOKED")) {
      FILE *f;

      f=fopen("/tmp/booked.mt", "w+");
      if (f) {
        if (fwrite(GWEN_Buffer_GetStart(tbooked),
                   GWEN_Buffer_GetUsedBytes(tbooked), 1, f)!=1) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "fwrite: %s", strerror(errno));
        }
        if (fclose(f)) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "fclose: %s", strerror(errno));
        }
      }
    }

    if (AH_Job_GetTransactions__ReadTransactions(j, ai, "mt940", 0, tbooked)){
      GWEN_Buffer_free(tbooked);
      GWEN_Buffer_free(tnoted);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error parsing booked transactions");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return -1;
    }
  }

  /* read noted transactions */
  if (GWEN_Buffer_GetUsedBytes(tnoted)) {
    if (getenv("AQHBCI_LOGNOTED")) {
      FILE *f;

      f=fopen("/tmp/noted.mt", "w+");
      if (f) {
        if (fwrite(GWEN_Buffer_GetStart(tnoted),
                   GWEN_Buffer_GetUsedBytes(tnoted), 1, f)!=1) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "fwrite: %s", strerror(errno));
        }
        if (fclose(f)) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "fclose: %s", strerror(errno));
        }
      }
    }

    if (AH_Job_GetTransactions__ReadTransactions(j, ai, "mt942", 1, tnoted)) {
      GWEN_Buffer_free(tbooked);
      GWEN_Buffer_free(tnoted);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error parsing noted transactions");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return -1;
    }
  }

  GWEN_Buffer_free(tbooked);
  GWEN_Buffer_free(tnoted);
  return 0;
}

int AH_Job_GetTransactionsCreditCard_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_GETTRANSACTIONS *aj;
  AB_ACCOUNT *a;
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  AB_USER *u;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  GWEN_BUFFER *tbooked;
  GWEN_BUFFER *tnoted;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobGetTransactionsCreditCard");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETTRANSACTIONS, j);
  assert(aj);

  tbooked=GWEN_Buffer_new(0, 1024, 0, 1);
  tnoted=GWEN_Buffer_new(0, 1024, 0, 1);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);
  DBG_INFO(AQHBCI_LOGDOMAIN, "Response:");
  GWEN_DB_Dump(dbResponses, 2);
  DBG_INFO(AQHBCI_LOGDOMAIN, "Response end");


  a=AH_AccountJob_GetAccount(j);
  assert(a);
  ai=AB_ImExporterContext_GetAccountInfo(ctx,
                                         AB_Account_GetBankCode(a),
                                         AB_Account_GetAccountNumber(a));
  assert(ai);
  AB_ImExporterAccountInfo_SetAccountId(ai, AB_Account_GetUniqueId(a));

  u=AH_Job_GetUser(j);
  assert(u);

  /* search for "Transactions" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while(dbCurr) {
    GWEN_DB_NODE *dbXA;

    rv=AH_Job_CheckEncryption(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (encryption)");
      GWEN_Buffer_free(tbooked);
      GWEN_Buffer_free(tnoted);
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }
    rv=AH_Job_CheckSignature(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (signature)");
      GWEN_Buffer_free(tbooked);
      GWEN_Buffer_free(tnoted);
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }

    dbXA=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                          "data/transactionscreditcard");
    if (dbXA) {
      GWEN_DB_NODE *dbT;
      GWEN_DB_NODE *dbV;
      GWEN_TIME *date;
      GWEN_TIME *valutaDate;
      const char *p;

      dbT=GWEN_DB_GetGroup(dbXA, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                           "entries");
      while (dbT) {
        AB_VALUE *v1;
        AB_VALUE *v2;
        GWEN_STRINGLIST* purpose;
        AB_TRANSACTION *t;

        /* read date (Buchungsdatum) */
        p=GWEN_DB_GetCharValue(dbT, "date", 0, 0);
        if (p)
          date=GWEN_Time_fromString(p, "YYYYMMDD");
        else
          date=NULL;

        /* read valutaData (Umsatzdatum) */
        p=GWEN_DB_GetCharValue(dbT, "valutaDate", 0, 0);
        if (p)
          valutaDate=GWEN_Time_fromString(p, "YYYYMMDD");
        else
          valutaDate=NULL;

        /* read value */
        dbV=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                             "value");
        if (dbV)
          v1=AB_Value_fromDb(dbV);
        else
          v1=NULL;
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
          AB_Value_free(v1);
        }

        /* read purpose */
        p=GWEN_DB_GetCharValue(dbT, "purpose", 0, 0);
        if (p)
          purpose=GWEN_StringList_fromTabString(p, 0);
        else
          purpose=NULL;

        /* read reference */
        p=GWEN_DB_GetCharValue(dbT, "reference", 0, 0);
        if (p)
          GWEN_StringList_AppendString(purpose, p, 0, 0);

        t=AB_Transaction_new();
        AB_Transaction_SetLocalBankCode(t, AB_User_GetBankCode(u));
        AB_Transaction_SetLocalAccountNumber(t, AB_Account_GetAccountNumber(a));
        AB_Transaction_SetValutaDate(t, valutaDate);
        AB_Transaction_SetDate(t, date);
        AB_Transaction_SetValue(t, v2);
        AB_Transaction_SetPurpose(t, purpose);
        DBG_INFO(AQHBCI_LOGDOMAIN, "Adding transaction");
        AB_ImExporterAccountInfo_AddTransaction(ai, t);

        GWEN_StringList_free(purpose);
        AB_Value_free(v2);
        GWEN_Time_free(date);
        GWEN_Time_free(valutaDate);

        dbT = GWEN_DB_FindNextGroup(dbT, "entries");
      } //while (dbT)
    } //if (dbXA)
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetTransactions_Exchange(AH_JOB *j, AB_JOB *bj,
				    AH_JOB_EXCHANGE_MODE m,
				    AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_GETTRANSACTIONS *aj;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging (%d)", m);

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETTRANSACTIONS, j);
  assert(aj);

  if (AB_Job_GetType(bj)!=AB_Job_TypeGetTransactions) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Not a GetTransactions job");
    return GWEN_ERROR_INVALID;
  }

  switch(m) {
  case AH_Job_ExchangeModeParams: {
    GWEN_DB_NODE *dbParams;

    dbParams=AH_Job_GetParams(j);
    AB_JobGetTransactions_SetMaxStoreDays(bj,
					  GWEN_DB_GetIntValue(dbParams,
							      "storeDays",
							      0, 0)
                                         );
    return 0;
  }

  case AH_Job_ExchangeModeArgs: {
    const GWEN_TIME *ti;

    ti=AB_JobGetTransactions_GetFromTime(bj);
    if (ti) {
      int year, month, day;
      char dbuf[16];
      GWEN_DB_NODE *dbArgs;

      dbArgs=AH_Job_GetArguments(j);
      if (GWEN_Time_GetBrokenDownDate(ti, &day, &month, &year)) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Internal error: bad fromTime");
	return GWEN_ERROR_INVALID;
      }
      snprintf(dbuf, sizeof(dbuf), "%04d%02d%02d", year, month+1, day);
      GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "fromDate", dbuf);
    }

    ti=AB_JobGetTransactions_GetToTime(bj);
    if (ti) {
      int year, month, day;
      char dbuf[16];
      GWEN_DB_NODE *dbArgs;

      dbArgs=AH_Job_GetArguments(j);
      if (GWEN_Time_GetBrokenDownDate(ti, &day, &month, &year)) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Internal error: bad toTime");
	return GWEN_ERROR_INVALID;
      }
      snprintf(dbuf, sizeof(dbuf), "%04d%02d%02d", year, month+1, day);
      GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "toDate", dbuf);
    }

    return 0;
  }

  case AH_Job_ExchangeModeResults:
    return 0;

  default:
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Unsupported exchange mode");
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */
}










