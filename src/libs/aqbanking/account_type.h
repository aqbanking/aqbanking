/***************************************************************************
 begin       : Sat Jun 30 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/** @file account_type.h
 * @short Definiton of account types.
 */

#ifndef AQBANKING_ACCOUNT_TYPE_H
#define AQBANKING_ACCOUNT_TYPE_H




typedef enum {
  AB_AccountType_Invalid=-1,
  AB_AccountType_Unknown=0,
  AB_AccountType_Bank,
  AB_AccountType_CreditCard,
  AB_AccountType_Checking,
  AB_AccountType_Savings,
  AB_AccountType_Investment,
  AB_AccountType_Cash,
  AB_AccountType_MoneyMarket,
  AB_AccountType_Credit,
  AB_AccountType_Last
} AB_ACCOUNT_TYPE;


#include <aqbanking/error.h>



/**
 * Translate account type to a string (e.g. AB_AccountType_CreditCard -> "creditcard").
 */
AQBANKING_API
const char *AB_AccountType_toChar(AB_ACCOUNT_TYPE ty);


/**
 * Translate account type from a string (e.g. "creditcard" -> AB_AccountType_CreditCard).
 * @return account type (or AB_AccountType_Invalid on error).
 */
AQBANKING_API
AB_ACCOUNT_TYPE AB_AccountType_fromChar(const char *s);





#endif

