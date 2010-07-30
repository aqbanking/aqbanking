/***************************************************************************
 begin       : Fri Jul 30 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQBANKING_DLG_SETUP_NEWUSER_P_H
#define AQBANKING_DLG_SETUP_NEWUSER_P_H

#include "dlg_setup_newuser.h"



typedef struct AB_SETUP_NEWUSER_DIALOG AB_SETUP_NEWUSER_DIALOG;
struct AB_SETUP_NEWUSER_DIALOG {
  AB_BANKING *banking;
  GWEN_DIALOG_LIST2 *backendDialogs;
  GWEN_STRINGLIST *backendRadioNames;
  GWEN_STRINGLIST *backendNames;

  char *selectedBackend;
  int selectedType;
};


static GWENHYWFAR_CB void AB_SetupNewUserDialog_FreeData(void *bp, void *p);

static GWENHYWFAR_CB int AB_SetupNewUserDialog_SignalHandler(GWEN_DIALOG *dlg,
							     GWEN_DIALOG_EVENTTYPE t,
							     const char *sender);

static int AB_SetupNewUserDialog_DetermineBackendIndex(GWEN_DIALOG *dlg);


#endif

