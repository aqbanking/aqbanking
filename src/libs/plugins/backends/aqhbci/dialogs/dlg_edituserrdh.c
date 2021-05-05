/***************************************************************************
 begin       : Thu Jul 08 2010
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_edituserrdh_p.h"

#include "aqhbci/banking/provider_l.h"

#include "aqhbci/banking/user.h"
#include "aqhbci/banking/provider.h"
#include "aqhbci/banking/provider_online.h"
#include "aqhbci/banking/provider_iniletter.h"

#include "aqbanking/i18n_l.h"
#include <aqbanking/backendsupport/user.h>
#include <aqbanking/banking_be.h>
#include <aqbanking/dialogs/dlg_selectbankinfo.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>


#define DIALOG_MINWIDTH  200
#define DIALOG_MINHEIGHT 200



GWEN_INHERIT(GWEN_DIALOG, AH_EDIT_USER_RDH_DIALOG)




GWEN_DIALOG *AH_EditUserRdhDialog_new(AB_PROVIDER *pro, AB_USER *u, int doLock)
{
  GWEN_DIALOG *dlg;
  AH_EDIT_USER_RDH_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("ah_edit_user_rdh");
  GWEN_NEW_OBJECT(AH_EDIT_USER_RDH_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AH_EDIT_USER_RDH_DIALOG, dlg, xdlg,
                       AH_EditUserRdhDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, AH_EditUserRdhDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
                               "aqbanking/backends/aqhbci/dialogs/dlg_edituserrdh.dlg",
                               fbuf);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Dialog description file not found (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }

  /* read dialog from dialog description file */
  rv=GWEN_Dialog_ReadXmlFile(dlg, GWEN_Buffer_GetStart(fbuf));
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }
  GWEN_Buffer_free(fbuf);

  /* preset */
  xdlg->provider=pro;
  xdlg->banking=AB_Provider_GetBanking(pro);
  xdlg->user=u;
  xdlg->doLock=doLock;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB AH_EditUserRdhDialog_FreeData(void *bp, void *p)
{
  AH_EDIT_USER_RDH_DIALOG *xdlg;

  xdlg=(AH_EDIT_USER_RDH_DIALOG *) p;
  GWEN_FREE_OBJECT(xdlg);
}



void AH_EditUserRdhDialog_Init(GWEN_DIALOG *dlg)
{
  AH_EDIT_USER_RDH_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;
  const char *s;
  uint32_t flags;
  const GWEN_URL *gu;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_RDH_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  /* init */
  GWEN_Dialog_SetCharProperty(dlg,
                              "",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("Edit User"),
                              0);

  s=AB_User_GetUserName(xdlg->user);
  GWEN_Dialog_SetCharProperty(dlg, "userNameEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AB_User_GetBankCode(xdlg->user);
  GWEN_Dialog_SetCharProperty(dlg, "bankCodeEdit", GWEN_DialogProperty_Value, 0, s, 0);

  gu=AH_User_GetServerUrl(xdlg->user);
  if (gu) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Url_toString(gu, tbuf);
    GWEN_Dialog_SetCharProperty(dlg, "urlEdit", GWEN_DialogProperty_Value, 0, GWEN_Buffer_GetStart(tbuf), 0);
    GWEN_Buffer_free(tbuf);
  }

  s=AB_User_GetUserId(xdlg->user);
  GWEN_Dialog_SetCharProperty(dlg, "userIdEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AB_User_GetCustomerId(xdlg->user);
  GWEN_Dialog_SetCharProperty(dlg, "customerIdEdit", GWEN_DialogProperty_Value, 0, s, 0);

  GWEN_Dialog_SetCharProperty(dlg, "hbciVersionCombo", GWEN_DialogProperty_AddValue, 0, "2.01", 0);
  GWEN_Dialog_SetCharProperty(dlg, "hbciVersionCombo", GWEN_DialogProperty_AddValue, 0, "2.10", 0);
  GWEN_Dialog_SetCharProperty(dlg, "hbciVersionCombo", GWEN_DialogProperty_AddValue, 0, "2.20", 0);
  GWEN_Dialog_SetCharProperty(dlg, "hbciVersionCombo", GWEN_DialogProperty_AddValue, 0, "3.0", 0);

  GWEN_Dialog_SetCharProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_AddValue, 0, "1.0", 0);
  GWEN_Dialog_SetCharProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_AddValue, 0, "1.1", 0);

  GWEN_Dialog_SetCharProperty(dlg, "statusCombo", GWEN_DialogProperty_AddValue, 0, I18N("HBCIUserStatus|new"), 0);
  GWEN_Dialog_SetCharProperty(dlg, "statusCombo", GWEN_DialogProperty_AddValue, 0, I18N("HBCIUserStatus|enabled"), 0);
  GWEN_Dialog_SetCharProperty(dlg, "statusCombo", GWEN_DialogProperty_AddValue, 0, I18N("HBCIUserStatus|pending"), 0);
  GWEN_Dialog_SetCharProperty(dlg, "statusCombo", GWEN_DialogProperty_AddValue, 0, I18N("HBCIUserStatus|disabled"), 0);

  /* toGui */
  switch (AH_User_GetHbciVersion(xdlg->user)) {
  case 201:
    GWEN_Dialog_SetIntProperty(dlg, "hbciVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0);
    break;
  case 210:
    GWEN_Dialog_SetIntProperty(dlg, "hbciVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0);
    break;
  case 220:
    GWEN_Dialog_SetIntProperty(dlg, "hbciVersionCombo", GWEN_DialogProperty_Value, 0, 2, 0);
    break;
  case 300:
    GWEN_Dialog_SetIntProperty(dlg, "hbciVersionCombo", GWEN_DialogProperty_Value, 0, 3, 0);
    break;
  default:
    break;
  }

  GWEN_Dialog_SetCharProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_AddValue, 0, I18N("(auto)"), 0);
  GWEN_Dialog_SetCharProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_AddValue, 0, "RDH-1", 0);
  GWEN_Dialog_SetCharProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_AddValue, 0, "RDH-2", 0);
  GWEN_Dialog_SetCharProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_AddValue, 0, "RDH-3", 0);
  GWEN_Dialog_SetCharProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_AddValue, 0, "RDH-5", 0);
  GWEN_Dialog_SetCharProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_AddValue, 0, "RDH-6", 0);
  GWEN_Dialog_SetCharProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_AddValue, 0, "RDH-7", 0);
  GWEN_Dialog_SetCharProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_AddValue, 0, "RDH-8", 0);
  GWEN_Dialog_SetCharProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_AddValue, 0, "RDH-9", 0);
  GWEN_Dialog_SetCharProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_AddValue, 0, "RDH-10", 0);
  GWEN_Dialog_SetCharProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_AddValue, 0, "RAH-7", 0);
  GWEN_Dialog_SetCharProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_AddValue, 0, "RAH-9", 0);
  GWEN_Dialog_SetCharProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_AddValue, 0, "RAH-10", 0);

  /* toGui */
  switch (AH_User_GetCryptMode(xdlg->user)) {
  case AH_CryptMode_Rdh:
    switch (AH_User_GetRdhType(xdlg->user)) {
    case 0:
      GWEN_Dialog_SetIntProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0);
      break;
    case 1:
      GWEN_Dialog_SetIntProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0);
      break;
    case 2:
      GWEN_Dialog_SetIntProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_Value, 0, 2, 0);
      break;
    case 3:
      GWEN_Dialog_SetIntProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_Value, 0, 3, 0);
      break;
    case 5:
      GWEN_Dialog_SetIntProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_Value, 0, 4, 0);
      break;
    case 6:
      GWEN_Dialog_SetIntProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_Value, 0, 5, 0);
      break;
    case 7:
      GWEN_Dialog_SetIntProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_Value, 0, 6, 0);
      break;
    case 8:
      GWEN_Dialog_SetIntProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_Value, 0, 7, 0);
      break;
    case 9:
      GWEN_Dialog_SetIntProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_Value, 0, 8, 0);
      break;
    case 10:
      GWEN_Dialog_SetIntProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_Value, 0, 9, 0);
      break;
    default:
      break;
    }
    break;
  case AH_CryptMode_Rah:
    switch (AH_User_GetRdhType(xdlg->user)) {
    case 7:
      GWEN_Dialog_SetIntProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_Value, 0, 10, 0);
      break;
    case 9:
      GWEN_Dialog_SetIntProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_Value, 0, 11, 0);
      break;
    case 10:
      GWEN_Dialog_SetIntProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_Value, 0, 12, 0);
      break;
    default:
      break;
    }
    break;
  default:
    GWEN_Dialog_SetIntProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0);
    break;
  }

  switch (AH_User_GetStatus(xdlg->user)) {
  case AH_UserStatusNew:
    GWEN_Dialog_SetIntProperty(dlg, "statusCombo", GWEN_DialogProperty_Value, 0, 0, 0);
    break;
  case AH_UserStatusEnabled:
    GWEN_Dialog_SetIntProperty(dlg, "statusCombo", GWEN_DialogProperty_Value, 0, 1, 0);
    break;
  case AH_UserStatusPending:
    GWEN_Dialog_SetIntProperty(dlg, "statusCombo", GWEN_DialogProperty_Value, 0, 2, 0);
    break;
  case AH_UserStatusDisabled:
    GWEN_Dialog_SetIntProperty(dlg, "statusCombo", GWEN_DialogProperty_Value, 0, 3, 0);
    break;
  default:
    break;
  }

  flags=AH_User_GetFlags(xdlg->user);
  GWEN_Dialog_SetIntProperty(dlg, "bankDoesntSignCheck", GWEN_DialogProperty_Value, 0,
                             (flags & AH_USER_FLAGS_BANK_DOESNT_SIGN)?1:0,
                             0);

  GWEN_Dialog_SetIntProperty(dlg, "bankUsesSignSeqCheck", GWEN_DialogProperty_Value, 0,
                             (flags & AH_USER_FLAGS_BANK_USES_SIGNSEQ)?1:0,
                             0);


  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);
}



static void removeAllSpaces(uint8_t *s)
{
  uint8_t *d;

  d=s;
  while (*s) {
    if (*s>33)
      *(d++)=*s;
    s++;
  }
  *d=0;
}



int AH_EditUserRdhDialog_fromGui(GWEN_DIALOG *dlg, AB_USER *u, int quiet)
{
  AH_EDIT_USER_RDH_DIALOG *xdlg;
  const char *s;
  int i;
  uint32_t flags;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_RDH_DIALOG, dlg);
  assert(xdlg);

  /* fromGui */
  s=GWEN_Dialog_GetCharProperty(dlg, "userNameEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    if (u)
      AB_User_SetUserName(u, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "bankCodeEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    removeAllSpaces((uint8_t *)GWEN_Buffer_GetStart(tbuf));
    if (u)
      AB_User_SetBankCode(u, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "urlEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;
    GWEN_URL *gu;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    removeAllSpaces((uint8_t *)GWEN_Buffer_GetStart(tbuf));
    gu=GWEN_Url_fromString(GWEN_Buffer_GetStart(tbuf));
    if (gu==NULL) {
      if (!quiet) {
        GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Invalid URL"));
      }
      GWEN_Buffer_free(tbuf);
      return GWEN_ERROR_BAD_DATA;
    }

    /* set port to 3000 if not set */
    if (GWEN_Url_GetPort(gu)==0)
      GWEN_Url_SetPort(gu, 3000);
    if (u)
      AH_User_SetServerUrl(u, gu);
    GWEN_Url_free(gu);
    GWEN_Buffer_free(tbuf);
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "userIdEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    if (u)
      AB_User_SetUserId(u, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "customerIdEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    if (u)
      AB_User_SetCustomerId(u, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  /*  get country */
  if (u)
    AB_User_SetCountry(u, "de");

  i=GWEN_Dialog_GetIntProperty(dlg, "hbciVersionCombo", GWEN_DialogProperty_Value, 0, -1);
  switch (i) {
  case 0:
    AH_User_SetHbciVersion(xdlg->user, 201);
    break;
  case 1:
    AH_User_SetHbciVersion(xdlg->user, 210);
    break;
  case 2:
    AH_User_SetHbciVersion(xdlg->user, 220);
    break;
  default:
  case 3:
    AH_User_SetHbciVersion(xdlg->user, 300);
    break;
  }

  i=GWEN_Dialog_GetIntProperty(dlg, "rdhVersionCombo", GWEN_DialogProperty_Value, 0, -1);
  switch (i) {
  case 1:
    AH_User_SetRdhType(xdlg->user, 1);
    AH_User_SetCryptMode(xdlg->user, AH_CryptMode_Rdh);
    break;
  case 2:
    AH_User_SetRdhType(xdlg->user, 2);
    AH_User_SetCryptMode(xdlg->user, AH_CryptMode_Rdh);
    break;
  case 3:
    AH_User_SetRdhType(xdlg->user, 3);
    AH_User_SetCryptMode(xdlg->user, AH_CryptMode_Rdh);
    break;
  case 4:
    AH_User_SetRdhType(xdlg->user, 5);
    AH_User_SetCryptMode(xdlg->user, AH_CryptMode_Rdh);
    break;
  case 5:
    AH_User_SetRdhType(xdlg->user, 6);
    AH_User_SetCryptMode(xdlg->user, AH_CryptMode_Rdh);
    break;
  case 6:
    AH_User_SetRdhType(xdlg->user, 7);
    AH_User_SetCryptMode(xdlg->user, AH_CryptMode_Rdh);
    break;
  case 7:
    AH_User_SetRdhType(xdlg->user, 8);
    AH_User_SetCryptMode(xdlg->user, AH_CryptMode_Rdh);
    break;
  case 8:
    AH_User_SetRdhType(xdlg->user, 9);
    AH_User_SetCryptMode(xdlg->user, AH_CryptMode_Rdh);
    break;
  case 9:
    AH_User_SetRdhType(xdlg->user, 10);
    AH_User_SetCryptMode(xdlg->user, AH_CryptMode_Rdh);
    break;
  case 10:
    AH_User_SetRdhType(xdlg->user, 7);
    AH_User_SetCryptMode(xdlg->user, AH_CryptMode_Rah);
    break;
  case 11:
    AH_User_SetRdhType(xdlg->user, 9);
    AH_User_SetCryptMode(xdlg->user, AH_CryptMode_Rah);
    break;
  case 12:
    AH_User_SetRdhType(xdlg->user, 10);
    AH_User_SetCryptMode(xdlg->user, AH_CryptMode_Rah);
    break;
  default:
    AH_User_SetRdhType(xdlg->user, 0);
    AH_User_SetCryptMode(xdlg->user, AH_CryptMode_Rdh);
    break;
  }

  i=GWEN_Dialog_GetIntProperty(dlg, "statusCombo", GWEN_DialogProperty_Value, 0, -1);
  switch (i) {
  case 0:
    AH_User_SetStatus(xdlg->user, AH_UserStatusNew);
    break;
  case 1:
    AH_User_SetStatus(xdlg->user, AH_UserStatusEnabled);
    break;
  case 2:
    AH_User_SetStatus(xdlg->user, AH_UserStatusPending);
    break;
  case 3:
    AH_User_SetStatus(xdlg->user, AH_UserStatusDisabled);
    break;
  default:
    break;
  }

  flags=0;
  if (GWEN_Dialog_GetIntProperty(dlg, "bankDoesntSignCheck", GWEN_DialogProperty_Value, 0, 0))
    flags|=AH_USER_FLAGS_BANK_DOESNT_SIGN;
  if (GWEN_Dialog_GetIntProperty(dlg, "bankUsesSignSeqCheck", GWEN_DialogProperty_Value, 0, 0))
    flags|=AH_USER_FLAGS_BANK_USES_SIGNSEQ;
  AH_User_SetFlags(xdlg->user, flags);

  return 0;
}



void AH_EditUserRdhDialog_Fini(GWEN_DIALOG *dlg)
{
  AH_EDIT_USER_RDH_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_RDH_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  /* store dialog width */
  i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, -1);
  GWEN_DB_SetIntValue(dbPrefs,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "dialog_width",
                      i);

  /* store dialog height */
  i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, -1);
  GWEN_DB_SetIntValue(dbPrefs,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "dialog_height",
                      i);
}



int AH_EditUserRdhDialog_HandleActivatedBankCode(GWEN_DIALOG *dlg)
{
  AH_EDIT_USER_RDH_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_RDH_DIALOG, dlg);
  assert(xdlg);

  dlg2=AB_SelectBankInfoDialog_new(xdlg->banking, "de", GWEN_Dialog_GetCharProperty(dlg, "bankCodeEdit",
                                                                                    GWEN_DialogProperty_Value, 0, NULL));
  if (dlg2==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create dialog");
    GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Could create dialog, maybe incomplete installation?"));
    return GWEN_DialogEvent_ResultHandled;
  }

  rv=GWEN_Gui_ExecDialog(dlg2, 0);
  if (rv==0) {
    /* rejected */
    GWEN_Dialog_free(dlg2);
    return GWEN_DialogEvent_ResultHandled;
  }
  else {
    const AB_BANKINFO *bi;

    bi=AB_SelectBankInfoDialog_GetSelectedBankInfo(dlg2);
    if (bi) {
      const char *s;

      s=AB_BankInfo_GetBankId(bi);
      GWEN_Dialog_SetCharProperty(dlg,
                                  "bankCodeEdit",
                                  GWEN_DialogProperty_Value,
                                  0,
                                  (s && *s)?s:"",
                                  0);
    }
  }
  GWEN_Dialog_free(dlg2);

  return GWEN_DialogEvent_ResultHandled;
}



int AH_EditUserRdhDialog_HandleActivatedOk(GWEN_DIALOG *dlg)
{
  AH_EDIT_USER_RDH_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_RDH_DIALOG, dlg);
  assert(xdlg);

  rv=AH_EditUserRdhDialog_fromGui(dlg, NULL, 0);
  if (rv<0) {
    return GWEN_DialogEvent_ResultHandled;
  }

  if (xdlg->doLock) {
    int rv;

    rv=AB_Provider_BeginExclUseUser(xdlg->provider, xdlg->user);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL |
                          GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
                          GWEN_GUI_MSG_FLAGS_CONFIRM_B1,
                          I18N("Error"),
                          I18N("Unable to lock user. Maybe already in use?"),
                          I18N("Dismiss"),
                          NULL,
                          NULL,
                          0);
      return GWEN_DialogEvent_ResultHandled;
    }
  }

  AH_EditUserRdhDialog_fromGui(dlg, xdlg->user, 1);

  if (xdlg->doLock) {
    int rv;

    rv=AB_Provider_EndExclUseUser(xdlg->provider, xdlg->user, 0);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL |
                          GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
                          GWEN_GUI_MSG_FLAGS_CONFIRM_B1,
                          I18N("Error"),
                          I18N("Unable to unlock user."),
                          I18N("Dismiss"),
                          NULL,
                          NULL,
                          0);
      AB_Provider_EndExclUseUser(xdlg->provider, xdlg->user, 1);
      return GWEN_DialogEvent_ResultHandled;
    }
  }

  return GWEN_DialogEvent_ResultAccept;
}



static int AH_EditUserRdhDialog_HandleActivatedGetSysId(GWEN_DIALOG *dlg)
{
  AH_EDIT_USER_RDH_DIALOG *xdlg;
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_RDH_DIALOG, dlg);
  assert(xdlg);

  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetSysId(xdlg->provider,
                          xdlg->user,
                          ctx,
                          1,   /* withProgress */
                          0,   /* nounmount */
                          xdlg->doLock);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }

  AB_ImExporterContext_free(ctx);
  return GWEN_DialogEvent_ResultHandled;
}



static int AH_EditUserRdhDialog_HandleActivatedGetAccounts(GWEN_DIALOG *dlg)
{
  AH_EDIT_USER_RDH_DIALOG *xdlg;
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_RDH_DIALOG, dlg);
  assert(xdlg);

  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetAccounts(xdlg->provider,
                             xdlg->user,
                             ctx,
                             1,   /* withProgress */
                             0,   /* nounmount */
                             xdlg->doLock);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }

  AB_ImExporterContext_free(ctx);
  return GWEN_DialogEvent_ResultHandled;
}



static int AH_EditUserRdhDialog_HandleActivatedIniLetter(GWEN_DIALOG *dlg)
{
  AH_EDIT_USER_RDH_DIALOG *xdlg;
  int rv;
  GWEN_BUFFER *tbuf;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_RDH_DIALOG, dlg);
  assert(xdlg);

  tbuf=GWEN_Buffer_new(0, 16536, 0, 1);

  /* add HTML version of the INI letter */
  GWEN_Buffer_AppendString(tbuf, "<html>");
  rv=AH_Provider_GetIniLetterHtml(xdlg->provider,
                                  xdlg->user,
                                  0,
                                  0,
                                  tbuf,
                                  1);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    // TODO: show error message
    AB_Banking_ClearCryptTokenList(xdlg->banking);
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }
  GWEN_Buffer_AppendString(tbuf, "</html>");


  /* add ASCII version of the INI letter for frontends which don't support HTML */
  rv=AH_Provider_GetIniLetterTxt(xdlg->provider,
                                 xdlg->user,
                                 0,
                                 0,
                                 tbuf,
                                 0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    // TODO: show error message
    AB_Banking_ClearCryptTokenList(xdlg->banking);
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }

  /* show INI letter before printing (workaround for missing GWEN_Gui_Print under qt4/5) */
  GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL |
                      GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
                      GWEN_GUI_MSG_FLAGS_CONFIRM_B1,
                      I18N("INI Letter for HBCI"),
                      GWEN_Buffer_GetStart(tbuf),
                      I18N("Dismiss"), NULL, NULL, 0);

  // GWEN_Gui_Print does not seem to be implemented for qt4/5 yet
  rv=GWEN_Gui_Print(I18N("INI Letter"),
                    "HBCI-INILETTER",
                    I18N("INI Letter for HBCI"),
                    GWEN_Buffer_GetStart(tbuf),
                    0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    // TODO: show error message
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }

  GWEN_Buffer_free(tbuf);
  return GWEN_DialogEvent_ResultHandled;
}



int AH_EditUserRdhDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender)
{
  if (strcasecmp(sender, "bankCodeButton")==0)
    return AH_EditUserRdhDialog_HandleActivatedBankCode(dlg);
  else if (strcasecmp(sender, "getSysIdButton")==0)
    return AH_EditUserRdhDialog_HandleActivatedGetSysId(dlg);
  else if (strcasecmp(sender, "getAccountsButton")==0)
    return AH_EditUserRdhDialog_HandleActivatedGetAccounts(dlg);
  else if (strcasecmp(sender, "iniLetterButton")==0)
    return AH_EditUserRdhDialog_HandleActivatedIniLetter(dlg);
  else if (strcasecmp(sender, "okButton")==0)
    return AH_EditUserRdhDialog_HandleActivatedOk(dlg);
  else if (strcasecmp(sender, "abortButton")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "helpButton")==0) {
    /* TODO: open u help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB AH_EditUserRdhDialog_SignalHandler(GWEN_DIALOG *dlg,
                                                     GWEN_DIALOG_EVENTTYPE t,
                                                     const char *sender)
{
  AH_EDIT_USER_RDH_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_RDH_DIALOG, dlg);
  assert(xdlg);

  switch (t) {
  case GWEN_DialogEvent_TypeInit:
    AH_EditUserRdhDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AH_EditUserRdhDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeActivated:
    return AH_EditUserRdhDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}




