/***************************************************************************
 begin       : Thu Aug 19 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQOFXCONNECT_DLG_OFX_SPECIAL_P_H
#define AQOFXCONNECT_DLG_OFX_SPECIAL_P_H


#include "dlg_ofx_special_l.h"



typedef struct AO_OFX_SPECIAL_DIALOG AO_OFX_SPECIAL_DIALOG;
struct AO_OFX_SPECIAL_DIALOG {
  AB_BANKING *banking;

  int httpVMajor;
  int httpVMinor;

  uint32_t flags;

  char *clientUid;
  char *securityType;
};


static void GWENHYWFAR_CB AO_OfxSpecialDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB AO_OfxSpecialDialog_SignalHandler(GWEN_DIALOG *dlg,
                                                           GWEN_DIALOG_EVENTTYPE t,
                                                           const char *sender);





#endif

