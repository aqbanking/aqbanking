/***************************************************************************
    copyright   : (C) 2026 by AqBanking Contributors

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobsepab2bdebit_l.h"
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


typedef struct AH_JOB_CREATESEPAMULTIB2B AH_JOB_CREATESEPAMULTIB2B;
struct AH_JOB_CREATESEPAMULTIB2B {
  int singleBookingAllowed;
};

GWEN_INHERIT(AH_JOB, AH_JOB_CREATESEPAMULTIB2B);


static int _singlePrepare(AH_JOB *j);
static void GWENHYWFAR_CB _multiFreeData(void *bp, void *p);
static int _multiAddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);
static int _multiPrepare(AH_JOB *j);



AH_JOB *AH_Job_SepaB2bDebitDatedSingleCreate_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account)
{
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;

  j=AH_Job_TransferBase_new("JobSepaB2bDebitDatedSingleCreate",
                            AB_Transaction_TypeDebitNote,
                            AB_Transaction_SubTypeStandard,
                            pro, u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 29);
  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandSepaB2bDebitNote);

  AH_Job_SetPrepareFn(j, _singlePrepare);
  AH_Job_SetAddChallengeParamsFn(j, AH_Job_TransferBase_AddChallengeParams29);
  AH_Job_SetGetLimitsFn(j, AH_Job_TransferBase_GetLimits_SepaDated);
  AH_Job_SetHandleCommandFn(j, AH_Job_TransferBase_HandleCommand_SepaDatedDebit);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  return j;
}



int _singlePrepare(AH_JOB *j)
{
  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing SEPA B2B single debit");
  assert(j);
  if (AH_Job_GetMsgNum(j)==0) {
    int rv;

    rv=AH_Job_TransferBase_SelectPainProfile(j, 8);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    AH_Job_TransferBase_SetLocalInstrumentationCode(j, "B2B");

    rv=AH_Job_TransferBase_SepaExportTransactions(j);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



AH_JOB *AH_Job_SepaB2bDebitDatedMultiCreate_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account)
{
  AH_JOB *j;
  AH_JOB_CREATESEPAMULTIB2B *aj;
  GWEN_DB_NODE *dbParams;
  GWEN_DB_NODE *dbArgs;
  const char *s;

  j=AH_Job_TransferBase_new("JobSepaB2bDebitDatedMultiCreate",
                            AB_Transaction_TypeDebitNote,
                            AB_Transaction_SubTypeStandard,
                            pro, u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 32);
  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandSepaB2bDebitNote);

  GWEN_NEW_OBJECT(AH_JOB_CREATESEPAMULTIB2B, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_CREATESEPAMULTIB2B, j, aj, _multiFreeData);

  AH_Job_SetPrepareFn(j, _multiPrepare);
  AH_Job_SetAddChallengeParamsFn(j, _multiAddChallengeParams);
  AH_Job_SetGetLimitsFn(j, AH_Job_TransferBase_GetLimits_SepaDated);
  AH_Job_SetHandleCommandFn(j, AH_Job_TransferBase_HandleCommand_SepaDatedDebit);

  dbParams=AH_Job_GetParams(j);
  assert(dbParams);

  AH_Job_SetMaxTransfers(j, GWEN_DB_GetIntValue(dbParams, "maxTransfers", 0, 0));

  s=GWEN_DB_GetCharValue(dbParams, "singleBookingAllowed", 0, "j");
  if (s && toupper((unsigned char)*s)=='J')
    aj->singleBookingAllowed=1;
  else
    aj->singleBookingAllowed=0;

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  return j;
}



void GWENHYWFAR_CB _multiFreeData(void *bp, void *p)
{
  AH_JOB_CREATESEPAMULTIB2B *aj;

  (void)bp;
  aj=(AH_JOB_CREATESEPAMULTIB2B *)p;
  GWEN_FREE_OBJECT(aj);
}



int _multiAddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod)
{
  AH_JOB_CREATESEPAMULTIB2B *aj;
  const AB_TRANSACTION *t;
  const char *s;
  int tanVer=AH_JOB_TANVER_1_4;

  (void)hkTanVer;
  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_CREATESEPAMULTIB2B, j);
  assert(aj);
  (void)aj;

  t=AH_Job_GetFirstTransfer(j);
  if (t==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction");
    return GWEN_ERROR_INVALID;
  }

  s=GWEN_DB_GetCharValue(dbMethod, "zkaTanVersion", 0, NULL);
  if (s && *s && strncasecmp(s, "1.3", 3)==0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "TAN version is 1.3 (%s)", s);
    tanVer=AH_JOB_TANVER_1_3;
  }

  if (tanVer==AH_JOB_TANVER_1_4) {
    int rv;

    rv=AH_HHD14_AddChallengeParams_32(j,
                                      AH_Job_GetTransferCount(j),
                                      AH_Job_TransferBase_GetSumValues(j),
                                      AB_Transaction_GetLocalIban(t),
                                      AB_Transaction_GetDate(t));
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
  AH_JOB_CREATESEPAMULTIB2B *aj;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing SEPA B2B multi debit notes");
  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_CREATESEPAMULTIB2B, j);
  assert(aj);
  return AH_Job_TransferBase_Prepare(j, 8, "B2B", aj->singleBookingAllowed);
}
