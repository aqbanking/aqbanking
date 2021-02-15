/***************************************************************************
    begin       : Sat Apr 04 2020
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "./xml_p.h"
#include "./xml_sepa_imp.h"

#include "aqbanking/i18n_l.h"

#include <aqbanking/banking.h>
#include <aqbanking/banking_be.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/xml2db.h>




static int _importDbData(GWEN_DB_NODE *dbSource, AB_IMEXPORTER_CONTEXT *ctx);
static AB_TRANSACTION *_dbToTransaction(AB_IMEXPORTER_ACCOUNTINFO *accountInfo,
                                        GWEN_DB_NODE *dbPaymentGroup,
                                        GWEN_DB_NODE *dbTransaction);
static void _transformValue(GWEN_DB_NODE *dbData, const char *varNameValue, const char *varNameCurrency,
                            const char *destVarName);






int AB_ImExporterXML_ImportSepa(AB_IMEXPORTER *ie,
                                AB_IMEXPORTER_CONTEXT *ctx,
                                GWEN_SYNCIO *sio,
                                GWEN_DB_NODE *dbParams)
{
  GWEN_XMLNODE *xmlDocData;
  GWEN_XMLNODE *xmlDocSchema=NULL;
  GWEN_XMLNODE *xmlNodeSchema=NULL;
  const char *schemaName;
  GWEN_DB_NODE *dbData;
  int rv;

  xmlDocData=AB_ImExporterXML_ReadXmlFromSio(ie, sio, GWEN_XML_FLAGS_HANDLE_COMMENTS | GWEN_XML_FLAGS_HANDLE_HEADERS);
  if (xmlDocData==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not read XML input");
    return GWEN_ERROR_INVALID;
  }

  schemaName=GWEN_DB_GetCharValue(dbParams, "params/schema", 0, NULL);
  if (!(schemaName && *schemaName)) {
    xmlDocSchema=AB_ImExporterXML_DetermineSchema(ie, xmlDocData);
    if (xmlDocSchema==NULL) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Could not determine schema file.");
      return GWEN_ERROR_GENERIC;
    }
  }
  else {
    xmlDocSchema=AB_ImExporterXML_ReadSchemaFromFile(ie, schemaName);
    if (xmlDocSchema==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not load schema file for \"%s\"", schemaName);
      return GWEN_ERROR_GENERIC;
    }
  }

  xmlNodeSchema=GWEN_XMLNode_FindFirstTag(xmlDocSchema, "Import", NULL, NULL);
  if (!xmlNodeSchema) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing \"Import\" in schema file.");
    return GWEN_ERROR_GENERIC;
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
    return rv;
  }

  rv=_importDbData(dbData, ctx);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbData);
    return rv;
  }

  GWEN_DB_Group_free(dbData);
  return 0;
}




int _importDbData(GWEN_DB_NODE *dbSource, AB_IMEXPORTER_CONTEXT *ctx)
{
  GWEN_DB_NODE *dbPaymentGroup;

  dbPaymentGroup=GWEN_DB_FindFirstGroup(dbSource, "paymentGroup");
  while (dbPaymentGroup) {
    AB_IMEXPORTER_ACCOUNTINFO *accountInfo;
    GWEN_DB_NODE *dbTransaction;

    accountInfo=AB_ImExporterContext_GetOrAddAccountInfo(ctx,
                                                         0,
                                                         GWEN_DB_GetCharValue(dbPaymentGroup, "iban", 0, NULL),
                                                         NULL,
                                                         NULL,
                                                         AB_AccountType_Unknown);
    assert(accountInfo);

    dbTransaction=GWEN_DB_FindFirstGroup(dbPaymentGroup, "transaction");
    while (dbTransaction) {
      AB_TRANSACTION *t;

      t=_dbToTransaction(accountInfo, dbPaymentGroup, dbTransaction);
      if (t)
        AB_ImExporterAccountInfo_AddTransaction(accountInfo, t);

      dbTransaction=GWEN_DB_FindNextGroup(dbTransaction, "transaction");
    }

    dbPaymentGroup=GWEN_DB_FindNextGroup(dbPaymentGroup, "paymentGroup");
  }

  return 0;
}



AB_TRANSACTION *_dbToTransaction(AB_IMEXPORTER_ACCOUNTINFO *accountInfo,
                                 GWEN_DB_NODE *dbPaymentGroup,
                                 GWEN_DB_NODE *dbTransaction)
{
  AB_TRANSACTION *t;
  const char *s;

  /* translate some entries from DB */
  _transformValue(dbTransaction, "value_value", "value_currency", "value");

  s=GWEN_DB_GetCharValue(dbTransaction, "date", 0, NULL);
  if (!(s && *s)) {
    s=GWEN_DB_GetCharValue(dbPaymentGroup, "requestedExecutionDate", 0, NULL);
    if (s && *s)
      GWEN_DB_SetCharValue(dbTransaction, GWEN_DB_FLAGS_OVERWRITE_VARS, "date", s);
  }

  /* prepare sequence */
  s=GWEN_DB_GetCharValue(dbPaymentGroup, "sequence", 0, NULL);
  if (s && *s) {
    if (strcasecmp(s, "OOFF")==0)
      GWEN_DB_SetCharValue(dbTransaction, GWEN_DB_FLAGS_OVERWRITE_VARS, "sequence", "once");
    else if (strcasecmp(s, "FRST")==0)
      GWEN_DB_SetCharValue(dbTransaction, GWEN_DB_FLAGS_OVERWRITE_VARS, "sequence", "first");
    else if (strcasecmp(s, "RCUR")==0)
      GWEN_DB_SetCharValue(dbTransaction, GWEN_DB_FLAGS_OVERWRITE_VARS, "sequence", "following");
    else if (strcasecmp(s, "FNAL")==0)
      GWEN_DB_SetCharValue(dbTransaction, GWEN_DB_FLAGS_OVERWRITE_VARS, "sequence", "final");
  }

  s=GWEN_DB_GetCharValue(dbPaymentGroup, "creditorSchemeId", 0, NULL);
  if (s && *s)
    GWEN_DB_SetCharValue(dbTransaction, GWEN_DB_FLAGS_OVERWRITE_VARS, "creditorSchemeId", s);


  /* read all values from db */
  t=AB_Transaction_fromDb(dbTransaction);

  s=AB_Transaction_GetLocalIban(t);
  if (!(s && *s))
    AB_Transaction_SetLocalIban(t, GWEN_DB_GetCharValue(dbPaymentGroup, "iban", 0, NULL));

  s=AB_Transaction_GetLocalBic(t);
  if (!(s && *s))
    AB_Transaction_SetLocalBic(t, GWEN_DB_GetCharValue(dbPaymentGroup, "bic", 0, NULL));

  s=AB_Transaction_GetLocalName(t);
  if (!(s && *s))
    AB_Transaction_SetLocalName(t, GWEN_DB_GetCharValue(dbPaymentGroup, "ownerName", 0, NULL));

  return t;
}



void _transformValue(GWEN_DB_NODE *dbData, const char *varNameValue, const char *varNameCurrency,
                     const char *destVarName)
{
  const char *sValue;
  const char *sCurrency=NULL;
  GWEN_BUFFER *tbuf;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  sValue=GWEN_DB_GetCharValue(dbData, varNameValue, 0, NULL);

  if (sValue)
    GWEN_Buffer_AppendString(tbuf, sValue);
  if (varNameCurrency)
    sCurrency=GWEN_DB_GetCharValue(dbData, varNameCurrency, 0, NULL);

  if (sCurrency) {
    GWEN_Buffer_AppendString(tbuf, ":");
    GWEN_Buffer_AppendString(tbuf, sCurrency);
  }

  if (destVarName)
    GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, destVarName, GWEN_Buffer_GetStart(tbuf));

  GWEN_Buffer_free(tbuf);
}




