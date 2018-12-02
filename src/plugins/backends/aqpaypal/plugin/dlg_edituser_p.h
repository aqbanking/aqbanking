/***************************************************************************
 begin       : Tue Aug 03 2010
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQPAYPAL_DLG_EDITUSER_P_H
#define AQPAYPAL_DLG_EDITUSER_P_H


#include "dlg_edituser_l.h"

#include <aqpaypal/user.h>



typedef struct APY_EDITUSER_DIALOG APY_EDITUSER_DIALOG;
struct APY_EDITUSER_DIALOG {
  AB_BANKING *banking;
  AB_PROVIDER *provider;
  AB_USER *user;
  int doLock;

  char *userName;
  char *userId;
  char *customerId;
  char *url;

  uint32_t flags;

  char *apiUserId;
  char *apiPassword;
  char *apiSignature;
};


static void GWENHYWFAR_CB APY_EditUserDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB APY_EditUserDialog_SignalHandler(GWEN_DIALOG *dlg,
							  GWEN_DIALOG_EVENTTYPE t,
							  const char *sender);





#endif

