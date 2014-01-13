/***************************************************************************
    begin       : Tue Dec 31 2013
    copyright   : (C) 2004-2013 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobsepadebitdatedsinglecreate_p.h"
#include "aqhbci_l.h"
#include "accountjob_l.h"
#include "job_l.h"
#include "provider_l.h"
#include "hhd_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/text.h>

#include <aqbanking/jobsepadebitnote_be.h>
#include <aqbanking/job_be.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_INHERIT(AH_JOB, AH_JOB_CREATESEPASINGLEDEBIT);




/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_SepaDebitDatedSingleCreate_new(AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;
  AH_JOB_CREATESEPASINGLEDEBIT *aj;
  GWEN_DB_NODE *dbArgs;

  j=AH_AccountJob_new("JobSepaDebitDatedSingleCreate", u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 29);

  GWEN_NEW_OBJECT(AH_JOB_CREATESEPASINGLEDEBIT, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_CREATESEPASINGLEDEBIT, j, aj,
                       AH_Job_SepaDebitDatedSingleCreate_FreeData);

  /* overwrite some virtual functions */
  AH_Job_SetExchangeFn(j, AH_Job_SepaDebitDatedSingleCreate_Exchange);
  AH_Job_SetProcessFn(j, AH_Job_SepaDebitDatedSingleCreate_Process);
  AH_Job_SetAddChallengeParamsFn(j, AH_Job_SepaDebitDatedSingleCreate_AddChallengeParams);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
void GWENHYWFAR_CB AH_Job_SepaDebitDatedSingleCreate_FreeData(void *bp, void *p){
  AH_JOB_CREATESEPASINGLEDEBIT *aj;

  aj=(AH_JOB_CREATESEPASINGLEDEBIT*)p;
  AB_Transaction_free(aj->validatedTransaction);
  free(aj->fiid);

  GWEN_FREE_OBJECT(aj);
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaDebitDatedSingleCreate_ExchangeParams(AH_JOB *j, AB_JOB *bj,
							 AB_IMEXPORTER_CONTEXT *ctx) {
  AH_JOB_CREATESEPASINGLEDEBIT *aj;
  AB_TRANSACTION_LIMITS *lim;
  GWEN_DB_NODE *dbParams;
  int i, i1, i2;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging params");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_CREATESEPASINGLEDEBIT, j);
  assert(aj);

  dbParams=AH_Job_GetParams(j);
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Have this parameters to exchange:");
  if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Debug)
    GWEN_DB_Dump(dbParams, 2);

  /* set some default limits */
  lim=AB_TransactionLimits_new();
  AB_TransactionLimits_SetMaxLenPurpose(lim, 140);
  AB_TransactionLimits_SetMaxLenRemoteName(lim, 70);
  AB_TransactionLimits_SetMaxLinesRemoteName(lim, 1);
  AB_TransactionLimits_SetMaxLinesPurpose(lim, 1);

  /* set info from BPD */
  i1=GWEN_DB_GetIntValue(dbParams, "minDelay_FNAL_RCUR", 0, 0);
  i2=GWEN_DB_GetIntValue(dbParams, "minDelay_FRST_OOFF", 0, 0);
  i=(i1>i2)?i1:i2;
  AB_TransactionLimits_SetMinValueSetupTime(lim, i);

  i1=GWEN_DB_GetIntValue(dbParams, "maxDelay_FNAL_RCUR", 0, 0);
  i2=GWEN_DB_GetIntValue(dbParams, "maxDelay_FRST_OOFF", 0, 0);
  i=(i1<i2)?i1:i2;
  AB_TransactionLimits_SetMaxValueSetupTime(lim, i);

  AB_Job_SetFieldLimits(bj, lim);
  AB_TransactionLimits_free(lim);

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaDebitDatedSingleCreate_ExchangeArgs(AH_JOB *j, AB_JOB *bj,
                                                       AB_IMEXPORTER_CONTEXT *ctx) {
  AH_JOB_CREATESEPASINGLEDEBIT *aj;
  GWEN_DB_NODE *dbArgs;
  const AB_TRANSACTION_LIMITS *lim=NULL;
  AB_BANKING *ab;
  AB_TRANSACTION *t=NULL;
  AB_ACCOUNT *a;
  AB_USER *u;
  int rv;
  const char *profileName="";
  const char *descriptor="";
  const char *s;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging args");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_CREATESEPASINGLEDEBIT, j);
  assert(aj);

  a=AB_Job_GetAccount(bj);
  assert(a);

  ab=AB_Account_GetBanking(AB_Job_GetAccount(bj));
  assert(ab);

  u=AH_Job_GetUser(j);
  assert(u);

  dbArgs=AH_Job_GetArguments(j);

  /* get limits and transaction */
  lim=AB_Job_GetFieldLimits(bj);
  t=AB_Job_GetTransaction(bj);
  if (t==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction in job");
    return GWEN_ERROR_INVALID;
  }

  /* choose from HISPAS */
  /* first check for any descriptor for pain 008.002.02 */
  s=AH_User_FindSepaDescriptor(u, "*008.002.02*");
  if (s) {
    profileName="008_003_02";
    descriptor=s;
  }
  else {
    /* look for pain 008.001.01 */
    s=AH_User_FindSepaDescriptor(u, "*008.001.01*");
    if (s) {
      profileName="008_001_01";
      descriptor=s;
    }
  }

  /* check for valid descriptor */
  if (!(descriptor && *descriptor)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No SEPA descriptor found, please update your SEPA account information");
    return GWEN_ERROR_GENERIC;
  }
  DBG_INFO(AQHBCI_LOGDOMAIN, "Using SEPA descriptor %s", descriptor);


  /* DISABLED according to a discussion on aqbanking-user:
   * The application should do this, not the library.
  AB_Transaction_FillLocalFromAccount(t, a); */

  /* validate transaction */
  rv=AB_Transaction_CheckForSepaConformity(t);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Transaction_CheckPurposeAgainstLimits(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Transaction_CheckNamesAgainstLimits(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Transaction_CheckDateAgainstLimits(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* set data in job */
  {
    AB_IMEXPORTER_CONTEXT *ioc;
    AB_TRANSACTION *cpy;
    GWEN_BUFFER *dbuf;

    ioc=AB_ImExporterContext_new();
    cpy=AB_Transaction_dup(t);
    AB_ImExporterContext_AddTransaction(ioc, cpy);

    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=AB_Banking_ExportToBuffer(ab, ioc, "sepa", profileName, dbuf);
    AB_ImExporterContext_free(ioc);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(dbuf);
      return rv;
    }

    /* store descriptor */
    GWEN_DB_SetCharValue(dbArgs,
			 GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "descriptor",
			 descriptor);
    /* store transfer */
    GWEN_DB_SetBinValue(dbArgs,
			GWEN_DB_FLAGS_OVERWRITE_VARS,
			"transfer",
			GWEN_Buffer_GetStart(dbuf),
			GWEN_Buffer_GetUsedBytes(dbuf));
    GWEN_Buffer_free(dbuf);
  }

  /* store transaction for later */
  aj->validatedTransaction=AB_Transaction_dup(t);

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaDebitDatedSingleCreate_ExchangeResults(AH_JOB *j, AB_JOB *bj,
                                                      AB_IMEXPORTER_CONTEXT *ctx) {
  AH_JOB_CREATESEPASINGLEDEBIT *aj;
  AH_RESULT_LIST *rl;
  AH_RESULT *r;
  int has10;
  int has20;
  AB_TRANSACTION_STATUS tStatus;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_CREATESEPASINGLEDEBIT, j);
  assert(aj);

  rl=AH_Job_GetSegResults(j);
  assert(rl);

  r=AH_Result_List_First(rl);
  if (!r) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No segment results");
    AB_Job_SetStatus(bj, AB_Job_StatusError);
    return GWEN_ERROR_NO_DATA;
  }
  has10=0;
  has20=0;
  while(r) {
    int rcode;

    rcode=AH_Result_GetCode(r);
    if (rcode>=10 && rcode<=19)
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

  if (aj->validatedTransaction) {
    AB_TRANSACTION *t;

    t=AB_Transaction_dup(aj->validatedTransaction);
    AB_Transaction_SetFiId(t, aj->fiid);
    AB_Transaction_SetStatus(t, tStatus);
    AB_Transaction_SetType(t, AB_Job_TypeSepaDebitNote);
    AB_Job_SetTransaction(bj, t);
    AB_ImExporterContext_AddTransfer(ctx, t);
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaDebitDatedSingleCreate_Exchange(AH_JOB *j, AB_JOB *bj,
						   AH_JOB_EXCHANGE_MODE m,
						   AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_CREATESEPASINGLEDEBIT *aj;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging (%d)", m);

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_CREATESEPASINGLEDEBIT, j);
  assert(aj);

  if (AB_Job_GetType(bj)!=AB_Job_TypeSepaDebitNote) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Not a SepaDebitNote job");
    return GWEN_ERROR_INVALID;
  }

  switch(m) {
  case AH_Job_ExchangeModeParams:
    return AH_Job_SepaDebitDatedSingleCreate_ExchangeParams(j, bj, ctx);
  case AH_Job_ExchangeModeArgs:
    return AH_Job_SepaDebitDatedSingleCreate_ExchangeArgs(j, bj, ctx);
  case AH_Job_ExchangeModeResults:
    return AH_Job_SepaDebitDatedSingleCreate_ExchangeResults(j, bj, ctx);
  default:
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Unsupported exchange mode");
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaDebitDatedSingleCreate_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx) {
  AH_JOB_CREATESEPASINGLEDEBIT *aj;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_CREATESEPASINGLEDEBIT, j);
  assert(aj);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing");
  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  /* search for "SepaDebitDatedSingleCreateResponse" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while(dbCurr) {
    GWEN_DB_NODE *dbXA;
    int rv;

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

    dbXA=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			  "data/SepaDebitDatedSingleCreateResponse");
    if (dbXA) {
      const char *s;

      s=GWEN_DB_GetCharValue(dbXA, "referenceId", 0, 0);
      if (s) {
	free(aj->fiid);
	aj->fiid=strdup(s);
      }
    } /* if "standingOrderResponse" */
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }


  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaDebitDatedSingleCreate_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod) {
  AH_JOB_CREATESEPASINGLEDEBIT *aj;
  const AB_TRANSACTION *t;
  const char *s;
  int tanVer=AH_JOB_TANVER_1_4;

  DBG_ERROR(AQHBCI_LOGDOMAIN, "AddChallengeParams function called");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_CREATESEPASINGLEDEBIT, j);
  assert(aj);

  t=aj->validatedTransaction;
  if (t==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No validated transaction");
    return GWEN_ERROR_INVALID;
  }

  s=GWEN_DB_GetCharValue(dbMethod, "zkaTanVersion", 0, NULL);
  if (s && *s && strncasecmp(s, "1.3", 3)==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "TAN version is 1.3 (%s)", s);
    tanVer=AH_JOB_TANVER_1_3;
  }

  if (tanVer==AH_JOB_TANVER_1_4) {
    int rv;

    DBG_ERROR(AQHBCI_LOGDOMAIN, "TAN version is 1.4.x");
    rv=AH_HHD14_AddChallengeParams_29(j,
				      AB_Transaction_GetValue(t),
				      AB_Transaction_GetRemoteIban(t),
				      AB_Transaction_GetDate(t));
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unhandled tan version %d for now", tanVer);
    return GWEN_ERROR_INTERNAL;
  }
  return 0;
}







