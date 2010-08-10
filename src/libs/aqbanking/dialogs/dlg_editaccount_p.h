/***************************************************************************
 begin       : Thu Apr 15 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQBANKING_EDITACCOUNT_DIALOG_P_H
#define AQBANKING_EDITACCOUNT_DIALOG_P_H


#include "dlg_editaccount.h"



typedef struct AB_EDIT_ACCOUNT_DIALOG AB_EDIT_ACCOUNT_DIALOG;
struct AB_EDIT_ACCOUNT_DIALOG {
  AB_BANKING *banking;
  AB_ACCOUNT *account;
  int doLock;

  AB_COUNTRY_CONSTLIST2 *countryList;
};


static void GWENHYWFAR_CB AB_EditAccountDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB AB_EditAccountDialog_SignalHandler(GWEN_DIALOG *dlg,
							    GWEN_DIALOG_EVENTTYPE t,
							    const char *sender);





#endif

