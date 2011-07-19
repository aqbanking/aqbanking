/***************************************************************************
 begin       : Thu Aug 19 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQOFXCONNECT_DLG_EDITUSER_P_H
#define AQOFXCONNECT_DLG_EDITUSER_P_H


#include "dlg_edituser_l.h"



typedef struct AO_EDITUSER_DIALOG AO_EDITUSER_DIALOG;
struct AO_EDITUSER_DIALOG {
  AB_BANKING *banking;

  int doLock;

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
  char *securityType;

  int httpVMajor;
  int httpVMinor;

  uint32_t flags;

  AB_USER *user;
};


static void GWENHYWFAR_CB AO_EditUserDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB AO_EditUserDialog_SignalHandler(GWEN_DIALOG *dlg,
							 GWEN_DIALOG_EVENTTYPE t,
							 const char *sender);


static void AO_EditUserDialog_Init(GWEN_DIALOG *dlg);
static void AO_EditUserDialog_Fini(GWEN_DIALOG *dlg);
static int AO_EditUserDialog_GetBankPageData(GWEN_DIALOG *dlg);
static int AO_EditUserDialog_GetUserPageData(GWEN_DIALOG *dlg);
static int AO_EditUserDialog_GetAppPageData(GWEN_DIALOG *dlg);
static int AO_EditUserDialog_FromGui(GWEN_DIALOG *dlg);
static int AO_EditUserDialog_HandleActivatedSpecial(GWEN_DIALOG *dlg);
static int AO_EditUserDialog_HandleActivatedBankSelect(GWEN_DIALOG *dlg);
static int AO_EditUserDialog_HandleActivatedApp(GWEN_DIALOG *dlg);
static int AO_EditUserDialog_HandleActivatedGetAccounts(GWEN_DIALOG *dlg);
static int AO_EditUserDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender);
static int AO_EditUserDialog_HandleValueChanged(GWEN_DIALOG *dlg, const char *sender);













#endif

