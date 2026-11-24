/***************************************************************************
    copyright   : (C) 2026 by AqBanking Contributors

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobsepainstantxfer_l.h"
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


typedef struct AH_JOB_SEPAINSTANTXFERMULTI AH_JOB_SEPAINSTANTXFERMULTI;
struct AH_JOB_SEPAINSTANTXFERMULTI {
  int singleBookingAllowed;
};

GWEN_INHERIT(AH_JOB, AH_JOB_SEPAINSTANTXFERMULTI);


static int _singlePrepare(AH_JOB *j);
static int _singleAddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);

static void GWENHYWFAR_CB _multiFreeData(void *bp, void *p);
static int _multiPrepare(AH_JOB *j);
static int _multiAddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);



AH_JOB *AH_Job_SepaInstantTransferSingle_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account)
{
  AH_JOB *j;

  j=AH_Job_TransferBase_new("JobSepaInstantTransferSingle",
                            AB_Transaction_TypeTransfer,
                            AB_Transaction_SubTypeStandard,
                            pro, u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 9);
  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandSepaInstantTransfer);

  AH_Job_SetPrepareFn(j, _singlePrepare);
  AH_Job_SetAddChallengeParamsFn(j, _singleAddChallengeParams);
  AH_Job_SetHandleCommandFn(j, AH_Job_TransferBase_HandleCommand_SepaUndated);
  AH_Job_SetGetLimitsFn(j, AH_Job_TransferBase_GetLimits_SepaUndated);

  return j;
}



int _singleAddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod)
{
  const AB_TRANSACTION *t;
  const char *s;
  int tanVer=AH_JOB_TANVER_1_4;

  (void)hkTanVer;
  t=AH_Job_GetFirstTransfer(j);
  if (t==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No validated transaction");
    return GWEN_ERROR_INVALID;
  }

  s=GWEN_DB_GetCharValue(dbMethod, "zkaTanVersion", 0, NULL);
  if (s && *s && strncasecmp(s, "1.3", 3)==0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "TAN version is 1.3 (%s)", s);
    tanVer=AH_JOB_TANVER_1_3;
  }

  if (tanVer==AH_JOB_TANVER_1_4) {
    int rv;

    rv=AH_HHD14_AddChallengeParams_09(j,
                                      AB_Transaction_GetValue(t),
                                      AB_Transaction_GetRemoteIban(t));
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



int _singlePrepare(AH_JOB *j)
{
  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing SEPA instant single transfer");
  assert(j);
  if (AH_Job_GetMsgNum(j)==0) {
    int rv;
    GWEN_DB_NODE *dbArgs;

    rv=AH_Job_TransferBase_SelectPainProfile(j, 1);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    AH_Job_TransferBase_SetLocalInstrumentationCode(j, "INST");

    rv=AH_Job_TransferBase_SepaExportTransactions(j);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    if (AH_Job_GetSegmentVersion(j)>=2) {
      GWEN_DB_NODE *dbParams=AH_Job_GetParams(j);
      const char *allowConv=GWEN_DB_GetCharValue(dbParams, "allowConvertSepaTransferAllowed", 0, "N");
      if (allowConv && toupper((unsigned char)*allowConv)=='J') {
        dbArgs=AH_Job_GetArguments(j);
        assert(dbArgs);
        GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "allowConvertSepaTransfer", "N");
      }
    }
  }

  return 0;
}



AH_JOB *AH_Job_SepaInstantTransferMulti_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account)
{
  AH_JOB *j;
  AH_JOB_SEPAINSTANTXFERMULTI *aj;
  GWEN_DB_NODE *dbParams;
  const char *s;

  j=AH_Job_TransferBase_new("JobSepaInstantTransferMulti",
                            AB_Transaction_TypeTransfer,
                            AB_Transaction_SubTypeStandard,
                            pro, u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 13);

  GWEN_NEW_OBJECT(AH_JOB_SEPAINSTANTXFERMULTI, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_SEPAINSTANTXFERMULTI, j, aj, _multiFreeData);

  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandSepaInstantTransfer);

  AH_Job_SetPrepareFn(j, _multiPrepare);
  AH_Job_SetGetLimitsFn(j, AH_Job_TransferBase_GetLimits_SepaUndated);
  AH_Job_SetHandleCommandFn(j, AH_Job_TransferBase_HandleCommand_SepaUndated);
  AH_Job_SetAddChallengeParamsFn(j, _multiAddChallengeParams);

  dbParams=AH_Job_GetParams(j);
  assert(dbParams);

  AH_Job_SetMaxTransfers(j, GWEN_DB_GetIntValue(dbParams, "maxTransfers", 0, 0));

  s=GWEN_DB_GetCharValue(dbParams, "singleBookingAllowed", 0, "N");
  if (s && toupper((unsigned char)*s)=='J')
    aj->singleBookingAllowed=1;
  else
    aj->singleBookingAllowed=0;

  return j;
}



void GWENHYWFAR_CB _multiFreeData(void *bp, void *p)
{
  AH_JOB_SEPAINSTANTXFERMULTI *aj;

  (void)bp;
  aj=(AH_JOB_SEPAINSTANTXFERMULTI *)p;
  GWEN_FREE_OBJECT(aj);
}



int _multiAddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod)
{
  AH_JOB_SEPAINSTANTXFERMULTI *aj;
  const AB_TRANSACTION *t;
  const char *s;
  int tanVer=AH_JOB_TANVER_1_4;

  (void)hkTanVer;
  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_SEPAINSTANTXFERMULTI, j);
  assert(aj);
  (void)aj;

  t=AH_Job_GetFirstTransfer(j);
  if (t==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No validated transaction");
    return GWEN_ERROR_INVALID;
  }

  s=GWEN_DB_GetCharValue(dbMethod, "zkaTanVersion", 0, NULL);
  if (s && *s && strncasecmp(s, "1.3", 3)==0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "TAN version is 1.3 (%s)", s);
    tanVer=AH_JOB_TANVER_1_3;
  }

  if (tanVer==AH_JOB_TANVER_1_4) {
    int rv;

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



int _multiPrepare(AH_JOB *j)
{
  AH_JOB_SEPAINSTANTXFERMULTI *aj;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing SEPA instant multi transfers");
  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_SEPAINSTANTXFERMULTI, j);
  assert(aj);

  rv=AH_Job_TransferBase_Prepare(j, 1, "INST", aj->singleBookingAllowed);
  if (rv<0)
    return rv;

  if (AH_Job_GetSegmentVersion(j)>=2) {
    GWEN_DB_NODE *dbParams=AH_Job_GetParams(j);
    const char *allowConv=GWEN_DB_GetCharValue(dbParams, "allowConvertSepaTransferAllowed", 0, "N");
    if (allowConv && toupper((unsigned char)*allowConv)=='J') {
      GWEN_DB_NODE *dbArgs=AH_Job_GetArguments(j);
      assert(dbArgs);
      GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "allowConvertSepaTransfer", "N");
    }
  }

  return 0;
}
