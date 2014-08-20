/***************************************************************************
    begin       : Sat Aug 03 2014
    copyright   : (C) 2014 by Martin Preuss
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
#include <aqbanking/jobgetstandingorders_be.h>
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

  /* overwrite some virtual functions */
  AH_Job_SetPrepareFn(j, AH_Job_SepaStandingOrderGet_Prepare);
  AH_Job_SetProcessFn(j, AH_Job_SepaStandingOrdersGet_Process);
  AH_Job_SetExchangeFn(j, AH_Job_SepaStandingOrdersGet_Exchange);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);
  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT,
                       "allAccounts", "N");
  return j;
}



/* --------------------------------------------------------------- FUNCTION */
static const char *_findPatternInStringList(const GWEN_STRINGLIST *sl, const char *pattern){
  GWEN_STRINGLISTENTRY *se;

  se=GWEN_StringList_FirstEntry(sl);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    if (s && *s && -1!=GWEN_Text_ComparePattern(s, pattern, 0)) {
      return s;
    }
    se=GWEN_StringListEntry_Next(se);
  }

  return NULL;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaStandingOrderGet_Prepare(AH_JOB *j) {
  GWEN_DB_NODE *dbArgs;
  const GWEN_STRINGLIST *descriptors;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing job");

  dbArgs=AH_Job_GetArguments(j);

  descriptors=AH_User_GetSepaDescriptors(AH_Job_GetUser(j));
  if (descriptors) {
    const char *s;

    s=_findPatternInStringList(descriptors, "*pain.001.*");
    if (s) {
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
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No list of supported SEPA descriptor found");
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaStandingOrdersGet_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){
  AB_ACCOUNT *a;
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  GWEN_BUFFER *bufStandingOrders;
  const char *responseName;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobSepaStandingOrdersGet");

  assert(j);

  responseName=AH_Job_GetResponseName(j);


  bufStandingOrders=GWEN_Buffer_new(0, 1024, 0, 1);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  /* search for "Transactions" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while(dbCurr) {
    rv=AH_Job_CheckEncryption(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (encryption)");
      GWEN_Buffer_free(bufStandingOrders);
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }
    rv=AH_Job_CheckSignature(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (signature)");
      GWEN_Buffer_free(bufStandingOrders);
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

	p=GWEN_DB_GetBinValue(dbXA, "transfer", 0, 0, 0, &bs);
	if (p && bs)
	  GWEN_Buffer_AppendBytes(bufStandingOrders, p, bs);
      }
    }

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  GWEN_Buffer_Rewind(bufStandingOrders);

  /* now the buffers contain data to be parsed by ImExporters */
  a=AH_AccountJob_GetAccount(j);
  assert(a);
  ai=AB_ImExporterContext_GetAccountInfo(ctx,
                                         AB_Account_GetBankCode(a),
                                         AB_Account_GetAccountNumber(a));
  assert(ai);
  AB_ImExporterAccountInfo_SetAccountId(ai, AB_Account_GetUniqueId(a));

  /* read booked transactions */
  if (GWEN_Buffer_GetUsedBytes(bufStandingOrders)) {
    if (getenv("AQHBCI_LOGBOOKED")) {
      FILE *f;

      f=fopen("/tmp/standingOrders.pain", "w+");
      if (f) {
        if (fwrite(GWEN_Buffer_GetStart(bufStandingOrders),
                   GWEN_Buffer_GetUsedBytes(bufStandingOrders), 1, f)!=1) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "fwrite: %s", strerror(errno));
        }
        if (fclose(f)) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "fclose: %s", strerror(errno));
        }
      }
    }

    /* TODO: parse data */
  }

  GWEN_Buffer_free(bufStandingOrders);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_SepaStandingOrdersGet_Exchange(AH_JOB *j, AB_JOB *bj,
                                          AH_JOB_EXCHANGE_MODE m,
                                          AB_IMEXPORTER_CONTEXT *ctx){
  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging (%d)", m);

  assert(j);

  if (AB_Job_GetType(bj)!=AB_Job_TypeSepaGetStandingOrders) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Not a GetStandingOrders job");
    return GWEN_ERROR_INVALID;
  }

  switch(m) {
  case AH_Job_ExchangeModeParams:
    return 0;

  case AH_Job_ExchangeModeArgs:
    return 0;

  case AH_Job_ExchangeModeResults:
    return 0;

  default:
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Unsupported exchange mode");
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */
}










