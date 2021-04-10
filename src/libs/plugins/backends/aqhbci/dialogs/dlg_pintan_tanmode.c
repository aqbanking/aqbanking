/***************************************************************************
 begin       : Tue Sep 17 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_pintan_tanmode_p.h"

#include "aqbanking/i18n_l.h"
#include <aqbanking/backendsupport/user.h>
#include <aqbanking/banking_be.h>

#include "aqhbci/banking/user.h"
#include "aqhbci/banking/provider.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>


#define DIALOG_MINWIDTH  200
#define DIALOG_MINHEIGHT 100



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void _init(GWEN_DIALOG *dlg);
static void _fini(GWEN_DIALOG *dlg);

static int _fromGui(GWEN_DIALOG *dlg, AB_USER *u, int quiet);
static int _handleActivated(GWEN_DIALOG *dlg, const char *sender);
static int _handleActivatedOk(GWEN_DIALOG *dlg, const char *sender);

static void GWENHYWFAR_CB _freeData(void *bp, void *p);
static int GWENHYWFAR_CB _signalHandler(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static const AH_TAN_METHOD *_getCurrentTanMethod(GWEN_DIALOG *dlg);
static void _updateTanMethods(GWEN_DIALOG *dlg);
static int _createTanMethodString(const AH_TAN_METHOD *tm, GWEN_BUFFER *tbuf);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */




GWEN_INHERIT(GWEN_DIALOG, AH_PINTAN_TANMODE_DIALOG)




GWEN_DIALOG *AH_PinTan_TanModeDialog_new(AB_PROVIDER *pro, AB_USER *u, int doLock)
{
  GWEN_DIALOG *dlg;
  AH_PINTAN_TANMODE_DIALOG *xdlg;

  dlg=GWEN_Dialog_CreateAndLoadWithPath("ah_setup_pintan_tanmode",
                                        AB_PM_LIBNAME, AB_PM_DATADIR,
                                        "aqbanking/backends/aqhbci/dialogs/dlg_pintan_tanmode.dlg");
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



void _init(GWEN_DIALOG *dlg)
{
  AH_PINTAN_TANMODE_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_TANMODE_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
                              "",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("Select TAN Mode"),
                              0);

  GWEN_Dialog_SetCharProperty(dlg,
                              "messageLabel",
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


  /* also selects currently selected TAN method */
  _updateTanMethods(dlg);


  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);
}



void _fini(GWEN_DIALOG *dlg)
{
  AH_PINTAN_TANMODE_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_TANMODE_DIALOG, dlg);
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



int _fromGui(GWEN_DIALOG *dlg, AB_USER *u, int quiet)
{
  AH_PINTAN_TANMODE_DIALOG *xdlg;
  const AH_TAN_METHOD *tm;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_TANMODE_DIALOG, dlg);
  assert(xdlg);

  tm=_getCurrentTanMethod(dlg);
  if (tm) {
    int fn;

    fn=(AH_TanMethod_GetGvVersion(tm)*1000)+AH_TanMethod_GetFunction(tm);
    AH_User_SetSelectedTanMethod(xdlg->user, fn);
  }

  return 0;
}



int GWENHYWFAR_CB _signalHandler(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  AH_PINTAN_TANMODE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_TANMODE_DIALOG, dlg);
  assert(xdlg);

  switch (t) {
  case GWEN_DialogEvent_TypeInit:
    _init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    _fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeActivated:
    return _handleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int _handleActivatedOk(GWEN_DIALOG *dlg, const char *sender)
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

  _fromGui(dlg, xdlg->user, 1);

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



int _handleActivated(GWEN_DIALOG *dlg, const char *sender)
{
  DBG_NOTICE(0, "Activated: %s", sender);
  if (strcasecmp(sender, "okButton")==0)
    return _handleActivatedOk(dlg, sender);
  else if (strcasecmp(sender, "abortButton")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "helpButton")==0) {
    /* TODO: open a help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



const AH_TAN_METHOD *_getCurrentTanMethod(GWEN_DIALOG *dlg)
{
  AH_PINTAN_TANMODE_DIALOG *xdlg;
  int idx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_TANMODE_DIALOG, dlg);
  assert(xdlg);

  idx=GWEN_Dialog_GetIntProperty(dlg, "tanMethodCombo", GWEN_DialogProperty_Value, 0, -1);
  if (idx>=0) {
    const char *currentText;

    currentText=GWEN_Dialog_GetCharProperty(dlg, "tanMethodCombo", GWEN_DialogProperty_Value, idx, NULL);
    if (currentText && *currentText && xdlg->tanMethodList) {
      AH_TAN_METHOD *tm;
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      tm=AH_TanMethod_List_First(xdlg->tanMethodList);
      while (tm) {
        if (_createTanMethodString(tm, tbuf)==0 &&
            strcasecmp(GWEN_Buffer_GetStart(tbuf), currentText)==0) {
          GWEN_Buffer_free(tbuf);
          return tm;
        }
        GWEN_Buffer_Reset(tbuf);

        tm=AH_TanMethod_List_Next(tm);
      }
      GWEN_Buffer_free(tbuf);
    }
  }

  return NULL;
}



void _updateTanMethods(GWEN_DIALOG *dlg)
{
  AH_PINTAN_TANMODE_DIALOG *xdlg;
  const AH_TAN_METHOD_LIST *ctl;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_TANMODE_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->tanMethodList) {
    AH_TanMethod_List_free(xdlg->tanMethodList);
    xdlg->tanMethodList=NULL;
  }
  ctl=AH_User_GetTanMethodDescriptions(xdlg->user);
  if (ctl)
    xdlg->tanMethodList=AH_TanMethod_List_dup(ctl);

  /* setup tanmethod combo */
  GWEN_Dialog_SetIntProperty(dlg, "tanMethodCombo", GWEN_DialogProperty_ClearValues, 0, 0, 0);
  GWEN_Dialog_SetCharProperty(dlg, "tanMethodCombo", GWEN_DialogProperty_AddValue, 0, I18N("-- select --"), 0);
  if (xdlg->tanMethodList) {
    AH_TAN_METHOD *tm;
    GWEN_BUFFER *tbuf;
    int i;
    int idx;
    int selectedMethod;
    int tjv;
    int tfn;

    selectedMethod=AH_User_GetSelectedTanMethod(xdlg->user);
    tjv=selectedMethod / 1000;
    tfn=selectedMethod % 1000;
    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    idx=-1;
    i=1;
    tm=AH_TanMethod_List_First(xdlg->tanMethodList);
    while (tm) {
      if (_createTanMethodString(tm, tbuf)==0) {
        if (AH_TanMethod_GetFunction(tm)==tfn && AH_TanMethod_GetGvVersion(tm)==tjv)
          idx=i;
        GWEN_Dialog_SetCharProperty(dlg, "tanMethodCombo", GWEN_DialogProperty_AddValue, 0, GWEN_Buffer_GetStart(tbuf), 0);
        i++;
      }
      GWEN_Buffer_Reset(tbuf);

      tm=AH_TanMethod_List_Next(tm);
    }
    GWEN_Buffer_free(tbuf);
    if (idx>=0)
      /* chooses selected entry in combo box */
      GWEN_Dialog_SetIntProperty(dlg, "tanMethodCombo", GWEN_DialogProperty_Value, 0, idx, 0);
  }
}



int _createTanMethodString(const AH_TAN_METHOD *tm, GWEN_BUFFER *tbuf)
{
  const char *s;
  char numbuf[32];

  snprintf(numbuf, sizeof(numbuf)-1, "%d", AH_TanMethod_GetFunction(tm));
  numbuf[sizeof(numbuf)-1]=0;
  GWEN_Buffer_AppendString(tbuf, numbuf);

  s=AH_TanMethod_GetMethodName(tm);
  if (!(s && *s))
    s=AH_TanMethod_GetMethodId(tm);
  if (s && *s) {
    GWEN_Buffer_AppendString(tbuf, " - ");
    GWEN_Buffer_AppendString(tbuf, s);
  }

  /* add HKTAN version */
  GWEN_Buffer_AppendString(tbuf, " (Version ");
  snprintf(numbuf, sizeof(numbuf)-1, "%d", AH_TanMethod_GetGvVersion(tm));
  numbuf[sizeof(numbuf)-1]=0;
  GWEN_Buffer_AppendString(tbuf, numbuf);
  GWEN_Buffer_AppendString(tbuf, ")");

  return 0;
}



