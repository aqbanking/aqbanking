/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2011 by Martin Preuss
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


/*#define CHALLENGE_ADD_ONLY_PREKOMMA_VALUES*/

#define AH_MULTI_CHALLENGE_CLASS_HKSUB 12
#define AH_MULTI_CHALLENGE_CLASS_HKSLA 19




GWEN_INHERIT(AH_JOB, AH_JOB_MULTITRANSFER);



/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_MultiTransfer_new(AB_USER *u,
                                 AB_ACCOUNT *account) {
  AH_JOB *j;

  j=AH_Job_MultiTransferBase_new(u, account, 1);
  if (j)
    AH_Job_SetChallengeClass(j, AH_MULTI_CHALLENGE_CLASS_HKSUB);
  return j;
}



/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_MultiDebitNote_new(AB_USER *u,
                                  AB_ACCOUNT *account) {
  AH_JOB *j;

  j=AH_Job_MultiTransferBase_new(u, account, 0);
  if (j)
    AH_Job_SetChallengeClass(j, AH_MULTI_CHALLENGE_CLASS_HKSLA);
  return j;
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
  int userMaxTrans;
  int maxTransfers;

  j=AH_AccountJob_new(isTransfer?"JobMultiTransfer":"JobMultiDebitNote",
                      u, account);
  if (!j)
    return 0;

  GWEN_NEW_OBJECT(AH_JOB_MULTITRANSFER, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_MULTITRANSFER, j, aj,
                       AH_Job_MultiTransfer_FreeData);
  aj->isTransfer=isTransfer;
  aj->sumRemoteAccountId=AB_Value_new();
  aj->sumRemoteBankCode=AB_Value_new();
  aj->sumValues=AB_Value_new();

  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, AH_Job_MultiTransfer_Process);
  AH_Job_SetExchangeFn(j, AH_Job_MultiTransfer_Exchange);
  AH_Job_SetPrepareFn(j, AH_Job_MultiTransfer_Prepare);
  AH_Job_SetAddChallengeParamsFn(j, AH_Job_MultiTransfer_AddChallengeParams);

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
  if (isTransfer)
    userMaxTrans=AH_User_GetMaxTransfersPerJob(u);
  else
    userMaxTrans=AH_User_GetMaxDebitNotesPerJob(u);
  maxTransfers=GWEN_DB_GetIntValue(dbParams, "maxTransfers", 0, 0);
  if (maxTransfers==0 || maxTransfers>userMaxTrans)
    AH_Job_SetMaxTransfers(j, userMaxTrans);
  else
    AH_Job_SetMaxTransfers(j, maxTransfers);

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
void GWENHYWFAR_CB AH_Job_MultiTransfer_FreeData(void *bp, void *p){
  AH_JOB_MULTITRANSFER *aj;

  aj=(AH_JOB_MULTITRANSFER*)p;

  AB_Value_free(aj->sumValues);
  AB_Value_free(aj->sumRemoteBankCode);
  AB_Value_free(aj->sumRemoteAccountId);

  GWEN_FREE_OBJECT(aj);
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

  lim=AB_Job_GetFieldLimits(bj);

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
    const char *p;

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      p=GWEN_StringListEntry_Data(se);
      if (p && *p) {
	int l;
	GWEN_BUFFER *tbuf;

	n++;
	if (maxn && n>maxn) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		    "Too many purpose lines (%d>%d)", n, maxn);
	  return GWEN_ERROR_INVALID;
	}
	tbuf=GWEN_Buffer_new(0, maxs, 0, 1);
	AB_ImExporter_Utf8ToDta(p, -1, tbuf);
	l=GWEN_Buffer_GetUsedBytes(tbuf);
	GWEN_Buffer_free(tbuf);
	if (l>maxs) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		    "Too many chars in purpose line %d (%d>%d)", n, l, maxs);
	  return GWEN_ERROR_INVALID;
	}
      }
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }
  if (!n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No purpose lines");
    return GWEN_ERROR_INVALID;
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
    const char *p;

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      p=GWEN_StringListEntry_Data(se);
      if (p && *p) {
	int l;
        GWEN_BUFFER *tbuf;

	n++;
	if (maxn && n>maxn) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		    "Too many remote name lines (%d>%d)",
		    n, maxn);
	  return GWEN_ERROR_INVALID;
	}
	tbuf=GWEN_Buffer_new(0, maxs, 0, 1);
        AB_ImExporter_Utf8ToDta(p, -1, tbuf);
	l=GWEN_Buffer_GetUsedBytes(tbuf);
	GWEN_Buffer_free(tbuf);
	if (l>maxs) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		   "Too many chars in remote name line %d (%d>%d)",
		   n, l, maxs);
	  return GWEN_ERROR_INVALID;
	}
      }
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }
  if (!n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No remote name lines");
    return GWEN_ERROR_INVALID;
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
      return GWEN_ERROR_INVALID;
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
    s=AB_Account_GetSubAccountId(a);
    if (s && *s)
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
	return GWEN_ERROR_INVALID;
      }
    }
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_MultiTransfer_Exchange(AH_JOB *j, AB_JOB *bj,
				  AH_JOB_EXCHANGE_MODE m,
				  AB_IMEXPORTER_CONTEXT *ctx){
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
    return GWEN_ERROR_INVALID;
  }

  switch(m) {
  case AH_Job_ExchangeModeParams: {
    GWEN_DB_NODE *dbParams;
    GWEN_DB_NODE *dbTk;
    AB_TRANSACTION_LIMITS *lim;
    int i;

    dbParams=AH_Job_GetParams(j);
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Have this parameters to exchange:");
    if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Debug)
      GWEN_DB_Dump(dbParams, 2);
    /* read limits */
    lim=AB_TransactionLimits_new();
    AB_TransactionLimits_SetMaxLenPurpose(lim, 27);
    AB_TransactionLimits_SetMaxLenRemoteName(lim, 27);
    AB_TransactionLimits_SetMaxLinesRemoteName(lim, 2);

    AB_TransactionLimits_SetNeedDate(lim, -1);

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

    AB_Job_SetFieldLimits(bj, lim);

    return 0;
  }

  case AH_Job_ExchangeModeArgs: {
    GWEN_DB_NODE *dbArgs;
    const AB_TRANSACTION *ot;
    char groupIdBuf[10];

    groupIdBuf[0]=0;
    dbArgs=AH_Job_GetArguments(j);
    assert(dbArgs);
    ot=AB_Job_GetTransaction(bj);
    if (ot) {
      GWEN_DB_NODE *dbT;
      AB_TRANSACTION *t;

      t=AB_Transaction_dup(ot);
      assert(t);
      if (AH_Job_MultiTransfer__ValidateTransfer(bj, j, t)) {
	DBG_ERROR(AQHBCI_LOGDOMAIN,
		  "Invalid transaction");
	AB_Job_SetStatus(bj, AB_Job_StatusError);
	return GWEN_ERROR_INVALID;
      }
      AB_Transaction_SetGroupId(t, AH_Job_GetId(j));
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Setting groupID to %d", AH_Job_GetId(j));
      if (groupIdBuf[0]==0) {
	snprintf(groupIdBuf, sizeof(groupIdBuf)-1, "%08d", AH_Job_GetId(j));
        groupIdBuf[sizeof(groupIdBuf)-1]=0;
      }

      /* store the validated transaction back into application job,
       * to allow the application to recognize answers to this job later */
      AB_Job_SetTransaction(bj, t);

      dbT=GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_DEFAULT,
                           "transfers");
      assert(dbT);
      /* store customer reference */
      GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
			   "custref",
			   groupIdBuf);

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
	AB_Transaction_free(t);
        return GWEN_ERROR_BAD_DATA;
      }
      else {
	const char *s;
	char tbuf[11];
        AB_VALUE *tv;

        /* sum up account ids */
	s=AB_Transaction_GetRemoteAccountNumber(t);
	assert(s);
        tbuf[0]=0;
	strncat(tbuf, s, 10);
	tv=AB_Value_fromString(tbuf);
	assert(tv);
	AB_Value_AddValue(aj->sumRemoteAccountId, tv);
	AB_Value_free(tv);

        /* sum up bank codes */
        s=AB_Transaction_GetRemoteBankCode(t);
	assert(s);
        tbuf[0]=0;
	strncat(tbuf, s, 10);
	tv=AB_Value_fromString(tbuf);
	assert(tv);
	AB_Value_AddValue(aj->sumRemoteBankCode, tv);
	AB_Value_free(tv);

#ifdef CHALLENGE_ADD_ONLY_PREKOMMA_VALUES
	if (1) {
	  GWEN_BUFFER *tbuf;
	  char *p;
	  AB_VALUE *tv;

	  tbuf=GWEN_Buffer_new(0, 32, 0, 1);
	  AB_Value_toHumanReadableString2(AB_Transaction_GetValue(t),
					  tbuf, 0, 0);
	  p=strchr(GWEN_Buffer_GetStart(tbuf), ',');
	  if (p)
	    *p=0;
	  tv=AB_Value_fromString(GWEN_Buffer_GetStart(tbuf));
	  assert(tv);
	  AB_Value_AddValue(aj->sumValues, tv);
	  AB_Value_free(tv);
          GWEN_Buffer_free(tbuf);
	}
#else
	AB_Value_AddValue(aj->sumValues, AB_Transaction_GetValue(t));
#endif
      }
      AB_Transaction_free(t);

      AH_Job_IncTransferCount(j);
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction");
      AB_Job_SetStatus(bj, AB_Job_StatusError);
      return GWEN_ERROR_NO_DATA;
    }
    return 0;
  }

  case AH_Job_ExchangeModeResults: {
    AH_RESULT_LIST *rl;
    AH_RESULT *r;
    int has10;
    int has20;
    AB_TRANSACTION_STATUS tStatus;

    rl=AH_Job_GetSegResults(j);
    assert(rl);

    r=AH_Result_List_First(rl);
    if (!r) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "No segment results");
      AB_Job_SetStatus(bj, AB_Job_StatusError);
      return GWEN_ERROR_NO_DATA;
    }
    has10=0;
    has20=0;
    while(r) {
      int rcode;

      rcode=AH_Result_GetCode(r);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Found job result: %d", rcode);
      if (rcode<=19)
	has10=1;
      else if (rcode>=20 && rcode <=29)
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
	       "Error status (neither 0010 nor 0020)");
      AB_Job_SetStatus(bj, AB_Job_StatusError);
    }

    if (has20)
      tStatus=AB_Transaction_StatusAccepted;
    else if (has10)
      tStatus=AB_Transaction_StatusPending;
    else
      tStatus=AB_Transaction_StatusRejected;

    if (aj->isTransfer) {
      const AB_TRANSACTION *ot;

      ot=AB_Job_GetTransaction(bj);
      if (ot) {
	AB_TRANSACTION *t;

	t=AB_Transaction_dup(ot);
	AB_Transaction_SetStatus(t, tStatus);
	AB_Transaction_SetType(t, AB_Transaction_TypeTransfer);
	AB_ImExporterContext_AddTransfer(ctx, t);
      }
    }
    else {
      const AB_TRANSACTION *ot;

      ot=AB_Job_GetTransaction(bj);
      if (ot) {
	AB_TRANSACTION *t;

	t=AB_Transaction_dup(ot);
	AB_Transaction_SetStatus(t, tStatus);
	AB_Transaction_SetType(t, AB_Transaction_TypeDebitNote);
	AB_ImExporterContext_AddTransfer(ctx, t);
      }
    }

    return 0;
  }

  default:
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Unsupported exchange mode %d", m);
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */
}



int AH_Job_MultiTransfer_Prepare(AH_JOB *j){
#if 0
  /* this code should no longer be needed since we use
   * AH_Job_MultiTransfer_AddChallengeParams now */

  AH_JOB_MULTITRANSFER *aj;
  GWEN_BUFFER *tbuf;
  char *p;

  DBG_ERROR(AQHBCI_LOGDOMAIN, "Prepare function called");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_MULTITRANSFER, j);
  assert(aj);

  tbuf=GWEN_Buffer_new(0, 32, 0, 1);

  /* set challenge parameter */

  /* add sum of account numbers */
  AB_Value_toHumanReadableString2(aj->sumRemoteAccountId,
				  tbuf, 0, 0);
  p=strchr(GWEN_Buffer_GetStart(tbuf), '.');
  if (p)
    *p=0;
  AH_Job_AddChallengeParam(j, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* add sum of bank codes */
  AB_Value_toHumanReadableString2(aj->sumRemoteBankCode,
                                  tbuf, 0, 0);
  p=strchr(GWEN_Buffer_GetStart(tbuf), '.');
  if (p)
    *p=0;
  AH_Job_AddChallengeParam(j, GWEN_Buffer_GetStart(tbuf));


  GWEN_Buffer_free(tbuf);

  if (aj->sumValues) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Setting value");
  }
  AH_Job_SetChallengeValue(j, aj->sumValues);

#endif

  return 0;
}


int AH_Job_MultiTransfer_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod) {
  AH_JOB_MULTITRANSFER *aj;
  const char *s;
  int tanVer=AH_JOB_TANVER_1_4;
  char *p;
  AB_ACCOUNT *acc=NULL;

  DBG_ERROR(AQHBCI_LOGDOMAIN, "AddChallengeParams function called");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_MULTITRANSFER, j);
  assert(aj);

  s=GWEN_DB_GetCharValue(dbMethod, "zkaTanVersion", 0, NULL);
  if (s && *s && strncasecmp(s, "1.3", 3)==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "TAN version is 1.3 (%s)", s);
    tanVer=AH_JOB_TANVER_1_3;
  }

  /* set challenge parameter */
  if (tanVer==AH_JOB_TANVER_1_4) {
    GWEN_BUFFER *tbuf;
    char numbuf[16];

    DBG_ERROR(AQHBCI_LOGDOMAIN, "TAN version is 1.4.x");
    tbuf=GWEN_Buffer_new(0, 32, 0, 1);

    if (aj->isTransfer)
      AH_Job_SetChallengeClass(j, 12);
    else
      AH_Job_SetChallengeClass(j, 19);

    /* P1: number of transfers */
    snprintf(numbuf, sizeof(numbuf)-1, "%d", AH_Job_GetTransferCount(j));
    numbuf[sizeof(numbuf)-1]=0;
    AH_Job_AddChallengeParam(j, numbuf);

    /* P2: sum of amount */
    AH_Job_ValueToChallengeString(aj->sumValues, tbuf);
    AH_Job_AddChallengeParam(j, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_Reset(tbuf);

    /* P3: local account number */
    acc=AH_AccountJob_GetAccount(j);
    assert(acc);
    s=AB_Account_GetAccountNumber(acc);
    if (s && *s) {
      int i;

      GWEN_Buffer_AppendString(tbuf, s);
      i=10-strlen(s);
      if (i>0) {
        /* need to left-fill the account number with leading zeroes
         * to a length of exactly 10 digits */
        GWEN_Buffer_Rewind(tbuf);
        GWEN_Buffer_FillLeftWithBytes(tbuf, '0', i);
      }
      AH_Job_AddChallengeParam(j, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_Reset(tbuf);
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No account number");
      GWEN_Buffer_free(tbuf);
      return GWEN_ERROR_INVALID;
    }

    /* add sum of account numbers */
    AB_Value_toHumanReadableString2(aj->sumRemoteAccountId,
                                    tbuf, 0, 0);
    p=strchr(GWEN_Buffer_GetStart(tbuf), '.');
    if (p)
      *p=0;
    /* only use first 10 digits */
    GWEN_Buffer_Crop(tbuf, 0, 10);
    AH_Job_AddChallengeParam(j, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_Reset(tbuf);


    /* done */
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unhandled tan version 0x%02x for now", tanVer);
    return GWEN_ERROR_INTERNAL;
  }

  return 0;
}










