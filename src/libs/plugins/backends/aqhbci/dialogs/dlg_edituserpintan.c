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

#include "dlg_edituserpintan_p.h"
#include "w_hbciversioncombo.h"
#include "w_tanmethodcombo.h"
#include "w_utils.h"

#include "banking/provider_l.h"

#include "aqhbci/banking/user.h"
#include "aqhbci/banking/provider.h"
#include "aqhbci/banking/provider_online.h"

#include <aqbanking/backendsupport/user.h>
#include <aqbanking/banking_be.h>
#include <aqbanking/dialogs/dlg_selectbankinfo.h>
#include "aqbanking/i18n_l.h"

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
#define DLG_UMOUNT       0
#define DLG_DIALOGFILE   "aqbanking/backends/aqhbci/dialogs/dlg_edituserpintan.dlg"


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

static int _saveUser(GWEN_DIALOG *dlg);

static int _fromGui(GWEN_DIALOG *dlg, AB_USER *u, int quiet);
static int _handleInit(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleFini(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedBankCode(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedOk(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedApply(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedReject(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedGetCert(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedGetSysId(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedGetBankInfo(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedGetItanModes(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedGetAccounts(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleValueChanged(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);

static void _setModified(GWEN_DIALOG *dlg, int enabled);

static void _toGui(GWEN_DIALOG *dlg, AB_USER *user);

static void _tanMechanismComboSetup(GWEN_DIALOG *dlg, const char *widgetName);
static void _tanMechanismComboSetCurrent(GWEN_DIALOG *dlg, const char *widgetName, int t);
static int _tanMechanismComboGetCurrent(GWEN_DIALOG *dlg, const char *widgetName);

static void _httpVersionComboSetup(GWEN_DIALOG *dlg, const char *widgetName);
static void _httpVersionComboSetCurrent(GWEN_DIALOG *dlg, const char *widgetName, int v);
static int _httpVersionComboGetCurrent(GWEN_DIALOG *dlg, const char *widgetName);

static void _userFlagsToGui(GWEN_DIALOG *dlg, uint32_t flags);
static uint32_t _userFlagsFromGui(GWEN_DIALOG *dlg);



/* ------------------------------------------------------------------------------------------------
 * static vars
 * ------------------------------------------------------------------------------------------------
 */

GWEN_INHERIT(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG)

static _DIALOG_SIGNAL_ENTRY _signalMap[]={
  {NULL,                  GWEN_DialogEvent_TypeInit,         _handleInit},
  {NULL,                  GWEN_DialogEvent_TypeFini,         _handleFini},
  {"bankCodeButton",      GWEN_DialogEvent_TypeActivated,    _handleActivatedBankCode},
  {"getCertButton",       GWEN_DialogEvent_TypeActivated,    _handleActivatedGetCert},
  {"getBankInfoButton",   GWEN_DialogEvent_TypeActivated,    _handleActivatedGetBankInfo},
  {"getSysIdButton",      GWEN_DialogEvent_TypeActivated,    _handleActivatedGetSysId},
  {"getItanModesButton",  GWEN_DialogEvent_TypeActivated,    _handleActivatedGetItanModes},
  {"getAccountsButton",   GWEN_DialogEvent_TypeActivated,    _handleActivatedGetAccounts},

  {"userNameEdit",        GWEN_DialogEvent_TypeValueChanged, _handleValueChanged},
  {"bankCodeEdit",        GWEN_DialogEvent_TypeValueChanged, _handleValueChanged},
  {"userIdEdit",          GWEN_DialogEvent_TypeValueChanged, _handleValueChanged},
  {"customerIdEdit",      GWEN_DialogEvent_TypeValueChanged, _handleValueChanged},
  {"tanMediumIdEdit",     GWEN_DialogEvent_TypeValueChanged, _handleValueChanged},
  {"urlEdit",             GWEN_DialogEvent_TypeValueChanged, _handleValueChanged},
  {"hbciVersionCombo",    GWEN_DialogEvent_TypeActivated,    _handleValueChanged},
  {"httpVersionCombo",    GWEN_DialogEvent_TypeActivated,    _handleValueChanged},
  {"tanMethodCombo",      GWEN_DialogEvent_TypeActivated,    _handleValueChanged},
  {"tanMechanismCombo",   GWEN_DialogEvent_TypeActivated,    _handleValueChanged},
  {"noBase64Check",       GWEN_DialogEvent_TypeActivated,    _handleValueChanged},
  {"omitSmsAccountCheck", GWEN_DialogEvent_TypeActivated,    _handleValueChanged},

  {"okButton",            GWEN_DialogEvent_TypeActivated,    _handleActivatedOk},
  {"applyButton",         GWEN_DialogEvent_TypeActivated,    _handleActivatedApply},
  {"abortButton",         GWEN_DialogEvent_TypeActivated,    _handleActivatedReject},

  {NULL, 0, NULL}
};



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

GWEN_DIALOG *AH_EditUserPinTanDialog_new(AB_PROVIDER *pro, AB_USER *u, int doLock)
{
  GWEN_DIALOG *dlg;
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;

  dlg=GWEN_Dialog_CreateAndLoadWithPath("ah_edit_user_pintan", AB_PM_LIBNAME, AB_PM_DATADIR, DLG_DIALOGFILE);
  if (dlg==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return NULL;
  }

  GWEN_NEW_OBJECT(AH_EDIT_USER_PINTAN_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg, xdlg, _freeData);
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
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;

  xdlg=(AH_EDIT_USER_PINTAN_DIALOG *) p;

  GWEN_FREE_OBJECT(xdlg);
}



int _handleInit(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  /* init */
  GWEN_Dialog_SetCharProperty(dlg, "", GWEN_DialogProperty_Title, 0, I18N("Edit User"), 0);

  _tanMechanismComboSetup(dlg, "tanMechanismCombo");

  AH_Widget_HbciVersionComboSetup(dlg, "hbciVersionCombo");
  _httpVersionComboSetup(dlg, "httpVersionCombo");

  GWEN_Dialog_SetCharProperty(dlg, "tanMediumIdEdit", GWEN_DialogProperty_ToolTip, 0,
                              I18N("For smsTAN or mTAN this is your mobile phone number. "
                                   "Please ask your bank for the necessary format of this number."),
                              0);

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

  AH_Widget_TanMethodComboRebuild(dlg, "tanMethodCombo", AH_User_GetTanMethodDescriptions(user));
  AH_Widget_TanMethodComboSetCurrent(dlg, "tanMethodCombo", AH_User_GetSelectedTanMethod(user));

  AH_Widget_UserToGuiText(dlg, "userNameEdit",    user, AB_User_GetUserName);
  AH_Widget_UserToGuiText(dlg, "bankCodeEdit",    user, AB_User_GetBankCode);
  AH_Widget_UserToGuiText(dlg, "userIdEdit",      user, AB_User_GetUserId);
  AH_Widget_UserToGuiText(dlg, "customerIdEdit",  user, AB_User_GetCustomerId);
  AH_Widget_UserToGuiText(dlg, "tanMediumIdEdit", user, AH_User_GetTanMediumId);

  _tanMechanismComboSetCurrent(dlg, "tanMechanismCombo", AH_User_GetSelectedTanInputMechanism(user));
  AH_Widget_HbciVersionComboSetCurrent(dlg, "hbciVersionCombo", AH_User_GetHbciVersion(user));
  _httpVersionComboSetCurrent(dlg, "httpVersionCombo", ((AH_User_GetHttpVMajor(user))<<8)+AH_User_GetHttpVMinor(user));

  _userFlagsToGui(dlg, AH_User_GetFlags(user));

  gu=AH_User_GetServerUrl(user);
  if (gu) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Url_toString(gu, tbuf);
    GWEN_Dialog_SetCharProperty(dlg, "urlEdit", GWEN_DialogProperty_Value, 0, GWEN_Buffer_GetStart(tbuf), 0);
    GWEN_Buffer_free(tbuf);
  }
  _setModified(dlg, 0);
}



int _fromGui(GWEN_DIALOG *dlg, AB_USER *u, int quiet)
{
  int i;
  GWEN_URL *gu;

  if (AH_Widget_GuiTextToUserKeepSpaces(dlg, "userNameEdit",    u, AB_User_SetUserName,    NULL)<0 ||
      AH_Widget_GuiTextToUserDeleSpaces(dlg, "bankCodeEdit",    u, AB_User_SetBankCode,    NULL)<0 ||
      AH_Widget_GuiTextToUserKeepSpaces(dlg, "userIdEdit",      u, AB_User_SetUserId,      quiet?NULL:I18N("Missing user id"))<0 ||
      AH_Widget_GuiTextToUserKeepSpaces(dlg, "customerIdEdit",  u, AB_User_SetCustomerId,  NULL)<0 ||
      AH_Widget_GuiTextToUserKeepSpaces(dlg, "tanMediumIdEdit", u, AH_User_SetTanMediumId, NULL)<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return GWEN_ERROR_INVALID;
  }

  if (u)
    AB_User_SetCountry(u, "de");

  if (u)
    AH_User_SetHbciVersion(u, AH_Widget_HbciVersionComboGetCurrent(dlg, "hbciVersionCombo"));

  i=_httpVersionComboGetCurrent(dlg, "httpVersionCombo");
  AH_User_SetHttpVMajor(u, ((i>>8) & 0xff));
  AH_User_SetHttpVMinor(u, (i & 0xff));

  AH_User_SetSelectedTanInputMechanism(u, _tanMechanismComboGetCurrent(dlg, "tanMechanismCombo"));

  i=AH_Widget_TanMethodComboGetCurrent(dlg, "tanMethodCombo");
  if (i>0) {
    AH_User_SetSelectedTanMethod(u, i);
  }
  else {
    if (!quiet) {
      GWEN_Gui_ShowError(I18N("Error on Input"), "%s", I18N("Please select tan method."));
      GWEN_Dialog_SetIntProperty(dlg, "tanMethodCombo", GWEN_DialogProperty_Focus, 0, 1, 0);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Missing tan method");
      return GWEN_ERROR_INVALID;
    }
  }

  gu=AH_Widget_GuiTextToUrl(dlg, "urlEdit", 443);
  if (gu==NULL) {
    if (!quiet) {
      GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Invalid URL"));
      return GWEN_ERROR_BAD_DATA;
    }
  }
  if (u)
    AH_User_SetServerUrl(u, gu);
  GWEN_Url_free(gu);

  AH_User_SetFlags(u, _userFlagsFromGui(dlg));

  return 0;
}



int _handleFini(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
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
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
  assert(xdlg);

  dlg2=AB_SelectBankInfoDialog_new(xdlg->banking, "de", NULL);
  if (dlg2==NULL) {
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
      GWEN_Dialog_SetCharProperty(dlg, "bankCodeEdit", GWEN_DialogProperty_Value, 0, (s && *s)?s:"", 0);
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
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
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



int _handleActivatedGetCert(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->modified) {
    GWEN_Gui_ShowError(I18N("User Modified"), "%s", I18N("Please apply current changes first."));
    return GWEN_DialogEvent_ResultHandled;
  }

  rv=AH_Provider_GetCert(xdlg->provider, xdlg->user, DLG_WITHPROGRESS, DLG_UMOUNT, xdlg->doLock);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }

  return GWEN_DialogEvent_ResultHandled;
}



int _handleActivatedGetSysId(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->modified) {
    GWEN_Gui_ShowError(I18N("User Modified"), "%s", I18N("Please apply current changes first."));
    return GWEN_DialogEvent_ResultHandled;
  }

  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetSysId(xdlg->provider, xdlg->user, ctx, DLG_WITHPROGRESS, DLG_UMOUNT, xdlg->doLock);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }

  _toGui(dlg, xdlg->user);

  AB_ImExporterContext_free(ctx);
  return GWEN_DialogEvent_ResultHandled;
}



int _handleActivatedGetBankInfo(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->modified) {
    GWEN_Gui_ShowError(I18N("User Modified"), "%s", I18N("Please apply current changes first."));
    return GWEN_DialogEvent_ResultHandled;
  }

  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetBankInfo(xdlg->provider,
                             xdlg->user,
                             ctx,
                             0,   /* without TAN segment, maybe later add button for call with TAN segment */
                             DLG_WITHPROGRESS,
                             DLG_UMOUNT,
                             xdlg->doLock);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }

  _toGui(dlg, xdlg->user);

  AB_ImExporterContext_free(ctx);
  return GWEN_DialogEvent_ResultHandled;
}



int _handleActivatedGetItanModes(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->modified) {
    GWEN_Gui_ShowError(I18N("User Modified"), "%s", I18N("Please apply current changes first."));
    return GWEN_DialogEvent_ResultHandled;
  }

  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetItanModes(xdlg->provider, xdlg->user, ctx, DLG_WITHPROGRESS, DLG_UMOUNT, xdlg->doLock);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }

  _toGui(dlg, xdlg->user);

  AB_ImExporterContext_free(ctx);
  return GWEN_DialogEvent_ResultHandled;
}



int _handleActivatedGetAccounts(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->modified) {
    GWEN_Gui_ShowError(I18N("User Modified"), "%s", I18N("Please apply current changes first."));
    return GWEN_DialogEvent_ResultHandled;
  }

  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetAccounts(xdlg->provider, xdlg->user, ctx, DLG_WITHPROGRESS, DLG_UMOUNT, xdlg->doLock);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }

  _toGui(dlg, xdlg->user);

  AB_ImExporterContext_free(ctx);
  return GWEN_DialogEvent_ResultHandled;
}



int _handleValueChanged(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  _setModified(dlg, 1);
  return GWEN_DialogEvent_ResultHandled;
}



void _setModified(GWEN_DIALOG *dlg, int enabled)
{
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
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






void _tanMechanismComboSetup(GWEN_DIALOG *dlg, const char *widgetName)
{
  const GWEN_DIALOG_PROPERTY addValue=GWEN_DialogProperty_AddValue;
  const GWEN_DIALOG_PROPERTY clrValue=GWEN_DialogProperty_ClearValues;
  const GWEN_DIALOG_PROPERTY toolTip=GWEN_DialogProperty_ToolTip;

  GWEN_Dialog_SetCharProperty(dlg, widgetName, toolTip, 0,
                              I18N("Please only change this value if you know what you are doing, otherwise leave it at \"auto\"."),
                              0);
  GWEN_Dialog_SetIntProperty(dlg,  widgetName, clrValue, 0, 0, 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("tanMechanism|auto"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("tanMechanism|text"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("tanMechanism|chipTAN optic"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("tanMechanism|QR image"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("tanMechanism|photoTAN"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("tanMechanism|chipTAN USB"), 0);
}



void _tanMechanismComboSetCurrent(GWEN_DIALOG *dlg, const char *widgetName, int t)
{
  const GWEN_DIALOG_PROPERTY setValue=GWEN_DialogProperty_Value;

  switch (t) {
  case AB_BANKING_TANMETHOD_TEXT:          GWEN_Dialog_SetIntProperty(dlg, widgetName, setValue, 0, 1, 0); break;
  case AB_BANKING_TANMETHOD_CHIPTAN_OPTIC: GWEN_Dialog_SetIntProperty(dlg, widgetName, setValue, 0, 2, 0); break;
  case AB_BANKING_TANMETHOD_CHIPTAN_QR:    GWEN_Dialog_SetIntProperty(dlg, widgetName, setValue, 0, 3, 0); break;
  case AB_BANKING_TANMETHOD_PHOTOTAN:      GWEN_Dialog_SetIntProperty(dlg, widgetName, setValue, 0, 4, 0); break;
  case AB_BANKING_TANMETHOD_CHIPTAN_USB:   GWEN_Dialog_SetIntProperty(dlg, widgetName, setValue, 0, 5, 0); break;
  default:
    DBG_WARN(AQHBCI_LOGDOMAIN, "Using default tanMechanism");
    GWEN_Dialog_SetIntProperty(dlg, widgetName, setValue, 0, 0, 0);
    break;
  }
}



int _tanMechanismComboGetCurrent(GWEN_DIALOG *dlg, const char *widgetName)
{
  int idx;

  idx=GWEN_Dialog_GetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, -1);
  switch(idx) {
    case 1:  return AB_BANKING_TANMETHOD_TEXT;
    case 2:  return AB_BANKING_TANMETHOD_CHIPTAN_OPTIC;
    case 3:  return AB_BANKING_TANMETHOD_CHIPTAN_QR;
    case 4:  return AB_BANKING_TANMETHOD_PHOTOTAN;
    case 5:  return AB_BANKING_TANMETHOD_CHIPTAN_USB;
    default: return AB_BANKING_TANMETHOD_TEXT;
  }
}





void _httpVersionComboSetup(GWEN_DIALOG *dlg, const char *widgetName)
{
  const GWEN_DIALOG_PROPERTY addValue=GWEN_DialogProperty_AddValue;
  const GWEN_DIALOG_PROPERTY clrValue=GWEN_DialogProperty_ClearValues;

  GWEN_Dialog_SetIntProperty(dlg,  widgetName, clrValue, 0, 0, 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, I18N("-- select --"), 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, "1.0", 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, addValue, 0, "1.1", 0);
}



void _httpVersionComboSetCurrent(GWEN_DIALOG *dlg, const char *widgetName, int v)
{
  const GWEN_DIALOG_PROPERTY setValue=GWEN_DialogProperty_Value;

  switch (v) {
    case 0x0100: GWEN_Dialog_SetIntProperty(dlg, widgetName, setValue, 0, 1, 0); break;
    case 0x0101: GWEN_Dialog_SetIntProperty(dlg, widgetName, setValue, 0, 2, 0); break;
    default:     GWEN_Dialog_SetIntProperty(dlg, widgetName, setValue, 0, 2, 0); break;
  }
}



int _httpVersionComboGetCurrent(GWEN_DIALOG *dlg, const char *widgetName)
{
  int idx;

  idx=GWEN_Dialog_GetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, -1);
  switch(idx) {
    case 1:  return 0x0100;
    case 2:  return 0x0101;
    default: return 0x0101;
  }
}



void _userFlagsToGui(GWEN_DIALOG *dlg, uint32_t flags)
{
  const GWEN_DIALOG_PROPERTY setValue=GWEN_DialogProperty_Value;

  GWEN_Dialog_SetIntProperty(dlg, "noBase64Check", setValue, 0, (flags & AH_USER_FLAGS_NO_BASE64)?1:0, 0);
  GWEN_Dialog_SetIntProperty(dlg, "omitSmsAccountCheck", setValue, 0, (flags & AH_USER_FLAGS_TAN_OMIT_SMS_ACCOUNT)?1:0, 0);
}



uint32_t _userFlagsFromGui(GWEN_DIALOG *dlg)
{
  uint32_t flags=0;

  if (GWEN_Dialog_GetIntProperty(dlg, "noBase64Check", GWEN_DialogProperty_Value, 0, 0))
    flags|=AH_USER_FLAGS_NO_BASE64;
  if (GWEN_Dialog_GetIntProperty(dlg, "omitSmsAccountCheck", GWEN_DialogProperty_Value, 0, 0))
    flags|=AH_USER_FLAGS_TAN_OMIT_SMS_ACCOUNT;

  return flags;
}




