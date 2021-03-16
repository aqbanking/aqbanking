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


#include "jobgetestatements2_p.h"
#include "aqhbci_l.h"
#include "accountjob_l.h"
#include "job_l.h"
#include "aqhbci/joblayer/job_crypt.h"
#include "user_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>

#include <aqbanking/types/document.h>

#include <assert.h>





/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_GetEStatements2_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account)
{
  AH_JOB *j;

  j=AH_AccountJob_new("JobGetEStatements2", pro, u, account);
  if (!j) 
    return NULL;

  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandGetEStatements2);

  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, AH_Job_GetEStatements2_Process);
  AH_Job_SetGetLimitsFn(j, AH_Job_GetLimits_EmptyLimits);
  AH_Job_SetHandleCommandFn(j, AH_Job_HandleCommand_Accept);
  AH_Job_SetHandleResultsFn(j, AH_Job_HandleResults_Empty);

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_GetEStatements2_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  const char *responseName;
  AB_ACCOUNT *acc;
  int rv;
  AB_IMEXPORTER_ACCOUNTINFO *iea=NULL;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobGetEStatements2");

  assert(j);

  acc=AH_AccountJob_GetAccount(j);
  assert(acc);

  responseName=AH_Job_GetResponseName(j);


  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  /* search for "Transactions" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while (dbCurr) {
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
//      GWEN_DB_NODE *dbXA;
//
//      dbXA=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
//      if (dbXA)
//        dbXA=GWEN_DB_GetGroup(dbXA, GWEN_PATH_FLAGS_NAMEMUSTEXIST, responseName);
//      if (dbXA) {
//        const void *p;
//        unsigned int bs;
//
//        p=GWEN_DB_GetBinValue(dbXA, "eStatement", 0, 0, 0, &bs);
//        if (p && bs) {
//          AB_DOCUMENT *doc;
//
//          /* TODO: base64-decode if necessary */
//
//          /* add eStatement (PDF) to imExporterContext */
//          doc=AB_Document_new();
//          AB_Document_SetOwnerId(doc, AB_Account_GetUniqueId(acc));
//
//          AB_Document_SetData(doc, p, bs);
//
//          p=GWEN_DB_GetBinValue(dbXA, "ackCode", 0, 0, 0, &bs);
//          if (p && bs) {
//            AB_Document_SetAcknowledgeCode(doc, p, bs);
//          }
//
//          /* get account info for this account */
//          if (iea==NULL) {
//            /* not set yet, find or create it */
//            iea=AB_ImExporterContext_GetOrAddAccountInfo(ctx,
//                                                         AB_Account_GetUniqueId(acc),
//                                                         AB_Account_GetIban(acc),
//                                                         AB_Account_GetBankCode(acc),
//                                                         AB_Account_GetAccountNumber(acc),
//                                                         AB_Account_GetAccountType(acc));
//            assert(iea);
//          }
//
//          /* add document to imexporter context */
//          AB_ImExporterAccountInfo_AddEStatement(iea, doc);
//        }
//      }
    }

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  return 0;
}



