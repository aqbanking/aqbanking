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


#include "w_userstatuscombo.h"

#include "aqhbci/banking/user.h"

#include "aqbanking/i18n_l.h"




void AH_Widget_UserStatusComboSetup(GWEN_DIALOG *dlg, const char *widgetName)
{
  const GWEN_DIALOG_PROPERTY addValue=GWEN_DialogProperty_AddValue;
  const GWEN_DIALOG_PROPERTY clrValue=GWEN_DialogProperty_ClearValues;

  GWEN_Dialog_SetIntProperty(dlg,  widgetName, clrValue, 0, 0, 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("HBCIUserStatus|new"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("HBCIUserStatus|enabled"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("HBCIUserStatus|pending"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("HBCIUserStatus|disabled"), 0);
}



void AH_Widget_UserStatusComboSetCurrent(GWEN_DIALOG *dlg, const char *widgetName, int v)
{
  switch (v) {
    case AH_UserStatusNew:      GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, 0, 0); break;
    case AH_UserStatusEnabled:  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, 1, 0); break;
    case AH_UserStatusPending:  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, 2, 0); break;
    case AH_UserStatusDisabled: GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, 3, 0); break;
    default: break;
  }
}



int AH_Widget_UserStatusComboGetCurrent(GWEN_DIALOG *dlg, const char *widgetName)
{
  int i;

  i=GWEN_Dialog_GetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, -1);
  switch (i) {
    case 0:  return AH_UserStatusNew;
    case 1:  return AH_UserStatusEnabled;
    case 2:  return AH_UserStatusPending;
    case 3:  return AH_UserStatusDisabled;
    default: return AH_UserStatusNew;
  }
}






