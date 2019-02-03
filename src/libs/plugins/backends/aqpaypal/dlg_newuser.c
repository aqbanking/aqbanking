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



#include "dlg_newuser_p.h"
#include "i18n_l.h"

#include <aqbanking/user.h>
#include <aqbanking/banking_be.h>

#include <aqpaypal/user.h>
#include <aqpaypal/provider.h>
//#include <aqpaypal/dlg_newuser_special.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>


#define PAGE_BEGIN     0
#define PAGE_USER      1
#define PAGE_SECRET    2
#define PAGE_CREATE    3
#define PAGE_END       4


#define DIALOG_MINWIDTH  400
#define DIALOG_MINHEIGHT 200



GWEN_INHERIT(GWEN_DIALOG, APY_NEWUSER_DIALOG)




GWEN_DIALOG *APY_NewUserDialog_new(AB_PROVIDER *pro)
{
  GWEN_DIALOG *dlg;
  APY_NEWUSER_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("apy_newuser");
  GWEN_NEW_OBJECT(APY_NEWUSER_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg, xdlg, APY_NewUserDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, APY_NewUserDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
                               "aqbanking/backends/aqpaypal/dialogs/dlg_newuser.dlg",
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

  xdlg->provider=pro;
  xdlg->banking=AB_Provider_GetBanking(pro);

  /* preset */
  xdlg->httpVMajor=1;
  xdlg->httpVMinor=1;

  xdlg->url=strdup("https://api-3t.paypal.com/nvp");

  /* done */
  return dlg;
}



void GWENHYWFAR_CB APY_NewUserDialog_FreeData(void *bp, void *p)
{
  APY_NEWUSER_DIALOG *xdlg;

  xdlg=(APY_NEWUSER_DIALOG *) p;
  free(xdlg->apiUserId);
  free(xdlg->apiPassword);
  free(xdlg->apiSignature);
  free(xdlg->userName);
  free(xdlg->userId);
  free(xdlg->url);
  AB_User_free(xdlg->user);
  GWEN_FREE_OBJECT(xdlg);
}



AB_USER *APY_NewUserDialog_GetUser(const GWEN_DIALOG *dlg)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->user;
}



const char *APY_NewUserDialog_GetApiUserId(const GWEN_DIALOG *dlg)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->apiUserId;
}



void APY_NewUserDialog_SetApiUserId(GWEN_DIALOG *dlg, const char *s)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->apiUserId);
  if (s)
    xdlg->apiUserId=strdup(s);
  else
    xdlg->apiUserId=NULL;
}



const char *APY_NewUserDialog_GetApiPassword(const GWEN_DIALOG *dlg)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->apiPassword;
}



void APY_NewUserDialog_SetApiPassword(GWEN_DIALOG *dlg, const char *s)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->apiPassword);
  if (s)
    xdlg->apiPassword=strdup(s);
  else
    xdlg->apiPassword=NULL;
}



const char *APY_NewUserDialog_GetApiSignature(const GWEN_DIALOG *dlg)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->apiSignature;
}



void APY_NewUserDialog_SetApiSignature(GWEN_DIALOG *dlg, const char *s)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->apiSignature);
  if (s)
    xdlg->apiSignature=strdup(s);
  else
    xdlg->apiSignature=NULL;
}



const char *APY_NewUserDialog_GetUserName(const GWEN_DIALOG *dlg)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->userName;
}



void APY_NewUserDialog_SetUserName(GWEN_DIALOG *dlg, const char *s)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userName);
  if (s)
    xdlg->userName=strdup(s);
  else
    xdlg->userName=NULL;
}



const char *APY_NewUserDialog_GetUserId(const GWEN_DIALOG *dlg)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->userId;
}



void APY_NewUserDialog_SetUserId(GWEN_DIALOG *dlg, const char *s)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userId);
  if (s)
    xdlg->userId=strdup(s);
  else
    xdlg->userId=NULL;
}



const char *APY_NewUserDialog_GetUrl(const GWEN_DIALOG *dlg)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->url;
}



void APY_NewUserDialog_SetUrl(GWEN_DIALOG *dlg, const char *s)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->url);
  if (s)
    xdlg->url=strdup(s);
  else
    xdlg->url=NULL;
}



int APY_NewUserDialog_GetHttpVMajor(const GWEN_DIALOG *dlg)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->httpVMajor;
}



int APY_NewUserDialog_GetHttpVMinor(const GWEN_DIALOG *dlg)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->httpVMinor;
}



void APY_NewUserDialog_SetHttpVersion(GWEN_DIALOG *dlg, int vmajor, int vminor)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  xdlg->httpVMajor=vmajor;
  xdlg->httpVMinor=vminor;
}



uint32_t APY_NewUserDialog_GetFlags(const GWEN_DIALOG *dlg)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->flags;
}



void APY_NewUserDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags=fl;
}



void APY_NewUserDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}



void APY_NewUserDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}






void APY_NewUserDialog_Init(GWEN_DIALOG *dlg)
{
  APY_NEWUSER_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
                              "",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("Paypal Setup Wizard"),
                              0);

  /* select first page */
  GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, 0, 0);

  /* setup intro page */
  GWEN_Dialog_SetCharProperty(dlg,
                              "wiz_begin_label",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("<html>"
                                   "<p>This dialog assists you in setting up a Paypal User.</p>"
                                   "<p>Please note that you have to apply for API access with Paypal. "
                                   "The following procedure helps you getting there:</p>"
                                   "<p>Login into your Paypal account via web browser, enter the <i>My Profile</i> "
                                   "page, click  <i>API access</i> under <i>Account information</i>.</p>"
                                   "<p>Choose <b>Option 2</b>.</p>"
                                   "</html>"
                                   "This dialog assists you in setting up a Paypal User.\n"
                                   "Please note that you have to apply for API access with Paypal.\n"
                                   "The following procedure helps you getting there:\n"
                                   "Login into your Paypal account via web browser, enter the \"My Profile\"\n"
                                   "page, click  \"API access\" under \"Account information\".\n"
                                   "Choose OPTION 2."
                                  ),
                              0);

  /* setup user page */
  GWEN_Dialog_SetCharProperty(dlg,
                              "wiz_user_label",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("<html>"
                                   "<p>You can find the information needed here after logging into your "
                                   "Paypal account via web browser. The information can then be found "
                                   "under <i>My Profile</i>, <i>Account Information</i>, <i>API Access</i>.</p>"
                                   "</html>"
                                   "You can find the information needed here after logging into your\n"
                                   "Paypal account via web browser. The information can then be found\n"
                                   "under <\"My Profile\", \"Account Information\", \"API Access\"."
                                  ),
                              0);

  GWEN_Dialog_SetCharProperty(dlg,
                              "wiz_url_edit",
                              GWEN_DialogProperty_Value,
                              0,
                              "https://api-3t.paypal.com/nvp",
                              0);


  /* setup secret page */
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

  /* setup creation page */
  GWEN_Dialog_SetCharProperty(dlg,
                              "wiz_create_label",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("<html>"
                                   "<p>We are now ready to create the user.</p>"
                                   "<p>Click the <i>next</i> button to proceed or <i>abort</i> to abort.</p>"
                                   "<p>If you proceed you will be asked to enter a new password. This is the password "
                                   "for the credentials file described in previous steps.</p>"
                                   "<p>Please be carefull to enter a sufficiently secure password</p>"
                                   "</html>"
                                   "We are now ready to create the user.\n"
                                   "Click the \"next\" button to proceed or \"abort\" to abort.\n"
                                   "If you proceed you will be asked to enter a new password. This is the password\n"
                                   "for the credentials file described in previous steps.\n"
                                   "Please be carefull to enter a sufficiently secure password."
                                  ),
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



void APY_NewUserDialog_Fini(GWEN_DIALOG *dlg)
{
  APY_NEWUSER_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
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



int APY_NewUserDialog_GetSecretPageData(GWEN_DIALOG *dlg)
{
  APY_NEWUSER_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_apiuserid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    APY_NewUserDialog_SetApiUserId(dlg, s);
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "Missing API User ID");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_apipass_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    APY_NewUserDialog_SetApiPassword(dlg, s);
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "Missing API Password");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_apisig_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    APY_NewUserDialog_SetApiSignature(dlg, s);
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "Missing API Signature");
    return GWEN_ERROR_NO_DATA;
  }

  return 0;
}



int APY_NewUserDialog_GetUserPageData(GWEN_DIALOG *dlg)
{
  APY_NEWUSER_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_username_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    APY_NewUserDialog_SetUserName(dlg, s);
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "Missing user name");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_userid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    APY_NewUserDialog_SetUserId(dlg, s);
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "Missing user id");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_url_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    APY_NewUserDialog_SetUrl(dlg, s);
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "Missing URL");
    return GWEN_ERROR_NO_DATA;
  }

  return 0;
}



int APY_NewUserDialog_EnterPage(GWEN_DIALOG *dlg, int page, int forwards)
{
  APY_NEWUSER_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  switch (page) {
  case PAGE_BEGIN:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_USER:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    rv=APY_NewUserDialog_GetUserPageData(dlg);
    if (rv<0)
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_SECRET:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    rv=APY_NewUserDialog_GetSecretPageData(dlg);
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



int APY_NewUserDialog_DoIt(GWEN_DIALOG *dlg)
{
  APY_NEWUSER_DIALOG *xdlg;
  AB_USER *u;
  int rv;
  uint32_t pid;

  DBG_INFO(0, "Doit");
  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  DBG_INFO(0, "Creating user");
  u=AB_Provider_CreateUserObject(xdlg->provider);
  if (u==NULL) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Could not create user, maybe backend missing?");
    // TODO: show error message
    return GWEN_DialogEvent_ResultHandled;
  }

  /* generic setup */
  AB_User_SetUserName(u, xdlg->userName);
  AB_User_SetUserId(u, xdlg->userId);
  AB_User_SetCustomerId(u, xdlg->userId);
  AB_User_SetCountry(u, "de");
  AB_User_SetBankCode(u, "PAYPAL");

  APY_User_SetServerUrl(u, xdlg->url);
  APY_User_SetHttpVMajor(u, xdlg->httpVMajor);
  APY_User_SetHttpVMinor(u, xdlg->httpVMinor);

  DBG_INFO(0, "Adding user");
  rv=AB_Provider_AddUser(xdlg->provider, u);
  if (rv<0) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Could not add user (%d)", rv);
    AB_User_free(u);
    return GWEN_DialogEvent_ResultHandled;
  }

  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
                             GWEN_GUI_PROGRESS_ALLOW_EMBED |
                             GWEN_GUI_PROGRESS_SHOW_PROGRESS |
                             GWEN_GUI_PROGRESS_SHOW_ABORT,
                             I18N("Setting Up Paypal User"),
                             I18N("The user will be created."),
                             3,
                             0);
  /* lock new user */
  DBG_INFO(0, "Locking user");
  rv=AB_Provider_BeginExclUseUser(xdlg->provider, u);
  if (rv<0) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Could not lock user (%d)", rv);
    GWEN_Gui_ProgressLog(pid,
                         GWEN_LoggerLevel_Error,
                         I18N("Unable to lock users"));
    AB_Provider_DeleteUser(xdlg->provider, AB_User_GetUniqueId(u));
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }

#if 0
  DBG_INFO(0, "Getting certs (%08x)", AH_User_GetFlags(u));
  GWEN_Gui_ProgressLog(pid,
                       GWEN_LoggerLevel_Notice,
                       I18N("Retrieving SSL certificate"));
  rv=APY_Provider_GetCert(pro, u, 0, 1, 0);
  if (rv<0) {
    // TODO: retry with SSLv3 if necessary
    AB_Banking_EndExclUseUser(xdlg->banking, u, 1);
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    AB_Banking_DeleteUser(xdlg->banking, u);
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }

  rv=GWEN_Gui_ProgressAdvance(pid, GWEN_GUI_PROGRESS_ONE);
  if (rv==GWEN_ERROR_USER_ABORTED) {
    AB_Banking_EndExclUseUser(xdlg->banking, u, 1);
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    AB_Banking_DeleteUser(xdlg->banking, u);
    GWEN_Gui_ProgressLog(pid,
                         GWEN_LoggerLevel_Error,
                         I18N("Aborted by user."));
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }
#endif

  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Notice, I18N("Creating API credentials file"));
  rv=APY_User_SetApiSecrets(u, xdlg->apiPassword, xdlg->apiSignature, xdlg->apiUserId);
  if (rv<0) {
    AB_Provider_EndExclUseUser(xdlg->provider, u, 1);
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    AB_Provider_DeleteUser(xdlg->provider, AB_User_GetUniqueId(u));
    GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Error, I18N("Aborted by user."));
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }


  /* unlock user */
  DBG_INFO(0, "Unlocking user");
  rv=AB_Provider_EndExclUseUser(xdlg->provider, u, 0);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN,
             "Could not unlock user [%s] (%d)",
             AB_User_GetUserId(u), rv);
    GWEN_Gui_ProgressLog2(pid,
                          GWEN_LoggerLevel_Error,
                          I18N("Could not unlock user %s (%d)"),
                          AB_User_GetUserId(u), rv);
    AB_Provider_EndExclUseUser(xdlg->provider, u, 1);
    AB_Provider_DeleteUser(xdlg->provider, AB_User_GetUniqueId(u));
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }

  if (1) {
    AB_ACCOUNT *account;
    int rv;
    static char accountname[256];

    account=AB_Provider_CreateAccountObject(xdlg->provider);
    assert(account);
#if 0
    AB_User_SetUserName(u, xdlg->userName);
    AB_User_SetUserId(u, xdlg->userId);
#endif
    AB_Account_SetOwnerName(account, AB_User_GetUserName(u));
    AB_Account_SetAccountNumber(account, AB_User_GetUserId(u));
    AB_Account_SetBankCode(account, "PAYPAL");
    AB_Account_SetBankName(account, "PAYPAL");
    strcpy(accountname, "PP ");
    strcat(accountname, AB_User_GetUserName(u));
    AB_Account_SetAccountName(account, accountname);
    AB_Account_SetUserId(account, AB_User_GetUniqueId(u));

    rv=AB_Provider_AddAccount(xdlg->provider, account, 1); /* do lock corresponding user */
    if (rv<0) {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "Error adding account (%d)", rv);
      AB_Account_free(account);
      AB_Provider_DeleteUser(xdlg->provider, AB_User_GetUniqueId(u));
      GWEN_Gui_ProgressEnd(pid);
      return GWEN_DialogEvent_ResultHandled;
    }
  }

  GWEN_Dialog_SetCharProperty(dlg,
                              "wiz_end_label",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("The user has been successfully setup."),
                              0);
  GWEN_Gui_ProgressEnd(pid);
  APY_NewUserDialog_EnterPage(dlg, PAGE_END, 1);

  xdlg->user=u;

  return GWEN_DialogEvent_ResultHandled;
}



int APY_NewUserDialog_Next(GWEN_DIALOG *dlg)
{
  APY_NEWUSER_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  if (page==PAGE_CREATE) {
    return APY_NewUserDialog_DoIt(dlg);
  }
  else if (page<PAGE_END) {
    page++;
    return APY_NewUserDialog_EnterPage(dlg, page, 1);
  }
  else if (page==PAGE_END)
    return GWEN_DialogEvent_ResultAccept;

  return GWEN_DialogEvent_ResultHandled;
}



int APY_NewUserDialog_Previous(GWEN_DIALOG *dlg)
{
  APY_NEWUSER_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  if (page>PAGE_BEGIN) {
    page--;
    return APY_NewUserDialog_EnterPage(dlg, page, 0);
  }

  return GWEN_DialogEvent_ResultHandled;
}



int APY_NewUserDialog_HandleActivatedSpecial(GWEN_DIALOG *dlg)
{
#if 0
  APY_NEWUSER_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  dlg2=AH_PinTanSpecialDialog_new(xdlg->banking);
  if (dlg2==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not create dialog");
    return GWEN_DialogEvent_ResultHandled;
  }

  AH_PinTanSpecialDialog_SetHttpVersion(dlg2, xdlg->httpVMajor, xdlg->httpVMinor);
  AH_PinTanSpecialDialog_SetHbciVersion(dlg2, xdlg->hbciVersion);
  AH_PinTanSpecialDialog_SetFlags(dlg2, xdlg->flags);

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
  }

  GWEN_Dialog_free(dlg2);
#endif
  return GWEN_DialogEvent_ResultHandled;
}



int APY_NewUserDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender)
{
  DBG_INFO(0, "Activated: %s", sender);
  if (strcasecmp(sender, "wiz_prev_button")==0)
    return APY_NewUserDialog_Previous(dlg);
  else if (strcasecmp(sender, "wiz_next_button")==0)
    return APY_NewUserDialog_Next(dlg);
  else if (strcasecmp(sender, "wiz_abort_button")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "wiz_special_button")==0)
    return APY_NewUserDialog_HandleActivatedSpecial(dlg);
  else if (strcasecmp(sender, "wiz_help_button")==0) {
    /* TODO: open a help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int APY_NewUserDialog_HandleValueChanged(GWEN_DIALOG *dlg, const char *sender)
{
  if (strcasecmp(sender, "wiz_username_edit")==0 ||
      strcasecmp(sender, "wiz_userid_edit")==0 ||
      strcasecmp(sender, "wiz_url_edit")==0 ||
      strcasecmp(sender, "wiz_apiuserid_edit")==0 ||
      strcasecmp(sender, "wiz_apipass_edit")==0 ||
      strcasecmp(sender, "wiz_apisig_edit")==0) {
    int rv;

    if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_USER) {
      rv=APY_NewUserDialog_GetUserPageData(dlg);
      if (rv<0)
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    else if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_SECRET) {
      rv=APY_NewUserDialog_GetSecretPageData(dlg);
      if (rv<0)
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    return GWEN_DialogEvent_ResultHandled;
  }
  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB APY_NewUserDialog_SignalHandler(GWEN_DIALOG *dlg,
                                                  GWEN_DIALOG_EVENTTYPE t,
                                                  const char *sender)
{
  APY_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  switch (t) {
  case GWEN_DialogEvent_TypeInit:
    APY_NewUserDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    APY_NewUserDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return APY_NewUserDialog_HandleValueChanged(dlg, sender);

  case GWEN_DialogEvent_TypeActivated:
    return APY_NewUserDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}




