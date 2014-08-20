/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: jobmultitransfer.c 1309 2007-10-24 01:48:05Z martin $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobforeignxferwh_p.h"
#include "aqhbci_l.h"
#include "accountjob_l.h"
#include <aqhbci/account.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/text.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>


GWEN_INHERIT(AH_JOB, AH_JOB_FOREIGNXFERWH);



/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_ForeignTransferWH_new(AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;
  AH_JOB_FOREIGNXFERWH *aj;
  GWEN_DB_NODE *dbArgs;
  GWEN_DB_NODE *dbParams;

  j=AH_AccountJob_new("JobForeignTransferWH", u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 60);

  GWEN_NEW_OBJECT(AH_JOB_FOREIGNXFERWH, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_FOREIGNXFERWH, j, aj,
                       AH_Job_ForeignTransferWH_FreeData);
  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, AH_Job_ForeignTransferWH_Process);
  AH_Job_SetExchangeFn(j, AH_Job_ForeignTransferWH_Exchange);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "bankCode",
		       AB_Account_GetBankCode(account));
  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "accountId",
		       AB_Account_GetAccountNumber(account));

  dbParams=AH_Job_GetParams(j);
  aj->maxTransfers=GWEN_DB_GetIntValue(dbParams, "maxTransfers", 0, 0);
  if (aj->maxTransfers==0 || aj->maxTransfers>AH_JOBFOREIGNXFERWH_MAXTRANS)
    aj->maxTransfers=AH_JOBFOREIGNXFERWH_MAXTRANS;

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
void GWENHYWFAR_CB AH_Job_ForeignTransferWH_FreeData(void *bp, void *p){
  AH_JOB_FOREIGNXFERWH *aj;

  aj=(AH_JOB_FOREIGNXFERWH*)p;

  GWEN_FREE_OBJECT(aj);
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_ForeignTransferWH_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_FOREIGNXFERWH *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_FOREIGNXFERWH, j);
  assert(aj);
  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobForeignTransferWH");

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_ForeignTransferWH_Exchange(AH_JOB *j, AB_JOB *bj,
				      AH_JOB_EXCHANGE_MODE m,
				      AB_IMEXPORTER_CONTEXT *ctx){
  /* this function is not needed since there is no AB_Job for this */
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_ForeignTransferWH_SetDtazv(AH_JOB *j,
				      const uint8_t *dataPtr,
				      uint32_t dataLen) {
  GWEN_DB_NODE *dbArgs;

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  GWEN_DB_SetBinValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "transfers",
		      dataPtr, dataLen);

  return 0;
}





