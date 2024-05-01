/***************************************************************************
 begin       : Wed May 01 2024
 copyright   : (C) 2024 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_WIDGET_UTILS_H
#define AQHBCI_WIDGET_UTILS_H


#include <aqbanking/backendsupport/user.h>
#include <aqbanking/backendsupport/account.h>

#include <gwenhywfar/dialog.h>



typedef const char*(*AH_WIDGET_USER_GETCHARVALUE_FN)(const AB_USER *user);
typedef void (*AH_WIDGET_USER_SETCHARVALUE_FN)(AB_USER *user, const char *s);

void AH_Widget_UserToGuiText(GWEN_DIALOG *dlg, const char *widgetName, const AB_USER *user, AH_WIDGET_USER_GETCHARVALUE_FN fn);
int AH_Widget_GuiTextToUserDeleSpaces(GWEN_DIALOG *dlg, const char *widgetName,
                                      AB_USER *user, AH_WIDGET_USER_SETCHARVALUE_FN fn,
                                      const char *errMsgIfMissing);
int AH_Widget_GuiTextToUserKeepSpaces(GWEN_DIALOG *dlg, const char *widgetName,
                                      AB_USER *user, AH_WIDGET_USER_SETCHARVALUE_FN fn,
                                      const char *errMsgIfMissing);




typedef const char*(*AH_WIDGET_ACCOUNT_GETCHARVALUE_FN)(const AB_ACCOUNT *acc);
typedef void (*AH_WIDGET_ACCOUNT_SETCHARVALUE_FN)(AB_ACCOUNT *acc, const char *s);

void AH_Widget_AccountToGuiText(GWEN_DIALOG *dlg, const char *widgetName, const AB_ACCOUNT *acc, AH_WIDGET_ACCOUNT_GETCHARVALUE_FN fn);
int AH_Widget_GuiTextToAccountDeleSpaces(GWEN_DIALOG *dlg, const char *widgetName,
                                         AB_ACCOUNT *acc, AH_WIDGET_ACCOUNT_SETCHARVALUE_FN fn,
                                         const char *errMsgIfMissing);
int AH_Widget_GuiTextToAccountKeepSpaces(GWEN_DIALOG *dlg, const char *widgetName,
                                         AB_ACCOUNT *acc, AH_WIDGET_ACCOUNT_SETCHARVALUE_FN fn,
                                         const char *errMsgIfMissing);


#endif
