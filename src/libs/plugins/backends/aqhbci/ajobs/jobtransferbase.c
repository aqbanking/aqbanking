/***************************************************************************
    begin       : Tue Dec 31 2013
    copyright   : (C) 2022 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobtransferbase_p.h"
#include "aqhbci/aqhbci_l.h"
#include "aqhbci/joblayer/job_swift.h"
#include "aqhbci/joblayer/job_crypt.h"
#include "aqhbci/applayer/hhd_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/gui.h>

#include <assert.h>
#include <ctype.h>



GWEN_INHERIT(AH_JOB, AH_JOB_TRANSFERBASE);



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void GWENHYWFAR_CB _freeData(void *bp, void *p);
static int _process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int _handleResults(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);

/**
 * Set given status on all transfers and add copies of them to the given context.
 */
static void _setStatusOnTransfersAndAddToCtx(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx, AB_TRANSACTION_STATUS status);

static void _replaceCtrlCharsInPurpose(AB_TRANSACTION *t);
static void _setProfileName(AH_JOB *j, const char *s);
static void _setDescriptor(AH_JOB *j, const char *s);
static void _setLimitsCycleMonth(AB_TRANSACTION_LIMITS *lim, GWEN_DB_NODE *dbParams);
static void _setLimitsExecDaysOfMonth(AB_TRANSACTION_LIMITS *lim, GWEN_DB_NODE *dbParams);
static void _setLimitsCycleWeek(AB_TRANSACTION_LIMITS *lim, GWEN_DB_NODE *dbParams);
static void _setLimitsExecDaysOfWeek(AB_TRANSACTION_LIMITS *lim, GWEN_DB_NODE *dbParams);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



AH_JOB *AH_Job_TransferBase_new(const char *jobName,
                                AB_TRANSACTION_TYPE tt,
                                AB_TRANSACTION_SUBTYPE tst,
                                AB_PROVIDER *pro,
                                AB_USER *u,
                                AB_ACCOUNT *account)
{
  AH_JOB *j;
  AH_JOB_TRANSFERBASE *aj;

  j=AH_AccountJob_new(jobName, pro, u, account);
  if (!j)
    return 0;

  GWEN_NEW_OBJECT(AH_JOB_TRANSFERBASE, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_TRANSFERBASE, j, aj, _freeData);

  aj->transactionType=tt;
  aj->transactionSubType=tst;

  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, _process);
  AH_Job_SetHandleResultsFn(j, _handleResults);

  return j;
}



void GWENHYWFAR_CB _freeData(void *bp, void *p)
{
  AH_JOB_TRANSFERBASE *aj;

  aj=(AH_JOB_TRANSFERBASE *)p;
  free(aj->localInstrumentationCode);
  free(aj->profileName);
  free(aj->fiid);

  GWEN_FREE_OBJECT(aj);
}



const char *AH_Job_TransferBase_GetFiid(const AH_JOB *j)
{
  AH_JOB_TRANSFERBASE *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TRANSFERBASE, j);
  assert(aj);

  return aj->fiid;
}



int AH_Job_TransferBase_SepaExportTransactions(AH_JOB *j)
{
  AH_JOB_TRANSFERBASE *aj;
  GWEN_DB_NODE *dbArgs;
  AB_BANKING *ab;
  const AB_TRANSACTION *t;
  int rv;
  AB_ACCOUNT *a;
  GWEN_DB_NODE *dbProfile;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exporting transaction");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TRANSFERBASE, j);
  assert(aj);

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  a=AH_AccountJob_GetAccount(j);
  assert(a);

  if (aj->profileName==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No profile set. SNH!!");
    return GWEN_ERROR_INTERNAL;
  }

  if (aj->descriptor==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No descriptor set. SNH!!");
    return GWEN_ERROR_INTERNAL;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Using SEPA descriptor %s and profile %s", aj->descriptor, aj->profileName);
  GWEN_Gui_ProgressLog2(0,
                        GWEN_LoggerLevel_Notice,
                        I18N("Using SEPA descriptor %s and profile %s"),
                        aj->descriptor, aj->profileName);

  dbProfile=AB_Banking_GetImExporterProfile(ab, "xml", aj->profileName);
  if (dbProfile==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Profile \"%s\" not found.", aj->profileName);
    return GWEN_ERROR_INTERNAL;
  }

  if (aj->localInstrumentationCode)
    GWEN_DB_SetCharValue(dbProfile, GWEN_DB_FLAGS_OVERWRITE_VARS, "LocalInstrumentSEPACode", aj->localInstrumentationCode);

  /* set data in job */
  t=AH_Job_GetFirstTransfer(j);
  if (t) {
    AB_IMEXPORTER_CONTEXT *ioc;
    AB_TRANSACTION *cpy;
    GWEN_BUFFER *dbuf;

    /* add copies of transfers */
    ioc=AB_ImExporterContext_new();
    while (t) {
      cpy=AB_Transaction_dup(t);
      _replaceCtrlCharsInPurpose(cpy);
      AB_Transaction_SetUniqueAccountId(cpy, AB_Account_GetUniqueId(a));
      AB_ImExporterContext_AddTransaction(ioc, cpy);
      t=AB_Transaction_List_Next(t);
    }

    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=AB_Banking_ExportToBuffer(ab, "xml", ioc, dbuf, dbProfile);
    AB_ImExporterContext_free(ioc);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(dbuf);
      GWEN_DB_Group_free(dbProfile);
      return rv;
    }

    /* store descriptor */
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "descriptor", aj->descriptor);
    /* store transfer */
    GWEN_DB_SetBinValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "transfer", GWEN_Buffer_GetStart(dbuf),
                        GWEN_Buffer_GetUsedBytes(dbuf));
    GWEN_Buffer_free(dbuf);
    GWEN_DB_Group_free(dbProfile);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction in job");
    GWEN_DB_Group_free(dbProfile);
    return GWEN_ERROR_INTERNAL;
  }

  return 0;
}



void _replaceCtrlCharsInPurpose(AB_TRANSACTION *trans)
{
  const char *s;

  s=AB_Transaction_GetPurpose(trans);
  if (s && *s) {
    GWEN_BUFFER *buf;
    char *t;

    buf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(buf, s);
    t=GWEN_Buffer_GetStart(buf);
    while (*t) {
      if (iscntrl(*t))
        *t=' ';
      t++;
    }
    AB_Transaction_SetPurpose(trans, GWEN_Buffer_GetStart(buf));
    GWEN_Buffer_free(buf);
  }
}



int AH_Job_TransferBase_GetLimits_SepaUndated(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits)
{
  AB_TRANSACTION_LIMITS *lim;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging params");

  /* set some default limits */
  lim=AB_TransactionLimits_new();
  AB_TransactionLimits_SetCommand(lim, AH_Job_GetSupportedCommand(j));

  AB_TransactionLimits_SetMaxLenPurpose(lim, 35);
  AB_TransactionLimits_SetMaxLinesPurpose(lim, 4);
  AB_TransactionLimits_SetMaxLenRemoteName(lim, 70);

  AB_TransactionLimits_SetNeedDate(lim, -1);

  *pLimits=lim;

  return 0;
}



int AH_Job_TransferBase_GetLimits_SepaDated(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits)
{
  AB_TRANSACTION_LIMITS *lim;
  GWEN_DB_NODE *dbParams;
  int i, i1, i2;

  dbParams=AH_Job_GetParams(j);

  lim=AB_TransactionLimits_new();
  AB_TransactionLimits_SetCommand(lim, AH_Job_GetSupportedCommand(j));

  AB_TransactionLimits_SetMaxLenPurpose(lim, 35);
  AB_TransactionLimits_SetMaxLinesPurpose(lim, 4);
  AB_TransactionLimits_SetMaxLenRemoteName(lim, 70);

  AB_TransactionLimits_SetNeedDate(lim, 1);

  /* set info from BPD */
  i1=GWEN_DB_GetIntValue(dbParams, "minDelay_FNAL_RCUR", 0, 0);
  AB_TransactionLimits_SetMinValueSetupTimeRecurring(lim, i1);
  AB_TransactionLimits_SetMinValueSetupTimeFinal(lim, i1);

  i2=GWEN_DB_GetIntValue(dbParams, "minDelay_FRST_OOFF", 0, 0);
  AB_TransactionLimits_SetMinValueSetupTimeFirst(lim, i2);
  AB_TransactionLimits_SetMinValueSetupTimeOnce(lim, i2);

  /* combine into minimum values for older apps */
  i=(i1>i2)?i1:i2;
  AB_TransactionLimits_SetMinValueSetupTime(lim, i);

  i1=GWEN_DB_GetIntValue(dbParams, "maxDelay_FNAL_RCUR", 0, 0);
  AB_TransactionLimits_SetMaxValueSetupTimeRecurring(lim, i1);
  AB_TransactionLimits_SetMinValueSetupTimeFinal(lim, i1);

  i2=GWEN_DB_GetIntValue(dbParams, "maxDelay_FRST_OOFF", 0, 0);
  AB_TransactionLimits_SetMaxValueSetupTimeFirst(lim, i2);
  AB_TransactionLimits_SetMaxValueSetupTimeOnce(lim, i2);

  /* combine into minimum values for older apps */
  i=(i1<i2)?i1:i2;
  AB_TransactionLimits_SetMaxValueSetupTime(lim, i);

  /* nothing more to set for this kind of job */
  *pLimits=lim;
  return 0;
}



int AH_Job_TransferBase_GetLimits_SepaStandingOrder(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits)
{
  AB_TRANSACTION_LIMITS *lim;
  GWEN_DB_NODE *dbParams;
  int i;

  dbParams=AH_Job_GetParams(j);
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Have this parameters to exchange:");
  if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Debug)
    GWEN_DB_Dump(dbParams, 2);

  /* set some default limits */
  lim=AB_TransactionLimits_new();
  AB_TransactionLimits_SetCommand(lim, AH_Job_GetSupportedCommand(j));

  if (AH_Job_GetSupportedCommand(j)!=AB_Transaction_CommandSepaDeleteStandingOrder) {
    AB_TransactionLimits_SetMaxLenPurpose(lim, 35);
    AB_TransactionLimits_SetMaxLinesPurpose(lim, 4);
    AB_TransactionLimits_SetMaxLenRemoteName(lim, 70);

    /* get specific limits for creation of standing orders */
    _setLimitsCycleMonth(lim, dbParams);
    _setLimitsExecDaysOfMonth(lim, dbParams);

    _setLimitsCycleWeek(lim, dbParams);
    _setLimitsExecDaysOfWeek(lim, dbParams);
  }

  i=GWEN_DB_GetIntValue(dbParams, "mindelay", 0, 0);
  AB_TransactionLimits_SetMinValueSetupTime(lim, i);

  i=GWEN_DB_GetIntValue(dbParams, "maxdelay", 0, 0);
  AB_TransactionLimits_SetMaxValueSetupTime(lim, i);

  /* nothing more to set for this kind of job */
  *pLimits=lim;
  return 0;
}






int AH_Job_TransferBase_HandleCommand_SepaUndated(AH_JOB *j, const AB_TRANSACTION *t)
{
  AB_TRANSACTION_LIMITS *lim=NULL;
  AB_BANKING *ab;
  AB_TRANSACTION *tCopy=NULL;
  int rv;
  AB_USER *u;
  uint32_t uflags;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging args");

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  u=AH_Job_GetUser(j);
  assert(u);

  uflags=AH_User_GetFlags(u);

  /* get limits and transaction */
  if (t==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction in job");
    return GWEN_ERROR_INVALID;
  }
  rv=AH_Job_GetLimits(j, &lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* validate transaction */
  rv=AB_Banking_CheckTransactionForSepaConformity(t, (uflags & AH_USER_FLAGS_USE_STRICT_SEPA_CHARSET)?1:0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_TransactionLimits_free(lim);
    return rv;
  }

  rv=AB_Banking_CheckTransactionAgainstLimits_Purpose(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_TransactionLimits_free(lim);
    return rv;
  }

  rv=AB_Banking_CheckTransactionAgainstLimits_Names(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_TransactionLimits_free(lim);
    return rv;
  }
  AB_TransactionLimits_free(lim);

  tCopy=AB_Transaction_dup(t);

  /* set group id so the application can know which transfers went together in one setting */
  AB_Transaction_SetGroupId(tCopy, AH_Job_GetId(j));

  /* store copy of transaction for later */
  AH_Job_AddTransfer(j, tCopy);

  return 0;
}



int AH_Job_TransferBase_HandleCommand_SepaDated(AH_JOB *j, const AB_TRANSACTION *t)
{
  AB_TRANSACTION_LIMITS *lim=NULL;
  AB_BANKING *ab;
  AB_TRANSACTION *tCopy=NULL;
  int rv;
  AB_USER *u;
  uint32_t uflags;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging args");

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  u=AH_Job_GetUser(j);
  assert(u);

  uflags=AH_User_GetFlags(u);

  /* get limits and transaction */
  if (t==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction in job");
    return GWEN_ERROR_INVALID;
  }
  rv=AH_Job_GetLimits(j, &lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* validate transaction */
  rv=AB_Banking_CheckTransactionForSepaConformity(t, (uflags & AH_USER_FLAGS_USE_STRICT_SEPA_CHARSET)?1:0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_TransactionLimits_free(lim);
    return rv;
  }

  rv=AB_Banking_CheckTransactionAgainstLimits_Purpose(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_TransactionLimits_free(lim);
    return rv;
  }

  rv=AB_Banking_CheckTransactionAgainstLimits_Names(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_TransactionLimits_free(lim);
    return rv;
  }

  rv=AB_Banking_CheckTransactionAgainstLimits_Date(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_TransactionLimits_free(lim);
    return rv;
  }
  AB_TransactionLimits_free(lim);


  tCopy=AB_Transaction_dup(t);

  /* set group id so the application can know which transfers went together in one setting */
  AB_Transaction_SetGroupId(tCopy, AH_Job_GetId(j));

  /* store copy of transaction for later */
  AH_Job_AddTransfer(j, tCopy);

  return 0;
}



int AH_Job_TransferBase_HandleCommand_SepaDatedDebit(AH_JOB *j, const AB_TRANSACTION *t)
{
  AB_TRANSACTION_LIMITS *lim=NULL;
  AB_BANKING *ab;
  AB_TRANSACTION *tCopy=NULL;
  int rv;
  AB_USER *u;
  uint32_t uflags;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging args");

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  u=AH_Job_GetUser(j);
  assert(u);

  uflags=AH_User_GetFlags(u);

  /* get limits and transaction */
  if (t==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction in job");
    return GWEN_ERROR_INVALID;
  }
  rv=AH_Job_GetLimits(j, &lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* validate transaction */
  rv=AB_Banking_CheckTransactionForSepaConformity(t, (uflags & AH_USER_FLAGS_USE_STRICT_SEPA_CHARSET)?1:0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_TransactionLimits_free(lim);
    return rv;
  }

  rv=AB_Banking_CheckTransactionAgainstLimits_Purpose(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_TransactionLimits_free(lim);
    return rv;
  }

  rv=AB_Banking_CheckTransactionAgainstLimits_Names(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_TransactionLimits_free(lim);
    return rv;
  }

  rv=AB_Banking_CheckTransactionAgainstLimits_Date(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_TransactionLimits_free(lim);
    return rv;
  }
  AB_TransactionLimits_free(lim);


  tCopy=AB_Transaction_dup(t);

  /* set group id so the application can know which transfers went together in one setting */
  AB_Transaction_SetGroupId(tCopy, AH_Job_GetId(j));

  /* store copy of transaction for later */
  AH_Job_AddTransfer(j, tCopy);

  return 0;
}



int AH_Job_TransferBase_HandleCommand_SepaStandingOrder(AH_JOB *j, const AB_TRANSACTION *t)
{
  AB_TRANSACTION_LIMITS *lim=NULL;
  AB_BANKING *ab;
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
  if (t==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction in job");
    return GWEN_ERROR_INVALID;
  }
  rv=AH_Job_GetLimits(j, &lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* validate transaction */
  rv=AB_Banking_CheckTransactionForSepaConformity(t, (uflags & AH_USER_FLAGS_USE_STRICT_SEPA_CHARSET)?1:0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_TransactionLimits_free(lim);
    return rv;
  }

  rv=AB_Banking_CheckTransactionAgainstLimits_Purpose(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_TransactionLimits_free(lim);
    return rv;
  }

  rv=AB_Banking_CheckTransactionAgainstLimits_Names(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_TransactionLimits_free(lim);
    return rv;
  }

  rv=AB_Banking_CheckTransactionAgainstLimits_Recurrence(t, lim);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* CheckFirstExecutionDateAgainstLimits for standingordercreate only */
  s=AB_Transaction_GetFiId(t);
  if (s) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Check FirstExecutionDate for delete or modify DISABLED");
  }
  else {
    rv=AB_Banking_CheckTransactionAgainstLimits_ExecutionDate(t, lim);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  AB_TransactionLimits_free(lim);


  tCopy=AB_Transaction_dup(t);

  /* set group id so the application can know which transfers went together in one setting */
  AB_Transaction_SetGroupId(tCopy, AH_Job_GetId(j));

  /* store copy of transaction for later */
  AH_Job_AddTransfer(j, tCopy);

  return 0;
}



int AH_Job_TransferBase_Prepare_SepaStandingOrder(AH_JOB *j)
{
  GWEN_DB_NODE *dbArgs;
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

  /* select pain profile from group "001" */
  rv=AH_Job_TransferBase_SelectPainProfile(j, 1);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* export transfers to SEPA */
  rv=AH_Job_TransferBase_SepaExportTransactions(j);
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
  switch (AB_Transaction_GetPeriod(t)) {
  case AB_Transaction_PeriodMonthly:
    s="M";
    break;
  case AB_Transaction_PeriodWeekly:
    s="W";
    break;
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



int AH_Job_TransferBase_AddChallengeParams29(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod)
{
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
    DBG_INFO(AQHBCI_LOGDOMAIN, "TAN version is 1.3 (%s)", s);
    tanVer=AH_JOB_TANVER_1_3;
  }

  if (tanVer==AH_JOB_TANVER_1_4) {
    int rv;

    DBG_INFO(AQHBCI_LOGDOMAIN, "TAN version is 1.4.x");
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



int AH_Job_TransferBase_AddChallengeParams35(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod)
{
  const AB_TRANSACTION *t;
  const char *s;
  int tanVer=AH_JOB_TANVER_1_4;

  t=AH_Job_GetFirstTransfer(j);
  if (t==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No validated transaction");
    return GWEN_ERROR_INVALID;
  }

  s=GWEN_DB_GetCharValue(dbMethod, "zkaTanVersion", 0, NULL);
  if (s && *s && strncasecmp(s, "1.3", 3)==0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "TAN version is 1.3 (%s)", s);
    tanVer=AH_JOB_TANVER_1_3;
  }

  if (tanVer==AH_JOB_TANVER_1_4) {
    int rv;

    DBG_INFO(AQHBCI_LOGDOMAIN, "TAN version is 1.4.x");
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



void _setStatusOnTransfersAndAddToCtx(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx, AB_TRANSACTION_STATUS status)
{
  AH_JOB_TRANSFERBASE *aj;
  const AB_TRANSACTION *t;
  AB_ACCOUNT *a;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TRANSFERBASE, j);
  assert(aj);

  a=AH_AccountJob_GetAccount(j);
  assert(a);

  t=AH_Job_GetFirstTransfer(j);
  while (t) {
    AB_TRANSACTION *cpy;

    cpy=AB_Transaction_dup(t);
    AB_Transaction_SetFiId(cpy, aj->fiid);
    AB_Transaction_SetStatus(cpy, status);
    AB_Transaction_SetType(cpy, aj->transactionType);
    AB_Transaction_SetSubType(cpy, aj->transactionSubType);
    if (AB_Transaction_GetDate(cpy)==NULL) {
      GWEN_DATE *dt;

      dt=GWEN_Date_CurrentDate();
      AB_Transaction_SetDate(cpy, dt);
      GWEN_Date_free(dt);
    }

    AB_Transaction_SetUniqueAccountId(cpy, AB_Account_GetUniqueId(a));

    AB_ImExporterContext_AddTransaction(ctx, cpy);      /* takes over cpy */

    t=AB_Transaction_List_Next(t);
  }
}



int _handleResults(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  AH_JOB_TRANSFERBASE *aj;
  AH_RESULT_LIST *rl;
  AH_RESULT *r;
  AB_TRANSACTION_STATUS tStatus;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TRANSFERBASE, j);
  assert(aj);

  rl=AH_Job_GetSegResults(j);
  assert(rl);

  r=AH_Result_List_First(rl);
  if (!r) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No segment results");
    tStatus=AB_Transaction_StatusError;
  }
  else {
    int has10=0;
    int has20=0;

    while (r) {
      int rcode;

      rcode=AH_Result_GetCode(r);
      if (rcode>=10 && rcode<=19) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "Has10: %d (%s)", rcode, AH_Result_GetText(r));
        has10=1;
      }
      else if (rcode>=20 && rcode <=29) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "Has20: %d (%s)", rcode, AH_Result_GetText(r));
        has20=1;
      }
      r=AH_Result_List_Next(r);
    }

    if (has20)
      tStatus=AB_Transaction_StatusAccepted;
    else if (has10)
      tStatus=AB_Transaction_StatusPending;
    else
      tStatus=AB_Transaction_StatusRejected;
  }

  _setStatusOnTransfersAndAddToCtx(j, ctx, tStatus);
  AH_Job_SetStatusOnCommands(j, tStatus);
  return 0;
}



int _process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  AH_JOB_TRANSFERBASE *aj;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  const char *responseName;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TRANSFERBASE, j);
  assert(aj);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing");
  responseName=AH_Job_GetResponseName(j);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  /* search for "TransferBaseSingleResponse" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while (dbCurr) {
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

    if (responseName && *responseName) {
      GWEN_DB_NODE *dbXA;

      dbXA=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
      if (dbXA)
        dbXA=GWEN_DB_GetGroup(dbXA, GWEN_PATH_FLAGS_NAMEMUSTEXIST, responseName);
      if (dbXA) {
        const char *s;

        s=GWEN_DB_GetCharValue(dbXA, "referenceId", 0, 0);
        if (s) {
          free(aj->fiid);
          aj->fiid=strdup(s);
        }
      }
    }
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }


  return 0;
}



int AH_Job_TransferBase_SelectPainProfile(AH_JOB *j, int version1)
{
  GWEN_DB_NODE *dbArgs;
  AB_SWIFT_DESCR_LIST *descrList;

  /* get arguments DB */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  /* check for "pain.VERSION1.*" */
  descrList=AH_Job_GetSwiftDescriptorsSupportedByUser(j, "pain", version1);
  if (descrList) {
    AB_SWIFT_DESCR *descr;
    const char *s;

    descr=AB_SwiftDescr_List_First(descrList);
    assert(descr);

    s=AB_SwiftDescr_GetAlias2(descr);
    DBG_INFO(AQHBCI_LOGDOMAIN, "Selecting PAIN format [%s]", s);
    _setDescriptor(j, s);
    _setProfileName(j, AB_SwiftDescr_GetAlias1(descr));

    AB_SwiftDescr_List_free(descrList);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No supported PAIN profile found, job not supported");
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



void AH_Job_TransferBase_SetLocalInstrumentationCode(AH_JOB *j, const char *s)
{
  AH_JOB_TRANSFERBASE *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TRANSFERBASE, j);
  assert(aj);

  if (aj->localInstrumentationCode)
    free(aj->localInstrumentationCode);
  if (s)
    aj->localInstrumentationCode=strdup(s);
  else
    aj->localInstrumentationCode=NULL;
}



void _setProfileName(AH_JOB *j, const char *s)
{
  AH_JOB_TRANSFERBASE *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TRANSFERBASE, j);
  assert(aj);

  if (aj->profileName)
    free(aj->profileName);
  if (s)
    aj->profileName=strdup(s);
  else
    aj->profileName=NULL;
}



void _setDescriptor(AH_JOB *j, const char *s)
{
  AH_JOB_TRANSFERBASE *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_TRANSFERBASE, j);
  assert(aj);

  if (aj->descriptor)
    free(aj->descriptor);
  if (s)
    aj->descriptor=strdup(s);
  else
    aj->descriptor=NULL;
}



void _setLimitsCycleMonth(AB_TRANSACTION_LIMITS *lim, GWEN_DB_NODE *dbParams)
{
  const char *s;

  AB_TransactionLimits_PresetValuesCycleMonth(lim, 0);
  AB_TransactionLimits_SetValuesCycleMonthUsed(lim, 0);
  s=GWEN_DB_GetCharValue(dbParams, "AllowedTurnusMonths", 0, 0);
  if (s && *s) {
    AB_TransactionLimits_SetAllowMonthly(lim, 1);

    while (*s) {
      /* read 2-digit number */
      if (isdigit(*s) && isdigit(s[1]))
	AB_TransactionLimits_ValuesCycleMonthAdd(lim, ((int)((s[0]-'0')*10))+((int)(s[1]-'0')));
      else {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Invalid number in params [%s]", s);
      }
      s+=2;
    } /* while */
  }
  else
    AB_TransactionLimits_SetAllowMonthly(lim, -1);
}



void _setLimitsExecDaysOfMonth(AB_TRANSACTION_LIMITS *lim, GWEN_DB_NODE *dbParams)
{
  const char *s;

  AB_TransactionLimits_PresetValuesExecutionDayMonth(lim, 0);
  AB_TransactionLimits_SetValuesExecutionDayMonthUsed(lim, 0);
  s=GWEN_DB_GetCharValue(dbParams, "AllowedMonthDays", 0, 0);
  if (s && *s) {
    while (*s) {
      /* read 2-digit number */
      if (isdigit(*s) && isdigit(s[1]))
	AB_TransactionLimits_ValuesExecutionDayMonthAdd(lim, ((int)((s[0]-'0')*10))+((int)(s[1]-'0')));
      else {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Invalid number in params [%s]", s);
      }
      s+=2;
    } /* while */
  }
}



void _setLimitsCycleWeek(AB_TRANSACTION_LIMITS *lim, GWEN_DB_NODE *dbParams)
{
  const char *s;

  AB_TransactionLimits_PresetValuesCycleWeek(lim, 0);
  AB_TransactionLimits_SetValuesCycleWeekUsed(lim, 0);
  s=GWEN_DB_GetCharValue(dbParams, "AllowedTurnusWeeks", 0, 0);
  if (s && *s) {
    AB_TransactionLimits_SetAllowWeekly(lim, 1);
    while (*s) {
      /* read 2-digit number */
      if (isdigit(*s) && isdigit(s[1]))
	AB_TransactionLimits_ValuesCycleWeekAdd(lim, ((int)((s[0]-'0')*10))+((int)(s[1]-'0')));
      else {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Invalid number in params [%s]", s);
      }
      s+=2;
    } /* while */
  }
  else
    AB_TransactionLimits_SetAllowWeekly(lim, -1);
}



void _setLimitsExecDaysOfWeek(AB_TRANSACTION_LIMITS *lim, GWEN_DB_NODE *dbParams)
{
  const char *s;

  AB_TransactionLimits_PresetValuesExecutionDayWeek(lim, 0);
  AB_TransactionLimits_SetValuesExecutionDayWeekUsed(lim, 0);
  s=GWEN_DB_GetCharValue(dbParams, "AllowedWeekDays", 0, 0);
  if (s && *s) {
    while (*s) {
      if (isdigit(*s))
	AB_TransactionLimits_ValuesExecutionDayWeekAdd(lim, (int)((*s)-'0')); /* 1-digit number */
      else {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Invalid number in params [%s]", s);
      }
      s++;
    }
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Invalid empty param starting with [%s])", s);
  }
}









