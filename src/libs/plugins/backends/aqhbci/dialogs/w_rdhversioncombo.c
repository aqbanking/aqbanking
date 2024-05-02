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


#include "w_rdhversioncombo.h"

#include "aqhbci/banking/user.h"

#include "aqbanking/i18n_l.h"



void AH_Widget_RdhVersionComboSetup(GWEN_DIALOG *dlg, const char *widgetName)
{
  const GWEN_DIALOG_PROPERTY addValue=GWEN_DialogProperty_AddValue;
  const GWEN_DIALOG_PROPERTY clrValue=GWEN_DialogProperty_ClearValues;

  GWEN_Dialog_SetIntProperty(dlg,  widgetName, clrValue, 0, 0, 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("(auto)"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, "RDH-1", 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, "RDH-2", 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, "RDH-3", 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, "RDH-5", 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, "RDH-6", 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, "RDH-7", 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, "RDH-8", 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, "RDH-9", 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, "RDH-10", 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, "RAH-7", 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, "RAH-9", 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, "RAH-10", 0);
}



void AH_Widget_RdhVersionComboSetCurrent(GWEN_DIALOG *dlg, const char *widgetName, int v)
{
  int cryptMode;
  int version;

  cryptMode=(v>>16);
  version=v & 0xff;

  if (cryptMode==AH_CryptMode_Rdh) {
    switch(version) {
      case 0:  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, 0, 0);  break;
      case 1:  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, 1, 0);  break;
      case 2:  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, 2, 0);  break;
      case 3:  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, 3, 0);  break;
      case 4:  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, 4, 0);  break;
      case 5:  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, 5, 0);  break;
      case 6:  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, 6, 0);  break;
      case 7:  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, 7, 0);  break;
      case 8:  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, 8, 0);  break;
      case 9:  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, 9, 0);  break;
      case 10: GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, 10, 0); break;
      default: break;
    }
  }
  else if (cryptMode==AH_CryptMode_Rah) {
    switch(version) {
      case 7:  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, 10, 0); break;
      case 9:  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, 11, 0); break;
      case 10: GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, 12, 0); break;
      default: break;
    }
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Unhandled crypt mode %d", cryptMode);
  }
}



int AH_Widget_RdhVersionComboGetCurrent(GWEN_DIALOG *dlg, const char *widgetName)
{
  int i;

  i=GWEN_Dialog_GetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, -1);
  switch (i) {
    case 1:  return (AH_CryptMode_Rdh<<8)+1;
    case 2:  return (AH_CryptMode_Rdh<<8)+2;
    case 3:  return (AH_CryptMode_Rdh<<8)+3;
    case 4:  return (AH_CryptMode_Rdh<<8)+5;
    case 5:  return (AH_CryptMode_Rdh<<8)+6;
    case 6:  return (AH_CryptMode_Rdh<<8)+7;
    case 7:  return (AH_CryptMode_Rdh<<8)+8;
    case 8:  return (AH_CryptMode_Rdh<<8)+9;
    case 9:  return (AH_CryptMode_Rdh<<8)+10;
    case 10: return (AH_CryptMode_Rah<<8)+7;
    case 11: return (AH_CryptMode_Rah<<8)+9;
    case 12: return (AH_CryptMode_Rah<<8)+10;
    default: return (AH_CryptMode_Rdh<<8)+0;
  }
}





