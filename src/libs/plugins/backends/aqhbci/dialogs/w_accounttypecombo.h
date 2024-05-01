/***************************************************************************
 begin       : Wed May 01 2024
 copyright   : (C) 2024 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_WIDGET_ACCOUNTTYPECOMBO_H
#define AQHBCI_WIDGET_ACCOUNTTYPECOMBO_H


#include <aqbanking/account_type.h>

#include <gwenhywfar/dialog.h>


void AH_Widget_AccountTypeComboSetup(GWEN_DIALOG *dlg, const char *widgetName);
void AH_Widget_AccountTypeComboSetCurrent(GWEN_DIALOG *dlg, const char *widgetName, AB_ACCOUNT_TYPE t);
AB_ACCOUNT_TYPE AH_Widget_AccountTypeComboGetCurrent(GWEN_DIALOG *dlg, const char *widgetName);




#endif
