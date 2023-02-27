/***************************************************************************
    begin       : Sat Dec 01 2018
    copyright   : (C) 2022 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "aqpaypal/provider_getstm.h"

#include "aqpaypal/provider_request.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/i18n.h>


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)




static AB_TRANSACTION_LIST *_readTransactionsFromSearchResponse(GWEN_DB_NODE *dbResponse);
static AB_TRANSACTION *readOneTransactionFromSearchResponse(GWEN_DB_NODE *dbT);
static int _possiblyReadTransactionDetails(AB_PROVIDER *pro, AB_USER *u, AB_TRANSACTION_LIST *transactionList);
static void _addTransactionsToAccountInfo(AB_TRANSACTION_LIST *transactionList, AB_IMEXPORTER_ACCOUNTINFO *ai);
static void _setTypeFromLTYPE(const char *s, AB_TRANSACTION *t);
static AB_VALUE *_readValueFromString(const char *s, const char *currencyCode);
static AB_TRANSACTION_STATUS _paymentStatusFromString(const char *s);
static void _readPurposeLinesFromDetailsResponse(GWEN_DB_NODE *dbResponse, AB_TRANSACTION *t);
static int _requestTransactionDetails(AB_PROVIDER *pro, AB_USER *u, AB_TRANSACTION *t);





int APY_Provider_ExecGetTrans(AB_PROVIDER *pro,
                              AB_IMEXPORTER_ACCOUNTINFO *ai,
                              AB_USER *u,
                              AB_TRANSACTION *j)
{
  GWEN_BUFFER *tbuf;
  const GWEN_DATE *da;
  int rv;
  GWEN_DB_NODE *dbResponse;
  AB_TRANSACTION_LIST *transactionList;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=APY_Provider_SetupUrlString(pro, u, tbuf);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    AB_Transaction_SetStatus(j, AB_Transaction_StatusError);
    return rv;
  }

  GWEN_Buffer_AppendString(tbuf, "&method=transactionSearch");

  da=AB_Transaction_GetFirstDate(j);
  if (da==NULL) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing start date");
    GWEN_Buffer_free(tbuf);
    AB_Transaction_SetStatus(j, AB_Transaction_StatusError);
    return GWEN_ERROR_INVALID;
  }
  GWEN_Buffer_AppendString(tbuf, "&startdate=");
  GWEN_Date_toStringWithTemplate(da, "YYYY-MM-DDT00:00:00Z", tbuf);

  da=AB_Transaction_GetLastDate(j);
  if (da) {
    GWEN_Buffer_AppendString(tbuf, "&enddate=");
    GWEN_Date_toStringWithTemplate(da, "YYYY-MM-DDT23:59:59Z", tbuf);
  }

  /* send and receive */
  AB_Transaction_SetStatus(j, AB_Transaction_StatusSending);
  dbResponse=APY_Provider_SendRequestParseResponse(pro, u, GWEN_Buffer_GetStart(tbuf), "transactionSearch");
  if (dbResponse==NULL) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here");
    GWEN_Buffer_free(tbuf);
    AB_Transaction_SetStatus(j, AB_Transaction_StatusError);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_free(tbuf);

  /* handle response */
  AB_Transaction_SetStatus(j, AB_Transaction_StatusAccepted);
  transactionList=_readTransactionsFromSearchResponse(dbResponse);
  GWEN_DB_Group_free(dbResponse);
  if (transactionList) {
    rv=_possiblyReadTransactionDetails(pro, u, transactionList);
    if (rv<0) {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
      AB_Transaction_SetStatus(j, AB_Transaction_StatusError);
    }
    _addTransactionsToAccountInfo(transactionList, ai);
  }
  AB_Transaction_List_free(transactionList);

  return 0;
}



int _requestTransactionDetails(AB_PROVIDER *pro, AB_USER *u, AB_TRANSACTION *t)
{
  GWEN_BUFFER *tbuf;
  const char *s;
  int rv;
  GWEN_DB_NODE *dbResponse;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=APY_Provider_SetupUrlString(pro, u, tbuf);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  GWEN_Buffer_AppendString(tbuf, "&method=getTransactionDetails");

  s=AB_Transaction_GetFiId(t);
  if (!(s && *s)) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing transaction id");
    GWEN_Buffer_free(tbuf);
    return GWEN_ERROR_INVALID;
  }
  GWEN_Buffer_AppendString(tbuf, "&transactionId=");
  GWEN_Text_EscapeToBuffer(s, tbuf);

  /* send and receive */
  dbResponse=APY_Provider_SendRequestParseResponse(pro, u, GWEN_Buffer_GetStart(tbuf), "getTransactionDetails");
  if (dbResponse==NULL) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here");
    GWEN_Buffer_free(tbuf);
    return GWEN_ERROR_GENERIC;
  }

  GWEN_Buffer_free(tbuf);

  /* parse response */
  s=GWEN_DB_GetCharValue(dbResponse, "TRANSACTIONTYPE", 0, NULL);
  if (s && *s)
    AB_Transaction_SetTransactionText(t, s);
  /* address */
  s=GWEN_DB_GetCharValue(dbResponse, "SHIPTOSTREET", 0, NULL);
  if (s && *s)
    AB_Transaction_SetRemoteAddrStreet(t, s);
  s=GWEN_DB_GetCharValue(dbResponse, "SHIPTOCITY", 0, NULL);
  if (s && *s)
    AB_Transaction_SetRemoteAddrCity(t, s);
  s=GWEN_DB_GetCharValue(dbResponse, "SHIPTOZIP", 0, NULL);
  if (s && *s)
    AB_Transaction_SetRemoteAddrZipcode(t, s);

  AB_Transaction_SetStatus(t, _paymentStatusFromString(GWEN_DB_GetCharValue(dbResponse, "PAYMENTSTATUS", 0, NULL)));

  s=GWEN_DB_GetCharValue(dbResponse, "BUYERID", 0, NULL);
  if (s && *s)
    AB_Transaction_SetBankReference(t, s);
  s=GWEN_DB_GetCharValue(dbResponse, "NOTE", 0, NULL);
  if (s && *s)
    AB_Transaction_AddPurposeLine(t, s);

  _readPurposeLinesFromDetailsResponse(dbResponse, t);

  GWEN_DB_Group_free(dbResponse);

  return 0;
}



/* return a list of parsed transactions.
 * transactions with command==AB_Transaction_CommandGetTransactions will need a request for
 * details.
 * returns NULL if no transactions found.
 */
AB_TRANSACTION_LIST *_readTransactionsFromSearchResponse(GWEN_DB_NODE *dbResponse)
{
  GWEN_DB_NODE *dbT;
  AB_TRANSACTION_LIST *transactionList=NULL;

  /* now get the transactions */
  transactionList=AB_Transaction_List_new();
  dbT=GWEN_DB_GetFirstGroup(dbResponse);
  while (dbT) {
    const char *s;

    s=GWEN_DB_GetCharValue(dbT, "L_TYPE", 0, NULL);
    if (s && *s && !((strcasecmp(s, "Authorization")==0 || strcasecmp(s, "Order")==0))) {
      /* only if L_TYPE is given and it is neither "Authorization" nor "Order" */
      s=GWEN_DB_GetCharValue(dbT, "L_STATUS", 0, NULL);
      if (!(s && *s && (strcasecmp(s, "Placed")==0 || strcasecmp(s, "Removed")==0))) {
	AB_TRANSACTION *t;

	t=readOneTransactionFromSearchResponse(dbT);
	if (t)
	  AB_Transaction_List_Add(t, transactionList);
      }
    }
    dbT=GWEN_DB_GetNextGroup(dbT);
  } /* while(dbT) */

  /* dont return empty list */
  if (transactionList && AB_Transaction_List_GetCount(transactionList)<1) {
    AB_Transaction_List_free(transactionList);
    transactionList=NULL;
  }
  return transactionList;
}



AB_TRANSACTION *readOneTransactionFromSearchResponse(GWEN_DB_NODE *dbT)
{
  AB_TRANSACTION *t;
  AB_VALUE *v;
  const char *currencyCode;
  const char *s;
  
  currencyCode=GWEN_DB_GetCharValue(dbT, "L_CURRENCYCODE", 0, NULL);
  
  t=AB_Transaction_new();
  s=GWEN_DB_GetCharValue(dbT, "L_TIMESTAMP", 0, NULL);
  if (s && *s) {
    GWEN_DATE *da;
  
    da=GWEN_Date_fromStringWithTemplate(s, "YYYY-MM-DD");
    if (da) {
      AB_Transaction_SetDate(t, da);
      AB_Transaction_SetValutaDate(t, da);
      GWEN_Date_free(da);
    }
    else {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Invalid timespec [%s]", s);
    }
  }
  
  s=GWEN_DB_GetCharValue(dbT, "L_TYPE", 0, NULL);
  if (s && *s) {
    AB_Transaction_SetTransactionText(t, s);
    _setTypeFromLTYPE(s, t);
  }
  
  s=GWEN_DB_GetCharValue(dbT, "L_NAME", 0, NULL);
  if (s && *s) {
    const char *sEmail;

    sEmail=GWEN_DB_GetCharValue(dbT, "L_EMAIL", 0, NULL);
    if (sEmail && *sEmail) {
      GWEN_BUFFER *pbuf;

      pbuf=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Buffer_AppendArgs(pbuf, "%s (%s)", s, sEmail);
      AB_Transaction_SetRemoteName(t, GWEN_Buffer_GetStart(pbuf));
      GWEN_Buffer_free(pbuf);
    }
    else
      AB_Transaction_SetRemoteName(t, s);
  }
  else {
    const char *sEmail;

    sEmail=GWEN_DB_GetCharValue(dbT, "L_EMAIL", 0, NULL);
    if (sEmail && *sEmail)
      AB_Transaction_SetRemoteName(t, sEmail);
  }
  
  s=GWEN_DB_GetCharValue(dbT, "L_TRANSACTIONID", 0, NULL);
  if (s && *s)
    AB_Transaction_SetFiId(t, s);
  
  v=_readValueFromString(GWEN_DB_GetCharValue(dbT, "L_AMT", 0, NULL), currencyCode);
  if (v) {
    AB_Transaction_SetValue(t, v);
    AB_Value_free(v);
  }
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "No or invalid amount");
  }

  v=_readValueFromString(GWEN_DB_GetCharValue(dbT, "L_FEEAMT", 0, NULL), currencyCode);
  if (v) {
    AB_Transaction_SetFees(t, v);
    AB_Value_free(v);
  }
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "No or invalid amount");
  }

  s=GWEN_DB_GetCharValue(dbT, "L_STATUS", 0, NULL);
  if (s && *s) {
    if (strcasecmp(s, "Completed")==0)
      AB_Transaction_SetStatus(t, AB_Transaction_StatusAccepted);
    else
      AB_Transaction_SetStatus(t, AB_Transaction_StatusPending);
  }
  
  AB_Transaction_SetCommand(t, AB_Transaction_CommandNone);
  /* get transaction details */
  s=AB_Transaction_GetFiId(t);
  if (s && *s) {
    const char *s2;
  
    s2=GWEN_DB_GetCharValue(dbT, "L_TYPE", 0, NULL);
    if (s2 && *s2) {
      /* only get details for payments (maybe add other types later) */
      if (strcasecmp(s2, "Payment")==0 ||
	  strcasecmp(s2, "Purchase")==0 ||
	  strcasecmp(s2, "Donation")==0) {
	/* internal marker to mark transactions which need detailed requests later */
	AB_Transaction_SetCommand(t, AB_Transaction_CommandGetTransactions);
      }
    }
  }

  return t;
}



int _possiblyReadTransactionDetails(AB_PROVIDER *pro, AB_USER *u, AB_TRANSACTION_LIST *transactionList)
{
  AB_TRANSACTION *transaction;
  int count=0;
  int lastError=0;

  transaction=AB_Transaction_List_First(transactionList);
  while(transaction) {
    if (AB_Transaction_GetCommand(transaction)==AB_Transaction_CommandGetTransactions)
      count++;
    transaction=AB_Transaction_List_Next(transaction);
  }

  DBG_INFO(AQPAYPAL_LOGDOMAIN, "Need to read transaction details for %d transactions", count);
  if (count) {
    int i=1;

    GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Notice, I18N("Need to read details for %d transactions"), count);
    transaction=AB_Transaction_List_First(transactionList);
    while(transaction) {
      if (AB_Transaction_GetCommand(transaction)==AB_Transaction_CommandGetTransactions) {
	int rv;

	DBG_INFO(AQPAYPAL_LOGDOMAIN, "Reading details for transaction %d of %d", i, count);
	GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Notice, I18N("Reading details for transactions %d of %d"), i, count);
	rv=_requestTransactionDetails(pro, u, transaction);
	if (rv<0) {
	  DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
	  lastError=rv;
	}
	else
	  AB_Transaction_SetCommand(transaction, AB_Transaction_CommandNone); /* remove mark */
        i++;
      }
      transaction=AB_Transaction_List_Next(transaction);
    }
  }
  else
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice, I18N("No transaction details needed"));

  return lastError;
}



void _addTransactionsToAccountInfo(AB_TRANSACTION_LIST *transactionList, AB_IMEXPORTER_ACCOUNTINFO *ai)
{
  AB_TRANSACTION *transaction;

  while((transaction=AB_Transaction_List_First(transactionList))) {
    AB_Transaction_List_Del(transaction);
    AB_ImExporterAccountInfo_AddTransaction(ai, transaction);
  }
}



void _setTypeFromLTYPE(const char *s, AB_TRANSACTION *t)
{
  if (strcasecmp(s, "Transfer")==0) {
    AB_Transaction_SetType(t, AB_Transaction_TypeStatement);
    AB_Transaction_SetSubType(t, AB_Transaction_SubTypeStandard);
  }
  else if (strcasecmp(s, "Payment")==0) {
    AB_Transaction_SetType(t, AB_Transaction_TypeStatement);
    AB_Transaction_SetSubType(t, AB_Transaction_SubTypeStandard);
  }
  else {
    AB_Transaction_SetType(t, AB_Transaction_TypeStatement);
    AB_Transaction_SetSubType(t, AB_Transaction_SubTypeStandard);
  }
}



AB_TRANSACTION_STATUS _paymentStatusFromString(const char *s)
{
  if (s && *s) {
    if (strcasecmp(s, "Completed")==0)
      return AB_Transaction_StatusAccepted;
    else if (strcasecmp(s, "Denied")==0 ||
	     strcasecmp(s, "Failed")==0 ||
	     strcasecmp(s, "Expired")==0 ||
	     strcasecmp(s, "Voided")==0)
      return AB_Transaction_StatusRejected;
    else if (strcasecmp(s, "Pending")==0 ||
	     strcasecmp(s, "Processed")==0)
      return AB_Transaction_StatusPending;
    else if (strcasecmp(s, "Refunded")==0 ||
	     strcasecmp(s, "Reversed")==0)
      return AB_Transaction_StatusRevoked;
    else {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "Unknown payment status (%s)", s);
    }
  }
  return AB_Transaction_StatusUnknown;
}



AB_VALUE *_readValueFromString(const char *s, const char *currencyCode)
{
  if (s && *s) {
    AB_VALUE *v;
  
    v=AB_Value_fromString(s);
    if (v) {
      AB_Value_SetCurrency(v, currencyCode);
      return v;
    }
    else {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Invalid amount [%s]", s);
    }
  }

  return NULL;
}



void _readPurposeLinesFromDetailsResponse(GWEN_DB_NODE *dbResponse, AB_TRANSACTION *t)
{
  GWEN_DB_NODE *dbT;
  dbT=GWEN_DB_GetFirstGroup(dbResponse);
  if (dbT) {
    GWEN_BUFFER *pbuf;

    pbuf=GWEN_Buffer_new(0, 256, 0, 1);
    while (dbT) {
      const char *s;

      s=GWEN_DB_GetCharValue(dbT, "L_QTY", 0, NULL);
      if (s && *s)
	GWEN_Buffer_AppendArgs(pbuf, "%sx", s);

      s=GWEN_DB_GetCharValue(dbT, "L_NAME", 0, NULL);
      if (s && *s) {
	GWEN_Buffer_AppendString(pbuf, s);
	s=GWEN_DB_GetCharValue(dbT, "L_NUMBER", 0, NULL);
	if (s && *s)
	  GWEN_Buffer_AppendArgs(pbuf, "(%s)", s);
      }
      else {
	s=GWEN_DB_GetCharValue(dbT, "L_NUMBER", 0, NULL);
	if (s && *s)
	  GWEN_Buffer_AppendString(pbuf, s);
      }

      s=GWEN_DB_GetCharValue(dbT, "L_AMT", 0, NULL);
      if (s && *s) {
	GWEN_Buffer_AppendString(pbuf, "[");
	GWEN_Buffer_AppendString(pbuf, s);
	s=GWEN_DB_GetCharValue(dbT, "L_CURRENCYCODE", 0, NULL);
	if (s && *s)
	  GWEN_Buffer_AppendArgs(pbuf, " %s", s);
	GWEN_Buffer_AppendString(pbuf, "]");
      }

      AB_Transaction_AddPurposeLine(t, GWEN_Buffer_GetStart(pbuf));
      GWEN_Buffer_Reset(pbuf);

      dbT=GWEN_DB_GetNextGroup(dbT);
    } /* while(dbT) */
    GWEN_Buffer_free(pbuf);
  } /* if (dbT) */
}







