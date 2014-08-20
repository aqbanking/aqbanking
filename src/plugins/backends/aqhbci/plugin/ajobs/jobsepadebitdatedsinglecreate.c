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

#include <aqbanking/jobsepadebitnote_be.h>
#include <aqbanking/job_be.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_SepaDebitDatedSingleCreate_new(AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;

  j=AH_Job_TransferBase_new("JobSepaDebitDatedSingleCreate",
                            AB_Transaction_TypeSepaDebitNote,
                            AB_Transaction_SubTypeStandard,
                            u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 29);

  /* overwrite some virtual functions */
  AH_Job_SetPrepareFn(j, AH_Job_SepaDebitDatedSingleCreate_Prepare);
  AH_Job_SetAddChallengeParamsFn(j, AH_Job_SepaDebitDatedSingleCreate_AddChallengeParams);

  /* overwrite virtual functions of transferBase class */
  AH_Job_TransferBase_SetExchangeParamsFn(j, AH_Job_SepaDebitDatedSingleCreate_ExchangeParams);
  AH_Job_TransferBase_SetExchangeArgsFn(j, AH_Job_TransferBase_ExchangeArgs_SepaDatedDebit);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaDebitDatedSingleCreate_ExchangeParams(AH_JOB *j, AB_JOB *bj,
                                                     AB_IMEXPORTER_CONTEXT *ctx) {
  AB_TRANSACTION_LIMITS *lim;
  GWEN_DB_NODE *dbParams;
  int i, i1, i2;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging params");

  dbParams=AH_Job_GetParams(j);
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Have this parameters to exchange:");
  if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Debug)
    GWEN_DB_Dump(dbParams, 2);

  /* set some default limits */
  lim=AB_TransactionLimits_new();
  AB_TransactionLimits_SetMaxLenPurpose(lim, 35);
  AB_TransactionLimits_SetMaxLinesPurpose(lim, 4);
  AB_TransactionLimits_SetMaxLenRemoteName(lim, 27);
  AB_TransactionLimits_SetMaxLinesRemoteName(lim, 1);

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

  AB_Job_SetFieldLimits(bj, lim);
  AB_TransactionLimits_free(lim);

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaDebitDatedSingleCreate_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod) {
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



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaDebitDatedSingleCreate_Prepare(AH_JOB *j) {
  GWEN_DB_NODE *profile;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing transfer");

  /* find the right profile to produce pain.008 messages */
  profile=AH_Job_FindSepaProfile(j, "008*", AH_User_GetSepaDebitNoteProfile(AH_Job_GetUser(j)));
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

  return 0;
}





