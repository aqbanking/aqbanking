/***************************************************************************
 begin       : Wed Apr 14 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQBANKING_DLG_SELECTBACKEND_P_H
#define AQBANKING_DLG_SELECTBACKEND_P_H


#include "dlg_selectbackend.h"



typedef struct AB_SELECTBACKEND_DIALOG AB_SELECTBACKEND_DIALOG;
struct AB_SELECTBACKEND_DIALOG {
  AB_BANKING *banking;
  char *selectedProvider;
  char *text;
  GWEN_PLUGIN_DESCRIPTION_LIST *pluginDescrList;
};


static void GWENHYWFAR_CB AB_SelectBackendDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB AB_SelectBackendDialog_SignalHandler(GWEN_DIALOG *dlg,
							      GWEN_DIALOG_EVENTTYPE t,
							      const char *sender);





#endif

