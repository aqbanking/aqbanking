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


#include "jobgetestatements_l.h"
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
#include <gwenhywfar/directory.h>

#include <aqbanking/types/document.h>

#include <assert.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static AH_JOB *_createJob(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account, const char *jobName);
static int AH_Job_GetEstatements_HandleCommand(AH_JOB *j, const AB_TRANSACTION *t);
static int _process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int _writeDocToDataDirAndStorePath(AH_JOB *j, AB_DOCUMENT *doc, const char *fileNameExt);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


AH_JOB *AH_Job_GetEStatements_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account)
{
  return _createJob(pro, u, account, "JobGetEStatements");
}



AH_JOB *AH_Job_GetEStatements2_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account)
{
  return _createJob(pro, u, account, "JobGetEStatements2");
}



static int AH_Job_GetEstatements_HandleCommand(AH_JOB *j, const AB_TRANSACTION *t)
{
  const char *s;
  GWEN_DB_NODE *dbArgs=AH_Job_GetArguments(j);
  GWEN_DB_NODE *dbParams=AH_Job_GetParams(j);
  assert(dbArgs && dbParams);

  /*
   * FinTS restriction (for both HKEKA and HKEKP):
   * Filtering by "Kontoauszugsnummer" and "Kontoauszugsjahr" is optionally allowed
   * if "Kontoauszugsnummer erlaubt" (BPD) == "J". Else not allowd.
   */
  s=GWEN_DB_GetCharValue(dbParams, "eStatementNumAllowed", 0, 0);
  if (s && !strcmp(s, "J")) {
    const GWEN_DATE *da=AB_Transaction_GetFirstDate(t);
    uint32_t estatementNumber=AB_Transaction_GetEstatementNumber(t);

    if (da) {
      char dbuf[16];
      /* AB_TRANSACTION API specifies YYYYMMDD for the from-date,
       * but estatements can only be filtered by year + document number.
       * Take YYYY from the from-date, and discard the MMDD. */
      snprintf(dbuf, sizeof(dbuf), "%04d",
               GWEN_Date_GetYear(da));
      GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "eStatementYear", dbuf);
    }
    if (estatementNumber>0) {
      GWEN_DB_SetIntValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "eStatementNum", estatementNumber);
    }
  }

  /*
   * FinTS restriction (for both HKEKA and HKEKP):
   * Element "Maximale Anzahl Eintraege" is optionally allowed
   * if "Eingabe Anzahl Eintraege erlaubt" (BPD) == "J". Else not allowed.
   */
  s=GWEN_DB_GetCharValue(dbParams, "maxEntriesAllowed", 0, 0);
  if (s && !strcmp(s, "J")) {
    uint32_t maxEntries=AB_Transaction_GetEstatementMaxEntries(t);
    if (maxEntries>0) {
      GWEN_DB_SetIntValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "maxEntries", maxEntries);
    }
  }
  return 0;
}



AH_JOB *_createJob(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account, const char *jobName)
{
  AH_JOB *j;

  j=AH_AccountJob_new(jobName, pro, u, account);
  if (!j) 
    return NULL;

  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandGetEStatements);

  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, _process);
  AH_Job_SetGetLimitsFn(j, AH_Job_GetLimits_EmptyLimits);
  AH_Job_SetHandleCommandFn(j, AH_Job_GetEstatements_HandleCommand);

  AH_Job_SetHandleResultsFn(j, AH_Job_HandleResults_Empty);

  return j;
}



int _process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  const char *responseName;
  AB_ACCOUNT *acc;
  int rv;
  AB_IMEXPORTER_ACCOUNTINFO *iea=NULL;
  int runningDocNumber=0;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobGetEStatements");

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
      GWEN_DB_NODE *dbXA;

      dbXA=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
      if (dbXA)
        dbXA=GWEN_DB_GetGroup(dbXA, GWEN_PATH_FLAGS_NAMEMUSTEXIST, responseName);
      if (dbXA) {
        const void *p;
        unsigned int bs;

        p=GWEN_DB_GetBinValue(dbXA, "eStatement", 0, 0, 0, &bs);
        if (p && bs) {
          AB_DOCUMENT *doc;
          char *docId;

          /* TODO: base64-decode if necessary */

          /* add eStatement (PDF) to imExporterContext */
          doc=AB_Document_new();
          AB_Document_SetOwnerId(doc, AB_Account_GetUniqueId(acc));

          AB_Document_SetData(doc, p, bs);

          p=GWEN_DB_GetBinValue(dbXA, "ackCode", 0, 0, 0, &bs);
          if (p && bs) {
            AB_Document_SetAcknowledgeCode(doc, p, bs);
          }

          /* get account info for this account */
          if (iea==NULL) {
            /* not set yet, find or create it */
            iea=AB_ImExporterContext_GetOrAddAccountInfo(ctx,
                                                         AB_Account_GetUniqueId(acc),
                                                         AB_Account_GetIban(acc),
                                                         AB_Account_GetBankCode(acc),
                                                         AB_Account_GetAccountNumber(acc),
                                                         AB_Account_GetAccountType(acc));
            assert(iea);
          }

          /* assign document id derived from current datetime, job id and running number */
          docId=AH_Job_GenerateIdFromDateTimeAndJobId(j, ++runningDocNumber);
          if (docId) {
            AB_Document_SetId(doc, docId);
            free(docId);
          }
	  AB_Document_SetMimeType(doc, "application/pdf");

	  rv=_writeDocToDataDirAndStorePath(j, doc, "pdf");
	  if (rv<0) {
	    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not write document to storage, keeping data in document (%d)", rv);
	  }
	  else {
	    /* clear data in document, because it is written to disk (AB_Document_GetFilePath() has the path) */
	    AB_Document_SetData(doc, NULL, 0);
	  }

          /* add document to imexporter context */
          AB_ImExporterAccountInfo_AddEStatement(iea, doc);
        }
      }
    }

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  return 0;
}



int _writeDocToDataDirAndStorePath(AH_JOB *j, AB_DOCUMENT *doc, const char *fileNameExt)
{
  AH_HBCI *hbci;
  AB_USER *user;
  GWEN_BUFFER *pathBuffer;
  int rv;

  hbci=AH_Job_GetHbci(j);
  user=AH_Job_GetUser(j);

  /* pathname: customer data dir / docs / docId.ext */
  pathBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  if (AH_HBCI_AddUserPath(hbci, user, pathBuffer)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not add customer path, cannot store document");
    GWEN_Buffer_free(pathBuffer);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_AppendString(pathBuffer, GWEN_DIR_SEPARATOR_S "docs");

  /* create folder if it not already exists */
  rv=GWEN_Directory_GetPath(GWEN_Buffer_GetStart(pathBuffer), 0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(pathBuffer);
    return rv;
  }

  /* add file name to full path */
  GWEN_Buffer_AppendString(pathBuffer, GWEN_DIR_SEPARATOR_S);
  GWEN_Buffer_AppendString(pathBuffer, AB_Document_GetId(doc));
  GWEN_Buffer_AppendString(pathBuffer, ".");
  GWEN_Buffer_AppendString(pathBuffer, fileNameExt);

  /* check whether the full path (including filename) exists, it should not! */
  rv=GWEN_Directory_GetPath(GWEN_Buffer_GetStart(pathBuffer), GWEN_PATH_FLAGS_NAMEMUSTEXIST|GWEN_PATH_FLAGS_VARIABLE);
  if (rv==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Path \"%s\" already exists (%d)", GWEN_Buffer_GetStart(pathBuffer), rv);
    GWEN_Buffer_free(pathBuffer);
    return GWEN_ERROR_FOUND;
  }

  /* path exists, file does not, write file now */
  rv=GWEN_SyncIo_Helper_WriteFile(GWEN_Buffer_GetStart(pathBuffer),
				  AB_Document_GetDataPtr(doc),
				  AB_Document_GetDataLen(doc));
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(pathBuffer);
    return rv;
  }

  AB_Document_SetFilePath(doc, GWEN_Buffer_GetStart(pathBuffer));
  GWEN_Buffer_free(pathBuffer);
  return 0;
}



