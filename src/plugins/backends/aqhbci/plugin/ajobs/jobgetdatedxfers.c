/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobgetdatedxfers_p.h"
#include "aqhbci_l.h"
#include "accountjob_l.h"
#include "job_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/gui.h>

#include <aqbanking/jobgetdatedtransfers.h>
#include <aqbanking/jobgetdatedtransfers_be.h>
#include <aqbanking/job_be.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>


GWEN_INHERIT(AH_JOB, AH_JOB_GETDATEDTRANSFERS);




/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_GetDatedTransfers_new(AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;
  AH_JOB_GETDATEDTRANSFERS *aj;
  GWEN_DB_NODE *dbArgs;

  j=AH_AccountJob_new("JobGetDatedTransfers", u, account);
  if (!j)
    return 0;

  GWEN_NEW_OBJECT(AH_JOB_GETDATEDTRANSFERS, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_GETDATEDTRANSFERS, j, aj,
                       AH_Job_GetDatedTransfers_FreeData);
  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, AH_Job_GetDatedTransfers_Process);
  AH_Job_SetExchangeFn(j, AH_Job_GetDatedTransfers_Exchange);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
void GWENHYWFAR_CB AH_Job_GetDatedTransfers_FreeData(void *bp, void *p){
  AH_JOB_GETDATEDTRANSFERS *aj;

  aj=(AH_JOB_GETDATEDTRANSFERS*)p;
  GWEN_FREE_OBJECT(aj);
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetDatedTransfers_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_GETDATEDTRANSFERS *aj;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  AB_ACCOUNT *a;
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  int rv;
  int i;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobGetDatedTransfers");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETDATEDTRANSFERS, j);
  assert(aj);

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  a=AH_AccountJob_GetAccount(j);
  assert(a);
  ai=AB_ImExporterContext_GetAccountInfo(ctx,
                                         AB_Account_GetBankCode(a),
                                         AB_Account_GetAccountNumber(a));
  assert(ai);

  /* search for "DatedTransfer" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while(dbCurr) {
    GWEN_DB_NODE *dbXA;

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

    dbXA=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                          "data/GetDatedTransfersResponse");
    if (dbXA) {
      GWEN_DB_NODE *dbT;
      AB_TRANSACTION *t;
      const char *s;

      dbXA=GWEN_DB_GetGroup(dbXA, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			    "datedTransfer");
      assert(dbXA);

      /* create transaction: raw */
      t=AB_Transaction_fromDb(dbXA);
      assert(t);

      /* now work out some of the fields */

      /* date */
      s=GWEN_DB_GetCharValue(dbXA, "xDate", 0, 0);
      if (s && *s) {
	GWEN_BUFFER *dbuf;
	GWEN_TIME *ti;

	dbuf=GWEN_Buffer_new(0, 16, 0, 1);
	GWEN_Buffer_AppendString(dbuf, s);
	GWEN_Buffer_AppendString(dbuf, "-12:00");
	ti=GWEN_Time_fromUtcString(GWEN_Buffer_GetStart(dbuf),
				   "YYYYMMDD-hh:mm");
	assert(ti);
	AB_Transaction_SetDate(t, ti);
	GWEN_Time_free(ti);
      }

      /* local account */
      dbT=GWEN_DB_GetGroup(dbXA, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "ouraccount");
      if (dbT) {
	const AB_COUNTRY *cn;

	cn=AB_Banking_FindCountryByNumeric(AH_Job_GetBankingApi(j),
					   GWEN_DB_GetIntValue(dbT,
							       "country", 0,
							       280));
	assert(cn);
	AB_Transaction_SetLocalCountry(t, AB_Country_GetCode(cn));
	s=GWEN_DB_GetCharValue(dbT, "bankCode", 0, 0);
	if (s && *s)
	  AB_Transaction_SetLocalBankCode(t, s);
	s=GWEN_DB_GetCharValue(dbT, "accountId", 0, 0);
	if (s && *s)
	  AB_Transaction_SetLocalAccountNumber(t, s);
        s=GWEN_DB_GetCharValue(dbT, "accountSubId", 0, 0);
        if (s && *s)
	  AB_Transaction_SetLocalSuffix(t, s);
      }

      /* remote account */
      dbT=GWEN_DB_GetGroup(dbXA, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			   "otheraccount");
      if (dbT) {
	const AB_COUNTRY *cn;

	cn=AB_Banking_FindCountryByNumeric(AH_Job_GetBankingApi(j),
					   GWEN_DB_GetIntValue(dbT,
							       "country", 0,
							       280));
	assert(cn);
	AB_Transaction_SetRemoteCountry(t, AB_Country_GetCode(cn));
	s=GWEN_DB_GetCharValue(dbT, "bankCode", 0, 0);
	if (s && *s)
	  AB_Transaction_SetRemoteBankCode(t, s);
	s=GWEN_DB_GetCharValue(dbT, "accountId", 0, 0);
	if (s && *s)
	  AB_Transaction_SetRemoteAccountNumber(t, s);
        s=GWEN_DB_GetCharValue(dbT, "accountSubId", 0, 0);
        if (s && *s)
	  AB_Transaction_SetRemoteSuffix(t, s);
      }

      /* remote name */
      AB_Transaction_ClearRemoteName(t);
      for (i=0; ; i++) {
	s=GWEN_DB_GetCharValue(dbXA, "otherName", i, 0);
	if (s) {
	  if (*s) {
	    GWEN_BUFFER *dbuf;

	    dbuf=GWEN_Buffer_new(0, 27, 0, 1);
	    AB_ImExporter_DtaToUtf8(s, strlen(s), dbuf);
	    AB_Transaction_AddRemoteName(t, GWEN_Buffer_GetStart(dbuf), 1);
	    GWEN_Buffer_free(dbuf);
	  }
	}
	else
          break;
      }

      /* purpose name */
      AB_Transaction_ClearPurpose(t);
      for (i=0; ; i++) {
	s=GWEN_DB_GetCharValue(dbXA, "purpose", i, 0);
	if (s) {
	  if (*s) {
	    GWEN_BUFFER *dbuf;

	    dbuf=GWEN_Buffer_new(0, 27, 0, 1);
	    AB_ImExporter_DtaToUtf8(s, strlen(s), dbuf);
	    AB_Transaction_AddPurpose(t, GWEN_Buffer_GetStart(dbuf), 0);
	    GWEN_Buffer_free(dbuf);
	  }
	}
	else
	  break;
      }

      /* add to list */
      AB_ImExporterAccountInfo_AddDatedTransfer(ai, t);
    } /* if "datedTransfer" */
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetDatedTransfers_Exchange(AH_JOB *j, AB_JOB *bj,
				      AH_JOB_EXCHANGE_MODE m,
				      AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_GETDATEDTRANSFERS *aj;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging (%d)", m);

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETDATEDTRANSFERS, j);
  assert(aj);

  if (AB_Job_GetType(bj)!=AB_Job_TypeGetDatedTransfers) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Not a GetDatedTransfers job");
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










