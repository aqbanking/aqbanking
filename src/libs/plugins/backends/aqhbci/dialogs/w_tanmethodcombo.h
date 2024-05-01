/***************************************************************************
 begin       : Wed May 01 2024
 copyright   : (C) 2024 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_WIDGET_TANMETHODCOMBO_H
#define AQHBCI_WIDGET_TANMETHODCOMBO_H


#include "aqhbci/banking/user.h"

#include <gwenhywfar/dialog.h>


void AH_Widget_TanMethodComboRebuild(GWEN_DIALOG *dlg, const char *widgetName, const AH_TAN_METHOD_LIST *ctl);
int AH_Widget_TanMethodComboFindMethodById(GWEN_DIALOG *dlg, const char *widgetName, int id);
void AH_Widget_TanMethodComboSetCurrent(GWEN_DIALOG *dlg, const char *widgetName, int id);
int AH_Widget_TanMethodComboGetCurrent(GWEN_DIALOG *dlg, const char *widgetName);



#endif
