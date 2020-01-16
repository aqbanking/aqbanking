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


#include "n_statement.h"
#include "n_utils.h"
#include "aqofxconnect/user.h"

#include <gwenhywfar/gwendate.h>





/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static GWEN_XMLNODE *_mkBankStatementRqNode(AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j);
static GWEN_XMLNODE *_mkCreditCardStatementRqNode(AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j);
static GWEN_XMLNODE *_mkInvestmentStatementRqNode(AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j);

static const char *_getOfxAccountType(int t);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


GWEN_XMLNODE *AO_V2_MkStatementRqNode(AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j)
{
  assert(a);

  switch (AB_Account_GetAccountType(a)) {
  case AB_AccountType_CreditCard:
    return _mkCreditCardStatementRqNode(u, a, j);

  case AB_AccountType_Investment:
    return _mkInvestmentStatementRqNode(u, a, j);

  case AB_AccountType_Checking:
  case AB_AccountType_Savings:
  case AB_AccountType_Bank:
  case AB_AccountType_Cash:
  case AB_AccountType_Unknown:
  default:
    return _mkBankStatementRqNode(u, a, j);
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
  GWEN_XMLNode_AddChild(xmlMsg, xmlTrnRq);

  AO_V2_Util_SetCurrentTimeValue(xmlTrnRq, AO_User_GetFlags(u), "TRNUID");

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
    GWEN_XMLNode_AddChild(xmlRq, xmlInc);

    if (AB_Transaction_GetCommand(j)==AB_Transaction_CommandGetTransactions) {
      AO_V2_Util_SetDateValue(xmlInc, AB_Transaction_GetFirstDate(j), AO_User_GetFlags(u), "DTSTART");
      AO_V2_Util_SetDateValue(xmlInc, AB_Transaction_GetLastDate(j), AO_User_GetFlags(u), "DTEND");
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





