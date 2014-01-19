/***************************************************************************
 begin       : Wed Jan 15 2014
 copyright   : (C) 2014 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobsepastandingordercreate_p.h"
#include "jobtransferbase_l.h"
#include "aqhbci_l.h"
#include "accountjob_l.h"
#include "job_l.h"
#include "provider_l.h"
#include "hhd_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/text.h>

#include <aqbanking/jobsepatransfer_be.h>
#include <aqbanking/job_be.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>






/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_SepaStandingOrderCreate_new(AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;

  j=AH_Job_TransferBase_new("JobSepaStandingOrderCreate",
                            AB_Transaction_TypeSepaTransfer,
                            AB_Transaction_SubTypeStandingOrder,
                            u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 35);

  /* overwrite some virtual functions */
  AH_Job_SetPrepareFn(j, AH_Job_SepaStandingOrderCreate_Prepare);
  AH_Job_SetAddChallengeParamsFn(j, AH_Job_SepaStandingOrderCreate_AddChallengeParams);

  /* overwrite virtual functions of transferBase class */
  AH_Job_TransferBase_SetExchangeParamsFn(j, AH_Job_SepaStandingOrderCreate_ExchangeParams);
  AH_Job_TransferBase_SetExchangeArgsFn(j, AH_Job_SepaStandingOrderCreate_ExchangeArgs);

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaStandingOrderCreate_ExchangeParams(AH_JOB *j, AB_JOB *bj,
                                                  AB_IMEXPORTER_CONTEXT *ctx) {
  AB_TRANSACTION_LIMITS *lim;
  GWEN_DB_NODE *dbParams;
  const char *s;
  int i;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging params");

  dbParams=AH_Job_GetParams(j);
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Have this parameters to exchange:");
  if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Debug)
    GWEN_DB_Dump(dbParams, 2);

  /* set some default limits */
  lim=AB_TransactionLimits_new();
  AB_TransactionLimits_SetMaxLenPurpose(lim, 35);
  AB_TransactionLimits_SetMaxLinesPurpose(lim, 4);
  AB_TransactionLimits_SetMaxLenRemoteName(lim, 70);
  AB_TransactionLimits_SetMaxLinesRemoteName(lim, 1);

  /* get specific limits for creation of standing orders */
  s=GWEN_DB_GetCharValue(dbParams, "AllowedTurnusMonths", 0, 0);
  if (s && *s) {
    AB_TransactionLimits_SetAllowMonthly(lim, 1);
    while(*s) {
      char buf[3];
      const char *x;

      buf[2]=0;
      strncpy(buf, s, 2);
      x=buf;
      if (*x=='0')
        x++;

      AB_TransactionLimits_AddValuesCycleMonth(lim, x, 0);
      s+=2;
    } /* while */
    GWEN_StringList_Sort(AB_TransactionLimits_GetValuesCycleMonth(lim),
                         1, GWEN_StringList_SortModeInt);
  }
  else
    AB_TransactionLimits_SetAllowMonthly(lim, -1);

  s=GWEN_DB_GetCharValue(dbParams, "AllowedMonthDays", 0, 0);
  if (s && *s) {
    while(*s) {
      char buf[3];
      const char *x;

      buf[2]=0;
      strncpy(buf, s, 2);
      x=buf;
      if (*x=='0')
        x++;
      AB_TransactionLimits_AddValuesExecutionDayMonth(lim, x, 0);
      s+=2;
    } /* while */
    GWEN_StringList_Sort(AB_TransactionLimits_GetValuesExecutionDayMonth(lim),
                         1, GWEN_StringList_SortModeInt);
  }

  s=GWEN_DB_GetCharValue(dbParams, "AllowedTurnusWeeks", 0, 0);
  if (s && *s) {
    AB_TransactionLimits_SetAllowWeekly(lim, 1);
    while(*s) {
      char buf[3];
      const char *x;

      buf[2]=0;
      strncpy(buf, s, 2);
      x=buf;
      if (*x=='0')
        x++;
      AB_TransactionLimits_AddValuesCycleWeek(lim, x, 0);
      s+=2;
    } /* while */
    GWEN_StringList_Sort(AB_TransactionLimits_GetValuesCycleWeek(lim),
                         1, GWEN_StringList_SortModeInt);
  }
  else
    AB_TransactionLimits_SetAllowWeekly(lim, -1);

  s=GWEN_DB_GetCharValue(dbParams, "AllowedWeekDays", 0, 0);
  if (s && *s) {
    while(*s) {
      char buf[2];
      const char *x;

      buf[0]=*s;
      buf[1]=0;
      x=buf;
      if (*x=='0')
        x++;
      AB_TransactionLimits_AddValuesExecutionDayWeek(lim, x, 0);
      s++;
    } /* while */
    GWEN_StringList_Sort(AB_TransactionLimits_GetValuesExecutionDayWeek(lim),
                         1, GWEN_StringList_SortModeInt);
  }

  i=GWEN_DB_GetIntValue(dbParams, "minDelay", 0, 0);
  AB_TransactionLimits_SetMinValueSetupTime(lim, i);

  i=GWEN_DB_GetIntValue(dbParams, "maxDelay", 0, 0);
  AB_TransactionLimits_SetMaxValueSetupTime(lim, i);


  AB_Job_SetFieldLimits(bj, lim);
  AB_TransactionLimits_free(lim);

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaStandingOrderCreate_ExchangeArgs(AH_JOB *j, AB_JOB *bj, AB_IMEXPORTER_CONTEXT *ctx) {
  const AB_TRANSACTION_LIMITS *lim=NULL;
  AB_BANKING *ab;
  const AB_TRANSACTION *t=NULL;
  AB_TRANSACTION *tCopy=NULL;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging args");

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

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

  rv=AB_Transaction_CheckFirstExecutionDateAgainstLimits(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
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
  AB_BANKING *ab;
  AB_USER *u;
  int rv;
  const char *profileName="";
  const char *descriptor="";
  const GWEN_TIME *ti;
  const char *s;
  AB_TRANSACTION *t;
  GWEN_BUFFER *dbuf;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing transfer");

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  u=AH_Job_GetUser(j);
  assert(u);

  dbArgs=AH_Job_GetArguments(j);

  /* choose from HISPAS */
  /* first check for any descriptor for pain 001.002.03 */
  s=AH_User_FindSepaDescriptor(u, "*001.002.03*");
  if (s) {
    profileName="001_002_03";
    descriptor=s;
  }
  else {
    /* look for pain 001.001.02 */
    s=AH_User_FindSepaDescriptor(u, "*001.001.02*");
    if (s) {
      profileName="ccm";
      descriptor=s;
    }
  }

  /* check for valid descriptor */
  if (!(descriptor && *descriptor)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No SEPA descriptor found, please update your SEPA account information");
    return GWEN_ERROR_GENERIC;
  }
  DBG_INFO(AQHBCI_LOGDOMAIN, "Using SEPA descriptor %s", descriptor);


  /* add transactions to ImExporter context */
  t=AH_Job_GetFirstTransfer(j);
  if (t==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction in job");
    assert(t); /* debug */
    return GWEN_ERROR_INTERNAL;
  }

  /* export transfers to SEPA */
  dbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=AH_Job_TransferBase_SepaExportTransactions(j, profileName, dbuf);
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

  /* first execution date */
  ti=AB_Transaction_GetFirstExecutionDate(t);
  if (ti) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 16, 0, 1);
    GWEN_Time_toString(ti, "YYYYMMDD", tbuf);
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

  return 0;
}







