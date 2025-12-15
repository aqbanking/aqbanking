/***************************************************************************
 begin       : Thu Jul 08 2010
 copyright   : (C) 2024 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_edituserrdh_p.h"
#include "w_hbciversioncombo.h"
#include "w_rdhversioncombo.h"
#include "w_userstatuscombo.h"
#include "w_utils.h"

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



/* ------------------------------------------------------------------------------------------------
 * defines, types
 * ------------------------------------------------------------------------------------------------
 */

#define DIALOG_MINWIDTH  200
#define DIALOG_MINHEIGHT 200

/* for improved readability */
#define DLG_WITHPROGRESS 1
#define DLG_NOUMOUNT       0
#define DLG_DIALOGFILE   "aqbanking/backends/aqhbci/dialogs/dlg_edituserrdh.dlg"

typedef int (*_DIALOG_SIGNAL_HANDLER_FN)(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
typedef struct _DIALOG_SIGNAL_ENTRY _DIALOG_SIGNAL_ENTRY;
struct _DIALOG_SIGNAL_ENTRY {
  const char *sender;
  GWEN_DIALOG_EVENTTYPE eventType;
  _DIALOG_SIGNAL_HANDLER_FN handlerFn;
};



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void GWENHYWFAR_CB _freeData(void *bp, void *p);
static int GWENHYWFAR_CB _dlgApi_signalHandler(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static void _toGui(GWEN_DIALOG *dlg, AB_USER *user);
static int _fromGui(GWEN_DIALOG *dlg, AB_USER *u, int quiet);
static int _handleInit(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleFini(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedBankCode(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedOk(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedApply(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedReject(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedGetSysId(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedGetAccounts(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedGetSepaInfo(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedIniLetter(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleValueChanged(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _saveUser(GWEN_DIALOG *dlg);
static void _setModified(GWEN_DIALOG *dlg, int enabled);



/* ------------------------------------------------------------------------------------------------
 * static vars
 * ------------------------------------------------------------------------------------------------
 */

GWEN_INHERIT(GWEN_DIALOG, AH_EDIT_USER_RDH_DIALOG)

static _DIALOG_SIGNAL_ENTRY _signalMap[]={
  {NULL,                   GWEN_DialogEvent_TypeInit,         _handleInit},
  {NULL,                   GWEN_DialogEvent_TypeFini,         _handleFini},
  {"bankCodeButton",       GWEN_DialogEvent_TypeActivated,    _handleActivatedBankCode},
  {"getSysIdButton",       GWEN_DialogEvent_TypeActivated,    _handleActivatedGetSysId},
  {"getAccountsButton",    GWEN_DialogEvent_TypeActivated,    _handleActivatedGetAccounts},
  {"getSepaButton",        GWEN_DialogEvent_TypeActivated,    _handleActivatedGetSepaInfo},
  {"iniLetterButton",      GWEN_DialogEvent_TypeActivated,    _handleActivatedIniLetter},

  {"userNameEdit",         GWEN_DialogEvent_TypeValueChanged, _handleValueChanged},
  {"bankCodeEdit",         GWEN_DialogEvent_TypeValueChanged, _handleValueChanged},
  {"userIdEdit",           GWEN_DialogEvent_TypeValueChanged, _handleValueChanged},
  {"customerIdEdit",       GWEN_DialogEvent_TypeValueChanged, _handleValueChanged},
  {"urlEdit",              GWEN_DialogEvent_TypeValueChanged, _handleValueChanged},
  {"hbciVersionCombo",     GWEN_DialogEvent_TypeActivated,    _handleValueChanged},
  {"rdhVersionCombo",      GWEN_DialogEvent_TypeActivated,    _handleValueChanged},
  {"statusCombo",          GWEN_DialogEvent_TypeActivated,    _handleValueChanged},
  {"bankDoesntSignCheck",  GWEN_DialogEvent_TypeActivated,    _handleValueChanged},
  {"bankUsesSignSeqCheck", GWEN_DialogEvent_TypeActivated,    _handleValueChanged},

  {"okButton",             GWEN_DialogEvent_TypeActivated,    _handleActivatedOk},
  {"applyButton",          GWEN_DialogEvent_TypeActivated,    _handleActivatedApply},
  {"abortButton",          GWEN_DialogEvent_TypeActivated,    _handleActivatedReject},

  {NULL, 0, NULL}
};



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

GWEN_DIALOG *AH_EditUserRdhDialog_new(AB_PROVIDER *pro, AB_USER *u, int doLock)
{
  GWEN_DIALOG *dlg;
  AH_EDIT_USER_RDH_DIALOG *xdlg;

  dlg=GWEN_Dialog_CreateAndLoadWithPath("ah_edit_user_rdh", AB_PM_LIBNAME, AB_PM_DATADIR, DLG_DIALOGFILE);
  if (dlg==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return NULL;
  }

  GWEN_NEW_OBJECT(AH_EDIT_USER_RDH_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AH_EDIT_USER_RDH_DIALOG, dlg, xdlg, _freeData);
  GWEN_Dialog_SetSignalHandler(dlg, _dlgApi_signalHandler);

  /* preset */
  xdlg->provider=pro;
  xdlg->banking=AB_Provider_GetBanking(pro);
  xdlg->user=u;
  xdlg->doLock=doLock;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB _freeData(void *bp, void *p)
{
  AH_EDIT_USER_RDH_DIALOG *xdlg;

  xdlg=(AH_EDIT_USER_RDH_DIALOG *) p;
  GWEN_FREE_OBJECT(xdlg);
}



int _handleInit(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  AH_EDIT_USER_RDH_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_RDH_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg, "", GWEN_DialogProperty_Title, 0, I18N("Edit User"), 0);

  AH_Widget_HbciVersionComboSetup(dlg, "hbciVersionCombo");
  AH_Widget_RdhVersionComboSetup(dlg, "rdhVersionCombo");
  AH_Widget_UserStatusComboSetup(dlg, "statusCombo");

  _toGui(dlg, xdlg->user);

  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);
  return GWEN_DialogEvent_ResultHandled;
}



void _toGui(GWEN_DIALOG *dlg, AB_USER *user)
{
  const GWEN_URL *gu;
  uint32_t flags;

  AH_Widget_UserToGuiText(dlg, "userNameEdit",    user, AB_User_GetUserName);
  AH_Widget_UserToGuiText(dlg, "bankCodeEdit",    user, AB_User_GetBankCode);
  AH_Widget_UserToGuiText(dlg, "userIdEdit",      user, AB_User_GetUserId);
  AH_Widget_UserToGuiText(dlg, "customerIdEdit",  user, AB_User_GetCustomerId);

  gu=AH_User_GetServerUrl(user);
  if (gu) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Url_toString(gu, tbuf);
    GWEN_Dialog_SetCharProperty(dlg, "urlEdit", GWEN_DialogProperty_Value, 0, GWEN_Buffer_GetStart(tbuf), 0);
    GWEN_Buffer_free(tbuf);
  }

  AH_Widget_HbciVersionComboSetCurrent(dlg, "hbciVersionCombo", AH_User_GetHbciVersion(user));
  AH_Widget_RdhVersionComboSetCurrent(dlg, "rdhVersionCombo", (AH_User_GetCryptMode(user)<<8)+AH_User_GetRdhType(user));
  AH_Widget_UserStatusComboSetCurrent(dlg, "statusCombo", AH_User_GetStatus(user));

  flags=AH_User_GetFlags(user);
  GWEN_Dialog_SetIntProperty(dlg, "bankDoesntSignCheck", GWEN_DialogProperty_Value, 0,
                             (flags & AH_USER_FLAGS_BANK_DOESNT_SIGN)?1:0,
                             0);

  GWEN_Dialog_SetIntProperty(dlg, "bankUsesSignSeqCheck", GWEN_DialogProperty_Value, 0,
                             (flags & AH_USER_FLAGS_BANK_USES_SIGNSEQ)?1:0,
                             0);
  _setModified(dlg, 0);
}



int _fromGui(GWEN_DIALOG *dlg, AB_USER *u, int quiet)
{
  int i;
  uint32_t flags;
  GWEN_URL *gu;

  if (AH_Widget_GuiTextToUserKeepSpaces(dlg, "userNameEdit",    u, AB_User_SetUserName,    NULL)<0 ||
      AH_Widget_GuiTextToUserDeleSpaces(dlg, "bankCodeEdit",    u, AB_User_SetBankCode,    NULL)<0 ||
      AH_Widget_GuiTextToUserKeepSpaces(dlg, "userIdEdit",      u, AB_User_SetUserId,      quiet?NULL:I18N("Missing user id"))<0 ||
      AH_Widget_GuiTextToUserKeepSpaces(dlg, "customerIdEdit",  u, AB_User_SetCustomerId,  NULL)<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return GWEN_ERROR_INVALID;
  }

  /*  get country */
  if (u)
    AB_User_SetCountry(u, "de");

  i=AH_Widget_HbciVersionComboGetCurrent(dlg, "hbciVersionCombo");
  if (u)
    AH_User_SetHbciVersion(u, i);

  i=AH_Widget_RdhVersionComboGetCurrent(dlg, "rdhVersionCombo");
  if (u) {
    AH_User_SetCryptMode(u, i>>8);
    AH_User_SetRdhType(u, i & 0xff);
  }

  i=AH_Widget_UserStatusComboGetCurrent(dlg, "statusCombo");
  if (u)
    AH_User_SetStatus(u, i);

  gu=AH_Widget_GuiTextToUrl(dlg, "urlEdit", 3000);
  if (gu==NULL) {
    if (!quiet) {
      GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Invalid URL"));
      return GWEN_ERROR_BAD_DATA;
    }
  }
  if (u)
    AH_User_SetServerUrl(u, gu);
  GWEN_Url_free(gu);

  flags=0;
  if (GWEN_Dialog_GetIntProperty(dlg, "bankDoesntSignCheck", GWEN_DialogProperty_Value, 0, 0))
    flags|=AH_USER_FLAGS_BANK_DOESNT_SIGN;
  if (GWEN_Dialog_GetIntProperty(dlg, "bankUsesSignSeqCheck", GWEN_DialogProperty_Value, 0, 0))
    flags|=AH_USER_FLAGS_BANK_USES_SIGNSEQ;
  AH_User_SetFlags(u, flags);

  return 0;
}



int _handleFini(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
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
  GWEN_DB_SetIntValue(dbPrefs, GWEN_DB_FLAGS_OVERWRITE_VARS, "dialog_width", i);

  /* store dialog height */
  i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, -1);
  GWEN_DB_SetIntValue(dbPrefs, GWEN_DB_FLAGS_OVERWRITE_VARS, "dialog_height", i);
  return GWEN_DialogEvent_ResultHandled;
}



int _handleActivatedBankCode(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
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
    GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Could not create dialog, maybe incomplete installation?"));
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



int _handleActivatedOk(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  int rv;

  rv=_saveUser(dlg);
  if (rv==0)
    return GWEN_DialogEvent_ResultAccept;
  return GWEN_DialogEvent_ResultHandled;
}



int _handleActivatedApply(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  int rv;

  rv=_saveUser(dlg);
  if (rv==0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }
  return GWEN_DialogEvent_ResultHandled;
}



int _saveUser(GWEN_DIALOG *dlg)
{
  AH_EDIT_USER_RDH_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_RDH_DIALOG, dlg);
  assert(xdlg);

  rv=_fromGui(dlg, NULL, 0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (xdlg->doLock) {
    int rv;

    rv=AB_Provider_BeginExclUseUser(xdlg->provider, xdlg->user);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Unable to lock user. Maybe already in use?"));
      return rv;
    }
  }

  _fromGui(dlg, xdlg->user, 1);

  if (xdlg->doLock) {
    int rv;

    rv=AB_Provider_EndExclUseUser(xdlg->provider, xdlg->user, 0);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Unable to unlock user."));
      AB_Provider_EndExclUseUser(xdlg->provider, xdlg->user, 1);
      return rv;
    }
  }

  return 0;
}



int _handleActivatedReject(GWEN_DIALOG *dlg, GWEN_UNUSED GWEN_DIALOG_EVENTTYPE t, GWEN_UNUSED const char *sender)
{
  return GWEN_DialogEvent_ResultReject;
}



int _handleActivatedGetSysId(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  AH_EDIT_USER_RDH_DIALOG *xdlg;
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_RDH_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->modified) {
    GWEN_Gui_ShowError(I18N("User Modified"), "%s", I18N("Please apply current changes first."));
    return GWEN_DialogEvent_ResultHandled;
  }

  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetSysId(xdlg->provider, xdlg->user, ctx, DLG_WITHPROGRESS, DLG_NOUMOUNT, xdlg->doLock);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }

  _toGui(dlg, xdlg->user);

  AB_ImExporterContext_free(ctx);
  return GWEN_DialogEvent_ResultHandled;
}



int _handleActivatedGetAccounts(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  AH_EDIT_USER_RDH_DIALOG *xdlg;
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_RDH_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->modified) {
    GWEN_Gui_ShowError(I18N("User Modified"), "%s", I18N("Please apply current changes first."));
    return GWEN_DialogEvent_ResultHandled;
  }

  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetAccounts(xdlg->provider, xdlg->user, ctx, DLG_WITHPROGRESS, DLG_NOUMOUNT, xdlg->doLock);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }

  _toGui(dlg, xdlg->user);

  AB_ImExporterContext_free(ctx);
  return GWEN_DialogEvent_ResultHandled;
}



int _handleActivatedGetSepaInfo(GWEN_DIALOG *dlg, GWEN_UNUSED GWEN_DIALOG_EVENTTYPE t, GWEN_UNUSED const char *sender)
{
  AH_EDIT_USER_RDH_DIALOG *xdlg;
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_RDH_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->modified) {
    GWEN_Gui_ShowError(I18N("User Modified"), "%s", I18N("Please apply current changes first."));
    return GWEN_DialogEvent_ResultHandled;
  }

  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetAccountSepaInfo(xdlg->provider, xdlg->user, ctx, DLG_WITHPROGRESS, DLG_NOUMOUNT, xdlg->doLock);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }

  _toGui(dlg, xdlg->user);

  AB_ImExporterContext_free(ctx);
  return GWEN_DialogEvent_ResultHandled;
}



int _handleActivatedIniLetter(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
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
  rv=AH_Provider_GetIniLetterHtml(xdlg->provider, xdlg->user, 0, 0, tbuf, 1);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    // TODO: show error message
    AB_Banking_ClearCryptTokenList(xdlg->banking);
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }
  GWEN_Buffer_AppendString(tbuf, "</html>");


  /* add ASCII version of the INI letter for frontends which don't support HTML */
  rv=AH_Provider_GetIniLetterTxt(xdlg->provider, xdlg->user, 0, 0, tbuf, 0);
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
    GWEN_Gui_ShowError(I18N("Error"), I18N("Error creating INI-Letter (%d)"), rv);
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }

  GWEN_Buffer_free(tbuf);
  return GWEN_DialogEvent_ResultHandled;
}



int _handleValueChanged(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  _setModified(dlg, 1);
  return GWEN_DialogEvent_ResultHandled;
}



void _setModified(GWEN_DIALOG *dlg, int enabled)
{
  AH_EDIT_USER_RDH_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_RDH_DIALOG, dlg);
  assert(xdlg);

  xdlg->modified=enabled;
  GWEN_Dialog_SetIntProperty(dlg, "applyButton", GWEN_DialogProperty_Enabled, 0, enabled, 0);
}



int GWENHYWFAR_CB _dlgApi_signalHandler(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  const _DIALOG_SIGNAL_ENTRY *entry;

  entry=_signalMap;
  while(entry->handlerFn) {
    if (entry->eventType==t && (entry->sender==NULL || (sender && strcasecmp(sender, entry->sender)==0))) {
      return entry->handlerFn(dlg, t, sender);
    }
    entry++;
  }

  return GWEN_DialogEvent_ResultNotHandled;
}




