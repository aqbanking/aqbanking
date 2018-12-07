/***************************************************************************
 begin       : Thu Aug 19 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQPAYPAL_DLG_OFX_SPECIAL_P_H
#define AQPAYPAL_DLG_OFX_SPECIAL_P_H


#include "dlg_editsecret_l.h"



typedef struct APY_EDITSECRET_DIALOG APY_EDITSECRET_DIALOG;
struct APY_EDITSECRET_DIALOG {
  AB_BANKING *banking;

  char *apiUserId;
  char *apiPassword;
  char *apiSignature;
};


void GWENHYWFAR_CB APY_EditSecretDialog_FreeData(void *bp, void *p);

int GWENHYWFAR_CB APY_EditSecretDialog_SignalHandler(GWEN_DIALOG *dlg,
                                                           GWEN_DIALOG_EVENTTYPE t,
                                                           const char *sender);





#endif

