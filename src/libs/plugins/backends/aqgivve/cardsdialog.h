/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AG_CARDSDIALOG_H
#define AG_CARDSDIALOG_H


#include "provider.h"
#include "voucherlist.h"

typedef struct AG_CARDS_DIALOG AG_CARDS_DIALOG;


GWEN_DIALOG *AG_CardsDialog_new (AB_PROVIDER *pro, AB_USER *user, AG_VOUCHERLIST *card_list);
static void GWENHYWFAR_CB AG_CardsDialog_FreeData(void *bp, void *p);

#endif


