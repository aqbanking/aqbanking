/***************************************************************************
    begin       : Thu Dec 16 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobsepab2bdatedmulticreate_l.h"
#include "jobsepagenericmulticreate_l.h"

#include <gwenhywfar/debug.h>

#include <assert.h>




AH_JOB *AH_Job_SepaB2bDebitDatedMultiCreate_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account)
{
  AH_JOB *j;

  j=AH_Job_SepaGenericMultiCreate_new("JobSepaB2bDebitDatedMultiCreate",
                                      AB_Transaction_TypeDebitNote,
                                      AB_Transaction_SubTypeStandard,
                                      "B2B",
                                      8,
                                      pro, u, account);

  if (!j)
    return NULL;

  AH_Job_SetChallengeClass(j, 32);
  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandSepaB2bDebitNote);

  /* get params */
  return j;
}



