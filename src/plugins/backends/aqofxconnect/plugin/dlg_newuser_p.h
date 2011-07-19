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
  char *securityType;

  int httpVMajor;
  int httpVMinor;

  uint32_t flags;

  AB_USER *user;
};


static void GWENHYWFAR_CB AO_NewUserDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB AO_NewUserDialog_SignalHandler(GWEN_DIALOG *dlg,
							 GWEN_DIALOG_EVENTTYPE t,
							 const char *sender);


static void AO_NewUserDialog_Init(GWEN_DIALOG *dlg);
static void AO_NewUserDialog_Fini(GWEN_DIALOG *dlg);
static int AO_NewUserDialog_GetBankPageData(GWEN_DIALOG *dlg);
static int AO_NewUserDialog_GetUserPageData(GWEN_DIALOG *dlg);
static int AO_NewUserDialog_GetAppPageData(GWEN_DIALOG *dlg);
static int AO_NewUserDialog_EnterPage(GWEN_DIALOG *dlg, int page, int forwards);
static int AO_NewUserDialog_DoIt(GWEN_DIALOG *dlg);
static int AO_NewUserDialog_UndoIt(GWEN_DIALOG *dlg);
static int AO_NewUserDialog_Next(GWEN_DIALOG *dlg);
static int AO_NewUserDialog_Previous(GWEN_DIALOG *dlg);
static int AO_NewUserDialog_HandleActivatedSpecial(GWEN_DIALOG *dlg);
static int AO_NewUserDialog_HandleActivatedBankSelect(GWEN_DIALOG *dlg);
static int AO_NewUserDialog_HandleActivatedApp(GWEN_DIALOG *dlg);
static int AO_NewUserDialog_HandleActivatedGetAccounts(GWEN_DIALOG *dlg);
static int AO_NewUserDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender);
static int AO_NewUserDialog_HandleValueChanged(GWEN_DIALOG *dlg, const char *sender);













#endif

