/***************************************************************************
 begin       : Tue Apr 03 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobgetestatements_p.h"
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

#include <aqbanking/jobgetstandingorders.h> /* TODO: use new class */
#include <aqbanking/job_be.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>





/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_GetEStatements_new(AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;

  j=AH_AccountJob_new("JobGetEStatements", u, account);
  if (!j)
    return 0;

  /* overwrite some virtual functions */
  AH_Job_SetPrepareFn(j, AH_Job_GetEStatements_Prepare);
  AH_Job_SetProcessFn(j, AH_Job_GetEStatements_Process);
  AH_Job_SetExchangeFn(j, AH_Job_GetEStatements_Exchange);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);
  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT,
                       "allAccounts", "N");
  return j;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetEStatements_Prepare(AH_JOB *j) {
  GWEN_DB_NODE *dbArgs;
  GWEN_DB_NODE *profile;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing job");

  dbArgs=AH_Job_GetArguments(j);

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetEStatements_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  const char *responseName;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobGetEStatements");

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
	p=GWEN_DB_GetBinValue(dbXA, "eStatement", 0, 0, 0, &bs);
	if (p && bs) {
	  /* TODO: add eStatement (PDF) to imExporterContext */
	}
      }
    }

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetEStatements_Exchange(AH_JOB *j, AB_JOB *bj,
                                   AH_JOB_EXCHANGE_MODE m,
                                   AB_IMEXPORTER_CONTEXT *ctx){
  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging (%d)", m);

  assert(j);

  if (AB_Job_GetType(bj)!=AB_Job_TypeGetEStatements) {
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










