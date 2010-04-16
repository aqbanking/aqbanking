/***************************************************************************
 begin       : Fri Apr 16 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQBANKING_EDITUSER_DIALOG_P_H
#define AQBANKING_EDITUSER_DIALOG_P_H


#include "dlg_edituser.h"



typedef struct AB_EDIT_USER_DIALOG AB_EDIT_USER_DIALOG;
struct AB_EDIT_USER_DIALOG {
  AB_BANKING *banking;
  AB_USER *user;
  int doLock;

  AB_COUNTRY_CONSTLIST2 *countryList;
};


static void GWENHYWFAR_CB AB_EditUserDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB AB_EditUserDialog_SignalHandler(GWEN_DIALOG *dlg,
							 GWEN_DIALOG_EVENTTYPE t,
							 const char *sender);





#endif

