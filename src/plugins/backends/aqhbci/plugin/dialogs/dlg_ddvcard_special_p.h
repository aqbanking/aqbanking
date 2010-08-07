/***************************************************************************
 begin       : Tue Apr 20 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_DDVCARD_SPECIAL_P_H
#define AQHBCI_DLG_DDVCARD_SPECIAL_P_H


#include "dlg_ddvcard_special_l.h"



typedef struct AH_DDVCARD_SPECIAL_DIALOG AH_DDVCARD_SPECIAL_DIALOG;
struct AH_DDVCARD_SPECIAL_DIALOG {
  AB_BANKING *banking;

  int hbciVersion;

  uint32_t flags;
};


static void GWENHYWFAR_CB AH_DdvCardSpecialDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB AH_DdvCardSpecialDialog_SignalHandler(GWEN_DIALOG *dlg,
							       GWEN_DIALOG_EVENTTYPE t,
							       const char *sender);





#endif

