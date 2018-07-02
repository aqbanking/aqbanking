/***************************************************************************
 begin       : Wed Jan 15 2014
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobsepastandingordercreate_l.h"
#include "jobtransferbase_l.h"
#include "aqhbci_l.h"
#include "accountjob_l.h"
#include "job_l.h"
#include "user_l.h"
#include "provider_l.h"
#include "hhd_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/text.h>

#include <aqbanking/jobsepatransfer.h>
#include <aqbanking/job_be.h>
#include <aqbanking/transactionfns.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_SepaStandingOrderCreate_new(AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;

  j=AH_Job_TransferBase_new("JobSepaStandingOrderCreate",
                            AB_Transaction_TypeTransfer,
                            AB_Transaction_SubTypeStandingOrder,
                            u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 35);
  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandSepaCreateStandingOrder);

  /* overwrite some virtual functions */
  AH_Job_SetPrepareFn(j, AH_Job_SepaStandingOrderCreate_Prepare);
  AH_Job_SetAddChallengeParamsFn(j, AH_Job_SepaStandingOrderCreate_AddChallengeParams);
  AH_Job_SetGetLimitsFn(j, AH_Job_TransferBase_GetLimits_SepaStandingOrder);

  /* overwrite virtual functions of transferBase class */
  AH_Job_TransferBase_SetExchangeArgsFn(j, AH_Job_SepaStandingOrderCreate_ExchangeArgs);

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaStandingOrderCreate_ExchangeArgs(AH_JOB *j, AB_JOB *bj, AB_IMEXPORTER_CONTEXT *ctx) {
  const AB_TRANSACTION_LIMITS *lim=NULL;
  AB_BANKING *ab;
  const AB_TRANSACTION *t=NULL;
  AB_TRANSACTION *tCopy=NULL;
  int rv;
  AB_USER *u;
  uint32_t uflags;
  const char *s;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging args");

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  u=AH_Job_GetUser(j);
  assert(u);

  uflags=AH_User_GetFlags(u);

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
  rv=AB_Transaction_CheckForSepaConformity(t, (uflags & AH_USER_FLAGS_USE_STRICT_SEPA_CHARSET)?1:0);
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

  rv=AB_Transaction_CheckRecurrenceAgainstLimits(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* CheckFirstExecutionDateAgainstLimits for standingordercreate only */
  s=AB_Transaction_GetFiId(t);
  if (s) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
           "Check FirstExecutionDate for delete or modify DISABLED");
  }
  else {
    rv=AB_Transaction_CheckFirstExecutionDateAgainstLimits(t, lim);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }


  tCopy=AB_Transaction_dup(t);

  /* set group id so the application can now which transfers went together in one setting */
  AB_Transaction_SetGroupId(tCopy, AH_Job_GetId(j));

  /* store validated transaction in job */
  AB_Job_SetTransaction(bj, tCopy);

  /* store copy of transaction for later */
  AH_Job_AddTransfer(j, tCopy);

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaStandingOrderCreate_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod) {
  const AB_TRANSACTION *t;
  const char *s;
  int tanVer=AH_JOB_TANVER_1_4;

  DBG_ERROR(AQHBCI_LOGDOMAIN, "AddChallengeParams function called");

  t=AH_Job_GetFirstTransfer(j);
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
    rv=AH_HHD14_AddChallengeParams_35(j,
                                      AB_Transaction_GetValue(t),
                                      AB_Transaction_GetRemoteIban(t));
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
int AH_Job_SepaStandingOrderCreate_Prepare(AH_JOB *j) {
  GWEN_DB_NODE *dbArgs;
  GWEN_DB_NODE *profile;
  int rv;
  const GWEN_DATE *da;
  const char *s;
  AB_TRANSACTION *t;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing transfer");

  dbArgs=AH_Job_GetArguments(j);

  t=AH_Job_GetFirstTransfer(j);
  if (t==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction in job");
    assert(t); /* debug */
    return GWEN_ERROR_INTERNAL;
  }

  /* find the right profile to produce pain.001 messages */
  profile=AH_Job_FindSepaProfile(j, "001*", AH_User_GetSepaTransferProfile(AH_Job_GetUser(j)));
  if (!profile) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No suitable profile found");
    return GWEN_ERROR_GENERIC;
  }

  /* export transfers to SEPA */
  rv=AH_Job_TransferBase_SepaExportTransactions(j, profile);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* first execution date */
  da=AB_Transaction_GetFirstDate(t);
  if (da) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 16, 0, 1);
    GWEN_Date_toStringWithTemplate(da, "YYYYMMDD", tbuf);
    GWEN_DB_SetCharValue(dbArgs,
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "details/xfirstExecutionDate",
                         GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing first execution date.");
  }

  /* period */
  switch(AB_Transaction_GetPeriod(t)) {
  case AB_Transaction_PeriodMonthly: s="M"; break;
  case AB_Transaction_PeriodWeekly:  s="W"; break;
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unsupported period %d",
              AB_Transaction_GetPeriod(t));
    return GWEN_ERROR_INVALID;
  }
  GWEN_DB_SetCharValue(dbArgs,
                       GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "details/xperiod",
                       s);

  /* cycle */
  GWEN_DB_SetIntValue(dbArgs,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "details/cycle",
                      AB_Transaction_GetCycle(t));

  /* execution day */
  GWEN_DB_SetIntValue(dbArgs,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "details/executionDay",
                      AB_Transaction_GetExecutionDay(t));


  /* SET fiId, if present */
  s=AB_Transaction_GetFiId(t);
  if (s) {
    GWEN_DB_SetCharValue(dbArgs,
                       GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "fiId",
                       AB_Transaction_GetFiId(t));
  }

  return 0;
}
