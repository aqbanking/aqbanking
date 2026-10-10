/***************************************************************************
 begin       : Thu Jun 18 2009
 copyright   : (C) 2026 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_GUI_P_H
#define AQBANKING_GUI_P_H


#include "abgui.h"


typedef struct AB_GUI AB_GUI;
struct AB_GUI {
  AB_BANKING *banking;
  GWEN_GUI_CHECKCERT_FN checkCertFn;

  GWEN_GUI_READ_DIALOG_PREFS_FN readDialogPrefsFn;
  GWEN_GUI_WRITE_DIALOG_PREFS_FN writeDialogPrefsFn;
  GWEN_GUI_GETPASSWORD_FN getPasswordFn;
  const char *opticalTanTool;
};



#endif


