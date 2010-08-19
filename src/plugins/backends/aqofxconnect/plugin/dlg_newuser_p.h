/***************************************************************************
 begin       : Thu Aug 19 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQOFXCONNECT_DLG_NEWUSER_P_H
#define AQOFXCONNECT_DLG_NEWUSER_P_H


#include "dlg_newuser_l.h"



typedef struct AO_NEWUSER_DIALOG AO_NEWUSER_DIALOG;
struct AO_NEWUSER_DIALOG {
  AB_BANKING *banking;

  char *userName;
  char *userId;
  char *url;

  char *bankName;
  char *brokerId;
  char *org;
  char *fid;

  char *appId;
  char *appVer;
  char *headerVer;
  char *clientUid;

  int httpVMajor;
  int httpVMinor;

  uint32_t flags;

  AB_USER *user;
};


static void GWENHYWFAR_CB AO_NewUserDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB AO_NewUserDialog_SignalHandler(GWEN_DIALOG *dlg,
							 GWEN_DIALOG_EVENTTYPE t,
							 const char *sender);





#endif

