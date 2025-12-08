/***************************************************************************
 begin       : Wed Apr 14 2010
 copyright   : (C) 2025 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_PINTAN_SPECIAL_P_H
#define AQHBCI_DLG_PINTAN_SPECIAL_P_H


#include "dlg_pintan_special_l.h"



typedef struct AH_PINTAN_SPECIAL_DIALOG AH_PINTAN_SPECIAL_DIALOG;
struct AH_PINTAN_SPECIAL_DIALOG {
  AB_BANKING *banking;
  AB_PROVIDER *provider;

  int httpVMajor;
  int httpVMinor;

  int hbciVersion;

  uint32_t flags;

  char *tanMediumId;
};




#endif

