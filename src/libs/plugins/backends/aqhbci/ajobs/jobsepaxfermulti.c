/***************************************************************************
    begin       : Wed Jan 08 2014
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobsepaxfermulti_l.h"
#include "jobsepagenericmulticreate_l.h"

#include "jobtransferbase_l.h"
#include "aqhbci/applayer/hhd_l.h"

#include <gwenhywfar/debug.h>

#include <assert.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _addChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



AH_JOB *AH_Job_SepaTransferMulti_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account)
{
  AH_JOB *j;

  j=AH_Job_SepaGenericMultiCreate_new("JobSepaTransferMulti",
                                      AB_Transaction_TypeTransfer,
                                      AB_Transaction_SubTypeStandard,
                                      "CORE",
                                      1,
                                      pro, u, account);
  if (!j)
    return NULL;

  AH_Job_SetChallengeClass(j, 13);
  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandSepaTransfer);

  AH_Job_SetGetLimitsFn(j, AH_Job_TransferBase_GetLimits_SepaUndated);
  AH_Job_SetHandleCommandFn(j, AH_Job_TransferBase_HandleCommand_SepaUndated);
  AH_Job_SetAddChallengeParamsFn(j, _addChallengeParams);

  return j;
}



int _addChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod)
{
  const AB_TRANSACTION *t;
  const char *s;
  int tanVer=AH_JOB_TANVER_1_4;

  DBG_ERROR(AQHBCI_LOGDOMAIN, "AddChallengeParams function called");

  /* get data from first transaction */
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
    rv=AH_HHD14_AddChallengeParams_13(j,
                                      AH_Job_GetTransferCount(j),
                                      AH_Job_SepaGenericMultiCreate_GetSumValues(j),
                                      AB_Transaction_GetLocalIban(t));
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


