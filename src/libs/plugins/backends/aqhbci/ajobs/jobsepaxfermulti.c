/***************************************************************************
    begin       : Wed Jan 08 2014
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobsepaxfermulti_p.h"
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
#include <ctype.h>



GWEN_INHERIT(AH_JOB, AH_JOB_SEPAXFERMULTI);


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void GWENHYWFAR_CB _jobApi_FreeData(void *bp, void *p);
static int _jobApi_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);
static int _jobApi_Prepare(AH_JOB *j);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



AH_JOB *AH_Job_SepaTransferMulti_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account)
{
  AH_JOB *j;
  AH_JOB_SEPAXFERMULTI *aj;
  GWEN_DB_NODE *dbParams;
  const char *s;

  j=AH_Job_TransferBase_new("JobSepaTransferMulti",
                            AB_Transaction_TypeTransfer,
                            AB_Transaction_SubTypeStandard,
                            pro, u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 13);

  GWEN_NEW_OBJECT(AH_JOB_SEPAXFERMULTI, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_SEPAXFERMULTI, j, aj, _jobApi_FreeData);

  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandSepaTransfer);

  /* overwrite some virtual functions */
  AH_Job_SetPrepareFn(j, _jobApi_Prepare);
  AH_Job_SetGetLimitsFn(j, AH_Job_TransferBase_GetLimits_SepaUndated);
  AH_Job_SetHandleCommandFn(j, AH_Job_TransferBase_HandleCommand_SepaUndated);
  AH_Job_SetAddChallengeParamsFn(j, _jobApi_AddChallengeParams);

  /* get params */
  dbParams=AH_Job_GetParams(j);
  assert(dbParams);

  AH_Job_SetMaxTransfers(j, GWEN_DB_GetIntValue(dbParams, "maxTransfers", 0, 0));

  s=GWEN_DB_GetCharValue(dbParams, "sumFieldNeeded", 0, "j");
  if (s && toupper(*s)=='J')
    aj->sumFieldNeeded=1;
  else
    aj->sumFieldNeeded=0;

  s=GWEN_DB_GetCharValue(dbParams, "singleBookingAllowed", 0, "j");
  if (s && toupper(*s)=='J')
    aj->singleBookingAllowed=1;
  else
    aj->singleBookingAllowed=0;

  return j;
}



void GWENHYWFAR_CB _jobApi_FreeData(void *bp, void *p)
{
  AH_JOB_SEPAXFERMULTI *aj;

  aj=(AH_JOB_SEPAXFERMULTI *)p;

  GWEN_FREE_OBJECT(aj);
}



int _jobApi_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod)
{
  AH_JOB_SEPAXFERMULTI *aj;
  const AB_TRANSACTION *t;
  const char *s;
  int tanVer=AH_JOB_TANVER_1_4;

  DBG_ERROR(AQHBCI_LOGDOMAIN, "AddChallengeParams function called");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_SEPAXFERMULTI, j);
  assert(aj);

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
                                      AH_Job_TransferBase_GetSumValues(j),
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



int _jobApi_Prepare(AH_JOB *j)
{
  AH_JOB_SEPAXFERMULTI *aj;

  DBG_ERROR(AQHBCI_LOGDOMAIN, "Preparing transfers");
  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_SEPAXFERMULTI, j);
  assert(aj);
  return AH_Job_TransferBase_Prepare(j, 1, NULL, aj->singleBookingAllowed);
}







