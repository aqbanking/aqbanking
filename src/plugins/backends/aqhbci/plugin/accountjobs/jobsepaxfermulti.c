/***************************************************************************
    begin       : Wed Jan 08 2014
    copyright   : (C) 2014 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobsepaxfermulti_p.h"
#include "jobtransferbase_l.h"
#include "aqhbci_l.h"
#include "accountjob_l.h"
#include "job_l.h"
#include "user_l.h"
#include "provider_l.h"
#include "hhd_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/text.h>

#include <aqbanking/jobsepatransfer_be.h>
#include <aqbanking/job_be.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>



GWEN_INHERIT(AH_JOB, AH_JOB_SEPAXFERMULTI);




/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_SepaTransferMulti_new(AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;
  AH_JOB_SEPAXFERMULTI *aj;
  GWEN_DB_NODE *dbParams;
  const char *s;

  j=AH_Job_TransferBase_new("JobSepaTransferMulti",
                            AB_Transaction_TypeSepaTransfer,
                            AB_Transaction_SubTypeStandard,
                            u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 13);

  GWEN_NEW_OBJECT(AH_JOB_SEPAXFERMULTI, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_SEPAXFERMULTI, j, aj,
                       AH_Job_SepaTransferMulti_FreeData);

  /* overwrite some virtual functions */
  AH_Job_SetPrepareFn(j, AH_Job_SepaTransferMulti_Prepare);
  AH_Job_SetAddChallengeParamsFn(j, AH_Job_SepaTransferMulti_AddChallengeParams);

  /* overwrite virtual functions of transferBase class */
  AH_Job_TransferBase_SetExchangeParamsFn(j, AH_Job_TransferBase_ExchangeParams_SepaUndated);
  AH_Job_TransferBase_SetExchangeArgsFn(j, AH_Job_TransferBase_ExchangeArgs_SepaUndated);

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



/* --------------------------------------------------------------- FUNCTION */
void GWENHYWFAR_CB AH_Job_SepaTransferMulti_FreeData(void *bp, void *p){
  AH_JOB_SEPAXFERMULTI *aj;

  aj=(AH_JOB_SEPAXFERMULTI*)p;

  AB_Value_free(aj->sumValues);

  GWEN_FREE_OBJECT(aj);
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaTransferMulti_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod) {
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
                                      aj->sumValues,
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



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaTransferMulti_Prepare(AH_JOB *j) {
  AH_JOB_SEPAXFERMULTI *aj;
  GWEN_DB_NODE *dbArgs;
  GWEN_DB_NODE *profile;
  int rv;
  AB_TRANSACTION *t;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing transfers");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_SEPAXFERMULTI, j);
  assert(aj);

  dbArgs=AH_Job_GetArguments(j);

  /* calculate sum */
  AB_Value_free(aj->sumValues);
  aj->sumValues=AB_Value_new();
  AB_Value_SetCurrency(aj->sumValues, "EUR");
  t=AH_Job_GetFirstTransfer(j);
  if (t==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction in job");
    return GWEN_ERROR_INTERNAL;
  }
  while(t) {
    const AB_VALUE *v;

    v=AB_Transaction_GetValue(t);
    if (v) {
      const char *s;

      s=AB_Value_GetCurrency(v);
      if (s && strcmp(s, "EUR")) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "EUR required in SEPA transactions (%s)", s);
        return GWEN_ERROR_BAD_DATA;
      }
      AB_Value_AddValue(aj->sumValues, v);
    }
    t=AB_Transaction_List_Next(t);
  }

  /* find the right profile to produce pain.001 messages */
  profile=AH_Job_FindSepaProfile(j, "001*", AH_User_GetSepaTransferProfile(AH_Job_GetUser(j)));
  if (!profile) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No suitable profile found");
    return GWEN_ERROR_GENERIC;
  }

  /* set singleBookingWanted */
  if (aj->singleBookingAllowed &&
      GWEN_DB_GetIntValue(profile, "singleBookingWanted", 0, 1))
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "singleBookingWanted", "J");
  else {
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "singleBookingWanted", "N");
    GWEN_DB_SetIntValue(profile, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "singleBookingWanted", 0);
  }

  /* export transfers to SEPA */
  rv=AH_Job_TransferBase_SepaExportTransactions(j, profile);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* store sum value */
  if (1) {
    GWEN_DB_NODE *dbV;
    GWEN_BUFFER *nbuf;
    const char *s;

    dbV=GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "totalSum");
    assert(dbV);

    nbuf=GWEN_Buffer_new(0, 32, 0, 1);
    AB_Value_toHbciString(aj->sumValues, nbuf);
    if (GWEN_Buffer_GetUsedBytes(nbuf)<1) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error in conversion");
      GWEN_Buffer_free(nbuf);
      return GWEN_ERROR_BAD_DATA;
    }

    /* store value */
    GWEN_DB_SetCharValue(dbV, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "value",
                         GWEN_Buffer_GetStart(nbuf));
    GWEN_Buffer_free(nbuf);

    /* store currency */
    s=AB_Value_GetCurrency(aj->sumValues);
    assert(s);
    GWEN_DB_SetCharValue(dbV, GWEN_DB_FLAGS_OVERWRITE_VARS, "currency", s);
  }
  return 0;
}







