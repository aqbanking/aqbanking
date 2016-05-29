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


#include "jobsepastandingordermodify_l.h"
#include "jobsepastandingordercreate_l.h"
#include "jobtransferbase_l.h"



/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_SepaStandingOrderModify_new(AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;

  j=AH_Job_TransferBase_new("JobSepaStandingOrderModify",
                            AB_Transaction_TypeSepaTransfer,
                            AB_Transaction_SubTypeStandingOrder,
                            u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 35);

  /* overwrite some virtual functions (use those from AH_Job_SepaStandingOrderCreate) */
  AH_Job_SetPrepareFn(j, AH_Job_SepaStandingOrderCreate_Prepare);
  AH_Job_SetAddChallengeParamsFn(j, AH_Job_SepaStandingOrderCreate_AddChallengeParams);

  /* overwrite virtual functions of transferBase class */
  AH_Job_TransferBase_SetExchangeParamsFn(j, AH_Job_SepaStandingOrderCreate_ExchangeParams);
  AH_Job_TransferBase_SetExchangeArgsFn(j, AH_Job_SepaStandingOrderCreate_ExchangeArgs);

  return j;
}
