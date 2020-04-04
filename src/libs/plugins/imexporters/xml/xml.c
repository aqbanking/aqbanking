/***************************************************************************
    begin       : Sun Dec 16 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "xml_p.h"
#include "xml_sepa_exp.h"
#include "xml_sepa_imp.h"

#include "aqbanking/i18n_l.h"

#include <aqbanking/banking.h>
#include <aqbanking/banking_be.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/xml2db.h>




GWEN_INHERIT(AB_IMEXPORTER, AB_IMEXPORTER_XML);




/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static AB_TRANSACTION *dbToTransaction(AB_IMEXPORTER *ie, GWEN_DB_NODE *db);
static void handleTransactionDetails(AB_TRANSACTION *t, const char *sDetails);


static void GWENHYWFAR_CB AB_ImExporterXML_FreeData(void *bp, void *p);


static int AB_ImExporterXML_Import(AB_IMEXPORTER *ie,
                                   AB_IMEXPORTER_CONTEXT *ctx,
                                   GWEN_SYNCIO *sio,
                                   GWEN_DB_NODE *params);

static int AB_ImExporterXML_Export(AB_IMEXPORTER *ie,
                                   AB_IMEXPORTER_CONTEXT *ctx,
                                   GWEN_SYNCIO *sio,
                                   GWEN_DB_NODE *params);

static int AB_ImExporterXML_CheckFile(AB_IMEXPORTER *ie, const char *fname);

static GWEN_DB_NODE *AB_ImExporterXML_ImportIntoDbWithSchema(AB_IMEXPORTER *ie, GWEN_XMLNODE *xmlDocData,
                                                             const char *schemaName);
static GWEN_DB_NODE *AB_ImExporterXML_ImportIntoDbWithoutSchema(AB_IMEXPORTER *ie, GWEN_XMLNODE *xmlDocData);

static GWEN_DB_NODE *AB_ImExporterXML_ImportIntoDbWithSchemaDoc(AB_IMEXPORTER *ie, GWEN_XMLNODE *xmlDocData,
                                                                GWEN_XMLNODE *xmlDocSchema);


static GWEN_XMLNODE *AB_ImExporterXML_ReadSchemaFiles(AB_IMEXPORTER *ie);

static GWEN_XMLNODE *AB_ImExporterXML_FindMatchingSchema(AB_IMEXPORTER *ie, GWEN_XMLNODE *xmlNodeAllSchemata,
                                                         GWEN_XMLNODE *xmlDocData);
static const char *AB_ImExporterXML_GetCharValueByPath(GWEN_XMLNODE *xmlNode, const char *path, const char *defValue);


static int AB_ImExporterXML_ImportDb(AB_IMEXPORTER *ie,
                                     AB_IMEXPORTER_CONTEXT *ctx,
                                     GWEN_DB_NODE *dbData);





/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



#ifndef strndup
static char *my_strndup(const char *src, size_t n)
{
  int len;

  len=strlen(src);
  if (len<n)
    return strdup(src);
  else {
    char *cpy;

    cpy=(char *) malloc(n+1);
    assert(cpy);
    memmove(cpy, src, n);
    cpy[n]=0;
    return cpy;
  }
}

#define strndup my_strndup
#endif

AB_IMEXPORTER *AB_ImExporterXML_new(AB_BANKING *ab)
{
  AB_IMEXPORTER *ie;
  AB_IMEXPORTER_XML *ieh;

  ie=AB_ImExporter_new(ab, "xml");
  GWEN_NEW_OBJECT(AB_IMEXPORTER_XML, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AB_IMEXPORTER_XML, ie, ieh, AB_ImExporterXML_FreeData);

  AB_ImExporter_SetImportFn(ie, AB_ImExporterXML_Import);
  AB_ImExporter_SetExportFn(ie, AB_ImExporterXML_Export);
  AB_ImExporter_SetCheckFileFn(ie, AB_ImExporterXML_CheckFile);
  return ie;
}



void GWENHYWFAR_CB AB_ImExporterXML_FreeData(void *bp, void *p)
{
  AB_IMEXPORTER_XML *ieh;

  ieh=(AB_IMEXPORTER_XML *)p;

  GWEN_FREE_OBJECT(ieh);
}



int AB_ImExporterXML_Import(AB_IMEXPORTER *ie,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_SYNCIO *sio,
                            GWEN_DB_NODE *dbParams)
{
  AB_IMEXPORTER_XML *ieh;
  const char *sDocumentType;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AB_IMEXPORTER_XML, ie);
  assert(ieh);

  sDocumentType=GWEN_DB_GetCharValue(dbParams, "params/documentType", 0, NULL);
  if (sDocumentType && strcasecmp(sDocumentType, "SEPA")==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Importing as SEPA document");
    return AB_ImExporterXML_ImportSepa(ie, ctx, sio, dbParams);
  }
  else {
    GWEN_DB_NODE *dbSubParams;
    const char *schemaName;
    GWEN_XMLNODE *xmlDocData;
    GWEN_DB_NODE *dbData;
    int rv;

    dbSubParams=GWEN_DB_GetGroup(dbParams, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "params");
    if (!dbSubParams) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing \"params\" section in profile");
      return GWEN_ERROR_INVALID;
    }
  
    xmlDocData=AB_ImExporterXML_ReadXmlFromSio(ie, sio);
    if (xmlDocData==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not read XML input");
      return GWEN_ERROR_INVALID;
    }
  
    schemaName=GWEN_DB_GetCharValue(dbSubParams, "schema", 0, NULL);
    if (!(schemaName && *schemaName)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Importing file without specified schema.");
      dbData=AB_ImExporterXML_ImportIntoDbWithoutSchema(ie, xmlDocData);
    }
    else {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Importing file with schema \"%s\".", schemaName);
      dbData=AB_ImExporterXML_ImportIntoDbWithSchema(ie, xmlDocData, schemaName);
    }
    if (dbData==NULL) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      GWEN_XMLNode_free(xmlDocData);
      return GWEN_ERROR_BAD_DATA;
    }
    GWEN_XMLNode_free(xmlDocData);
  
    /* import into context */
    rv=AB_ImExporterXML_ImportDb(ie, ctx, dbData);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_DB_Group_free(dbData);
      return rv;
    }
  
    /* done */
    GWEN_DB_Group_free(dbData);
    return 0;
  }
}



int AB_ImExporterXML_Export(AB_IMEXPORTER *ie,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_SYNCIO *sio,
                            GWEN_DB_NODE *dbParams)
{
  AB_IMEXPORTER_XML *ieh;
  const char *sDocumentType;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AB_IMEXPORTER_XML, ie);
  assert(ieh);

  sDocumentType=GWEN_DB_GetCharValue(dbParams, "params/documentType", 0, NULL);
  if (sDocumentType && strcasecmp(sDocumentType, "SEPA")==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Exporting as SEPA document");
    return AB_ImExporterXML_ExportSepa(ie, ctx, sio, dbParams);
  }

  return GWEN_ERROR_NOT_SUPPORTED;
}



int AB_ImExporterXML_CheckFile(AB_IMEXPORTER *ie, const char *fname)
{
  AB_IMEXPORTER_XML *ieh;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AB_IMEXPORTER_XML, ie);
  assert(ieh);

  return 0;
}




GWEN_DB_NODE *AB_ImExporterXML_ImportIntoDbWithSchema(AB_IMEXPORTER *ie, GWEN_XMLNODE *xmlDocData,
                                                      const char *schemaName)
{
  AB_IMEXPORTER_XML *ieh;
  GWEN_XMLNODE *xmlDocSchema;
  GWEN_DB_NODE *dbData;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AB_IMEXPORTER_XML, ie);
  assert(ieh);

  xmlDocSchema=AB_ImExporterXML_ReadSchemaFromFile(ie, schemaName);
  if (xmlDocSchema==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not load schema file for \"%s\"", schemaName);
    return NULL;
  }

  dbData=AB_ImExporterXML_ImportIntoDbWithSchemaDoc(ie, xmlDocData, xmlDocSchema);
  if (dbData==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here");
    return NULL;
  }
  return dbData;
}



GWEN_DB_NODE *AB_ImExporterXML_ImportIntoDbWithoutSchema(AB_IMEXPORTER *ie, GWEN_XMLNODE *xmlDocData)
{
  AB_IMEXPORTER_XML *ieh;
  GWEN_XMLNODE *xmlDocSchema;
  GWEN_DB_NODE *dbData;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AB_IMEXPORTER_XML, ie);
  assert(ieh);

  xmlDocSchema=AB_ImExporterXML_DetermineSchema(ie, xmlDocData);
  if (xmlDocSchema==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Could not determine schema file.");
    return NULL;
  }

  dbData=AB_ImExporterXML_ImportIntoDbWithSchemaDoc(ie, xmlDocData, xmlDocSchema);
  if (dbData==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here");
    return NULL;
  }
  return dbData;
}



GWEN_DB_NODE *AB_ImExporterXML_ImportIntoDbWithSchemaDoc(AB_IMEXPORTER *ie, GWEN_XMLNODE *xmlDocData,
                                                         GWEN_XMLNODE *xmlDocSchema)
{
  AB_IMEXPORTER_XML *ieh;
  GWEN_XMLNODE *xmlNodeSchema;
  GWEN_DB_NODE *dbData;
  int rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AB_IMEXPORTER_XML, ie);
  assert(ieh);

  xmlNodeSchema=GWEN_XMLNode_FindFirstTag(xmlDocSchema, "Import", NULL, NULL);
  if (!xmlNodeSchema) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing \"Import\" in schema file.");
    return NULL;
  }

  dbData=GWEN_DB_Group_new("data");
  rv=GWEN_Xml2Db(xmlDocData, xmlNodeSchema, dbData);
#if 0
  DBG_ERROR(AQBANKING_LOGDOMAIN, "Data received:");
  GWEN_DB_Dump(dbData, 2);
#endif
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbData);
    return NULL;
  }

  return dbData;
}



GWEN_XMLNODE *AB_ImExporterXML_ReadSchemaFromFile(AB_IMEXPORTER *ie, const char *schemaName)
{
  GWEN_BUFFER *tbuf;
  GWEN_BUFFER *fullPathBuffer;
  GWEN_XMLNODE *xmlNodeFile;
  GWEN_XMLNODE *xmlNodeSchema;
  int rv;

  fullPathBuffer=GWEN_Buffer_new(0, 256, 0, 1);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(tbuf, schemaName);
  GWEN_Buffer_AppendString(tbuf, ".xml");

  rv=AB_Banking_FindDataFileForImExporter(AB_ImExporter_GetBanking(ie), "xml", GWEN_Buffer_GetStart(tbuf),
                                          fullPathBuffer);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_Buffer_free(fullPathBuffer);
    return NULL;
  }
  GWEN_Buffer_free(tbuf);

  xmlNodeFile=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "schemaFile");
  rv=GWEN_XML_ReadFile(xmlNodeFile, GWEN_Buffer_GetStart(fullPathBuffer),
                       GWEN_XML_FLAGS_HANDLE_COMMENTS | GWEN_XML_FLAGS_HANDLE_HEADERS);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_XMLNode_free(xmlNodeFile);
    GWEN_Buffer_free(fullPathBuffer);
    return NULL;
  }

  xmlNodeSchema=GWEN_XMLNode_FindFirstTag(xmlNodeFile, "Schema", NULL, NULL);
  if (xmlNodeSchema) {
    GWEN_XMLNode_UnlinkChild(xmlNodeFile, xmlNodeSchema);
    GWEN_XMLNode_free(xmlNodeFile);
    GWEN_Buffer_free(fullPathBuffer);
    return xmlNodeSchema;
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing \"Schema\" in schema file \"%s\", ignoring.",
              GWEN_Buffer_GetStart(fullPathBuffer));
    GWEN_XMLNode_free(xmlNodeFile);
    GWEN_Buffer_free(fullPathBuffer);
    return NULL;
  }
}



GWEN_XMLNODE *AB_ImExporterXML_DetermineSchema(AB_IMEXPORTER *ie, GWEN_XMLNODE *xmlDocData)
{
  GWEN_XMLNODE *xmlNodeAllSchemata;

  xmlNodeAllSchemata=AB_ImExporterXML_ReadSchemaFiles(ie);
  if (xmlNodeAllSchemata) {
    GWEN_XMLNODE *xmlNodeSchema;

    xmlNodeSchema=AB_ImExporterXML_FindMatchingSchema(ie, xmlNodeAllSchemata, xmlDocData);
    if (xmlNodeSchema) {
      GWEN_XMLNode_UnlinkChild(xmlNodeAllSchemata, xmlNodeSchema);
      GWEN_XMLNode_free(xmlNodeAllSchemata);
      return xmlNodeSchema;
    }
    else {
      DBG_INFO(AQBANKING_LOGDOMAIN, "No matching schema");
      GWEN_XMLNode_free(xmlNodeAllSchemata);
      return NULL;
    }
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No schemata");
    return NULL;
  }


}



GWEN_XMLNODE *AB_ImExporterXML_ReadSchemaFiles(AB_IMEXPORTER *ie)
{
  GWEN_STRINGLIST *slDataFiles;

  /* get list of all schema files */
  slDataFiles=AB_Banking_ListDataFilesForImExporter(AB_ImExporter_GetBanking(ie), "xml", "*.xml");
  if (slDataFiles) {
    GWEN_XMLNODE *xmlNodeAllSchemata;
    GWEN_STRINGLISTENTRY *seDataFile;

    xmlNodeAllSchemata=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "allSchemaFiles");

    seDataFile=GWEN_StringList_FirstEntry(slDataFiles);
    while (seDataFile) {
      GWEN_XMLNODE *xmlNodeFile;
      int rv;

      xmlNodeFile=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "schemaFile");
      rv=GWEN_XML_ReadFile(xmlNodeFile, GWEN_StringListEntry_Data(seDataFile),
                           GWEN_XML_FLAGS_HANDLE_COMMENTS | GWEN_XML_FLAGS_HANDLE_HEADERS);
      if (rv<0) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading schema file \"%s\" (%d), ignoring.",
                  GWEN_StringListEntry_Data(seDataFile), rv);
      }
      else {
        GWEN_XMLNODE *xmlNodeSchema;

        xmlNodeSchema=GWEN_XMLNode_FindFirstTag(xmlNodeFile, "Schema", NULL, NULL);
        if (xmlNodeSchema) {
          GWEN_XMLNode_UnlinkChild(xmlNodeFile, xmlNodeSchema);
          GWEN_XMLNode_AddChild(xmlNodeAllSchemata, xmlNodeSchema);
        }
        else {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing \"Schema\" in schema file \"%s\", ignoring.",
                    GWEN_StringListEntry_Data(seDataFile));
        }
      } /* if (xmlNode) */
      GWEN_XMLNode_free(xmlNodeFile);

      seDataFile=GWEN_StringListEntry_Next(seDataFile);
    } /* while(se) */

    GWEN_StringList_free(slDataFiles);

    return xmlNodeAllSchemata;
  } /* if (sl) */
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No data files");
    return NULL;
  }
}



GWEN_XMLNODE *AB_ImExporterXML_FindMatchingSchema(AB_IMEXPORTER *ie, GWEN_XMLNODE *xmlNodeAllSchemata,
                                                  GWEN_XMLNODE *xmlDocData)
{
  GWEN_XMLNODE *xmlNodeSchema;

  xmlNodeSchema=GWEN_XMLNode_FindFirstTag(xmlNodeAllSchemata, "Schema", NULL, NULL);
  while (xmlNodeSchema) {
    GWEN_XMLNODE *xmlNodeDocMatches;

    xmlNodeDocMatches=GWEN_XMLNode_FindFirstTag(xmlNodeSchema, "DocMatches", NULL, NULL);
    if (xmlNodeDocMatches) {
      GWEN_XMLNODE *xmlNodeMatch;

      xmlNodeMatch=GWEN_XMLNode_FindFirstTag(xmlNodeDocMatches, "Match", NULL, NULL);
      if (xmlNodeMatch) {
        const char *xmlPropPath;
        const char *sPattern;

        xmlPropPath=GWEN_XMLNode_GetProperty(xmlNodeMatch, "path", NULL);
        sPattern=GWEN_XMLNode_GetCharValue(xmlNodeMatch, NULL, NULL);
        if (xmlPropPath && *xmlPropPath && sPattern && *sPattern) {
          const char *sDocData;

          sDocData=AB_ImExporterXML_GetCharValueByPath(xmlDocData, xmlPropPath, NULL);
          if (sDocData && *sDocData) {
            if (-1!=GWEN_Text_ComparePattern(sDocData, sPattern, 0)) {
              /* found match */
              DBG_INFO(AQBANKING_LOGDOMAIN, "Document data matches (path=%s, data=%s, pattern=%s",
                       xmlPropPath, sDocData, sPattern);
              return xmlNodeSchema;
            } /* if (-1!=GWEN_Text_ComparePattern(sDocData, sPattern)) */
            else {
              DBG_INFO(AQBANKING_LOGDOMAIN, "Document data does not match (path=%s, data=%s, pattern=%s",
                       xmlPropPath, sDocData, sPattern);
            }
          }
          else {
            DBG_INFO(AQBANKING_LOGDOMAIN, "Missing or empty match data in document (path=%s)", xmlPropPath);
          }
        }  /* if (xmlPropPath && *xmlPropPath && sPattern && *sPattern) */
        else {
          DBG_INFO(AQBANKING_LOGDOMAIN, "Missing data in schema file: path=%s, pattern=%s",
                   (xmlPropPath && *xmlPropPath)?xmlPropPath:"-- empty --",
                   (sPattern && *sPattern)?sPattern:"-- empty --");
        }
      } /* if (xmlNodeMatch) */
      else {
        DBG_INFO(AQBANKING_LOGDOMAIN, "<DocMatches> element has no <Match> element");
      }
    } /* xmlNodeDocMatches */
    else {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Schema has no <DocMatches> element");
      GWEN_XMLNode_Dump(xmlNodeSchema, 2);
    }
    xmlNodeSchema=GWEN_XMLNode_FindNextTag(xmlNodeSchema, "Schema", NULL, NULL);
  } /* while(xmlNodeSchema) */

  return NULL;
}



const char *AB_ImExporterXML_GetCharValueByPath(GWEN_XMLNODE *xmlNode, const char *path, const char *defValue)
{
  const char *s;

  s=strchr(path, '@');
  if (s) {
    int idx;
    char *cpyOfPath;
    char *property;
    GWEN_XMLNODE *n;


    idx=s-path;
    cpyOfPath=strdup(path);
    assert(cpyOfPath);
    cpyOfPath[idx]=0;
    property=cpyOfPath+idx+1;

    if (*cpyOfPath) {
      n=GWEN_XMLNode_GetNodeByXPath(xmlNode, cpyOfPath, GWEN_PATH_FLAGS_PATHMUSTEXIST);
    }
    else
      n=xmlNode;

    if (n) {
      const char *result;

      result=GWEN_XMLNode_GetProperty(n, property, defValue);
      DBG_INFO(GWEN_LOGDOMAIN, "Got XML property: %s = %s (%s)", property, result, path);
      free(cpyOfPath);
      return result;
    }
    free(cpyOfPath);
    return defValue;
  }
  else
    return GWEN_XMLNode_GetCharValueByPath(xmlNode, path, defValue);
}



GWEN_XMLNODE *AB_ImExporterXML_ReadXmlFromSio(AB_IMEXPORTER *ie, GWEN_SYNCIO *sio)
{
  int rv;
  GWEN_XMLNODE *xmlDocRoot;
  GWEN_XML_CONTEXT *xmlCtx;

  /* read whole document into XML tree */
  xmlDocRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "xmlDocRoot");
  xmlCtx=GWEN_XmlCtxStore_new(xmlDocRoot, GWEN_XML_FLAGS_HANDLE_COMMENTS | GWEN_XML_FLAGS_HANDLE_HEADERS);
  rv=GWEN_XMLContext_ReadFromIo(xmlCtx, sio);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_XmlCtx_free(xmlCtx);
    GWEN_XMLNode_free(xmlDocRoot);
    return NULL;
  }
  GWEN_XmlCtx_free(xmlCtx);

  return xmlDocRoot;
}



int AB_ImExporterXML_ImportDb(AB_IMEXPORTER *ie,
                              AB_IMEXPORTER_CONTEXT *ctx,
                              GWEN_DB_NODE *dbData)
{
  GWEN_DB_NODE *dbAccount;

  dbAccount=GWEN_DB_FindFirstGroup(dbData, "account");
  while (dbAccount) {
    AB_ACCOUNT_SPEC *accountSpec;
    AB_IMEXPORTER_ACCOUNTINFO *accountInfo;
    GWEN_DB_NODE *dbCurrent;
    const char *s;

    accountSpec=AB_AccountSpec_fromDb(dbAccount);
    assert(accountSpec);

    accountInfo=AB_ImExporterContext_GetOrAddAccountInfo(ctx,
                                                         0,
                                                         AB_AccountSpec_GetIban(accountSpec),
                                                         AB_AccountSpec_GetBankCode(accountSpec),
                                                         AB_AccountSpec_GetAccountNumber(accountSpec),
                                                         AB_AccountSpec_GetType(accountSpec));
    assert(accountInfo);

    s=AB_ImExporterAccountInfo_GetCurrency(accountInfo);
    if (!(s && *s)) {
      s=AB_AccountSpec_GetCurrency(accountSpec);
      if (s && *s)
        AB_ImExporterAccountInfo_SetCurrency(accountInfo, s);
    }

    /* import transactions */
    dbCurrent=GWEN_DB_FindFirstGroup(dbAccount, "transaction");
    while (dbCurrent) {
      AB_TRANSACTION *t;

      t=dbToTransaction(ie, dbCurrent);
      assert(t);

      AB_ImExporterAccountInfo_AddTransaction(accountInfo, t);
      dbCurrent=GWEN_DB_FindNextGroup(dbCurrent, "transaction");
    }


    /* import balances */
    dbCurrent=GWEN_DB_FindFirstGroup(dbAccount, "balance");
    while (dbCurrent) {
      AB_BALANCE *bal;

      bal=AB_Balance_fromDb(dbCurrent);
      AB_ImExporterAccountInfo_AddBalance(accountInfo, bal);
      dbCurrent=GWEN_DB_FindNextGroup(dbCurrent, "balance");
    }

    AB_AccountSpec_free(accountSpec);

    dbAccount=GWEN_DB_FindNextGroup(dbAccount, "account");
  }

  return 0;
}



AB_TRANSACTION *dbToTransaction(AB_IMEXPORTER *ie, GWEN_DB_NODE *db)
{
  AB_TRANSACTION *t;
  const char *s;

  t=AB_Transaction_fromDb(db);
  assert(t);
  s=GWEN_DB_GetCharValue(db, "transactionDetails", 0, NULL);
  if (s && *s)
    handleTransactionDetails(t, s);

  return t;
}



void handleTransactionDetails(AB_TRANSACTION *t, const char *sDetails)
{
  const char *sStart;
  const char *s;

  s=sDetails;
  if (*s!='N')
    return;
  s++;

  /* transactionKey */
  sStart=s;
  while (*s && *s!='+')
    s++;
  if (s>sStart) {
    char *sCopy;

    sCopy=strndup(sStart, s-sStart);
    assert(sCopy);
    AB_Transaction_SetTransactionKey(t, sCopy);
    free(sCopy);
  }

  /* transaction code */
  if (!(*s))
    return;
  s++;
  sStart=s;
  while (*s && *s!='+')
    s++;
  if (s>sStart) {
    char *sCopy;
    int num=0;

    sCopy=strndup(sStart, s-sStart);
    assert(sCopy);
    if (1!=sscanf(sCopy, "%d", &num)) {
      DBG_WARN(AQBANKING_LOGDOMAIN, "Transaction details with invalid code (2nd element) in \"%s\", ignoring", sDetails);
    }
    else
      AB_Transaction_SetTransactionCode(t, num);
    free(sCopy);
  }

  /* primanota */
  if (!(*s))
    return;
  s++;
  sStart=s;
  while (*s && *s!='+')
    s++;
  if (s>sStart) {
    char *sCopy;

    sCopy=strndup(sStart, s-sStart);
    assert(sCopy);
    AB_Transaction_SetPrimanota(t, sCopy);
    free(sCopy);
  }

#if 0 /* textKexExt, ignored for now */
  if (!(*s))
    return;
  s++;
  sStart=s;
  while (*s && *s!='+')
    s++;
  if (s>sStart) {
    /* 4th field */
  }
#endif
}



