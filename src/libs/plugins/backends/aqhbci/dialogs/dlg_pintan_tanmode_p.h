/***************************************************************************
 begin       : Tue Sep 17 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_PINTAN_TANMODE_P_H
#define AQHBCI_DLG_PINTAN_TANMODE_P_H


#include "dlg_pintan_tanmode_l.h"

#include "aqhbci/banking/user.h"



typedef struct AH_PINTAN_TANMODE_DIALOG AH_PINTAN_TANMODE_DIALOG;
struct AH_PINTAN_TANMODE_DIALOG {
  AB_BANKING *banking;
  AB_PROVIDER *provider;

  AB_USER *user;
  int doLock;

  AH_TAN_METHOD_LIST *tanMethodList;

};




#endif

