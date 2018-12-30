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

#include <stdlib.h>
#include <assert.h>
#include <string.h>



/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_SepaStandingOrderCreate_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;

  j=AH_Job_TransferBase_new("JobSepaStandingOrderCreate",
                            AB_Transaction_TypeTransfer,
                            AB_Transaction_SubTypeStandingOrder,
                            pro, u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 35);
  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandSepaCreateStandingOrder);

  /* overwrite some virtual functions */
  AH_Job_SetPrepareFn(j, AH_Job_SepaStandingOrderCreate_Prepare);
  AH_Job_SetAddChallengeParamsFn(j, AH_Job_TransferBase_AddChallengeParams35);
  AH_Job_SetGetLimitsFn(j, AH_Job_TransferBase_GetLimits_SepaStandingOrder);
  AH_Job_SetHandleCommandFn(j, AH_Job_TransferBase_HandleCommand_SepaStandingOrder);

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaStandingOrderCreate_Prepare(AH_JOB *j) {
  GWEN_DB_NODE *dbArgs;
  GWEN_DB_NODE *profile;
  int rv;
  const GWEN_DATE *da;
  GWEN_BUFFER *tbuf;
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

  /* execution date */
  tbuf=GWEN_Buffer_new(0, 16, 0, 1);
  da=AB_Transaction_GetDate(t);
  if (da) {
    GWEN_Date_toStringWithTemplate(da, "YYYYMMDD", tbuf);
    GWEN_DB_SetCharValue(dbArgs,
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "xnextExecutionDate",
                         GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_Reset(tbuf);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing execution date.");
  }

  /* first execution date */
  da=AB_Transaction_GetFirstDate(t);
  if (da) {
    GWEN_Date_toStringWithTemplate(da, "YYYYMMDD", tbuf);
    GWEN_DB_SetCharValue(dbArgs,
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "details/xfirstExecutionDate",
                         GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_Reset(tbuf);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing first execution date.");
  }

  /* last execution date */
  da=AB_Transaction_GetLastDate(t);
  if (da) {
    GWEN_Date_toStringWithTemplate(da, "YYYYMMDD", tbuf);
    GWEN_DB_SetCharValue(dbArgs,
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "details/xlastExecutionDate",
                         GWEN_Buffer_GetStart(tbuf));
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing last execution date.");
  }
  GWEN_Buffer_free(tbuf);

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
