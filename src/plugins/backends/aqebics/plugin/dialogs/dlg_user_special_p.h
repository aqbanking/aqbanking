/***************************************************************************
 begin       : Wed Apr 14 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQEBICS_DLG_PINTAN_SPECIAL_P_H
#define AQEBICS_DLG_PINTAN_SPECIAL_P_H


#include "dlg_user_special_l.h"



typedef struct EBC_USER_SPECIAL_DIALOG EBC_USER_SPECIAL_DIALOG;
struct EBC_USER_SPECIAL_DIALOG {
  AB_BANKING *banking;

  int httpVMajor;
  int httpVMinor;

  char *ebicsVersion;
  char *signVersion;
  char *cryptVersion;
  char *authVersion;

  int signKeySize;
  int cryptAndAuthKeySize;

  uint32_t flags;
};


static void GWENHYWFAR_CB EBC_UserSpecialDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB EBC_UserSpecialDialog_SignalHandler(GWEN_DIALOG *dlg,
                                                             GWEN_DIALOG_EVENTTYPE t,
                                                             const char *sender);





#endif

