/***************************************************************************
 begin       : Wed May 01 2024
 copyright   : (C) 2024 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_WIDGET_HBCIVERSIONCOMBO_H
#define AQHBCI_WIDGET_HBCIVERSIONCOMBO_H


#include <gwenhywfar/dialog.h>


void AH_Widget_HbciVersionComboSetup(GWEN_DIALOG *dlg, const char *widgetName);
void AH_Widget_HbciVersionComboSetCurrent(GWEN_DIALOG *dlg, const char *widgetName, int v);
int AH_Widget_HbciVersionComboGetCurrent(GWEN_DIALOG *dlg, const char *widgetName);



#endif
