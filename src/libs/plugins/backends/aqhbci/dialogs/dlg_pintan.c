/***************************************************************************
 begin       : Mon Apr 12 2010
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_pintan_p.h"

#include "aqhbci/banking/provider_l.h"

#include "aqbanking/i18n_l.h"
#include <aqbanking/dialogs/dlg_selectbankinfo.h>
#include <aqbanking/backendsupport/user.h>
#include <aqbanking/banking_be.h>

#include "aqhbci/banking/user.h"
#include "aqhbci/banking/provider.h"
#include "aqhbci/banking/provider_online.h"
#include "dlg_pintan_special_l.h"
#include "dlg_pintan_tanmode_l.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>


#define PAGE_BEGIN     0
#define PAGE_BANK      1
#define PAGE_USER      2
#define PAGE_CREATE    3
#define PAGE_END       4


#define DIALOG_MINWIDTH  400
#define DIALOG_MINHEIGHT 200


GWEN_INHERIT(GWEN_DIALOG, AH_PINTAN_DIALOG)



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void GWENHYWFAR_CB _dlgApi_FreeData(void *bp, void *p);
static void _dialogInit(GWEN_DIALOG *dlg);
static void _dialogFini(GWEN_DIALOG *dlg);
static int _dialogNext(GWEN_DIALOG *dlg);
static int _dialogPrevious(GWEN_DIALOG *dlg);

static int GWENHYWFAR_CB _dlgApi_SignalHandler(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleSignalActivated(GWEN_DIALOG *dlg, const char *sender);
static int _handleSignalActivatedBankCode(GWEN_DIALOG *dlg);
static int _handleSignalActivatedSpecial(GWEN_DIALOG *dlg);
static int _handleSignalValueChanged(GWEN_DIALOG *dlg, const char *sender);

static void _setBankCode(GWEN_DIALOG *dlg, const char *s);
static void _setBankName(GWEN_DIALOG *dlg, const char *s);
static void _setUserName(GWEN_DIALOG *dlg, const char *s);
static void _setUserId(GWEN_DIALOG *dlg, const char *s);
static void _setCustomerId(GWEN_DIALOG *dlg, const char *s);
static void _setUrl(GWEN_DIALOG *dlg, const char *s);
static void _setTanMediumId(GWEN_DIALOG *dlg, const char *s);

static int _addUserAndSetupWithBankServer(GWEN_DIALOG *dlg);
static int _setupLockedUserWithBankServer(GWEN_DIALOG *dlg, AB_USER *u, uint32_t pid);
static int _selectTanMethod(GWEN_DIALOG *dlg, AB_USER *u, int doLock);
static AB_USER *_createAndSetupUser(GWEN_DIALOG *dlg, uint32_t pid);

static int _getCertificate(AB_PROVIDER *pro, AB_USER *u, uint32_t pid);
static int _getBankInfoAnon(AB_PROVIDER *pro, AB_USER *u, uint32_t pid);
static int _getSystemId(AB_PROVIDER *pro, AB_USER *u, uint32_t pid);
static int _getAccountList(AB_PROVIDER *pro, AB_USER *u, uint32_t pid);

static void _removeAllSpaces(uint8_t *s);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



GWEN_DIALOG *AH_PinTanDialog_new(AB_PROVIDER *pro)
{
  GWEN_DIALOG *dlg;
  AH_PINTAN_DIALOG *xdlg;

  dlg=GWEN_Dialog_CreateAndLoadWithPath("ah_setup_pintan",
                                        AB_PM_LIBNAME,
                                        AB_PM_DATADIR,
                                        "aqbanking/backends/aqhbci/dialogs/dlg_pintan.dlg");
  if (dlg==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here.");
    return NULL;
  }
  GWEN_NEW_OBJECT(AH_PINTAN_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg, xdlg, _dlgApi_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, _dlgApi_SignalHandler);

  xdlg->banking=AB_Provider_GetBanking(pro);
  xdlg->provider=pro;

  /* preset */
  xdlg->hbciVersion=300;
  xdlg->httpVMajor=1;
  xdlg->httpVMinor=1;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB _dlgApi_FreeData(void *bp, void *p)
{
  AH_PINTAN_DIALOG *xdlg;

  xdlg=(AH_PINTAN_DIALOG *) p;
  free(xdlg->bankCode);
  free(xdlg->bankName);
  free(xdlg->userName);
  free(xdlg->userId);
  free(xdlg->customerId);
  free(xdlg->tanMediumId);

  GWEN_FREE_OBJECT(xdlg);
}



AB_USER *AH_PinTanDialog_GetUser(const GWEN_DIALOG *dlg)
{
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  return xdlg->user;
}



void _setBankCode(GWEN_DIALOG *dlg, const char *s)
{
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->bankCode);
  xdlg->bankCode=s?strdup(s):NULL;
}



void _setBankName(GWEN_DIALOG *dlg, const char *s)
{
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->bankName);
  xdlg->bankName=s?strdup(s):NULL;
}



void _setUserName(GWEN_DIALOG *dlg, const char *s)
{
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userName);
  xdlg->userName=s?strdup(s):NULL;
}



void _setUserId(GWEN_DIALOG *dlg, const char *s)
{
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userId);
  xdlg->userId=s?strdup(s):NULL;
}



void _setCustomerId(GWEN_DIALOG *dlg, const char *s)
{
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->customerId);
  xdlg->customerId=s?strdup(s):NULL;
}



void _setUrl(GWEN_DIALOG *dlg, const char *s)
{
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->url);
  xdlg->url=s?strdup(s):NULL;
}



void _setTanMediumId(GWEN_DIALOG *dlg, const char *s)
{
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->tanMediumId);
  xdlg->tanMediumId=s?strdup(s):NULL;
}



void _dialogInit(GWEN_DIALOG *dlg)
{
  AH_PINTAN_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
                              "",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("HBCI PIN/TAN Setup Wizard"),
                              0);

  /* select first page */
  GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, 0, 0);

  /* setup intro page */
  GWEN_Dialog_SetCharProperty(dlg,
                              "wiz_begin_label",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("This dialog assists you in setting up a Pin/TAN User.\n"),
                              0);

  /* setup bank page */
  GWEN_Dialog_SetCharProperty(dlg,
                              "wiz_bank_label",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("<html>"
                                   "<p>Please select the bank.</p>"
                                   "<p>AqBanking has an internal database which "
                                   "contains HBCI/FinTS information about many banks.<p>"
                                   "<p>If there is an entry for your bank this dialog will use the "
                                   "information from the database.</p>"
                                   "</html>"
                                   "Please select the bank.\n"
                                   "AqBanking has an internal database which contains\n"
                                   "HBCI/FinTS information about many banks.\n"
                                   "If there is an entry for your bank this dialog will use the\n"
                                   "information from the database."),
                              0);

  /* setup user page */
  GWEN_Dialog_SetCharProperty(dlg,
                              "wiz_user_label",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("<html>"
                                   "<p>For most banks the customer id must be the same as the user id.</p>"
                                   "<p>However, some banks actually use the customer id, so please look into "
                                   "the documentation provided by your bank to discover whether this is the "
                                   "case with your bank.</p>"
                                   "</html>"
                                   "For most banks the customer id must be the same as the user id.\n"
                                   "However, some banks actually use the customer id, so please look into\n"
                                   "the documentation provided by your bank to discover whether this is the\n"
                                   "case with your bank."),
                              0);

  /* setup creation page */
  GWEN_Dialog_SetCharProperty(dlg,
                              "wiz_create_label",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("<html>"
                                   "<p>We are now ready to create the user and retrieve the account list.</p>"
                                   "<p>Click the <i>next</i> button to proceed or <i>abort</i> to abort.</p>"
                                   "</html>"
                                   "We are now ready to create the user and retrieve the account list.\n"
                                   "Click the NEXT button to proceed or ABORT to abort."),
                              0);

  /* setup extro page */
  GWEN_Dialog_SetCharProperty(dlg,
                              "wiz_end_label",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("The user has been successfully setup."),
                              0);

  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);

  /* disable next and previous buttons */
  GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
  GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
}



void _dialogFini(GWEN_DIALOG *dlg)
{
  AH_PINTAN_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
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



int AH_PinTanDialog_GetBankPageData(GWEN_DIALOG *dlg)
{
  AH_PINTAN_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_bankcode_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    _setBankCode(dlg, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Missing bank code");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_bankname_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    _setBankName(dlg, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else
    _setBankName(dlg, NULL);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_url_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    _removeAllSpaces((uint8_t *)GWEN_Buffer_GetStart(tbuf));
    _setUrl(dlg, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Missing URL");
    return GWEN_ERROR_NO_DATA;
  }

  return 0;
}



int AH_PinTanDialog_GetUserPageData(GWEN_DIALOG *dlg)
{
  AH_PINTAN_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_username_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    _setUserName(dlg, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Missing user name");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_userid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    _setUserId(dlg, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Missing user id");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_customerid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    _setCustomerId(dlg, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else
    _setCustomerId(dlg, NULL);

  return 0;
}



int AH_PinTanDialog_EnterPage(GWEN_DIALOG *dlg, int page, int forwards)
{
  AH_PINTAN_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  switch (page) {
  case PAGE_BEGIN:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_BANK:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    rv=AH_PinTanDialog_GetBankPageData(dlg);
    if (rv<0)
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_USER:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    rv=AH_PinTanDialog_GetUserPageData(dlg);
    if (rv<0)
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_CREATE:
    if (!forwards)
      GWEN_Dialog_SetCharProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Title, 0, I18N("Next"), 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_END:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    GWEN_Dialog_SetCharProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Title, 0, I18N("Finish"), 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_abort_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    return GWEN_DialogEvent_ResultHandled;

  default:
    return GWEN_DialogEvent_ResultHandled;
  }

  return GWEN_DialogEvent_ResultHandled;
}



int _addUserAndSetupWithBankServer(GWEN_DIALOG *dlg)
{
  AH_PINTAN_DIALOG *xdlg;
  AB_USER *u;
  int rv;
  uint32_t pid;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Create and Setup PinTan HBCI User");
  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Creating user");
  u=_createAndSetupUser(dlg, 0);
  if (u==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create user, maybe backend missing?");
    // TODO: show error message
    return GWEN_DialogEvent_ResultHandled;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Adding user");
  rv=AB_Provider_AddUser(xdlg->provider, u);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not add user (%d)", rv);
    AB_User_free(u);
    return GWEN_DialogEvent_ResultHandled;
  }

  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
                             GWEN_GUI_PROGRESS_ALLOW_EMBED |
                             GWEN_GUI_PROGRESS_SHOW_PROGRESS |
                             GWEN_GUI_PROGRESS_SHOW_ABORT,
                             I18N("Setting Up PIN/TAN User"),
                             I18N("The system id and a list of accounts will be retrieved."),
                             4,
                             0);
  /* lock new user */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Locking user");
  rv=AB_Provider_BeginExclUseUser(xdlg->provider, u);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not lock user (%d)", rv);
    GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Error, I18N("Unable to lock users"));
    AB_Provider_DeleteUser(xdlg->provider, AB_User_GetUniqueId(u));
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }

  /* do all the magic stuff */
  rv=_setupLockedUserWithBankServer(dlg, u, pid);
  if (rv<0) {
    AB_Provider_EndExclUseUser(xdlg->provider, u, 1);
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_Provider_DeleteUser(xdlg->provider, AB_User_GetUniqueId(u));
    GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Error, I18N("Aborted by user."));
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }

  /* unlock user */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Unlocking user");
  rv=AB_Provider_EndExclUseUser(xdlg->provider, u, 0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not unlock customer [%s] (%d)", AB_User_GetCustomerId(u), rv);
    GWEN_Gui_ProgressLog2(pid,
                          GWEN_LoggerLevel_Error,
                          I18N("Could not unlock user %s (%d)"),
                          AB_User_GetUserId(u), rv);
    AB_Provider_EndExclUseUser(xdlg->provider, u, 1);
    AB_Provider_DeleteUser(xdlg->provider, AB_User_GetUniqueId(u));
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }

  GWEN_Dialog_SetCharProperty(dlg,
                              "wiz_end_label",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("The user has been successfully setup."),
                              0);
  GWEN_Gui_ProgressEnd(pid);
  AH_PinTanDialog_EnterPage(dlg, PAGE_END, 1);

  xdlg->user=u;

  return GWEN_DialogEvent_ResultHandled;
}



AB_USER *_createAndSetupUser(GWEN_DIALOG *dlg, uint32_t pid)
{
  AH_PINTAN_DIALOG *xdlg;
  AB_USER *u;
  GWEN_URL *url;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  DBG_INFO(0, "Creating user");
  u=AB_Provider_CreateUserObject(xdlg->provider);
  if (u==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create user, maybe backend missing?");
    // TODO: show error message
    return NULL;
  }

  /* generic setup */
  AB_User_SetUserName(u, xdlg->userName);
  AB_User_SetUserId(u, xdlg->userId);
  if (xdlg->customerId && *(xdlg->customerId))
    AB_User_SetCustomerId(u, xdlg->customerId);
  else
    AB_User_SetCustomerId(u, xdlg->userId);
  AB_User_SetCountry(u, "de");

  AB_User_SetBankCode(u, xdlg->bankCode);

  /* HBCI setup */
  AH_User_SetTokenType(u, "pintan");
  AH_User_SetCryptMode(u, AH_CryptMode_Pintan);
  AH_User_SetStatus(u, AH_UserStatusEnabled);

  url=GWEN_Url_fromString(xdlg->url);
  assert(url);
  GWEN_Url_SetProtocol(url, "https");
  if (GWEN_Url_GetPort(url)==0)
    GWEN_Url_SetPort(url, 443);
  AH_User_SetServerUrl(u, url);
  GWEN_Url_free(url);
  AH_User_SetHbciVersion(u, xdlg->hbciVersion);
  AH_User_SetHttpVMajor(u, xdlg->httpVMajor);
  AH_User_SetHttpVMinor(u, xdlg->httpVMinor);
  AH_User_SetFlags(u, xdlg->flags);
  AH_User_SetTanMediumId(u, xdlg->tanMediumId);

  return u;
}



int _setupLockedUserWithBankServer(GWEN_DIALOG *dlg, AB_USER *u, uint32_t pid)
{
  AH_PINTAN_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  rv=_getCertificate(xdlg->provider, u, pid);
  if (rv<0) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_getBankInfoAnon(xdlg->provider, u, pid);
  if (rv<0) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_getSystemId(xdlg->provider, u, pid);
  if (rv<0) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* select TAN method */
  rv=_selectTanMethod(dlg, u, 0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_getAccountList(xdlg->provider, u, pid);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int _getCertificate(AB_PROVIDER *pro, AB_USER *u, uint32_t pid)
{
  int rv;

  DBG_NOTICE(0, "Getting cert (%08x)", AH_User_GetFlags(u));
  GWEN_Gui_ProgressLog(pid,
                       GWEN_LoggerLevel_Notice,
                       I18N("Retrieving SSL certificate"));
  rv=AH_Provider_GetCert(pro, u, 0, 1, 0);
  if (rv<0) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  rv=GWEN_Gui_ProgressAdvance(pid, GWEN_GUI_PROGRESS_ONE);
  if (rv==GWEN_ERROR_USER_ABORTED) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int _getBankInfoAnon(AB_PROVIDER *pro, AB_USER *u, uint32_t pid)
{
  AB_IMEXPORTER_CONTEXT *ctx;
  int rv;

  /* get bank info (for SCA) */
  DBG_NOTICE(0, "Getting generic bank info");
  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Notice, "");
  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Notice, I18N("Retrieving generic bank info (SCA)"));

  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetBankInfo(pro, u, ctx, 0 /* without HKTAN */, 0, 1, 0);
  if (rv<0) {
    AB_ImExporterContext_free(ctx);
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error getting bank info (%d), ignoring", rv);
    GWEN_Gui_ProgressLog(pid,
                         GWEN_LoggerLevel_Notice,
                         I18N("This step failed but that's okay, some banks just don't support it."));
  }
  AB_ImExporterContext_free(ctx);

  rv=GWEN_Gui_ProgressAdvance(pid, GWEN_GUI_PROGRESS_ONE);
  if (rv==GWEN_ERROR_USER_ABORTED) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int _getSystemId(AB_PROVIDER *pro, AB_USER *u, uint32_t pid)
{
  AB_IMEXPORTER_CONTEXT *ctx;
  int rv;

  DBG_NOTICE(0, "Getting sysid");
  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Notice, "");
  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Notice, I18N("Retrieving system id"));
  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetSysId(pro, u, ctx, 0, 1, 0);
  if (rv<0) {
    AB_ImExporterContext_free(ctx);
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  AB_ImExporterContext_free(ctx);

  rv=GWEN_Gui_ProgressAdvance(pid, GWEN_GUI_PROGRESS_ONE);
  if (rv==GWEN_ERROR_USER_ABORTED) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int _getAccountList(AB_PROVIDER *pro, AB_USER *u, uint32_t pid)
{
  AB_IMEXPORTER_CONTEXT *ctx;
  int rv;

  /* get account list */
  DBG_NOTICE(0, "Getting account list");
  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Notice, "");
  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Notice, I18N("Retrieving account list"));
  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetAccounts(pro, u, ctx, 0, 1, 0);
  if (rv<0) {
    AB_ImExporterContext_free(ctx);
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  AB_ImExporterContext_free(ctx);

  rv=GWEN_Gui_ProgressAdvance(pid, GWEN_GUI_PROGRESS_ONE);
  if (rv==GWEN_ERROR_USER_ABORTED) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}





int _dialogNext(GWEN_DIALOG *dlg)
{
  AH_PINTAN_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  if (page==PAGE_CREATE) {
    return _addUserAndSetupWithBankServer(dlg);
  }
  else if (page<PAGE_END) {
    page++;
    return AH_PinTanDialog_EnterPage(dlg, page, 1);
  }
  else if (page==PAGE_END)
    return GWEN_DialogEvent_ResultAccept;

  return GWEN_DialogEvent_ResultHandled;
}



int _dialogPrevious(GWEN_DIALOG *dlg)
{
  AH_PINTAN_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  if (page>PAGE_BEGIN) {
    page--;
    return AH_PinTanDialog_EnterPage(dlg, page, 0);
  }

  return GWEN_DialogEvent_ResultHandled;
}



int _handleSignalActivatedBankCode(GWEN_DIALOG *dlg)
{
  AH_PINTAN_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  dlg2=AB_SelectBankInfoDialog_new(xdlg->banking, "de", NULL);
  if (dlg2==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create dialog");
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
      AB_BANKINFO_SERVICE *sv;

      s=AB_BankInfo_GetBankId(bi);
      GWEN_Dialog_SetCharProperty(dlg,
                                  "wiz_bankcode_edit",
                                  GWEN_DialogProperty_Value,
                                  0,
                                  (s && *s)?s:"",
                                  0);

      s=AB_BankInfo_GetBankName(bi);
      GWEN_Dialog_SetCharProperty(dlg,
                                  "wiz_bankname_edit",
                                  GWEN_DialogProperty_Value,
                                  0,
                                  (s && *s)?s:"",
                                  0);
      sv=AB_BankInfoService_List_First(AB_BankInfo_GetServices(bi));
      while (sv) {
        const char *s;

        s=AB_BankInfoService_GetType(sv);
        if (s && *s && strcasecmp(s, "HBCI")==0) {
          s=AB_BankInfoService_GetMode(sv);
          if (s && *s && strcasecmp(s, "PINTAN")==0)
            break;
        }
        sv=AB_BankInfoService_List_Next(sv);
      }

      if (sv) {
        /* PIN/TAN service found */
        s=AB_BankInfoService_GetAddress(sv);
        GWEN_Dialog_SetCharProperty(dlg,
                                    "wiz_url_edit",
                                    GWEN_DialogProperty_Value,
                                    0,
                                    (s && *s)?s:"",
                                    0);
        s=AB_BankInfoService_GetPversion(sv);
        if (s && *s) {
          if (strcasecmp(s, "2.01")==0 ||
              strcasecmp(s, "2")==0)
            xdlg->hbciVersion=201;
          else if (strcasecmp(s, "2.10")==0 ||
                   strcasecmp(s, "2.1")==0)
            xdlg->hbciVersion=210;
          else if (strcasecmp(s, "2.20")==0 ||
                   strcasecmp(s, "2.2")==0)
            xdlg->hbciVersion=220;
          else if (strcasecmp(s, "3.00")==0 ||
                   strcasecmp(s, "3.0")==0 ||
                   strcasecmp(s, "3")==0)
            xdlg->hbciVersion=300;
          else if (strcasecmp(s, "4.00")==0 ||
                   strcasecmp(s, "4.0")==0 ||
                   strcasecmp(s, "4")==0)
            xdlg->hbciVersion=400;
        }
      }
    }
  }

  GWEN_Dialog_free(dlg2);

  if (AH_PinTanDialog_GetBankPageData(dlg)<0)
    GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
  else
    GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);

  return GWEN_DialogEvent_ResultHandled;
}



int _handleSignalActivatedSpecial(GWEN_DIALOG *dlg)
{
  AH_PINTAN_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  dlg2=AH_PinTanSpecialDialog_new(xdlg->provider);
  if (dlg2==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create dialog");
    return GWEN_DialogEvent_ResultHandled;
  }

  AH_PinTanSpecialDialog_SetHttpVersion(dlg2, xdlg->httpVMajor, xdlg->httpVMinor);
  AH_PinTanSpecialDialog_SetHbciVersion(dlg2, xdlg->hbciVersion);
  AH_PinTanSpecialDialog_SetFlags(dlg2, xdlg->flags);
  AH_PinTanSpecialDialog_SetTanMediumId(dlg2, xdlg->tanMediumId);

  rv=GWEN_Gui_ExecDialog(dlg2, 0);
  if (rv==0) {
    /* rejected */
    GWEN_Dialog_free(dlg2);
    return GWEN_DialogEvent_ResultHandled;
  }
  else {
    xdlg->httpVMajor=AH_PinTanSpecialDialog_GetHttpVMajor(dlg2);
    xdlg->httpVMinor=AH_PinTanSpecialDialog_GetHttpVMinor(dlg2);
    xdlg->hbciVersion=AH_PinTanSpecialDialog_GetHbciVersion(dlg2);
    xdlg->flags=AH_PinTanSpecialDialog_GetFlags(dlg2);
    _setTanMediumId(dlg, AH_PinTanSpecialDialog_GetTanMediumId(dlg2));
  }

  GWEN_Dialog_free(dlg2);

  return GWEN_DialogEvent_ResultHandled;
}



int _handleSignalActivated(GWEN_DIALOG *dlg, const char *sender)
{
  DBG_NOTICE(0, "Activated: %s", sender);
  if (strcasecmp(sender, "wiz_bankcode_button")==0)
    return _handleSignalActivatedBankCode(dlg);
  else if (strcasecmp(sender, "wiz_prev_button")==0)
    return _dialogPrevious(dlg);
  else if (strcasecmp(sender, "wiz_next_button")==0)
    return _dialogNext(dlg);
  else if (strcasecmp(sender, "wiz_abort_button")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "wiz_special_button")==0)
    return _handleSignalActivatedSpecial(dlg);
  else if (strcasecmp(sender, "wiz_help_button")==0) {
    /* TODO: open a help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int _handleSignalValueChanged(GWEN_DIALOG *dlg, const char *sender)
{
  if (strcasecmp(sender, "wiz_bankcode_edit")==0 ||
      strcasecmp(sender, "wiz_url_edit")==0 ||
      strcasecmp(sender, "wiz_username_edit")==0 ||
      strcasecmp(sender, "wiz_userid_edit")==0 ||
      strcasecmp(sender, "wiz_customerid_edit")==0) {
    int rv;

    if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_BANK) {
      rv=AH_PinTanDialog_GetBankPageData(dlg);
      if (rv<0)
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    else if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_USER) {
      rv=AH_PinTanDialog_GetUserPageData(dlg);
      if (rv<0)
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    return GWEN_DialogEvent_ResultHandled;
  }
  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB _dlgApi_SignalHandler(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  switch (t) {
  case GWEN_DialogEvent_TypeInit:
    _dialogInit(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    _dialogFini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return _handleSignalValueChanged(dlg, sender);

  case GWEN_DialogEvent_TypeActivated:
    return _handleSignalActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
  default:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int _selectTanMethod(GWEN_DIALOG *dlg, AB_USER *u, int doLock)
{
  AH_PINTAN_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  dlg2=AH_PinTan_TanModeDialog_new(xdlg->provider, u, doLock);
  if (dlg2==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create dialog");
    return GWEN_DialogEvent_ResultHandled;
  }

  rv=GWEN_Gui_ExecDialog(dlg2, 0);
  if (rv==0) {
    /* rejected */
    GWEN_Dialog_free(dlg2);
    return GWEN_ERROR_USER_ABORTED;
  }

  GWEN_Dialog_free(dlg2);

  return 0;
}



void _removeAllSpaces(uint8_t *s)
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




