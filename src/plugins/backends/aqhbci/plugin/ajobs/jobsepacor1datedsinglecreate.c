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


#include "jobsepacor1datedsinglecreate_p.h"
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
AH_JOB *AH_Job_SepaCor1DebitDatedSingleCreate_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;

  j=AH_Job_TransferBase_new("JobSepaCor1DebitDatedSingleCreate",
                            AB_Transaction_TypeDebitNote,
                            AB_Transaction_SubTypeStandard,
                            pro, u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 29);
  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandSepaFlashDebitNote);

  /* overwrite some virtual functions */
  AH_Job_SetPrepareFn(j, AH_Job_SepaCor1DebitDatedSingleCreate_Prepare);
  AH_Job_SetAddChallengeParamsFn(j, AH_Job_TransferBase_AddChallengeParams29);
  AH_Job_SetGetLimitsFn(j, AH_Job_TransferBase_GetLimits_SepaDated);
  AH_Job_SetHandleCommandFn(j, AH_Job_TransferBase_HandleCommand_SepaDatedDebit);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaCor1DebitDatedSingleCreate_Prepare(AH_JOB *j) {
  GWEN_DB_NODE *profile;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing transfer");

  /* find the right profile to produce pain.008 messages */
  profile=AH_Job_FindSepaProfile(j, "008*", AH_User_GetSepaDebitNoteProfile(AH_Job_GetUser(j)));
  if (!profile) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No suitable profile found");
    return GWEN_ERROR_GENERIC;
  }

  /* adjust parameters for COR1 transactions */
  GWEN_DB_SetCharValue(profile, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "LocalInstrumentSEPACode", "COR1");

  /* export transfers to SEPA */
  rv=AH_Job_TransferBase_SepaExportTransactions(j, profile);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



