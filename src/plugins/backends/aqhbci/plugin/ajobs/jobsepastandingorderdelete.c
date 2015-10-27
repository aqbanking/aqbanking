/***************************************************************************
 begin       : Wed Jan 15 2014
 copyright   : (C) 2014 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/* --------------------------------------------------------------- FUNCTION */
/* --- rw ---                   Delete_new                                  */
AH_JOB *AH_Job_SepaStandingOrderDelete_new(AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;
  /* --- rw ---              JobSepaStandingOrderDelete */
  j=AH_Job_TransferBase_new("JobSepaStandingOrderDelete",
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
