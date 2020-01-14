/***************************************************************************
 begin       : Mon Jan 13 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif




static GWEN_XMLNODE *_mkBankStatementRqNode(AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j);
static GWEN_XMLNODE *_mkCreditCardStatementRqNode(AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j);
static GWEN_XMLNODE *_mkInvestmentStatementRqNode(AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j);

static const char *_getOfxAccountType(int t);
static void _setDateValue(GWEN_XMLNODE *xmlNode, GWEN_DATE *da, uint32_t userFlags, const char *varName);
static void _setTimeValue(GWEN_XMLNODE *xmlNode, GWEN_TIME *ti, uint32_t userFlags, const char *varName);
static void _setCurrentTimeValue(GWEN_XMLNODE *xmlNode, uint32_t userFlags, const char *varName);






GWEN_XMLNODE *AO_V2_MkStatementRqNode(AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j)
{
  GWEN_XMLNODE *xmlNode;

  assert(a);

  switch (AB_Account_GetAccountType(a)) {
  case AB_AccountType_CreditCard:
    return _mkCreditCardStatementRqNode(pro, u, a, j);

  case AB_AccountType_Investment:
    return _mkInvestmentStatementRqNode(pro, u, a, j);

  case AB_AccountType_Checking:
  case AB_AccountType_Savings:
  case AB_AccountType_Bank:
  case AB_AccountType_Cash:
  case AB_AccountType_Unknown:
  default:
    return _mkBankStatementRqNode(pro, u, a, j);
  }
}




GWEN_XMLNODE *_mkBankStatementRqNode(AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j)
{
  GWEN_XMLNODE *xmlMsg;
  GWEN_XMLNODE *xmlTrnRq;
  GWEN_XMLNODE *xmlRq;
  const char *s;

  xmlMsg=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "BANKMSGSRQV1");
  xmlTrnRq=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "STMTTRNRQ");
  GWEN_XMLNode_AddChild(xmlMsg, xmlRq);

  _setCurrentTimeValue(xmlTrnRq, AO_User_GetFlags(u), "TRNUID");

  xmlRq=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "STMTRQ");
  GWEN_XMLNode_AddChild(xmlTrnRq, xmlRq);

  if (1) {
    GWEN_XMLNODE *xmlAcc;
  
    xmlAcc=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "BANKACCTFROM");
    GWEN_XMLNode_AddChild(xmlRq, xmlAcc);
    if (!(AO_User_GetFlags(u) & AO_USER_FLAGS_EMPTY_BANKID)) {
      /* only copy bank code if not forbidden by user */
      s=AB_Account_GetBankCode(a);
      if (s)
	GWEN_XMLNode_SetCharValue(xmlAcc, "BANKID", s);
    }
  
    s=AB_Account_GetAccountNumber(a);
    if (s)
      GWEN_XMLNode_SetCharValue(xmlAcc, "ACCTID", s);

    GWEN_XMLNode_SetCharValue(xmlAcc, "ACCTTYPE", _getOfxAccountType(AB_Account_GetAccountType(a)));
  }

  if (1) {
    GWEN_XMLNODE *xmlInc;

    xmlInc=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "INCTRAN");
    GWEN_XMLNode_AddChild(xmlRq, xmlAInc);

    if (AB_Transaction_GetCommand(j)==AB_Transaction_CommandGetTransactions) {
      _setDateValue(xmlInc, AB_Transaction_GetFirstDate(j), AO_User_GetFlags(u), "DTSTART");
      _setDateValue(xmlInc, AB_Transaction_GetLastDate(j), AO_User_GetFlags(u), "DTEND");
      GWEN_XMLNode_SetCharValue(xmlInc, "INCLUDE", "Y");
    }
    else
      GWEN_XMLNode_SetCharValue(xmlInc, "INCLUDE", "N");
  }

  return xmlMsg;
}



GWEN_XMLNODE *_mkCreditCardStatementRqNode(AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j)
{
}



GWEN_XMLNODE *_mkInvestmentStatementRqNode(AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j)
{
}





const char *_getOfxAccountType(int t)
{
  switch (t) {
  case AB_AccountType_Checking:
    return "CHECKING";
  case AB_AccountType_Savings:
    return "SAVINGS";
  case AB_AccountType_Bank:
    return "CREDITLINE";
  case AB_AccountType_MoneyMarket:
    return "MONEYMRKT";
  case AB_AccountType_CreditCard:
  case AB_AccountType_Investment:
  case AB_AccountType_Cash:
  case AB_AccountType_Unknown:
  default:
    return "CHECKING";
  }
}



void _setDateValue(GWEN_XMLNODE *xmlNode, GWEN_DATE *da, uint32_t userFlags, const char *varName)
{
  if (da) {
    GWEN_BUFFER *tbuf;
  
    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
    if (userFlags & AO_USER_FLAGS_SEND_SHORT_DATE)
      GWEN_Date_toStringWithTemplate(da, "YYYYMMDD000000", tbuf);
    else
      GWEN_Date_toStringWithTemplate(da, "YYYYMMDD000000.000", tbuf);
    GWEN_XMLNode_SetCharValue(xmlTrnRq, varName, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
}



void _setTimeValue(GWEN_XMLNODE *xmlNode, GWEN_TIME *ti, uint32_t userFlags, const char *varName)
{
  GWEN_BUFFER *tbuf;

  tbuf=GWEN_Buffer_new(0, 32, 0, 1);
  if (userFlags & AO_USER_FLAGS_SEND_SHORT_DATE)
    GWEN_Time_toString(ti, "YYYYMMDDhhmmss", tbuf);
  else
    GWEN_Time_toString(ti, "YYYYMMDDhhmmss.000", tbuf);
  GWEN_Buffer_AppendString(buf, "\r\n");

  GWEN_XMLNode_SetCharValue(xmlNode, varName, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
}



void _setCurrentTimeValue(GWEN_XMLNODE *xmlNode, uint32_t userFlags, const char *varName)
{
  GWEN_TIME *ti;

  ti=GWEN_CurrentTime();
  assert(ti);
  _setTimeValue(xmlNode, ti, userFlags, varName);
  GWEN_Time_free(ti);
}




