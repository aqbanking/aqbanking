/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AG_USERDIALOG_H
#define AG_USERDIALOG_H


#include "provider.h"

typedef struct AG_USER_DIALOG AG_USER_DIALOG;

GWEN_DIALOG *AG_GetEditUserDialog (AB_PROVIDER *pro, AB_USER *u);
GWEN_DIALOG *AG_GetNewUserDialog (AB_PROVIDER *pro, int i);


static void GWENHYWFAR_CB AG_UserDialog_FreeData(void *bp, void *p);


#endif

