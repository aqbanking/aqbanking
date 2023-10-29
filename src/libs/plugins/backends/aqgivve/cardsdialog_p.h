/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AG_CARDSDIALOG_P_H
#define AG_CARDSDIALOG_P_H

#include "cardsdialog.h"
#include <aqbanking/backendsupport/provider_be.h>

struct AG_CARDS_DIALOG {
  AB_PROVIDER *provider;
    AG_VOUCHERLIST *cardlist;
  AB_USER *user;
};


int AG_CardsDialog_init ( GWEN_DIALOG *dlg );
static int GWENHYWFAR_CB AG_CardsDialog_SignalHandler(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
int AG_CardsDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender);
int AG_CardsDialog_AddAccount(GWEN_DIALOG *dlg);



static void GWENHYWFAR_CB AG_CardsDialog_FreeData(void *bp, void *p);

#endif


