/***************************************************************************
 begin       : Mon Apr 12 2010
 copyright   : (C) 2010 by Martin Preuss
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


static void GWENHYWFAR_CB AH_PinTanDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB AH_PinTanDialog_SignalHandler(GWEN_DIALOG *dlg,
						       GWEN_DIALOG_EVENTTYPE t,
						       const char *sender);





#endif

