/***************************************************************************
    begin       : Tue Dec 31 2013
    copyright   : (C) 2004-2013 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobsepaxfersingle_p.h"
#include "jobtransferbase_l.h"
#include "aqhbci_l.h"
#include "accountjob_l.h"
#include "job_l.h"
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






/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_SepaTransferSingle_new(AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;

  j=AH_AccountJob_new("JobSepaTransferSingle", u, account);

  j=AH_Job_TransferBase_new("JobSepaTransferSingle",
                            AB_Transaction_TypeSepaTransfer,
                            AB_Transaction_SubTypeStandard,
                            u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 9);

  /* overwrite some virtual functions */
  AH_Job_SetPrepareFn(j, AH_Job_SepaTransferSingle_Prepare);
  AH_Job_SetAddChallengeParamsFn(j, AH_Job_SepaTransferSingle_AddChallengeParams);

  /* overwrite virtual functions of transferBase class */
  AH_Job_TransferBase_SetExchangeParamsFn(j, AH_Job_SepaTransferSingle_ExchangeParams);
  AH_Job_TransferBase_SetExchangeArgsFn(j, AH_Job_TransferBase_ExchangeArgs_SepaUndated);

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaTransferSingle_ExchangeParams(AH_JOB *j, AB_JOB *bj,
                                             AB_IMEXPORTER_CONTEXT *ctx) {
  AB_TRANSACTION_LIMITS *lim;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging params");

  /* set some default limits */
  lim=AB_TransactionLimits_new();
  AB_TransactionLimits_SetMaxLenPurpose(lim, 35);
  AB_TransactionLimits_SetMaxLinesPurpose(lim, 4);
  AB_TransactionLimits_SetMaxLenRemoteName(lim, 70);
  AB_TransactionLimits_SetMaxLinesRemoteName(lim, 1);

  AB_Job_SetFieldLimits(bj, lim);
  AB_TransactionLimits_free(lim);

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaTransferSingle_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod) {
  const AB_TRANSACTION *t;
  const char *s;
  int tanVer=AH_JOB_TANVER_1_4;

  DBG_ERROR(AQHBCI_LOGDOMAIN, "AddChallengeParams function called");

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



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaTransferSingle_Prepare(AH_JOB *j) {
  GWEN_DB_NODE *dbArgs;
  AB_BANKING *ab;
  AB_USER *u;
  int rv;
  const char *profileName="";
  const char *descriptor="";
  const char *s;
  AB_TRANSACTION *t;
  GWEN_BUFFER *dbuf;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing transfer");

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  u=AH_Job_GetUser(j);
  assert(u);

  dbArgs=AH_Job_GetArguments(j);

  /* choose from HISPAS */
  /* first check for any descriptor for pain 001.002.03 */
  s=AH_User_FindSepaDescriptor(u, "*001.002.03*");
  if (s) {
    profileName="001_002_03";
    descriptor=s;
  }
  else {
    /* look for pain 001.001.02 */
    s=AH_User_FindSepaDescriptor(u, "*001.001.02*");
    if (s) {
      profileName="ccm";
      descriptor=s;
    }
  }

  /* check for valid descriptor */
  if (!(descriptor && *descriptor)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No SEPA descriptor found, please update your SEPA account information");
    return GWEN_ERROR_GENERIC;
  }
  DBG_INFO(AQHBCI_LOGDOMAIN, "Using SEPA descriptor %s", descriptor);


  /* add transactions to ImExporter context */
  t=AH_Job_GetFirstTransfer(j);
  if (t==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction in job");
    assert(t); /* debug */
    return GWEN_ERROR_INTERNAL;
  }

  /* export transfers to SEPA */
  dbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=AH_Job_TransferBase_SepaExportTransactions(j, profileName, dbuf);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(dbuf);
    return rv;
  }

  /* store descriptor */
  GWEN_DB_SetCharValue(dbArgs,
                       GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "descriptor",
                       descriptor);
  /* store transfer */
  GWEN_DB_SetBinValue(dbArgs,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "transfer",
                      GWEN_Buffer_GetStart(dbuf),
                      GWEN_Buffer_GetUsedBytes(dbuf));
  GWEN_Buffer_free(dbuf);

  return 0;
}







