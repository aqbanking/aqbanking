/***************************************************************************
    begin       : Wed Jan 08 2014
    copyright   : (C) 2014 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobsepadebitdatedmulticreate_p.h"
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
#include <ctype.h>



GWEN_INHERIT(AH_JOB, AH_JOB_CREATESEPAMULTIDEBIT);




/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_SepaDebitDatedMultiCreate_new(AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;
  AH_JOB_CREATESEPAMULTIDEBIT *aj;
  GWEN_DB_NODE *dbParams;
  GWEN_DB_NODE *dbArgs;
  const char *s;

  j=AH_AccountJob_new("JobSepaDebitDatedMultiCreate", u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 32);

  GWEN_NEW_OBJECT(AH_JOB_CREATESEPAMULTIDEBIT, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_CREATESEPAMULTIDEBIT, j, aj,
                       AH_Job_SepaDebitDatedMultiCreate_FreeData);

  /* overwrite some virtual functions */
  AH_Job_SetPrepareFn(j, AH_Job_SepaDebitDatedMultiCreate_Prepare);
  AH_Job_SetExchangeFn(j, AH_Job_SepaDebitDatedMultiCreate_Exchange);
  AH_Job_SetProcessFn(j, AH_Job_SepaDebitDatedMultiCreate_Process);
  AH_Job_SetAddChallengeParamsFn(j, AH_Job_SepaDebitDatedMultiCreate_AddChallengeParams);

  /* get params */
  dbParams=AH_Job_GetParams(j);
  assert(dbParams);

  AH_Job_SetMaxTransfers(j, GWEN_DB_GetIntValue(dbParams, "maxTransfers", 0, 0));

  s=GWEN_DB_GetCharValue(dbParams, "sumFieldNeeded", 0, "j");
  if (s && toupper(*s)=='J')
    aj->sumFieldNeeded=1;
  else
    aj->sumFieldNeeded=0;

  s=GWEN_DB_GetCharValue(dbParams, "singleBookingAllowed", 0, "j");
  if (s && toupper(*s)=='J')
    aj->singleBookingAllowed=1;
  else
    aj->singleBookingAllowed=0;

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);


  return j;
}



/* --------------------------------------------------------------- FUNCTION */
void GWENHYWFAR_CB AH_Job_SepaDebitDatedMultiCreate_FreeData(void *bp, void *p){
  AH_JOB_CREATESEPAMULTIDEBIT *aj;

  aj=(AH_JOB_CREATESEPAMULTIDEBIT*)p;

  free(aj->fiid);
  AB_Value_free(aj->sumValues);

  GWEN_FREE_OBJECT(aj);
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaDebitDatedMultiCreate_ExchangeParams(AH_JOB *j, AB_JOB *bj,
                                                    AB_IMEXPORTER_CONTEXT *ctx) {
  AH_JOB_CREATESEPAMULTIDEBIT *aj;
  AB_TRANSACTION_LIMITS *lim;
  GWEN_DB_NODE *dbParams;
  int i, i1, i2;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging params");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_CREATESEPAMULTIDEBIT, j);
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
int AH_Job_SepaDebitDatedMultiCreate_ExchangeArgs(AH_JOB *j, AB_JOB *bj,
                                                  AB_IMEXPORTER_CONTEXT *ctx) {
  AH_JOB_CREATESEPAMULTIDEBIT *aj;
  const AB_TRANSACTION_LIMITS *lim=NULL;
  AB_BANKING *ab;
  AB_TRANSACTION *t=NULL;
  AB_ACCOUNT *a;
  AB_USER *u;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging args");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_CREATESEPAMULTIDEBIT, j);
  assert(aj);

  a=AB_Job_GetAccount(bj);
  assert(a);

  ab=AB_Account_GetBanking(AB_Job_GetAccount(bj));
  assert(ab);

  u=AH_Job_GetUser(j);
  assert(u);

  /* get limits and transaction */
  lim=AB_Job_GetFieldLimits(bj);
  t=AB_Job_GetTransaction(bj);
  if (t==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction in job");
    return GWEN_ERROR_INVALID;
  }

  /* DISABLED according to a discussion on aqbanking-user:
   * The application should do this, not the library.
  AB_Transaction_FillLocalFromAccount(t, a); */

  /* validate transaction */
  rv=AB_Transaction_CheckForSepaConformity(t);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Transaction_ValidatePurposeAgainstLimits(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Transaction_ValidateNamesAgainstLimits(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Transaction_CheckDateAgainstLimits(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* set group id so the application can now which transfers went together in one setting */
  AB_Transaction_SetGroupId(t, AH_Job_GetId(j));

  /* store validated transaction in job */
  AB_Job_SetTransaction(bj, t);

  /* store copy of transaction for later */
  AH_Job_AddTransfer(j, AB_Transaction_dup(t));

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaDebitDatedMultiCreate_ExchangeResults(AH_JOB *j, AB_JOB *bj,
                                                     AB_IMEXPORTER_CONTEXT *ctx) {
  AH_JOB_CREATESEPAMULTIDEBIT *aj;
  AH_RESULT_LIST *rl;
  AH_RESULT *r;
  int has10;
  int has20;
  AB_TRANSACTION_STATUS tStatus;
  AB_TRANSACTION *t;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_CREATESEPAMULTIDEBIT, j);
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

  t=AB_Job_GetTransaction(bj);
  if (t) {
    AB_TRANSACTION *cpy;

    cpy=AB_Transaction_dup(t);
    AB_Transaction_SetStatus(cpy, tStatus);
    AB_Transaction_SetType(cpy, AB_Job_TypeSepaDebitNote);
    AB_ImExporterContext_AddTransfer(ctx, cpy);
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaDebitDatedMultiCreate_Exchange(AH_JOB *j, AB_JOB *bj,
                                              AH_JOB_EXCHANGE_MODE m,
                                              AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_CREATESEPAMULTIDEBIT *aj;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging (%d)", m);

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_CREATESEPAMULTIDEBIT, j);
  assert(aj);

  if (AB_Job_GetType(bj)!=AB_Job_TypeSepaDebitNote) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Not a SepaDebitNote job");
    return GWEN_ERROR_INVALID;
  }

  switch(m) {
  case AH_Job_ExchangeModeParams:
    return AH_Job_SepaDebitDatedMultiCreate_ExchangeParams(j, bj, ctx);
  case AH_Job_ExchangeModeArgs:
    return AH_Job_SepaDebitDatedMultiCreate_ExchangeArgs(j, bj, ctx);
  case AH_Job_ExchangeModeResults:
    return AH_Job_SepaDebitDatedMultiCreate_ExchangeResults(j, bj, ctx);
  default:
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Unsupported exchange mode");
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaDebitDatedMultiCreate_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx) {
  AH_JOB_CREATESEPAMULTIDEBIT *aj;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_CREATESEPAMULTIDEBIT, j);
  assert(aj);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing");
  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  /* search for "SepaDebitDatedMultiCreateResponse" */
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
			  "data/SepaDebitDatedMultiCreateResponse");
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
int AH_Job_SepaDebitDatedMultiCreate_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod) {
  AH_JOB_CREATESEPAMULTIDEBIT *aj;
  const AB_TRANSACTION *t;
  const char *s;
  int tanVer=AH_JOB_TANVER_1_4;

  DBG_ERROR(AQHBCI_LOGDOMAIN, "AddChallengeParams function called");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_CREATESEPAMULTIDEBIT, j);
  assert(aj);

  /* get data from first transaction */
  t=AH_Job_GetFirstTransfer(j);
  if (t==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction");
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
    rv=AH_HHD14_AddChallengeParams_32(j,
                                      AH_Job_GetTransferCount(j),
                                      aj->sumValues,
                                      AB_Transaction_GetLocalIban(t),
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



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaDebitDatedMultiCreate_Prepare(AH_JOB *j) {
  AH_JOB_CREATESEPAMULTIDEBIT *aj;
  GWEN_DB_NODE *dbArgs;
  AB_BANKING *ab;
  AB_USER *u;
  int rv;
  const char *profileName="";
  const char *descriptor="";
  const char *s;
  AB_IMEXPORTER_CONTEXT *ioc;
  AB_TRANSACTION *t;
  GWEN_BUFFER *dbuf;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing transfers");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_CREATESEPAMULTIDEBIT, j);
  assert(aj);

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  u=AH_Job_GetUser(j);
  assert(u);

  dbArgs=AH_Job_GetArguments(j);

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


  /* set data in job */
  ioc=AB_ImExporterContext_new();

  /* add transactions to ImExporter context, calculate sum on the fly */
  AB_Value_free(aj->sumValues);
  aj->sumValues=AB_Value_new();
  t=AH_Job_GetFirstTransfer(j);
  while(t) {
    AB_TRANSACTION *cpy;
    const AB_VALUE *v;

    v=AB_Transaction_GetValue(t);
    if (v)
      AB_Value_AddValue(aj->sumValues, v);

    cpy=AB_Transaction_dup(t);
    AB_ImExporterContext_AddTransaction(ioc, cpy);

    t=AB_Transaction_List_Next(t);
  }


  /* export ImExporterContext to SEPA */
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

  return 0;
}



