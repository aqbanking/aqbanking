/***************************************************************************
 begin       : Mon Apr 12 2010
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_PINTAN_P_H
#define AQHBCI_DLG_PINTAN_P_H


#include "dlg_pintan_l.h"



typedef struct AH_PINTAN_DIALOG AH_PINTAN_DIALOG;
struct AH_PINTAN_DIALOG {
  AB_BANKING *banking;
  AB_PROVIDER *provider;

  char *bankCode;
  char *bankName;

  char *userName;
  char *userId;
  char *customerId;
  char *url;

  int httpVMajor;
  int httpVMinor;

  int hbciVersion;

  uint32_t flags;

  char *tanMediumId;

  AB_USER *user;
};




#endif

