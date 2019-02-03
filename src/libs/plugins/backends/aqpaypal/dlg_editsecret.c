/***************************************************************************
 begin       : Thu Aug 19 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_editsecret_p.h"
#include "i18n_l.h"

#include <aqbanking/user.h>
#include <aqbanking/banking_be.h>

#include <aqpaypal/user.h>
#include <aqpaypal/provider.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>


#define DIALOG_MINWIDTH  200
#define DIALOG_MINHEIGHT 100



GWEN_INHERIT(GWEN_DIALOG, APY_EDITSECRET_DIALOG)



GWEN_DIALOG *APY_EditSecretDialog_new(AB_BANKING *ab)
{
  GWEN_DIALOG *dlg;
  APY_EDITSECRET_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("apy_editsecret");
  GWEN_NEW_OBJECT(APY_EDITSECRET_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, APY_EDITSECRET_DIALOG, dlg, xdlg,
                       APY_EditSecretDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, APY_EditSecretDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
                               "aqbanking/backends/aqpaypal/dialogs/dlg_editsecret.dlg",
                               fbuf);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "Dialog description file not found (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }

  /* read dialog from dialog description file */
  rv=GWEN_Dialog_ReadXmlFile(dlg, GWEN_Buffer_GetStart(fbuf));
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }
  GWEN_Buffer_free(fbuf);

  xdlg->banking=ab;


  /* done */
  return dlg;
}


void GWENHYWFAR_CB APY_EditSecretDialog_FreeData(void *bp, void *p)
{
  APY_EDITSECRET_DIALOG *xdlg;

  xdlg=(APY_EDITSECRET_DIALOG *) p;
  free(xdlg->apiUserId);
  free(xdlg->apiPassword);
  free(xdlg->apiSignature);

  GWEN_FREE_OBJECT(xdlg);
}



void APY_EditSecretDialog_SetApiUserId(GWEN_DIALOG *dlg, const char *s)
{
  APY_EDITSECRET_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITSECRET_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->apiUserId);
  if (s)
    xdlg->apiUserId=strdup(s);
  else
    xdlg->apiUserId=NULL;
}



char *APY_EditSecretDialog_GetApiUserId(const GWEN_DIALOG *dlg)
{
  APY_EDITSECRET_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITSECRET_DIALOG, dlg);
  assert(xdlg);

  return xdlg->apiUserId;
}



void APY_EditSecretDialog_SetApiPassword(GWEN_DIALOG *dlg, const char *s)
{
  APY_EDITSECRET_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITSECRET_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->apiPassword);
  if (s)
    xdlg->apiPassword=strdup(s);
  else
    xdlg->apiPassword=NULL;
}



char *APY_EditSecretDialog_GetApiPassword(const GWEN_DIALOG *dlg)
{
  APY_EDITSECRET_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITSECRET_DIALOG, dlg);
  assert(xdlg);

  return xdlg->apiPassword;
}



void APY_EditSecretDialog_SetApiSignature(GWEN_DIALOG *dlg, const char *s)
{
  APY_EDITSECRET_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITSECRET_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->apiSignature);
  if (s)
    xdlg->apiSignature=strdup(s);
  else
    xdlg->apiSignature=NULL;
}



char *APY_EditSecretDialog_GetApiSignature(const GWEN_DIALOG *dlg)
{
  APY_EDITSECRET_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITSECRET_DIALOG, dlg);
  assert(xdlg);

  return xdlg->apiSignature;
}



void APY_EditSecretDialog_Init(GWEN_DIALOG *dlg)
{
  APY_EDITSECRET_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITSECRET_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
                              "",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("PayPal Secret Settings"),
                              0);


  GWEN_Dialog_SetCharProperty(dlg,
                              "wiz_secret_label",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("<html>"
                                   "<p>Enter the API password and signature as it is found on the "
                                   "Paypal page described in the previous steps.</p>"
                                   "<p><font color=\"red\"><b>"
                                   "The API password and API signature are extremely sensitive "
                                   "information which you must under no circumstances reveal to "
                                   "anybody!</b></font></p>"
                                   "<p>That being said, these credentials are also quite hard to "
                                   "remember, so AqBanking stores them in a file which is very well "
                                   "encrypted.</p>"
                                   "<p>When the user is created in the next step you will be asked for "
                                   "the password to be set for that credential file.</p>"
                                   "</html>"
                                   "Enter the API password and signature as it is found on the\n"
                                   "Paypal page described in the previous steps.\n"
                                   "The API password and API signature are extremely sensitive\n"
                                   "information which you must under no circumstances reveal to\n"
                                   "anybody!\n"
                                   "That being said, these credentials are also quite hard to\n"
                                   "remember, so AqBanking stores them in a file which is very well\n"
                                   "encrypted.\n"
                                   "When the user is created in the next step you will be asked for\n"
                                   "the password to be set for that credential file."
                                  ),
                              0);

  if (xdlg->apiUserId)
    GWEN_Dialog_SetCharProperty(dlg, "wiz_apiuserid_edit",
                                GWEN_DialogProperty_Value, 0,
                                xdlg->apiUserId,
                                0);

  if (xdlg->apiPassword)
    GWEN_Dialog_SetCharProperty(dlg, "wiz_apipass_edit",
                                GWEN_DialogProperty_Value, 0,
                                xdlg->apiPassword,
                                0);

  if (xdlg->apiSignature)
    GWEN_Dialog_SetCharProperty(dlg, "wiz_apisig_edit",
                                GWEN_DialogProperty_Value, 0,
                                xdlg->apiSignature,
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



void APY_EditSecretDialog_Fini(GWEN_DIALOG *dlg)
{
  APY_EDITSECRET_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITSECRET_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  /* fromGui */
  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_apiuserid_edit",
                                GWEN_DialogProperty_Value, 0, NULL);
  APY_EditSecretDialog_SetApiUserId(dlg, s);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_apipass_edit",
                                GWEN_DialogProperty_Value, 0, NULL);
  APY_EditSecretDialog_SetApiPassword(dlg, s);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_apisig_edit",
                                GWEN_DialogProperty_Value, 0, NULL);
  APY_EditSecretDialog_SetApiSignature(dlg, s);

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



int APY_EditSecretDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender)
{
  DBG_INFO(0, "Activated: %s", sender);
  if (strcasecmp(sender, "okButton")==0)
    return GWEN_DialogEvent_ResultAccept;
  else if (strcasecmp(sender, "abortButton")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "helpButton")==0) {
    /* TODO: open a help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB APY_EditSecretDialog_SignalHandler(GWEN_DIALOG *dlg,
                                                     GWEN_DIALOG_EVENTTYPE t,
                                                     const char *sender)
{
  APY_EDITSECRET_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITSECRET_DIALOG, dlg);
  assert(xdlg);

  switch (t) {
  case GWEN_DialogEvent_TypeInit:
    APY_EditSecretDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    APY_EditSecretDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeActivated:
    return APY_EditSecretDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}
