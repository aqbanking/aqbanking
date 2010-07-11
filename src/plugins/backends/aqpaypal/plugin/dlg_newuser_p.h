/***************************************************************************
 begin       : Mon Apr 12 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQPAYPAL_DLG_NEWUSER_P_H
#define AQPAYPAL_DLG_NEWUSER_P_H


#include "dlg_newuser.h"



typedef struct APY_NEWUSER_DIALOG APY_NEWUSER_DIALOG;
struct APY_NEWUSER_DIALOG {
  AB_BANKING *banking;

  char *userName;
  char *userId;
  char *url;

  int httpVMajor;
  int httpVMinor;

  uint32_t flags;

  char *apiUserId;
  char *apiPassword;
  char *apiSignature;

  AB_USER *user;
};


static void GWENHYWFAR_CB APY_NewUserDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB APY_NewUserDialog_SignalHandler(GWEN_DIALOG *dlg,
							 GWEN_DIALOG_EVENTTYPE t,
							 const char *sender);





#endif

