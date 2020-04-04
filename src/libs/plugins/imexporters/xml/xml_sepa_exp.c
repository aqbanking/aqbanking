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
#include "./paymentgroup.h"

#include "aqbanking/i18n_l.h"

#include <aqbanking/banking.h>
#include <aqbanking/banking_be.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/xml2db.h>




/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static GWEN_DB_NODE *_ctxToSepaDb(AB_IMEXPORTER *ie, const AB_IMEXPORTER_CONTEXT *ctx, GWEN_DB_NODE *dbSubParams);
static AB_IMEXPORTER_XML_PAYMENTGROUP_LIST *_sortIntoPaymentGroups(const AB_IMEXPORTER_CONTEXT *ctx, GWEN_DB_NODE *dbSubParams);
static AB_IMEXPORTER_XML_PAYMENTGROUP *_getMatchingPaymentGroupForTransaction(AB_IMEXPORTER_XML_PAYMENTGROUP_LIST *paymentGroupList,
									      const AB_IMEXPORTER_ACCOUNTINFO *accountInfo,
									      const AB_TRANSACTION *t);
static void _sampleTotalTransactions(AB_IMEXPORTER_XML_PAYMENTGROUP_LIST *paymentGroupList, GWEN_DB_NODE *dbData);

static const char *_createAndWriteMessageId(AB_IMEXPORTER *ie, const char *varName, GWEN_DB_NODE *dbData);
static void _createPaymentInfoIds(AB_IMEXPORTER_XML_PAYMENTGROUP_LIST *paymentGroupList, const char *messageId);

static void _writePaymentGroups(const AB_IMEXPORTER_XML_PAYMENTGROUP_LIST *paymentGroupList,
				GWEN_DB_NODE *dbData,
				GWEN_DB_NODE *dbSubParams);
static void _writeTransactions(const AB_IMEXPORTER_XML_PAYMENTGROUP *paymentGroup, GWEN_DB_NODE *dbData);
static void _writeTransaction(const AB_TRANSACTION *t, GWEN_DB_NODE *dbData);

static void _writeAmountToDbWithoutCurrency(const AB_VALUE *v, const char *varName, GWEN_DB_NODE *dbGroup);
static void _writeAmountToDbWithCurrency(const AB_VALUE *v,
					const char *varNameAmount,
					const char *varNameCurrency,
					GWEN_DB_NODE *dbData);

static void _writeCurrentDateTime(const char *varName, GWEN_DB_NODE *dbData);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AB_ImExporterXML_ExportSepa(AB_IMEXPORTER *ie,
                                AB_IMEXPORTER_CONTEXT *ctx,
                                GWEN_SYNCIO *sio,
				GWEN_DB_NODE *dbParams)
{
  GWEN_DB_NODE *dbSubParams;
  const char *schemaName;
  GWEN_XMLNODE *xmlDocData;
  GWEN_XMLNODE *xmlDocSchema;
  GWEN_XMLNODE *xmlNodeExport;
  GWEN_XMLNODE *n;
  GWEN_DB_NODE *dbData;
  GWEN_XML_CONTEXT *xmlCtx;
  int rv;

  dbSubParams=GWEN_DB_GetGroup(dbParams, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "params");
  if (!dbSubParams) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing \"params\" section in profile");
    return GWEN_ERROR_INVALID;
  }

  schemaName=GWEN_DB_GetCharValue(dbSubParams, "schema", 0, NULL);
  if (!(schemaName && *schemaName)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Schema not specified.");
    return GWEN_ERROR_INVALID;
  }

  xmlDocSchema=AB_ImExporterXML_ReadSchemaFromFile(ie, schemaName);
  if (xmlDocSchema==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Schema \"%s\" not found.", schemaName);
    return GWEN_ERROR_INVALID;
  }

  xmlNodeExport=GWEN_XMLNode_FindFirstTag(xmlDocSchema, "Export", NULL, NULL);
  if (!xmlNodeExport) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing \"Export\" in schema file.");
    return GWEN_ERROR_BAD_DATA;
  }

  xmlDocData=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "xmlRoot");
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "?xml");
  if (n) {
    GWEN_XMLNode_AddHeader(xmlDocData, n);
    GWEN_XMLNode_SetProperty(n, "version", "1.0");
    GWEN_XMLNode_SetProperty(n, "encoding", "UTF-8");
  }

  /* prepare dbData */
  dbData=_ctxToSepaDb(ie, ctx, dbSubParams);
  if (dbData==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here");
    GWEN_XMLNode_free(xmlDocData);
    GWEN_XMLNode_free(xmlDocSchema);
    return GWEN_ERROR_BAD_DATA;
  }

  DBG_ERROR(AQBANKING_LOGDOMAIN, "Got this DB data:");
  GWEN_DB_Dump(dbData, 2);

  /* dbData -> XML node */
  rv=GWEN_XmlFromDb(xmlDocData, xmlNodeExport, dbData);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbData);
    GWEN_XMLNode_free(xmlDocData);
    GWEN_XMLNode_free(xmlDocSchema);
    return rv;
  }
  GWEN_DB_Group_free(dbData);

  /* write document to stream */
  xmlCtx=GWEN_XmlCtxStore_new(NULL, GWEN_XML_FLAGS_SIMPLE | GWEN_XML_FLAGS_HANDLE_HEADERS | GWEN_XML_FLAGS_INDENT);
  rv=GWEN_XMLNode_WriteToStream(xmlDocData, xmlCtx, sio);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_XMLNode_free(xmlDocData);
    GWEN_XMLNode_free(xmlDocSchema);
    return rv;
  }
  GWEN_XmlCtx_free(xmlCtx);

  GWEN_XMLNode_free(xmlDocData);
  GWEN_XMLNode_free(xmlDocSchema);
  return 0;
}



GWEN_DB_NODE *_ctxToSepaDb(AB_IMEXPORTER *ie, const AB_IMEXPORTER_CONTEXT *ctx, GWEN_DB_NODE *dbSubParams)
{
  GWEN_DB_NODE *dbData;
  AB_IMEXPORTER_XML_PAYMENTGROUP_LIST *paymentGroupList;
  const char *messageId;

  dbData=GWEN_DB_Group_new("dbRoot");

  paymentGroupList=_sortIntoPaymentGroups(ctx, dbSubParams);
  if (paymentGroupList==NULL) {
    GWEN_DB_Group_free(dbData);
    return NULL;
  }
  else {
    AB_IMEXPORTER_XML_PAYMENTGROUP *paymentGroup;

    paymentGroup=AB_ImExporterXML_PaymentGroup_List_First(paymentGroupList);
    if (paymentGroup) {
      const char *s;

      s=AB_ImExporterXML_PaymentGroup_GetLocalName(paymentGroup);
      if (s && *s)
	GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, "initiatorName", s);
    }
  }

  messageId=_createAndWriteMessageId(ie, "messageId", dbData);
  if (messageId==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not generate message id, aborting.");
    GWEN_DB_Group_free(dbData);
    return NULL;;
  }
  _writeCurrentDateTime("messageDateTime", dbData);
  _sampleTotalTransactions(paymentGroupList, dbData);

  _createPaymentInfoIds(paymentGroupList, messageId);
  _writePaymentGroups(paymentGroupList, dbData, dbSubParams);

  return dbData;
}



AB_IMEXPORTER_XML_PAYMENTGROUP_LIST *_sortIntoPaymentGroups(const AB_IMEXPORTER_CONTEXT *ctx, GWEN_DB_NODE *dbSubParams)
{
  const AB_IMEXPORTER_ACCOUNTINFO *accountInfo;

  accountInfo=AB_ImExporterContext_GetFirstAccountInfo(ctx);
  if (accountInfo) {
    AB_IMEXPORTER_XML_PAYMENTGROUP_LIST *paymentGroupList;

    paymentGroupList=AB_ImExporterXML_PaymentGroup_List_new();
    while(accountInfo) {
      AB_TRANSACTION *t;

      t=AB_ImExporterAccountInfo_GetFirstTransaction(accountInfo, 0, 0);
      while(t) {
	AB_IMEXPORTER_XML_PAYMENTGROUP *paymentGroup;

	paymentGroup=_getMatchingPaymentGroupForTransaction(paymentGroupList, accountInfo, t);
	if (paymentGroup) {
	  AB_ImExporterXML_PaymentGroup_AddTransaction(paymentGroup, t);
	}
	else {
	  DBG_ERROR(AQBANKING_LOGDOMAIN, "No payment group found?? SNH!");
	  AB_ImExporterXML_PaymentGroup_List_free(paymentGroupList);
	  return NULL;
	}
	t=AB_Transaction_List_Next(t);
      }
      accountInfo=AB_ImExporterAccountInfo_List_Next(accountInfo);
    }
    if (AB_ImExporterXML_PaymentGroup_List_GetCount(paymentGroupList)==0) {
      AB_ImExporterXML_PaymentGroup_List_free(paymentGroupList);
      return NULL;
    }

    return paymentGroupList;
  }
  return NULL;
}



const char *_createAndWriteMessageId(AB_IMEXPORTER *ie, const char *varName, GWEN_DB_NODE *dbData)
{
  GWEN_TIME *ti;
  GWEN_BUFFER *tbuf;
  uint32_t uid;
  char numbuf[32];
  
  ti=GWEN_CurrentTime();
  tbuf=GWEN_Buffer_new(0, 64, 0, 1);
  
  uid=AB_Banking_GetNamedUniqueId(AB_ImExporter_GetBanking(ie), "sepamsg", 1);
  GWEN_Time_toUtcString(ti, "YYYYMMDD-hh:mm:ss-", tbuf);
  snprintf(numbuf, sizeof(numbuf)-1, "%08x", uid);
  GWEN_Buffer_AppendString(tbuf, numbuf);
  GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, varName, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
  GWEN_Time_free(ti);
  return GWEN_DB_GetCharValue(dbData, varName, 0, NULL);
}



void _sampleTotalTransactions(AB_IMEXPORTER_XML_PAYMENTGROUP_LIST *paymentGroupList, GWEN_DB_NODE *dbData)
{ /* create controlSum and tx count for GrpHdr */
  AB_IMEXPORTER_XML_PAYMENTGROUP *paymentGroup;
  int totalNumOfTx=0;
  AB_VALUE *totalControlSum;

  totalControlSum=AB_Value_new();
  paymentGroup=AB_ImExporterXML_PaymentGroup_List_First(paymentGroupList);
  while(paymentGroup) {
    const AB_VALUE *controlSum;

    controlSum=AB_ImExporterXML_PaymentGroup_GetControlSum(paymentGroup);
    if (controlSum)
      AB_Value_AddValue(totalControlSum, controlSum);
    totalNumOfTx+=AB_ImExporterXML_PaymentGroup_GetTransactionCount(paymentGroup);
    paymentGroup=AB_ImExporterXML_PaymentGroup_List_Next(paymentGroup);
  }

  _writeAmountToDbWithoutCurrency(totalControlSum, "controlSum", dbData);
  GWEN_DB_SetCharValueFromInt(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, "numberOfTransactions", totalNumOfTx);
}



void _createPaymentInfoIds(AB_IMEXPORTER_XML_PAYMENTGROUP_LIST *paymentGroupList, const char *messageId)
{ /* create controlSum and tx count for GrpHdr */
  AB_IMEXPORTER_XML_PAYMENTGROUP *paymentGroup;
  int num=0;
  GWEN_BUFFER *tbuf;
  uint32_t pos;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(tbuf, messageId);
  GWEN_Buffer_AppendString(tbuf, "-");
  pos=GWEN_Buffer_GetPos(tbuf);

  paymentGroup=AB_ImExporterXML_PaymentGroup_List_First(paymentGroupList);
  while(paymentGroup) {
    char numbuf[32];

    snprintf(numbuf, sizeof(numbuf)-1, "%0d", (++num));
    GWEN_Buffer_AppendString(tbuf, numbuf);
    AB_ImExporterXML_PaymentGroup_SetId(paymentGroup, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_Crop(tbuf, 0, pos);
    paymentGroup=AB_ImExporterXML_PaymentGroup_List_Next(paymentGroup);
  }
  GWEN_Buffer_free(tbuf);
}



void _writePaymentGroups(const AB_IMEXPORTER_XML_PAYMENTGROUP_LIST *paymentGroupList, GWEN_DB_NODE *dbData, GWEN_DB_NODE *dbSubParams)
{ /* write payment groups */
  AB_IMEXPORTER_XML_PAYMENTGROUP *paymentGroup;

  paymentGroup=AB_ImExporterXML_PaymentGroup_List_First(paymentGroupList);
  while(paymentGroup) {
    GWEN_DB_NODE *dbPaymentGroup;
    const char *s;
    const AB_VALUE *controlSum;
    const GWEN_DATE *date;
    int sequence;

    dbPaymentGroup=GWEN_DB_GetGroup(dbData, GWEN_PATH_FLAGS_CREATE_GROUP, "paymentGroup");

    /*
     paymentInfoId
     batchBooking (true / false)
     numberOfTransactions
     controlSum
     requestedExecutionDate
     ownerName
     iban
     bic
     */

    controlSum=AB_ImExporterXML_PaymentGroup_GetControlSum(paymentGroup);
    if (controlSum)
      _writeAmountToDbWithoutCurrency(controlSum, "controlSum", dbPaymentGroup);
    GWEN_DB_SetCharValueFromInt(dbPaymentGroup, GWEN_DB_FLAGS_OVERWRITE_VARS,
				"numberOfTransactions",
				AB_ImExporterXML_PaymentGroup_GetTransactionCount(paymentGroup));

    s=AB_ImExporterXML_PaymentGroup_GetId(paymentGroup);
    if (s && *s)
      GWEN_DB_SetCharValue(dbPaymentGroup, GWEN_DB_FLAGS_OVERWRITE_VARS, "paymentInfoId", s);

    s=AB_ImExporterXML_PaymentGroup_GetLocalName(paymentGroup);
    if (s && *s)
      GWEN_DB_SetCharValue(dbPaymentGroup, GWEN_DB_FLAGS_OVERWRITE_VARS, "ownerName", s);

    s=AB_ImExporterXML_PaymentGroup_GetLocalBic(paymentGroup);
    if (s && *s)
      GWEN_DB_SetCharValue(dbPaymentGroup, GWEN_DB_FLAGS_OVERWRITE_VARS, "bic", s);

    s=AB_ImExporterXML_PaymentGroup_GetLocalIban(paymentGroup);
    if (s && *s)
      GWEN_DB_SetCharValue(dbPaymentGroup, GWEN_DB_FLAGS_OVERWRITE_VARS, "iban", s);

    date=AB_ImExporterXML_PaymentGroup_GetTransactionDate(paymentGroup);
    if (date)
      GWEN_DB_SetCharValue(dbPaymentGroup, GWEN_DB_FLAGS_OVERWRITE_VARS, "requestedExecutionDate", GWEN_Date_GetString(date));
    else
      GWEN_DB_SetCharValue(dbPaymentGroup, GWEN_DB_FLAGS_OVERWRITE_VARS, "requestedExecutionDate", "1999-01-01");

    s=AB_ImExporterXML_PaymentGroup_GetCreditorSchemeId(paymentGroup);
    if (s && *s)
      GWEN_DB_SetCharValue(dbPaymentGroup, GWEN_DB_FLAGS_OVERWRITE_VARS, "creditorSchemeId", s);

    /* write sequence (if any) */
    sequence=AB_ImExporterXML_PaymentGroup_GetSequence(paymentGroup);
    switch(sequence) {
    case AB_Transaction_SequenceOnce:       s="OOFF"; break;
    case AB_Transaction_SequenceFirst:      s="FRST"; break;
    case AB_Transaction_SequenceFollowing:  s="RCUR"; break;
    case AB_Transaction_SequenceFinal:      s="FNAL"; break;
    default:                                s=NULL;   break;
    }
    if (s && *s)
      GWEN_DB_SetCharValue(dbPaymentGroup, GWEN_DB_FLAGS_OVERWRITE_VARS, "sequence", s);

    _writeTransactions(paymentGroup, dbPaymentGroup);

    paymentGroup=AB_ImExporterXML_PaymentGroup_List_Next(paymentGroup);
  }
}



void _writeTransactions(const AB_IMEXPORTER_XML_PAYMENTGROUP *paymentGroup, GWEN_DB_NODE *dbData)
{
  AB_TRANSACTION_LIST2 *transactionList2;

  transactionList2=AB_ImExporterXML_PaymentGroup_GetTransactionList2(paymentGroup);
  if (transactionList2) {
    AB_TRANSACTION_LIST2_ITERATOR *it;

    it=AB_Transaction_List2_First(transactionList2);
    if (it) {
      const AB_TRANSACTION *t;

      t=AB_Transaction_List2Iterator_Data(it);
      while (t) {
	GWEN_DB_NODE *dbTransaction;

	dbTransaction=GWEN_DB_GetGroup(dbData, GWEN_PATH_FLAGS_CREATE_GROUP, "transaction");
	_writeTransaction(t, dbTransaction);
	t=AB_Transaction_List2Iterator_Next(it);
      }
      AB_Transaction_List2Iterator_free(it);
    }
  }
}



void _writeTransaction(const AB_TRANSACTION *t, GWEN_DB_NODE *dbData)
{
  const char *s;
  const AB_VALUE *v;

  s=AB_Transaction_GetEndToEndReference(t);
  if (s && *s)
    GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, "endToEndReference", s);

  s=AB_Transaction_GetRemoteBic(t);
  if (s && *s)
    GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, "remoteBic", s);

  s=AB_Transaction_GetRemoteIban(t);
  if (s && *s)
    GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, "remoteIban", s);

  s=AB_Transaction_GetRemoteName(t);
  if (s && *s)
    GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, "remoteName", s);

  s=AB_Transaction_GetPurpose(t);
  if (s && *s)
    GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, "purpose", s);

  v=AB_Transaction_GetValue(t);
  if (v)
    _writeAmountToDbWithCurrency(v, "value_value", "value_currency", dbData);

  // TODO: creditorSchemeId, mandateId, sequence etc
}



void _writeAmountToDbWithoutCurrency(const AB_VALUE *v, const char *varName, GWEN_DB_NODE *dbData)
{
  GWEN_BUFFER *tbuf;

  tbuf=GWEN_Buffer_new(0, 64, 0, 1);
  AB_Value_toHumanReadableString(v, tbuf, 2, 0);
  GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, varName, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
}



void _writeAmountToDbWithCurrency(const AB_VALUE *v,
				  const char *varNameAmount,
				  const char *varNameCurrency,
				  GWEN_DB_NODE *dbData)
{
  GWEN_BUFFER *tbuf;
  const char *s;

  tbuf=GWEN_Buffer_new(0, 64, 0, 1);
  AB_Value_toHumanReadableString(v, tbuf, 2, 0);
  GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, varNameAmount, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
  s=AB_Value_GetCurrency(v);
  if (s && *s)
    GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, varNameCurrency, s);
}



void _writeCurrentDateTime(const char *varName, GWEN_DB_NODE *dbData)
{
  GWEN_TIME *ti;
  GWEN_BUFFER *tbuf;

  tbuf=GWEN_Buffer_new(0, 64, 0, 1);
  ti=GWEN_CurrentTime();
  GWEN_Time_toUtcString(ti, "YYYY-MM-DDThh:mm:ss.000Z", tbuf);
  GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, varName, GWEN_Buffer_GetStart(tbuf));
  GWEN_Time_free(ti);
  GWEN_Buffer_free(tbuf);
}



AB_IMEXPORTER_XML_PAYMENTGROUP *_getMatchingPaymentGroupForTransaction(AB_IMEXPORTER_XML_PAYMENTGROUP_LIST *paymentGroupList,
								       const AB_IMEXPORTER_ACCOUNTINFO *accountInfo,
								       const AB_TRANSACTION *t)
{
  const char *tLocalName;
  const char *tLocalBic;
  const char *tLocalIban;
  const char *tCreditorSchemeId;
  const GWEN_DATE *tExecDate;
  int tSequence;
  AB_IMEXPORTER_XML_PAYMENTGROUP *paymentGroup;

  tLocalName=AB_Transaction_GetLocalName(t);
  if (!(tLocalName && *tLocalName))
    tLocalName=AB_ImExporterAccountInfo_GetOwner(accountInfo);
  if (tLocalName==NULL)
    tLocalName="";

  tLocalBic=AB_Transaction_GetLocalBic(t);
  if (!(tLocalBic && *tLocalBic))
    tLocalBic=AB_ImExporterAccountInfo_GetBic(accountInfo);
  if (tLocalBic==NULL)
    tLocalBic="";

  tLocalIban=AB_Transaction_GetLocalIban(t);
  if (!(tLocalIban && *tLocalIban))
    tLocalIban=AB_ImExporterAccountInfo_GetIban(accountInfo);
  if (tLocalIban==NULL)
    tLocalIban="";

  tCreditorSchemeId=AB_Transaction_GetCreditorSchemeId(t);
  if (tCreditorSchemeId==NULL)
    tCreditorSchemeId="";

  tExecDate=AB_Transaction_GetDate(t);

  tSequence=AB_Transaction_GetSequence(t);

  paymentGroup=AB_ImExporterXML_PaymentGroup_List_First(paymentGroupList);
  while(paymentGroup) {
    const char *pLocalName;
    const char *pLocalBic;
    const char *pLocalIban;
    const char *pCreditorSchemeId;
    const GWEN_DATE *pExecDate;
    int pSequence;

    pLocalName=AB_ImExporterXML_PaymentGroup_GetLocalName(paymentGroup);
    if (pLocalName==NULL)
      pLocalName="";
    pLocalBic=AB_ImExporterXML_PaymentGroup_GetLocalBic(paymentGroup);
    if (pLocalBic==NULL)
      pLocalBic="";
    pLocalIban=AB_ImExporterXML_PaymentGroup_GetLocalIban(paymentGroup);
    if (pLocalIban==NULL)
      pLocalIban="";

    pCreditorSchemeId=AB_ImExporterXML_PaymentGroup_GetCreditorSchemeId(paymentGroup);
    if (pCreditorSchemeId==NULL)
      pCreditorSchemeId="";

    pExecDate=AB_ImExporterXML_PaymentGroup_GetTransactionDate(paymentGroup);
    pSequence=AB_ImExporterXML_PaymentGroup_GetSequence(paymentGroup);

    if ( (tSequence==pSequence) &&
	(strcasecmp(tLocalName, pLocalName)==0) &&
	(strcasecmp(tLocalBic, pLocalBic)==0) &&
	(strcasecmp(tLocalIban, pLocalIban)==0) &&
	(strcasecmp(tCreditorSchemeId, pCreditorSchemeId)==0) &&
	(GWEN_Date_Compare(tExecDate, pExecDate)==0) )
      return paymentGroup;
    paymentGroup=AB_ImExporterXML_PaymentGroup_List_Next(paymentGroup);
  }

  /* not found, create new payment group */
  paymentGroup=AB_ImExporterXML_PaymentGroup_new();
  AB_ImExporterXML_PaymentGroup_SetLocalName(paymentGroup, tLocalName);
  AB_ImExporterXML_PaymentGroup_SetLocalBic(paymentGroup, tLocalBic);
  AB_ImExporterXML_PaymentGroup_SetLocalIban(paymentGroup, tLocalIban);
  AB_ImExporterXML_PaymentGroup_SetCreditorSchemeId(paymentGroup, tCreditorSchemeId);
  AB_ImExporterXML_PaymentGroup_SetTransactionDate(paymentGroup, tExecDate);
  AB_ImExporterXML_PaymentGroup_SetSequence(paymentGroup, tSequence);
  AB_ImExporterXML_PaymentGroup_List_Add(paymentGroup, paymentGroupList);
  return paymentGroup;
}









