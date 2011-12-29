/***************************************************************************
 begin       : Thu Apr 15 2010
 copyright   : (C) 2010-2011 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_EDITACCOUNT_DIALOG_P_H
#define AQHBCI_EDITACCOUNT_DIALOG_P_H


#include "dlg_editaccount_l.h"



typedef struct AH_EDIT_ACCOUNT_DIALOG AH_EDIT_ACCOUNT_DIALOG;
struct AH_EDIT_ACCOUNT_DIALOG {
  AB_BANKING *banking;
  AB_ACCOUNT *account;
  int doLock;

  AB_COUNTRY_CONSTLIST2 *countryList;
};


static void GWENHYWFAR_CB AH_EditAccountDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB AH_EditAccountDialog_SignalHandler(GWEN_DIALOG *dlg,
							    GWEN_DIALOG_EVENTTYPE t,
							    const char *sender);





#endif

