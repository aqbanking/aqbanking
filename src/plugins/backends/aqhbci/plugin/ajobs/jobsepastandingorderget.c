/***************************************************************************
    begin       : Sat Aug 03 2014
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobsepastandingorderget_p.h"
#include "aqhbci_l.h"
#include "accountjob_l.h"
#include "job_l.h"
#include "user_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>

#include <aqbanking/jobgetstandingorders.h>
#include <aqbanking/job_be.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>





/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_SepaStandingOrderGet_new(AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;

  j=AH_AccountJob_new("JobSepaStandingOrderGet", u, account);
  if (!j)
    return 0;

  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandSepaGetStandingOrders);

  /* overwrite some virtual functions */
  AH_Job_SetPrepareFn(j, AH_Job_SepaStandingOrderGet_Prepare);
  AH_Job_SetProcessFn(j, AH_Job_SepaStandingOrdersGet_Process);
  AH_Job_SetGetLimitsFn(j, AH_Job_GetLimits_EmptyLimits);
  AH_Job_SetHandleCommandFn(j, AH_Job_HandleCommand_Accept);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);
  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "allAccounts", "N");
  return j;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaStandingOrderGet_Prepare(AH_JOB *j) {
  GWEN_DB_NODE *dbArgs;
  GWEN_DB_NODE *profile;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing job");

  dbArgs=AH_Job_GetArguments(j);

  /* find the right profile to produce pain.001 messages */
  profile=AH_Job_FindSepaProfile(j, "001*", AH_User_GetSepaTransferProfile(AH_Job_GetUser(j)));
  if (!profile) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No suitable profile found");
    return GWEN_ERROR_GENERIC;
  }
  else {
    const char *s;

    s=GWEN_DB_GetCharValue(profile, "descriptor", 0, 0);
    if (s) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Using SEPA format \"%s\"", s);
      GWEN_DB_SetCharValue(dbArgs,
			   GWEN_DB_FLAGS_OVERWRITE_VARS,
			   "SupportedSepaFormats/Format",
			   s);
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No matching SEPA descriptor found");
      return GWEN_ERROR_GENERIC;
    }
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaStandingOrdersGet__ReadSto(AH_JOB *j,
					  AB_IMEXPORTER_CONTEXT *ctx,
					  const uint8_t *ptr,
					  uint32_t len,
					  const char *fiId){
  int rv;
  AB_IMEXPORTER_CONTEXT *tmpCtx;
  GWEN_BUFFER *tbuf;
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  AB_ACCOUNT *a;

  a=AH_AccountJob_GetAccount(j);
  assert(a);

  tmpCtx=AB_ImExporterContext_new();
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendBytes(tbuf, (const char*) ptr, len);

  rv=AB_Banking_ImportBuffer(AH_Job_GetBankingApi(j),
			     tmpCtx,
			     "sepa",
			     "default",
			     tbuf);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    AB_ImExporterContext_free(tmpCtx);
    return rv;
  }
  GWEN_Buffer_free(tbuf);

  ai=AB_ImExporterContext_GetFirstAccountInfo(tmpCtx);
  if (ai) {
    AB_TRANSACTION *t;

    while( (t=AB_ImExporterAccountInfo_GetFirstTransaction(ai, 0, 0)) ) {
      AB_Transaction_List_Del(t);
      AB_Transaction_SetFiId(t, fiId);
      AB_Transaction_SetUniqueAccountId(t, AB_Account_GetUniqueId(a));
      /* add to real im/exporter context */
      AB_ImExporterContext_AddTransaction(ctx, t);
    }

    if (AB_ImExporterAccountInfo_List_Next(ai)) {
      DBG_WARN(AQHBCI_LOGDOMAIN, "Multiple account infos returned by import!");
    }
  }
  AB_ImExporterContext_free(tmpCtx);

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaStandingOrdersGet_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  const char *responseName;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobSepaStandingOrdersGet");

  assert(j);

  responseName=AH_Job_GetResponseName(j);


  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  /* search for "Transactions" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while(dbCurr) {
    rv=AH_Job_CheckEncryption(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (encryption)");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }
    rv=AH_Job_CheckSignature(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (signature)");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }

    if (responseName && *responseName) {
      GWEN_DB_NODE *dbXA;

      dbXA=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
      if (dbXA)
        dbXA=GWEN_DB_GetGroup(dbXA, GWEN_PATH_FLAGS_NAMEMUSTEXIST, responseName);
      if (dbXA) {
	const void *p;
	unsigned int bs;
	const char *fiId;

	fiId=GWEN_DB_GetCharValue(dbXA, "fiId", 0, NULL);
	p=GWEN_DB_GetBinValue(dbXA, "transfer", 0, 0, 0, &bs);
	if (p && bs) {
	  rv=AH_Job_SepaStandingOrdersGet__ReadSto(j, ctx, p, bs, fiId);
	  if (rv<0) {
	    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	    DBG_WARN(AQHBCI_LOGDOMAIN, "Error reading standing order from data, ignoring (%d)", rv);
	  }
	}
      }
    }

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  return 0;
}



