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


#include "w_hbciversioncombo.h"

#include "aqbanking/i18n_l.h"



void AH_Widget_HbciVersionComboSetup(GWEN_DIALOG *dlg, const char *widgetName)
{
  const GWEN_DIALOG_PROPERTY addValue=GWEN_DialogProperty_AddValue;
  const GWEN_DIALOG_PROPERTY clrValue=GWEN_DialogProperty_ClearValues;

  GWEN_Dialog_SetIntProperty(dlg,  widgetName, clrValue, 0, 0, 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("-- select --"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, "2.20", 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, "3.0", 0);
}



void AH_Widget_HbciVersionComboSetCurrent(GWEN_DIALOG *dlg, const char *widgetName, int v)
{
  const GWEN_DIALOG_PROPERTY setValue=GWEN_DialogProperty_Value;

  switch (v) {
    case 220: GWEN_Dialog_SetIntProperty(dlg, widgetName, setValue, 0, 1, 0); break;
    case 300: GWEN_Dialog_SetIntProperty(dlg, widgetName, setValue, 0, 2, 0); break;
    default:  GWEN_Dialog_SetIntProperty(dlg, widgetName, setValue, 0, 2, 0); break;
  }
}



int AH_Widget_HbciVersionComboGetCurrent(GWEN_DIALOG *dlg, const char *widgetName)
{
  int idx;

  idx=GWEN_Dialog_GetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, -1);
  switch(idx) {
    case 1:  return 220;
    case 2:  return 300;
    default: return 300;
  }
}





