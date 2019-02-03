/***************************************************************************
 begin       : Sat Jun 30 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "account_type.h"




const char *AB_AccountType_toChar(AB_ACCOUNT_TYPE ty)
{
  switch (ty) {
  case AB_AccountType_Bank:
    return "bank";
  case AB_AccountType_CreditCard:
    return "creditcard";
  case AB_AccountType_Checking:
    return "checking";
  case AB_AccountType_Savings:
    return "savings";
  case AB_AccountType_Investment:
    return "investment";
  case AB_AccountType_Cash:
    return "cash";
  case AB_AccountType_MoneyMarket:
    return "moneymarket";
  case AB_AccountType_Credit:
    return "credit";
  case AB_AccountType_Unknown:
    return "unknown";
  case AB_AccountType_Invalid:
  case AB_AccountType_Last:
    break;
  }

  return NULL;
}



AB_ACCOUNT_TYPE AB_AccountType_fromChar(const char *s)
{
  if (s && *s) {
    if (strcasecmp(s, "bank")==0)
      return AB_AccountType_Bank;
    else if (strcasecmp(s, "creditcard")==0)
      return AB_AccountType_CreditCard;
    else if (strcasecmp(s, "checking")==0)
      return AB_AccountType_Checking;
    else if (strcasecmp(s, "savings")==0)
      return AB_AccountType_Savings;
    else if (strcasecmp(s, "investment")==0)
      return AB_AccountType_Investment;
    else if (strcasecmp(s, "cash")==0)
      return AB_AccountType_Cash;
    else if (strcasecmp(s, "moneymarket")==0)
      return AB_AccountType_MoneyMarket;
    else if (strcasecmp(s, "credit")==0)
      return AB_AccountType_Credit;
    else if (strcasecmp(s, "unknown")==0)
      return AB_AccountType_Unknown;
  }
  return AB_AccountType_Invalid;
}

