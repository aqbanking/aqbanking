/***************************************************************************
    begin       : Tue Dec 31 2013
    copyright   : (C) 2018 by Martin Preuss
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

#include <aqbanking/jobsepadebitnote.h>
#include <aqbanking/job_be.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_SepaDebitDatedSingleCreate_new(AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;

  j=AH_Job_TransferBase_new("JobSepaDebitDatedSingleCreate",
                            AB_Transaction_TypeDebitNote,
                            AB_Transaction_SubTypeStandard,
                            u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 29);
  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandSepaDebitNote);

  /* overwrite some virtual functions */
  AH_Job_SetPrepareFn(j, AH_Job_SepaDebitDatedSingleCreate_Prepare);
  AH_Job_SetAddChallengeParamsFn(j, AH_Job_SepaDebitDatedSingleCreate_AddChallengeParams);
  AH_Job_SetGetLimitsFn(j, AH_Job_TransferBase_GetLimits_SepaDated);

  /* overwrite virtual functions of transferBase class */
  AH_Job_TransferBase_SetExchangeArgsFn(j, AH_Job_TransferBase_ExchangeArgs_SepaDatedDebit);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  return j;
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





