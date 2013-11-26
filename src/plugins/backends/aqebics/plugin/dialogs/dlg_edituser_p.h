/***************************************************************************
 begin       : Thu Jul 08 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQEBICS_DLG_EDITUSER_P_H
#define AQEBICS_DLG_EDITUSER_P_H


#include "dlg_edituser_l.h"

#include <aqebics/user.h>



typedef struct EBC_EDIT_USER_DIALOG EBC_EDIT_USER_DIALOG;
struct EBC_EDIT_USER_DIALOG {
  AB_BANKING *banking;
  AB_USER *user;
  int doLock;

  AB_COUNTRY_CONSTLIST2 *countryList;
};


static void GWENHYWFAR_CB EBC_EditUserDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB EBC_EditUserDialog_SignalHandler(GWEN_DIALOG *dlg,
                                                          GWEN_DIALOG_EVENTTYPE t,
                                                          const char *sender);

static void EBC_EditUserDialog_toGui(GWEN_DIALOG *dlg);




#endif

