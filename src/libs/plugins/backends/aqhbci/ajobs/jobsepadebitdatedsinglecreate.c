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
#include "aqhbci/aqhbci_l.h"
#include "accountjob_l.h"
#include "aqhbci/joblayer/job_l.h"
#include "aqhbci/banking/user_l.h"
#include "aqhbci/banking/provider_l.h"
#include "aqhbci/applayer/hhd_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/text.h>

#include <assert.h>



/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_SepaDebitDatedSingleCreate_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account)
{
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;

  j=AH_Job_TransferBase_new("JobSepaDebitDatedSingleCreate",
                            AB_Transaction_TypeDebitNote,
                            AB_Transaction_SubTypeStandard,
                            pro, u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 29);
  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandSepaDebitNote);

  /* overwrite some virtual functions */
  AH_Job_SetPrepareFn(j, AH_Job_SepaDebitDatedSingleCreate_Prepare);
  AH_Job_SetAddChallengeParamsFn(j, AH_Job_TransferBase_AddChallengeParams29);
  AH_Job_SetGetLimitsFn(j, AH_Job_TransferBase_GetLimits_SepaDated);
  AH_Job_SetHandleCommandFn(j, AH_Job_TransferBase_HandleCommand_SepaDatedDebit);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaDebitDatedSingleCreate_Prepare(AH_JOB *j)
{
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing transfer");

  /* select pain profile from group "008" */
  rv=AH_Job_TransferBase_SelectPainProfile(j, 8);
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

  return 0;
}





