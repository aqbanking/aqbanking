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


#include "jobsepastandingordermodify_l.h"
#include "jobsepastandingordercreate_l.h"
#include "jobtransferbase_l.h"



/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_SepaStandingOrderModify_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;

  j=AH_Job_TransferBase_new("JobSepaStandingOrderModify",
                            AB_Transaction_TypeTransfer,
                            AB_Transaction_SubTypeStandingOrder,
                            pro, u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 35);
  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandSepaModifyStandingOrder);

  /* overwrite some virtual functions (use those from AH_Job_SepaStandingOrderCreate) */
  AH_Job_SetPrepareFn(j, AH_Job_SepaStandingOrderCreate_Prepare);
  AH_Job_SetAddChallengeParamsFn(j, AH_Job_TransferBase_AddChallengeParams35);
  AH_Job_SetGetLimitsFn(j, AH_Job_TransferBase_GetLimits_SepaStandingOrder);
  AH_Job_SetHandleCommandFn(j, AH_Job_TransferBase_HandleCommand_SepaStandingOrder);

  return j;
}

