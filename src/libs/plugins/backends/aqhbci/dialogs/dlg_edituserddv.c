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



#include "dlg_edituserddv_p.h"
#include "i18n_l.h"
#include "provider_l.h"

#include <aqhbci/user.h>
#include <aqhbci/provider.h>

#include <aqbanking/user.h>
#include <aqbanking/banking_be.h>
#include <aqbanking/dlg_selectbankinfo.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>


#define DIALOG_MINWIDTH  200
#define DIALOG_MINHEIGHT 200



GWEN_INHERIT(GWEN_DIALOG, AH_EDIT_USER_DDV_DIALOG)




GWEN_DIALOG *AH_EditUserDdvDialog_new(AB_PROVIDER *pro, AB_USER *u, int doLock)
{
  GWEN_DIALOG *dlg;
  AH_EDIT_USER_DDV_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("ah_edit_user_ddv");
  GWEN_NEW_OBJECT(AH_EDIT_USER_DDV_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AH_EDIT_USER_DDV_DIALOG, dlg, xdlg,
                       AH_EditUserDdvDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, AH_EditUserDdvDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
                               "aqbanking/backends/aqhbci/dialogs/dlg_edituserddv.dlg",
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



void GWENHYWFAR_CB AH_EditUserDdvDialog_FreeData(void *bp, void *p)
{
  AH_EDIT_USER_DDV_DIALOG *xdlg;

  xdlg=(AH_EDIT_USER_DDV_DIALOG *) p;
  GWEN_FREE_OBJECT(xdlg);
}



void AH_EditUserDdvDialog_Init(GWEN_DIALOG *dlg)
{
  AH_EDIT_USER_DDV_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;
  const char *s;
  const GWEN_URL *gu;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_DDV_DIALOG, dlg);
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



int AH_EditUserDdvDialog_fromGui(GWEN_DIALOG *dlg, AB_USER *u, int quiet)
{
  AH_EDIT_USER_DDV_DIALOG *xdlg;
  const char *s;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_DDV_DIALOG, dlg);
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

  return 0;
}



void AH_EditUserDdvDialog_Fini(GWEN_DIALOG *dlg)
{
  AH_EDIT_USER_DDV_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_DDV_DIALOG, dlg);
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



int AH_EditUserDdvDialog_HandleActivatedBankCode(GWEN_DIALOG *dlg)
{
  AH_EDIT_USER_DDV_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_DDV_DIALOG, dlg);
  assert(xdlg);

  dlg2=AB_SelectBankInfoDialog_new(xdlg->banking, "de", NULL);
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



int AH_EditUserDdvDialog_HandleActivatedOk(GWEN_DIALOG *dlg)
{
  AH_EDIT_USER_DDV_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_DDV_DIALOG, dlg);
  assert(xdlg);

  rv=AH_EditUserDdvDialog_fromGui(dlg, NULL, 0);
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

  AH_EditUserDdvDialog_fromGui(dlg, xdlg->user, 1);

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



static int AH_EditUserDdvDialog_HandleActivatedGetAccounts(GWEN_DIALOG *dlg)
{
  AH_EDIT_USER_DDV_DIALOG *xdlg;
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_DDV_DIALOG, dlg);
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



int AH_EditUserDdvDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender)
{
  if (strcasecmp(sender, "bankCodeButton")==0)
    return AH_EditUserDdvDialog_HandleActivatedBankCode(dlg);
  else if (strcasecmp(sender, "getAccountsButton")==0)
    return AH_EditUserDdvDialog_HandleActivatedGetAccounts(dlg);
  else if (strcasecmp(sender, "okButton")==0)
    return AH_EditUserDdvDialog_HandleActivatedOk(dlg);
  else if (strcasecmp(sender, "abortButton")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "helpButton")==0) {
    /* TODO: open u help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB AH_EditUserDdvDialog_SignalHandler(GWEN_DIALOG *dlg,
                                                     GWEN_DIALOG_EVENTTYPE t,
                                                     const char *sender)
{
  AH_EDIT_USER_DDV_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_DDV_DIALOG, dlg);
  assert(xdlg);

  switch (t) {
  case GWEN_DialogEvent_TypeInit:
    AH_EditUserDdvDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AH_EditUserDdvDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeActivated:
    return AH_EditUserDdvDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}




