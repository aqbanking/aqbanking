/***************************************************************************
 begin       : Thu Jul 08 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_EDITUSER_DDV_P_H
#define AQHBCI_DLG_EDITUSER_DDV_P_H


#include "dlg_edituserddv_l.h"

#include <aqhbci/user.h>



typedef struct AH_EDIT_USER_DDV_DIALOG AH_EDIT_USER_DDV_DIALOG;
struct AH_EDIT_USER_DDV_DIALOG {
  AB_BANKING *banking;
  AB_USER *user;
  int doLock;

  AB_COUNTRY_CONSTLIST2 *countryList;
};


static void GWENHYWFAR_CB AH_EditUserDdvDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB AH_EditUserDdvDialog_SignalHandler(GWEN_DIALOG *dlg,
							    GWEN_DIALOG_EVENTTYPE t,
							    const char *sender);





#endif

