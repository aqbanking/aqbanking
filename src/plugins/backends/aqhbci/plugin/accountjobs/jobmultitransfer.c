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


#include "jobmultitransfer_p.h"
#include "aqhbci_l.h"
#include "accountjob_l.h"
#include <aqhbci/account.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/text.h>

#include <aqbanking/jobsingletransfer_be.h>
#include <aqbanking/jobsingledebitnote_be.h>
#include <aqbanking/job_be.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>


GWEN_INHERIT(AH_JOB, AH_JOB_MULTITRANSFER);



/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_MultiTransfer_new(AB_USER *u,
                                 AB_ACCOUNT *account) {
  return AH_Job_MultiTransferBase_new(u, account, 1);
}



/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_MultiDebitNote_new(AB_USER *u,
                                  AB_ACCOUNT *account) {
  return AH_Job_MultiTransferBase_new(u, account, 0);
}



/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_MultiTransferBase_new(AB_USER *u,
                                     AB_ACCOUNT *account,
                                     int isTransfer) {
  AH_JOB *j;
  AH_JOB_MULTITRANSFER *aj;
  GWEN_DB_NODE *dbArgs;
  GWEN_DB_NODE *dbT;
  GWEN_DB_NODE *dbParams;
  const char *s;

  j=AH_AccountJob_new(isTransfer?"JobMultiTransfer":"JobMultiDebitNote",
                      u, account);
  if (!j)
    return 0;

  GWEN_NEW_OBJECT(AH_JOB_MULTITRANSFER, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_MULTITRANSFER, j, aj,
                       AH_Job_MultiTransfer_FreeData);
  aj->isTransfer=isTransfer;
  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, AH_Job_MultiTransfer_Process);
  AH_Job_SetExchangeFn(j, AH_Job_MultiTransfer_Exchange);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  dbT=GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_DEFAULT,
                       "transfers");
  assert(dbT);
  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "bankCode",
		       AB_Account_GetBankCode(account));
  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "accountId",
		       AB_Account_GetAccountNumber(account));
  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "currency", "EUR");
  s=AB_Account_GetOwnerName(account);
  if (s)
    GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "name", s);
  else {
    DBG_WARN(AQHBCI_LOGDOMAIN,
             "No owner name for account, some banks don't accept this");
  }
  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "type",
		       isTransfer?"transfer":"debitNote");

  dbParams=AH_Job_GetParams(j);
  aj->maxTransfers=GWEN_DB_GetIntValue(dbParams, "maxTransfers", 0, 0);
  if (aj->maxTransfers==0 || aj->maxTransfers>AH_JOBMULTITRANSFER_MAXTRANS)
    aj->maxTransfers=AH_JOBMULTITRANSFER_MAXTRANS;

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
void GWENHYWFAR_CB AH_Job_MultiTransfer_FreeData(void *bp, void *p){
  AH_JOB_MULTITRANSFER *aj;

  aj=(AH_JOB_MULTITRANSFER*)p;

  GWEN_FREE_OBJECT(aj);
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_MultiTransferBase_GetTransferCount(AH_JOB *j){
  AH_JOB_MULTITRANSFER *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_MULTITRANSFER, j);
  assert(aj);
  return aj->transferCount;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_MultiTransferBase_GetMaxTransfers(AH_JOB *j){
  AH_JOB_MULTITRANSFER *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_MULTITRANSFER, j);
  assert(aj);
  return aj->maxTransfers;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_MultiTransfer_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_MULTITRANSFER *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_MULTITRANSFER, j);
  assert(aj);
  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing %s",
           (aj->isTransfer)?"JobMultiTransfer":"JobMultiDebitNote");

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_MultiTransfer__ValidateTransfer(AB_JOB *bj,
					   AH_JOB *mj,
					   AB_TRANSACTION *t) {
  const GWEN_STRINGLIST *sl;
  int maxn;
  int maxs;
  int n;
  const char *s;
  AH_JOB_MULTITRANSFER *aj;
  const AB_TRANSACTION_LIMITS *lim;

  assert(mj);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_MULTITRANSFER, mj);
  assert(aj);

  if (aj->isTransfer)
    lim=AB_JobSingleTransfer_GetFieldLimits(bj);
  else
    lim=AB_JobSingleDebitNote_GetFieldLimits(bj);

  /* collapse splits if any */
  n=AB_Split_List_GetCount(AB_Transaction_GetSplits(t));
  if (n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
	      "Transfer contains splits, unable to handle");
    return -1;
  }

  /* check purpose */
  if (lim) {
    maxn=AB_TransactionLimits_GetMaxLinesPurpose(lim);
    maxs=AB_TransactionLimits_GetMaxLenPurpose(lim);
  }
  else {
    maxn=0;
    maxs=0;
  }
  sl=AB_Transaction_GetPurpose(t);
  n=0;
  if (sl) {
    GWEN_STRINGLISTENTRY *se;
    GWEN_STRINGLIST *nsl;
    const char *p;

    nsl=GWEN_StringList_new();
    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      p=GWEN_StringListEntry_Data(se);
      if (p && *p) {
	char *np;
	int l;
	GWEN_BUFFER *tbuf;

	n++;
	if (maxn && n>maxn) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		    "Too many purpose lines (%d>%d)", n, maxn);
	  GWEN_StringList_free(nsl);
	  return AB_ERROR_INVALID;
	}
	tbuf=GWEN_Buffer_new(0, maxs, 0, 1);
	AB_ImExporter_Utf8ToDta(p, -1, tbuf);
	l=GWEN_Buffer_GetUsedBytes(tbuf);
	if (l>maxs) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		    "Too many chars in purpose line %d (%d>%d)", n, l, maxs);
	  GWEN_StringList_free(nsl);
	  return AB_ERROR_INVALID;
	}
	np=(char*)malloc(l+1);
	memmove(np, GWEN_Buffer_GetStart(tbuf), l+1);
	GWEN_Buffer_free(tbuf);
	/* let string list take the newly alllocated string */
	GWEN_StringList_AppendString(nsl, np, 1, 0);
      }
      se=GWEN_StringListEntry_Next(se);
    } /* while */
    AB_Transaction_SetPurpose(t, nsl);
  }
  if (!n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No purpose lines");
    return AB_ERROR_INVALID;
  }

  /* check remote name */
  if (lim) {
    maxn=AB_TransactionLimits_GetMaxLinesRemoteName(lim);
    maxs=AB_TransactionLimits_GetMaxLenRemoteName(lim);
  }
  else {
    maxn=0;
    maxs=0;
  }
  sl=AB_Transaction_GetRemoteName(t);
  n=0;
  if (sl) {
    GWEN_STRINGLISTENTRY *se;
    GWEN_STRINGLIST *nsl;
    const char *p;

    nsl=GWEN_StringList_new();
    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      p=GWEN_StringListEntry_Data(se);
      if (p && *p) {
	char *np;
	int l;
        GWEN_BUFFER *tbuf;

	n++;
	if (maxn && n>maxn) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		    "Too many remote name lines (%d>%d)",
		    n, maxn);
          GWEN_StringList_free(nsl);
	  return AB_ERROR_INVALID;
	}
	tbuf=GWEN_Buffer_new(0, maxs, 0, 1);
        AB_ImExporter_Utf8ToDta(p, -1, tbuf);
	l=GWEN_Buffer_GetUsedBytes(tbuf);
	if (l>maxs) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		   "Too many chars in remote name line %d (%d>%d)",
		   n, l, maxs);
          GWEN_StringList_free(nsl);
	  return AB_ERROR_INVALID;
	}
	np=(char*)malloc(l+1);
	memmove(np, GWEN_Buffer_GetStart(tbuf), l+1);
	GWEN_Buffer_free(tbuf);
	/* let string list take the newly alllocated string */
	GWEN_StringList_AppendString(nsl, np, 1, 0);
      }
      se=GWEN_StringListEntry_Next(se);
    } /* while */
    AB_Transaction_SetRemoteName(t, nsl);
  }
  if (!n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No remote name lines");
    return AB_ERROR_INVALID;
  }

  /* check local name */
  s=AB_Transaction_GetLocalName(t);
  if (!s) {
    AB_ACCOUNT *a;

    DBG_WARN(AQHBCI_LOGDOMAIN,
	     "No local name, filling in");
    a=AB_Job_GetAccount(bj);
    assert(a);
    s=AB_Account_GetOwnerName(a);
    if (!s) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
		"No owner name in account, giving up");
      return AB_ERROR_INVALID;
    }
    AB_Transaction_SetLocalName(t, s);
  }

  /* check local bank code */
  s=AB_Transaction_GetLocalBankCode(t);
  if (!s) {
    AB_ACCOUNT *a;

    DBG_WARN(AQHBCI_LOGDOMAIN,
	     "No local bank code, filling in");
    a=AH_AccountJob_GetAccount(mj);
    assert(a);
    s=AB_Account_GetBankCode(a);
    assert(s);
    AB_Transaction_SetLocalBankCode(t, s);
  }

  /* check local account number */
  s=AB_Transaction_GetLocalAccountNumber(t);
  if (!s) {
    AB_ACCOUNT *a;

    DBG_WARN(AQHBCI_LOGDOMAIN,
	     "No local account number, filling in");
    a=AH_AccountJob_GetAccount(mj);
    assert(a);
    s=AB_Account_GetAccountNumber(a);
    assert(s);
    AB_Transaction_SetLocalAccountNumber(t, s);
  }

  /* check local account suffix */
  s=AB_Transaction_GetLocalSuffix(t);
  if (!s) {
    AB_ACCOUNT *a;

    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "No local suffix, filling in (if possible)");
    a=AH_AccountJob_GetAccount(mj);
    assert(a);
    s=AH_Account_GetSuffix(a);
    if (s)
      AB_Transaction_SetLocalSuffix(t, s);
  }

  /* check text key */
  if (lim) {
    if (GWEN_StringList_Count(AB_TransactionLimits_GetValuesTextKey(lim))){
      char numbuf[32];

      n=AB_Transaction_GetTextKey(t);
      if (n==0) {
	if (aj->isTransfer)
	  n=51; /* "Ueberweisung" */
	else
	  n=5;  /* "Lastschrift" */
	AB_Transaction_SetTextKey(t, n);
      }

      snprintf(numbuf, sizeof(numbuf), "%d", n);
      if (!AB_TransactionLimits_HasValuesTextKey(lim, numbuf)) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Text key \"%s\" not supported by bank",
		  numbuf);
	return AB_ERROR_INVALID;
      }
    }
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_MultiTransfer_Exchange(AH_JOB *j, AB_JOB *bj,
                                  AH_JOB_EXCHANGE_MODE m){
  AH_JOB_MULTITRANSFER *aj;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging (%d)", m);

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_MULTITRANSFER, j);
  assert(aj);

  if ((aj->isTransfer && AB_Job_GetType(bj)!=AB_Job_TypeTransfer) ||
      (!aj->isTransfer && AB_Job_GetType(bj)!=AB_Job_TypeDebitNote)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Not a %s job job",
	      (aj->isTransfer)?"MultiTransfer":"MultiDebitNote");
    return AB_ERROR_INVALID;
  }

  switch(m) {
  case AH_Job_ExchangeModeParams: {
    GWEN_DB_NODE *dbParams;
    GWEN_DB_NODE *dbTk;
    AB_TRANSACTION_LIMITS *lim;
    int i;

    dbParams=AH_Job_GetParams(j);
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Have this parameters to exchange:");
    if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevelDebug)
      GWEN_DB_Dump(dbParams, stderr, 2);
    /* read limits */
    lim=AB_TransactionLimits_new();
    AB_TransactionLimits_SetMaxLenPurpose(lim, 27);
    AB_TransactionLimits_SetMaxLenRemoteName(lim, 27);
    AB_TransactionLimits_SetMaxLinesRemoteName(lim, 2);

    i=GWEN_DB_GetIntValue(dbParams, "maxpurposeLines", 0, 0);
    AB_TransactionLimits_SetMaxLinesPurpose(lim, i);

    /* read text keys */
    dbTk=GWEN_DB_GetGroup(dbParams, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "textkey");
    if (dbTk) {
      for (i=0; ; i++) {
	int k;
	char numbuf[16];

	k=GWEN_DB_GetIntValue(dbTk, "key", i, -1);
	if (k==-1)
	  break;
	snprintf(numbuf, sizeof(numbuf), "%d", k);
	AB_TransactionLimits_AddValuesTextKey(lim, numbuf, 1);
      }
      GWEN_StringList_Sort(AB_TransactionLimits_GetValuesTextKey(lim),
                           1, GWEN_StringList_SortModeInt);
    }

    if (aj->isTransfer)
      AB_JobSingleTransfer_SetFieldLimits(bj, lim);
    else
      AB_JobSingleDebitNote_SetFieldLimits(bj, lim);

    return 0;
  }

  case AH_Job_ExchangeModeArgs: {
    GWEN_DB_NODE *dbArgs;
    const AB_TRANSACTION *ot;

    dbArgs=AH_Job_GetArguments(j);
    assert(dbArgs);
    if (aj->isTransfer)
      ot=AB_JobSingleTransfer_GetTransaction(bj);
    else
      ot=AB_JobSingleDebitNote_GetTransaction(bj);
    if (ot) {
      GWEN_DB_NODE *dbT;
      AB_TRANSACTION *t;

      t=AB_Transaction_dup(ot);
      assert(t);
      if (AH_Job_MultiTransfer__ValidateTransfer(bj, j, t)) {
	DBG_ERROR(AQHBCI_LOGDOMAIN,
		  "Invalid transaction");
	AB_Job_SetStatus(bj, AB_Job_StatusError);
	return AB_ERROR_INVALID;
      }
      /* store the validated transaction back into application job,
       * to allow the application to recognize answers to this job later */
      if (aj->isTransfer)
        AB_JobSingleTransfer_SetTransaction(bj, t);
      else
        AB_JobSingleDebitNote_SetTransaction(bj, t);

      dbT=GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_DEFAULT,
                           "transfers");
      assert(dbT);
      dbT=GWEN_DB_GetGroup(dbT, GWEN_DB_FLAGS_DEFAULT,
			   "transactions");
      assert(dbT);
      if (aj->isTransfer)
        dbT=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_CREATE_GROUP,
                             "transfer");
      else
        dbT=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_CREATE_GROUP,
                             "debitNote");
      assert(dbT);

      /* store transaction */
      if (AB_Transaction_toDb(t, dbT)) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Error storing transaction to db");
        return AB_ERROR_BAD_DATA;
      }
      aj->transferCount++;
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction");
      AB_Job_SetStatus(bj, AB_Job_StatusError);
      return AB_ERROR_NO_DATA;
    }
    return 0;
  }

  case AH_Job_ExchangeModeResults: {
    AH_RESULT_LIST *rl;
    AH_RESULT *r;
    int has10;
    int has20;

    rl=AH_Job_GetSegResults(j);
    assert(rl);

    r=AH_Result_List_First(rl);
    if (!r) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "No segment results");
      AB_Job_SetStatus(bj, AB_Job_StatusError);
      return AB_ERROR_NO_DATA;
    }
    has10=0;
    has20=0;
    while(r) {
      if (AH_Result_GetCode(r)==10)
        has10=1;
      else if (AH_Result_GetCode(r)==20)
        has20=1;
      r=AH_Result_List_Next(r);
    }

    if (has20) {
      AB_Job_SetStatus(bj, AB_Job_StatusFinished);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job finished");
    }
    else if (has10) {
      AB_Job_SetStatus(bj, AB_Job_StatusPending);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job pending");
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Can't determine the status (neither 0010 nor 0020)");
      AB_Job_SetStatus(bj, AB_Job_StatusError);
      return AB_ERROR_NO_DATA;
    }
    return 0;
  }

  default:
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Unsupported exchange mode %d", m);
    return AB_ERROR_NOT_SUPPORTED;
  } /* switch */
}










