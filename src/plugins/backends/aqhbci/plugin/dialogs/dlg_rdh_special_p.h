/***************************************************************************
 begin       : Sat Jun 26 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_RDH_SPECIAL_P_H
#define AQHBCI_DLG_RDH_SPECIAL_P_H


#include "dlg_rdh_special_l.h"



typedef struct AH_RDH_SPECIAL_DIALOG AH_RDH_SPECIAL_DIALOG;
struct AH_RDH_SPECIAL_DIALOG {
  AB_BANKING *banking;

  int hbciVersion;
  int rdhVersion;
  uint32_t flags;
};


static void GWENHYWFAR_CB AH_RdhSpecialDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB AH_RdhSpecialDialog_SignalHandler(GWEN_DIALOG *dlg,
							   GWEN_DIALOG_EVENTTYPE t,
							   const char *sender);





#endif

