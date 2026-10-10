/***************************************************************************
 begin       : Wed May 01 2024
 copyright   : (C) 2024 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_WIDGET_USERCOMBO_H
#define AQHBCI_WIDGET_USERCOMBO_H


#include "aqhbci/banking/user.h"
#include "aqhbci/banking/provider.h"

#include <gwenhywfar/dialog.h>


void AH_Widget_UserComboRebuild(GWEN_DIALOG *dlg, const char *widgetName, AB_PROVIDER *provider);
int AH_Widget_UserComboFindUserByUid(GWEN_DIALOG *dlg, const char *widgetName, uint32_t uid);
void AH_Widget_UserComboSetCurrent(GWEN_DIALOG *dlg, const char *widgetName, uint32_t uid);
uint32_t AH_Widget_UserComboGetCurrent(GWEN_DIALOG *dlg, const char *widgetName);







#endif
