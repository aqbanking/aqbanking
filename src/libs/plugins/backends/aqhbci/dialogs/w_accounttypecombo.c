/***************************************************************************
 begin       : Wed May 01 2024
 copyright   : (C) 2024 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "w_accounttypecombo.h"

#include "aqbanking/i18n_l.h"



void AH_Widget_AccountTypeComboSetup(GWEN_DIALOG *dlg, const char *widgetName)
{
  const GWEN_DIALOG_PROPERTY addValue=GWEN_DialogProperty_AddValue;
  const GWEN_DIALOG_PROPERTY clrValue=GWEN_DialogProperty_ClearValues;

  GWEN_Dialog_SetIntProperty(dlg,  widgetName, clrValue, 0, 0, 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("unknown"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("Bank Account"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("Credit Card Account"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("Checking Account"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("Savings Account"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("Investment Account"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("Cash Account"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("Moneymarket Account"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("Credit"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("Unspecified"), 0);
}



void AH_Widget_AccountTypeComboSetCurrent(GWEN_DIALOG *dlg, const char *widgetName, AB_ACCOUNT_TYPE t)
{
  int v;

  switch(t) {
    case AB_AccountType_Bank:        v=1; break;
    case AB_AccountType_CreditCard:  v=2; break;
    case AB_AccountType_Checking:    v=3; break;
    case AB_AccountType_Savings:     v=4; break;
    case AB_AccountType_Investment:  v=5; break;
    case AB_AccountType_Cash:        v=6; break;
    case AB_AccountType_MoneyMarket: v=7; break;
    case AB_AccountType_Credit:      v=8; break;
    case AB_AccountType_Unspecified: v=9; break;
    default:                         v=0; break;
  }
  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, v, 0);
}



AB_ACCOUNT_TYPE AH_Widget_AccountTypeComboGetCurrent(GWEN_DIALOG *dlg, const char *widgetName)
{
  int idx;

  idx=GWEN_Dialog_GetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, -1);
  switch(idx) {
    case 1:  return AB_AccountType_Bank;
    case 2:  return AB_AccountType_CreditCard;
    case 3:  return AB_AccountType_Checking;
    case 4:  return AB_AccountType_Savings;
    case 5:  return AB_AccountType_Investment;
    case 6:  return AB_AccountType_Cash;
    case 7:  return AB_AccountType_MoneyMarket;
    case 8:  return AB_AccountType_Credit;
    case 9:  return AB_AccountType_Unspecified;
    default: return AB_AccountType_Unknown;
  }
}






