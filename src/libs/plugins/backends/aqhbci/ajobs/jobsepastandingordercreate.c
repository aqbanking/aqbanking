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


AH_JOB *AH_Job_SepaStandingOrderCreate_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account)
{
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
  AH_Job_SetPrepareFn(j, AH_Job_TransferBase_Prepare_SepaStandingOrder);
  AH_Job_SetAddChallengeParamsFn(j, AH_Job_TransferBase_AddChallengeParams35);
  AH_Job_SetGetLimitsFn(j, AH_Job_TransferBase_GetLimits_SepaStandingOrder);
  AH_Job_SetHandleCommandFn(j, AH_Job_TransferBase_HandleCommand_SepaStandingOrder);

  return j;
}



