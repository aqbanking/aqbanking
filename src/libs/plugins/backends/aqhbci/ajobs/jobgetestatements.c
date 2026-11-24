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
#include "aqhbci/aqhbci_l.h"
#include "accountjob_l.h"
#include "aqhbci/joblayer/job_l.h"
#include "aqhbci/joblayer/job_crypt.h"
#include "aqhbci/joblayer/job_swift.h"
#include "aqhbci/banking/user_l.h"

#include "aqbanking/types/transaction.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/directory.h>

#include <aqbanking/types/document.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static AH_JOB *_createJob(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account, const char *jobName);
static int AH_Job_GetEstatements_HandleCommand(AH_JOB *j, const AB_TRANSACTION *t);
static int _handleListCommand(AH_JOB *j, const AB_TRANSACTION *t);
static int _process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int _processCamt(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int _processList(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int _addCamtEStatementFormats(AH_JOB *j, GWEN_DB_NODE *dbArgs);
static int _readCamtFormatsForJob(AH_JOB *j, GWEN_DB_NODE *dbArgs, int camtVersion);
static int _appendJsonStr(GWEN_BUFFER *b, const char *s);
static void _sanitizeDate8(const char *src, char *out, size_t outSize);
static AB_DOCUMENT *_createDocFromResponseDb(AH_JOB *j, GWEN_DB_NODE *dbResponse, int runningDocNumber);
static AB_DOCUMENT *_createCamtDocFromPayload(AH_JOB *j, GWEN_DB_NODE *dbResponse, const void *p, unsigned int bs,
                                              int runningDocNumber);
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



AH_JOB *AH_Job_GetEStatementsCAMT_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account)
{
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;
  int rv;

  j=AH_AccountJob_new("JobGetEStatementsCAMT", pro, u, account);
  if (!j)
    return NULL;

  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandGetCamtEStatements);

  AH_Job_SetProcessFn(j, _processCamt);
  AH_Job_SetGetLimitsFn(j, AH_Job_GetLimits_EmptyLimits);
  AH_Job_SetHandleCommandFn(j, AH_Job_GetEstatements_HandleCommand);
  AH_Job_SetHandleResultsFn(j, AH_Job_HandleResults_Empty);

  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  rv=_addCamtEStatementFormats(j, dbArgs);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No supported CAMT format for HKECA (%d)", rv);
    AH_Job_free(j);
    return NULL;
  }

  return j;
}



AH_JOB *AH_Job_GetEStatementList_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *account)
{
  AH_JOB *j;

  j=AH_AccountJob_new("JobGetEStatementList", pro, u, account);
  if (!j)
    return NULL;

  AH_Job_SetSupportedCommand(j, AB_Transaction_CommandGetEStatementList);

  AH_Job_SetProcessFn(j, _processList);
  AH_Job_SetGetLimitsFn(j, AH_Job_GetLimits_EmptyLimits);
  AH_Job_SetHandleCommandFn(j, _handleListCommand);
  AH_Job_SetHandleResultsFn(j, AH_Job_HandleResults_Empty);

  if (AH_Job_GetSegmentVersion(j)==1)
    AH_AccountJob_WriteNationalAccountInfoToArgs(j);

  return j;
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

  /*
   * If the user reqeusted to acknowledge this job,
   * and the bank also wants to acknowledge the job, flag it for acknowledgement.
   */
  if (AB_Transaction_GetAcknowledge(t) == AB_Transaction_AckJobsWithAckCode) {
    s=GWEN_DB_GetCharValue(dbParams, "ackNeeded", 0, 0);
    if (s && !strcmp(s, "J")) {
      AH_Job_AddFlags(j, AH_JOB_FLAGS_ACKNOWLEDGE);
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
        AB_DOCUMENT *doc;

        doc=_createDocFromResponseDb(j, dbXA, ++runningDocNumber);
        if (doc) {
          AB_Document_SetOwnerId(doc, AB_Account_GetUniqueId(acc));
          if (iea==NULL)
            iea=AB_Provider_GetOrAddAccountInfoForAccount(ctx, acc);
          AB_ImExporterAccountInfo_AddEStatement(iea, doc);
        }
      }
    }

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */

  return 0;
}



AB_DOCUMENT *_createDocFromResponseDb(AH_JOB *j, GWEN_DB_NODE *dbResponse, int runningDocNumber)
{
  const void *p;
  unsigned int bs;

  p=GWEN_DB_GetBinValue(dbResponse, "eStatement", 0, 0, 0, &bs);
  if (p && bs) {
    AB_DOCUMENT *doc;
    char *docId;
    int rv;

    /* TODO: base64-decode if necessary */

    /* add eStatement (PDF) to imExporterContext */
    doc=AB_Document_new();
    AB_Document_SetData(doc, p, bs);
    AB_Document_SetMimeType(doc, "application/pdf");

    docId=AH_Job_GenerateIdFromDateTimeAndJobId(j, runningDocNumber);
    if (docId) {
      AB_Document_SetId(doc, docId);
      free(docId);
    }

    p=GWEN_DB_GetBinValue(dbResponse, "ackCode", 0, 0, 0, &bs);
    if (p && bs) {
      AB_Document_SetAcknowledgeCode(doc, p, bs);
    }

    rv=_writeDocToDataDirAndStorePath(j, doc, "pdf");
    if (rv<0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not write document to storage, keeping data in document (%d)", rv);
    }
    else {
      /* clear data in document, because it is written to disk (AB_Document_GetFilePath() has the path) */
      AB_Document_SetData(doc, NULL, 0);
    }
    return doc;
  }

  return NULL;
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



int _handleListCommand(AH_JOB *j, const AB_TRANSACTION *t)
{
  GWEN_DB_NODE *dbArgs;
  GWEN_DB_NODE *dbParams;
  uint32_t maxEntries;
  const char *s;

  dbArgs=AH_Job_GetArguments(j);
  dbParams=AH_Job_GetParams(j);
  assert(dbArgs && dbParams);

  maxEntries=AB_Transaction_GetEstatementMaxEntries(t);
  s=GWEN_DB_GetCharValue(dbParams, "maxEntriesAllowed", 0, NULL);
  if (maxEntries>0 && (!s || strcmp(s, "J")==0))
    GWEN_DB_SetIntValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "maxEntries", maxEntries);

  return 0;
}



int _addCamtEStatementFormats(AH_JOB *j, GWEN_DB_NODE *dbArgs)
{
  int foundAtLeastOneDescr=0;

  if (_readCamtFormatsForJob(j, dbArgs, 53))
    foundAtLeastOneDescr=1;
  if (_readCamtFormatsForJob(j, dbArgs, 54))
    foundAtLeastOneDescr=1;
  if (_readCamtFormatsForJob(j, dbArgs, 52))
    foundAtLeastOneDescr=1;

  if (!foundAtLeastOneDescr) {
    GWEN_DB_NODE *dbParams;
    int i;

    dbParams=AH_Job_GetParams(j);
    if (dbParams) {
      for (i=0; i<100; i++) {
        const char *s;

        s=GWEN_DB_GetCharValue(dbParams, "supportedFormat", i, NULL);
        if (!(s && *s))
          break;
        if (strstr(s, "camt.")!=NULL) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Adding raw BPD CAMT format [%s]", s);
          GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "SupportedFormats/format", s);
          foundAtLeastOneDescr=1;
        }
      }
    }
  }

  if (!foundAtLeastOneDescr) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No supported CAMT profile found for HKECA, job not supported");
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



int _readCamtFormatsForJob(AH_JOB *j, GWEN_DB_NODE *dbArgs, int camtVersion)
{
  AB_SWIFT_DESCR_LIST *descrList;
  int foundAtLeastOneDescr=0;

  descrList=AH_Job_GetSwiftDescriptorsSupportedByJob(j, NULL, "supportedFormat", "camt", camtVersion);
  if (descrList) {
    AB_SWIFT_DESCR *descr;

    descr=AB_SwiftDescr_List_First(descrList);
    while (descr) {
      const char *s;

      s=AB_SwiftDescr_GetAlias2(descr);
      if (s && *s) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Adding supported CAMT format [%s]", s);
        GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "SupportedFormats/format", s);
        foundAtLeastOneDescr=1;
      }
      descr=AB_SwiftDescr_List_Next(descr);
    }
    AB_SwiftDescr_List_free(descrList);
  }

  return foundAtLeastOneDescr;
}



void _sanitizeDate8(const char *src, char *out, size_t outSize)
{
  size_t i;

  if (outSize<9) {
    if (outSize>0)
      out[0]='\0';
    return;
  }

  if (src && *src) {
    size_t len=0;

    for (i=0; src[i]!='\0' && len<8; i++) {
      if (src[i]>='0' && src[i]<='9') {
        out[len++]=src[i];
      }
    }
    if (len==8) {
      out[8]='\0';
      return;
    }
  }

  snprintf(out, outSize, "00000000");
}



AB_DOCUMENT *_createCamtDocFromPayload(AH_JOB *j, GWEN_DB_NODE *dbResponse, const void *p, unsigned int bs,
                                       int runningDocNumber)
{
  AB_DOCUMENT *doc;
  const uint8_t *bytes;
  const char *sDate;
  char dateBuf[16];
  char idBuf[64];
  int year;
  int num;
  const void *ackPtr;
  unsigned int ackLen=0;

  (void)j;

  if (!(p && bs))
    return NULL;

  doc=AB_Document_new();
  if (!doc)
    return NULL;

  bytes=(const uint8_t *)p;
  AB_Document_SetData(doc, bytes, bs);
  if (bs>=4 && bytes[0]=='P' && bytes[1]=='K' && bytes[2]==3 && bytes[3]==4)
    AB_Document_SetMimeType(doc, "application/zip");
  else
    AB_Document_SetMimeType(doc, "application/xml");

  year=GWEN_DB_GetIntValue(dbResponse, "year", 0, 0);
  if (year<0 || year>9999)
    year=0;
  num=GWEN_DB_GetIntValue(dbResponse, "num", 0, 0);
  if (num<0 || num>99999)
    num=0;

  sDate=GWEN_DB_GetCharValue(dbResponse, "date", 0, NULL);
  if (!(sDate && *sDate))
    sDate=GWEN_DB_GetCharValue(dbResponse, "ReportTimespan/endDate", 0, NULL);
  if (!(sDate && *sDate))
    sDate=GWEN_DB_GetCharValue(dbResponse, "ReportTimespan/startDate", 0, NULL);
  _sanitizeDate8(sDate, dateBuf, sizeof(dateBuf));

  snprintf(idBuf, sizeof(idBuf), "eca-%04d-%05d-%s-%d", year, num, dateBuf, runningDocNumber);
  AB_Document_SetId(doc, idBuf);

  ackPtr=GWEN_DB_GetBinValue(dbResponse, "ackCode", 0, 0, 0, &ackLen);
  if (ackPtr && ackLen)
    AB_Document_SetAcknowledgeCode(doc, (const uint8_t *)ackPtr, ackLen);

  return doc;
}



int _processCamt(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  const char *responseName;
  AB_ACCOUNT *acc;
  int rv;
  AB_IMEXPORTER_ACCOUNTINFO *iea=NULL;
  int runningDocNumber=0;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobGetEStatementsCAMT");

  assert(j);
  acc=AH_AccountJob_GetAccount(j);
  assert(acc);
  responseName=AH_Job_GetResponseName(j);
  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

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
        GWEN_DB_NODE *dbBooked;
        int addedInSeg=0;

        dbBooked=GWEN_DB_GetGroup(dbXA, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "booked");
        if (dbBooked) {
          int k;

          for (k=0; k<1000; k++) {
            const void *p;
            unsigned int bs=0;
            AB_DOCUMENT *doc;

            p=GWEN_DB_GetBinValue(dbBooked, "eStatement", k, 0, 0, &bs);
            if (!(p && bs))
              break;
            doc=_createCamtDocFromPayload(j, dbXA, p, bs, ++runningDocNumber);
            if (doc) {
              AB_Document_SetOwnerId(doc, AB_Account_GetUniqueId(acc));
              if (iea==NULL)
                iea=AB_Provider_GetOrAddAccountInfoForAccount(ctx, acc);
              AB_ImExporterAccountInfo_AddEStatement(iea, doc);
              addedInSeg++;
            }
          }
        }
        if (addedInSeg==0) {
          const void *p;
          unsigned int bs=0;

          p=GWEN_DB_GetBinValue(dbXA, "eStatement", 0, 0, 0, &bs);
          if (p && bs) {
            AB_DOCUMENT *doc;

            doc=_createCamtDocFromPayload(j, dbXA, p, bs, ++runningDocNumber);
            if (doc) {
              AB_Document_SetOwnerId(doc, AB_Account_GetUniqueId(acc));
              if (iea==NULL)
                iea=AB_Provider_GetOrAddAccountInfoForAccount(ctx, acc);
              AB_ImExporterAccountInfo_AddEStatement(iea, doc);
            }
          }
        }
      }
    }

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  return 0;
}



int _appendJsonStr(GWEN_BUFFER *b, const char *s)
{
  const unsigned char *p;

  if (!s)
    return GWEN_Buffer_AppendString(b, "null")<0?-1:0;

  if (GWEN_Buffer_AppendByte(b, '"')<0)
    return -1;

  for (p=(const unsigned char *)s; *p; p++) {
    unsigned char c=*p;

    if (c=='"' || c=='\\') {
      if (GWEN_Buffer_AppendByte(b, '\\')<0 || GWEN_Buffer_AppendByte(b, (char)c)<0)
        return -1;
    }
    else if (c=='\b') {
      if (GWEN_Buffer_AppendString(b, "\\b")<0)
        return -1;
    }
    else if (c=='\f') {
      if (GWEN_Buffer_AppendString(b, "\\f")<0)
        return -1;
    }
    else if (c=='\n') {
      if (GWEN_Buffer_AppendString(b, "\\n")<0)
        return -1;
    }
    else if (c=='\r') {
      if (GWEN_Buffer_AppendString(b, "\\r")<0)
        return -1;
    }
    else if (c=='\t') {
      if (GWEN_Buffer_AppendString(b, "\\t")<0)
        return -1;
    }
    else if (c<0x20 || c==0x7f) {
      char ubuf[8];
      int n;

      n=snprintf(ubuf, sizeof(ubuf), "\\u%04x", (unsigned int)c);
      if (n<0 || (size_t)n>=sizeof(ubuf) || GWEN_Buffer_AppendBytes(b, ubuf, (uint32_t)n)<0)
        return -1;
    }
    else if (c<0x80) {
      if (GWEN_Buffer_AppendByte(b, (char)c)<0)
        return -1;
    }
    else {
      int seqLen=0;

      if (c>=0xc2 && c<=0xdf && (p[1]>=0x80 && p[1]<=0xbf)) {
        seqLen=2;
      }
      else if (c>=0xe0 && c<=0xef && (p[1]>=0x80 && p[1]<=0xbf) && (p[2]>=0x80 && p[2]<=0xbf)) {
        if (!((c==0xe0 && p[1]<0xa0) || (c==0xed && p[1]>=0xa0)))
          seqLen=3;
      }
      else if (c>=0xf0 && c<=0xf4 && (p[1]>=0x80 && p[1]<=0xbf) && (p[2]>=0x80 && p[2]<=0xbf) &&
               (p[3]>=0x80 && p[3]<=0xbf)) {
        if (!((c==0xf0 && p[1]<0x90) || (c==0xf4 && p[1]>0x8f)))
          seqLen=4;
      }
      if (seqLen>0) {
        if (GWEN_Buffer_AppendBytes(b, (const char *)p, (uint32_t)seqLen)<0)
          return -1;
        p+=(seqLen-1);
      }
      else {
        /* Escape isolated FinTS ISO-8859-1 high bytes so hikau-list output stays valid UTF-8 JSON. */
        char ubuf[8];
        int n;

        n=snprintf(ubuf, sizeof(ubuf), "\\u%04x", (unsigned int)c);
        if (n<0 || (size_t)n>=sizeof(ubuf) || GWEN_Buffer_AppendBytes(b, ubuf, (uint32_t)n)<0)
          return -1;
      }
    }
  }

  return GWEN_Buffer_AppendByte(b, '"')<0?-1:0;
}



int _processList(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  const char *responseName;
  AB_ACCOUNT *acc;
  GWEN_BUFFER *jbuf;
  AB_IMEXPORTER_ACCOUNTINFO *iea;
  AB_DOCUMENT *doc;
  int first=1;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing JobGetEStatementList");

  assert(j);
  acc=AH_AccountJob_GetAccount(j);
  assert(acc);
  responseName=AH_Job_GetResponseName(j);
  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  jbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  if (!jbuf)
    return GWEN_ERROR_MEMORY_FULL;
  GWEN_Buffer_SubMode(jbuf, GWEN_BUFFER_MODE_ABORT_ON_MEMFULL);
  GWEN_Buffer_SetHardLimit(jbuf, 1048576);
  if (GWEN_Buffer_AppendByte(jbuf, '[')<0) {
    GWEN_Buffer_free(jbuf);
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }

  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while (dbCurr) {
    rv=AH_Job_CheckEncryption(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (encryption)");
      GWEN_Buffer_free(jbuf);
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }
    rv=AH_Job_CheckSignature(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (signature)");
      GWEN_Buffer_free(jbuf);
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }

    if (responseName && *responseName) {
      GWEN_DB_NODE *dbXA;

      dbXA=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
      if (dbXA)
        dbXA=GWEN_DB_GetGroup(dbXA, GWEN_PATH_FLAGS_NAMEMUSTEXIST, responseName);
      if (dbXA) {
        int num;
        int year;
        const char *dateStr;
        const char *timeStr;
        const char *downloadable;
        const char *ack;
        const char *createType;
        const char *docId;
        char hdrBuf[64];
        int hdrLen;

        num=GWEN_DB_GetIntValue(dbXA, "num", 0, 0);
        year=GWEN_DB_GetIntValue(dbXA, "year", 0, 0);
        dateStr=GWEN_DB_GetCharValue(dbXA, "date", 0, NULL);
        timeStr=GWEN_DB_GetCharValue(dbXA, "time", 0, NULL);
        downloadable=GWEN_DB_GetCharValue(dbXA, "downloadable", 0, NULL);
        ack=GWEN_DB_GetCharValue(dbXA, "ack", 0, NULL);
        createType=GWEN_DB_GetCharValue(dbXA, "createType", 0, NULL);
        docId=GWEN_DB_GetCharValue(dbXA, "docId", 0, NULL);

        if (!first && GWEN_Buffer_AppendByte(jbuf, ',') < 0) {
          GWEN_Buffer_free(jbuf);
          return GWEN_ERROR_BUFFER_OVERFLOW;
        }
        first=0;

        hdrLen=snprintf(hdrBuf, sizeof(hdrBuf), "{\"num\":%d,\"year\":%d", num, year);
        if (hdrLen<0 || (size_t)hdrLen>=sizeof(hdrBuf) ||
            GWEN_Buffer_AppendBytes(jbuf, hdrBuf, (uint32_t)hdrLen)<0 ||
            GWEN_Buffer_AppendString(jbuf, ",\"date\":")<0 || _appendJsonStr(jbuf, dateStr)<0 ||
            GWEN_Buffer_AppendString(jbuf, ",\"time\":")<0 || _appendJsonStr(jbuf, timeStr)<0 ||
            GWEN_Buffer_AppendString(jbuf, ",\"downloadable\":")<0 || _appendJsonStr(jbuf, downloadable)<0 ||
            GWEN_Buffer_AppendString(jbuf, ",\"ack\":")<0 || _appendJsonStr(jbuf, ack)<0 ||
            GWEN_Buffer_AppendString(jbuf, ",\"type\":")<0 || _appendJsonStr(jbuf, createType)<0 ||
            GWEN_Buffer_AppendString(jbuf, ",\"docId\":")<0 || _appendJsonStr(jbuf, docId)<0 ||
            GWEN_Buffer_AppendByte(jbuf, '}')<0) {
          GWEN_Buffer_free(jbuf);
          return GWEN_ERROR_BUFFER_OVERFLOW;
        }
      }
    }

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  if (GWEN_Buffer_AppendByte(jbuf, ']')<0) {
    GWEN_Buffer_free(jbuf);
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }

  doc=AB_Document_new();
  if (!doc) {
    GWEN_Buffer_free(jbuf);
    return GWEN_ERROR_MEMORY_FULL;
  }
  AB_Document_SetId(doc, "hikau-list");
  AB_Document_SetMimeType(doc, "application/json");
  AB_Document_SetOwnerId(doc, AB_Account_GetUniqueId(acc));
  AB_Document_SetData(doc, (const uint8_t *)GWEN_Buffer_GetStart(jbuf), GWEN_Buffer_GetUsedBytes(jbuf));
  GWEN_Buffer_free(jbuf);

  iea=AB_Provider_GetOrAddAccountInfoForAccount(ctx, acc);
  AB_ImExporterAccountInfo_AddEStatement(iea, doc);

  return 0;
}



