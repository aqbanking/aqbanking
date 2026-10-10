/***************************************************************************
 begin       : Thu Jul 08 2010
 copyright   : (C) 2024 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_EDITUSER_PINTAN_P_H
#define AQHBCI_DLG_EDITUSER_PINTAN_P_H


#include "dlg_edituserpintan_l.h"

#include "aqhbci/banking/user.h"



typedef struct AH_EDIT_USER_PINTAN_DIALOG AH_EDIT_USER_PINTAN_DIALOG;
struct AH_EDIT_USER_PINTAN_DIALOG {
  AB_BANKING *banking;
  AB_PROVIDER *provider;

  AB_USER *user;
  int doLock;
  int modified;
};



#endif

