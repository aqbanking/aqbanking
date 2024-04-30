/***************************************************************************
 begin       : Thu Apr 15 2010
 copyright   : (C) 2024 by Martin Preuss
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
  AB_PROVIDER *provider;

  AB_ACCOUNT *account;
  int doLock;
};


#endif

