/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
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
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/waitcallback.h>

#include <aqbanking/jobgettransactions.h>
#include <aqbanking/jobgettransactions_be.h>
#include <aqbanking/job_be.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>


GWEN_INHERIT(AH_JOB, AH_JOB_GETTRANSACTIONS);




/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_GetTransactions_new(AH_CUSTOMER *cu,
				   AH_ACCOUNT *account) {
  AH_JOB *j;
  AH_JOB_GETTRANSACTIONS *aj;
  GWEN_DB_NODE *dbArgs;

  j=AH_AccountJob_new("JobGetTransactions", cu, account);
  if (!j)
    return 0;

  GWEN_NEW_OBJECT(AH_JOB_GETTRANSACTIONS, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_GETTRANSACTIONS, j, aj,
                       AH_Job_GetTransactions_FreeData);
  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, AH_Job_GetTransactions_Process);
  AH_Job_SetExchangeFn(j, AH_Job_GetTransactions_Exchange);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);
  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT,
                       "allAccounts", "N");
  return j;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Job_GetTransactions_FreeData(void *bp, void *p){
  AH_JOB_GETTRANSACTIONS *aj;

  aj=(AH_JOB_GETTRANSACTIONS*)p;
  AB_Transaction_List2_free(aj->bookedTransactions);
  AB_Transaction_List2_free(aj->notedTransactions);
  GWEN_FREE_OBJECT(aj);
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetTransactions__ReadTransactions(AH_JOB *j,
                                             const char *docType,
					     GWEN_BUFFER *buf,
					     AB_TRANSACTION_LIST2 *tl){
  GWEN_BUFFEREDIO *bio;
  GWEN_DBIO *dbio;
  GWEN_ERRORCODE err;
  int rv;
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbDay;
  GWEN_DB_NODE *dbParams;
  AH_ACCOUNT *a;
  AH_BANK *b;
  GWEN_TYPE_UINT64 cnt=0;
  GWEN_TYPE_UINT64 done = 0;

  a=AH_AccountJob_GetAccount(j);
  assert(a);
  b=AH_Account_GetBank(a);
  assert(b);

  dbio=GWEN_DBIO_GetPlugin("swift");
  if (!dbio) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Plugin SWIFT is not supported");
    return -1;
  }
  bio=GWEN_BufferedIO_Buffer2_new(buf, 0);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);

  db=GWEN_DB_Group_new("transactions");
  dbParams=GWEN_DB_Group_new("params");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "type", docType);
  while(!GWEN_BufferedIO_CheckEOF(bio)) {
    rv=GWEN_DBIO_Import(dbio, bio, GWEN_PATH_FLAGS_CREATE_GROUP,
                        db, dbParams);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error parsing SWIFT %s", docType);
    }
  } /* while */
  GWEN_DB_Group_free(dbParams);
  err=GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "called from here");
    GWEN_DB_Group_free(db);
    return -1;
  }

  /* DEBUG */
  if (GWEN_DB_WriteFile(db,
                        "/tmp/transactions.trans",
                        GWEN_DB_FLAGS_DEFAULT)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not write transactions");
  }

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

  /* enter waitcallback context */
  GWEN_WaitCallback_EnterWithText(GWEN_WAITCALLBACK_ID_FAST,
                                  I18N("Importing transactions..."),
                                  I18N("transaction(s)"),
                                  GWEN_WAITCALLBACK_FLAGS_NO_REUSE);
  GWEN_WaitCallback_SetProgressTotal(cnt);
  GWEN_WaitCallback_SetProgressPos(0);

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
        GWEN_DB_Dump(dbT, stderr, 2);
      }
      else {
        if (AB_Transaction_GetLocalBankCode(t)==0)
          AB_Transaction_SetLocalBankCode(t, AH_Bank_GetBankId(b));
        if (AB_Transaction_GetLocalAccountNumber(t)==0)
          AB_Transaction_SetLocalAccountNumber(t, AH_Account_GetAccountId(a));
        DBG_INFO(AQHBCI_LOGDOMAIN, "Adding transaction");
        AB_Transaction_List2_PushBack(tl, t);
      }
      done++;
      if (GWEN_WaitCallbackProgress(done)==GWEN_WaitCallbackResult_Abort) {
        GWEN_WaitCallback_Leave();
        return AB_ERROR_USER_ABORT;
      }
      GWEN_WaitCallback_SetProgressPos(done);
      dbT=GWEN_DB_FindNextGroup(dbT, "transaction");
    } /* while */
    dbDay=GWEN_DB_FindNextGroup(dbDay, "day");
  } /* while */

  GWEN_WaitCallback_Leave();

  GWEN_DB_Group_free(db);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetTransactions_Process(AH_JOB *j){
  AH_JOB_GETTRANSACTIONS *aj;
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

      if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevelDebug)
        GWEN_DB_Dump(dbXA, stderr, 2);
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

  /* read booked transactions */
  if (GWEN_Buffer_GetUsedBytes(tbooked)) {
    AB_TRANSACTION_LIST2 *tl;

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

    tl=AB_Transaction_List2_new();
    if (AH_Job_GetTransactions__ReadTransactions(j, "mt940", tbooked, tl)) {
      AB_Transaction_List2_free(tl);
      GWEN_Buffer_free(tbooked);
      GWEN_Buffer_free(tnoted);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error parsing booked transactions");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return -1;
    }
    if (aj->bookedTransactions)
      AB_Transaction_List2_free(aj->bookedTransactions);
    aj->bookedTransactions=tl;
  }

  /* read noted transactions */
  if (GWEN_Buffer_GetUsedBytes(tnoted)) {
    AB_TRANSACTION_LIST2 *tl;

    tl=AB_Transaction_List2_new();
    if (AH_Job_GetTransactions__ReadTransactions(j, "mt942", tnoted, tl)) {
      AB_Transaction_List2_free(tl);
      GWEN_Buffer_free(tbooked);
      GWEN_Buffer_free(tnoted);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error parsing noted transactions");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return -1;
    }
    if (aj->notedTransactions)
      AB_Transaction_List2_free(aj->notedTransactions);
    aj->notedTransactions=tl;
  }

  GWEN_Buffer_free(tbooked);
  GWEN_Buffer_free(tnoted);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetTransactions_Exchange(AH_JOB *j, AB_JOB *bj,
				    AH_JOB_EXCHANGE_MODE m){
  AH_JOB_GETTRANSACTIONS *aj;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging (%d)", m);

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETTRANSACTIONS, j);
  assert(aj);

  if (AB_Job_GetType(bj)!=AB_Job_TypeGetTransactions) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Not a GetTransactions job");
    return AB_ERROR_INVALID;
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
	return AB_ERROR_INVALID;
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
	return AB_ERROR_INVALID;
      }
      snprintf(dbuf, sizeof(dbuf), "%04d%02d%02d", year, month+1, day);
      GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "toDate", dbuf);
    }

    return 0;
  }

  case AH_Job_ExchangeModeResults:
    if (aj->bookedTransactions) {
      AB_JobGetTransactions_SetTransactions(bj, aj->bookedTransactions);
      aj->bookedTransactions=0;
    }
    return 0;

  default:
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Unsupported exchange mode");
    return AB_ERROR_NOT_SUPPORTED;
  } /* switch */
}










