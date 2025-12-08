/***************************************************************************
 begin       : Tue Sep 17 2019
 copyright   : (C) 2025 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_pintan_tanmode_p.h"
#include "w_tanmethodcombo.h"

#include "aqbanking/i18n_l.h"
#include <aqbanking/banking_be.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>



/* ------------------------------------------------------------------------------------------------
 * defines
 * ------------------------------------------------------------------------------------------------
 */

#define DIALOG_MINWIDTH     200
#define DIALOG_MINHEIGHT    100

#define DLG_DIALOGFILE      "aqbanking/backends/aqhbci/dialogs/dlg_pintan_tanmode.dlg"

/* IDs for dialog widgets */
#define ID_SELF             ""
#define ID_MESSAGE_LABEL    "messageLabel"
#define ID_TANMETHOD_COMBO  "tanMethodCombo"
#define ID_HELP_BUTTON      "helpButton"
#define ID_OK_BUTTON        "okButton"
#define ID_ABORT_BUTTON     "abortButton"



/* ------------------------------------------------------------------------------------------------
 * types
 * ------------------------------------------------------------------------------------------------
 */

typedef int (*_DIALOG_SIGNAL_HANDLER_FN)(GWEN_DIALOG *dlg);
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

static int _handleInit(GWEN_DIALOG *dlg);
static int _handleFini(GWEN_DIALOG *dlg);

static int _fromGui(GWEN_DIALOG *dlg, AB_USER *u, int quiet);
static int _handleActivatedOk(GWEN_DIALOG *dlg);
static int _handleActivatedAbort(GWEN_DIALOG *dlg);

static void GWENHYWFAR_CB _freeData(void *bp, void *p);
static int GWENHYWFAR_CB _signalHandler(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);



/* ------------------------------------------------------------------------------------------------
 * static vars
 * ------------------------------------------------------------------------------------------------
 */

GWEN_INHERIT(GWEN_DIALOG, AH_PINTAN_TANMODE_DIALOG)


static _DIALOG_SIGNAL_ENTRY _signalMap[]={
  {NULL,                   GWEN_DialogEvent_TypeInit,         _handleInit},
  {NULL,                   GWEN_DialogEvent_TypeFini,         _handleFini},

  {ID_OK_BUTTON,           GWEN_DialogEvent_TypeActivated,    _handleActivatedOk},
  {ID_ABORT_BUTTON,        GWEN_DialogEvent_TypeActivated,    _handleActivatedAbort},

  {NULL, 0, NULL}
};



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

GWEN_DIALOG *AH_PinTan_TanModeDialog_new(AB_PROVIDER *pro, AB_USER *u, int doLock)
{
  GWEN_DIALOG *dlg;
  AH_PINTAN_TANMODE_DIALOG *xdlg;

  dlg=GWEN_Dialog_CreateAndLoadWithPath("ah_setup_pintan_tanmode", AB_PM_LIBNAME, AB_PM_DATADIR, DLG_DIALOGFILE);
  if (dlg==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return NULL;
  }
  GWEN_NEW_OBJECT(AH_PINTAN_TANMODE_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AH_PINTAN_TANMODE_DIALOG, dlg, xdlg, _freeData);
  GWEN_Dialog_SetSignalHandler(dlg, _signalHandler);

  xdlg->banking=AB_Provider_GetBanking(pro);
  xdlg->provider=pro;
  xdlg->user=u;

  /* preset */
  xdlg->doLock=doLock;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB _freeData(void *bp, void *p)
{
  AH_PINTAN_TANMODE_DIALOG *xdlg;

  xdlg=(AH_PINTAN_TANMODE_DIALOG *) p;
  GWEN_FREE_OBJECT(xdlg);
}



int _handleInit(GWEN_DIALOG *dlg)
{
  AH_PINTAN_TANMODE_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_TANMODE_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg, ID_SELF, GWEN_DialogProperty_Title, 0, I18N("Select TAN Mode"), 0);

  GWEN_Dialog_SetCharProperty(dlg,
                              ID_MESSAGE_LABEL,
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("<html>"
                                   "<p>Please select the TAN method to use for authentication purposes.</p>"
                                   "<p>You should choose a method with a version of 6 or higher, "
                                   "otherwise \"Strong Customer Authentication\" is disabled and connecting to most "
                                   "banks is not possible.</p>"
                                   "</html>"
                                   "Please select the TAN method to use for authentication purposes.\n"
                                   "You should choose a method with a version of 6 or higher, "
                                   "otherwise \"Strong Customer Authentication\" is disabled and connecting to most "
                                   "banks is not possible."),
                              0);


  AH_Widget_TanMethodComboRebuild(dlg, ID_TANMETHOD_COMBO, AH_User_GetTanMethodDescriptions(xdlg->user));
  AH_Widget_TanMethodComboSetCurrent(dlg, ID_TANMETHOD_COMBO, AH_User_GetSelectedTanMethod(xdlg->user));

  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, ID_SELF, GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, ID_SELF, GWEN_DialogProperty_Height, 0, i, 0);
  return GWEN_DialogEvent_ResultHandled;
}



int _handleFini(GWEN_DIALOG *dlg)
{
  AH_PINTAN_TANMODE_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_TANMODE_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);


  /* store dialog width */
  i=GWEN_Dialog_GetIntProperty(dlg, ID_SELF, GWEN_DialogProperty_Width, 0, -1);
  GWEN_DB_SetIntValue(dbPrefs, GWEN_DB_FLAGS_OVERWRITE_VARS, "dialog_width", i);

  /* store dialog height */
  i=GWEN_Dialog_GetIntProperty(dlg, ID_SELF, GWEN_DialogProperty_Height, 0, -1);
  GWEN_DB_SetIntValue(dbPrefs, GWEN_DB_FLAGS_OVERWRITE_VARS, "dialog_height", i);

  return GWEN_DialogEvent_ResultHandled;
}



int _fromGui(GWEN_DIALOG *dlg, AB_USER *u, int quiet)
{
  int i;

  i=AH_Widget_TanMethodComboGetCurrent(dlg, ID_TANMETHOD_COMBO);
  if (u && i>0)
    AH_User_SetSelectedTanMethod(u, i);

  return 0;
}



int GWENHYWFAR_CB _signalHandler(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  const _DIALOG_SIGNAL_ENTRY *entry;

  entry=_signalMap;
  while(entry->handlerFn) {
    if (entry->eventType==t && (entry->sender==NULL || (sender && strcasecmp(sender, entry->sender)==0))) {
      return entry->handlerFn(dlg);
    }
    entry++;
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int _handleActivatedOk(GWEN_DIALOG *dlg)
{
  AH_PINTAN_TANMODE_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_TANMODE_DIALOG, dlg);
  assert(xdlg);

  rv=_fromGui(dlg, NULL, 0);
  if (rv<0) {
    return GWEN_DialogEvent_ResultHandled;
  }

  if (xdlg->doLock) {
    int rv;

    rv=AB_Provider_BeginExclUseUser(xdlg->provider, xdlg->user);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Unable to lock user. Maybe already in use?"));
      return GWEN_DialogEvent_ResultHandled;
    }
  }

  _fromGui(dlg, xdlg->user, 1);

  if (xdlg->doLock) {
    int rv;

    rv=AB_Provider_EndExclUseUser(xdlg->provider, xdlg->user, 0);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Unable to unlock user. Maybe already in use?"));
      AB_Provider_EndExclUseUser(xdlg->provider, xdlg->user, 1);
      return GWEN_DialogEvent_ResultHandled;
    }
  }
  return GWEN_DialogEvent_ResultAccept;
}



int _handleActivatedAbort(GWEN_DIALOG *dlg)
{
  return GWEN_DialogEvent_ResultReject;
}



