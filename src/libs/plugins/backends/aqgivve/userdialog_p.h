/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AG_USERDIALOG_P_H
#define AG_USERDIALOG_P_H

#include "userdialog.h"
#include <aqbanking/backendsupport/providerqueue.h>


struct AG_USER_DIALOG {
  AB_PROVIDER *provider;
  AB_USER *user;
};


static int GWENHYWFAR_CB AG_UserDialog_SignalHandler(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
int AG_UserDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender);
int AG_UserDialog_AddUser(GWEN_DIALOG *dlg);

#endif

