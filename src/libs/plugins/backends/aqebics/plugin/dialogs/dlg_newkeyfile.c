/***************************************************************************
 begin       : Sat Jun 26 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_newkeyfile_p.h"

#include <aqbanking/dlg_selectbankinfo.h>
#include <aqbanking/user.h>
#include <aqbanking/banking_be.h>

#include <aqebics/user.h>
#include <aqebics/provider.h>
#include "dlg_user_special_l.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/ctplugin.h>
#include <gwenhywfar/i18n.h>

#include <unistd.h>


#define PAGE_BEGIN     0
#define PAGE_FILE      1
#define PAGE_BANK      2
#define PAGE_USER      3
#define PAGE_CREATE    4
#define PAGE_END       5


#define DIALOG_MINWIDTH  400
#define DIALOG_MINHEIGHT 200


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)
#define I18N_NOOP(msg) msg
#define I18S(msg) msg



GWEN_INHERIT(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG)




GWEN_DIALOG *EBC_NewKeyFileDialog_new(AB_BANKING *ab) {
  GWEN_DIALOG *dlg;
  EBC_NEWKEYFILE_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("ebc_setup_newkeyfile");
  GWEN_NEW_OBJECT(EBC_NEWKEYFILE_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg, xdlg,
		       EBC_NewKeyFileDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, EBC_NewKeyFileDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(GWEN_PM_LIBNAME, GWEN_PM_SYSDATADIR,
			       "aqbanking/backends/aqebics/dialogs/dlg_newkeyfile.dlg",
			       fbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "Dialog description file not found (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }

  /* read dialog from dialog description file */
  rv=GWEN_Dialog_ReadXmlFile(dlg, GWEN_Buffer_GetStart(fbuf));
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d).", rv);
    GWEN_Gui_ShowError(I18N("Error"),
		       I18N("Could not read dialog description file [%s], maybe an installation error (%d)?"),
		       GWEN_Buffer_GetStart(fbuf), rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }
  GWEN_Buffer_free(fbuf);

  xdlg->banking=ab;

  /* preset */
  xdlg->ebicsVersion=strdup("H003");
  xdlg->signVersion=strdup("A005");
  xdlg->cryptVersion=strdup("E002");
  xdlg->authVersion=strdup("X002");
  xdlg->signKeySize=256;
  xdlg->cryptAndAuthKeySize=256;

  xdlg->httpVMajor=1;
  xdlg->httpVMinor=1;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB EBC_NewKeyFileDialog_FreeData(void *bp, void *p) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  xdlg=(EBC_NEWKEYFILE_DIALOG*) p;
  free(xdlg->fileName);
  free(xdlg->bankCode);
  free(xdlg->bankName);
  free(xdlg->userName);
  free(xdlg->userId);
  free(xdlg->customerId);
  free(xdlg->ebicsVersion);
  free(xdlg->signVersion);
  free(xdlg->cryptVersion);
  free(xdlg->authVersion);
  GWEN_FREE_OBJECT(xdlg);
}



AB_USER *EBC_NewKeyFileDialog_GetUser(const GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->user;
}



const char *EBC_NewKeyFileDialog_GetFileName(const GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->fileName;
}



void EBC_NewKeyFileDialog_SetFileName(GWEN_DIALOG *dlg, const char *s) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->fileName);
  if (s) xdlg->fileName=strdup(s);
  else xdlg->fileName=NULL;
}



const char *EBC_NewKeyFileDialog_GetBankCode(const GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->bankCode;
}



void EBC_NewKeyFileDialog_SetBankCode(GWEN_DIALOG *dlg, const char *s) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->bankCode);
  if (s) xdlg->bankCode=strdup(s);
  else xdlg->bankCode=NULL;
}



const char *EBC_NewKeyFileDialog_GetBankName(const GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->bankName;
}



void EBC_NewKeyFileDialog_SetBankName(GWEN_DIALOG *dlg, const char *s) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->bankName);
  if (s) xdlg->bankName=strdup(s);
  else xdlg->bankName=NULL;
}



const char *EBC_NewKeyFileDialog_GetUserName(const GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->userName;
}



void EBC_NewKeyFileDialog_SetUserName(GWEN_DIALOG *dlg, const char *s) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userName);
  if (s) xdlg->userName=strdup(s);
  else xdlg->userName=NULL;
}



const char *EBC_NewKeyFileDialog_GetUserId(const GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->userId;
}



void EBC_NewKeyFileDialog_SetUserId(GWEN_DIALOG *dlg, const char *s) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userId);
  if (s) xdlg->userId=strdup(s);
  else xdlg->userId=NULL;
}



const char *EBC_NewKeyFileDialog_GetCustomerId(const GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->customerId;
}



void EBC_NewKeyFileDialog_SetCustomerId(GWEN_DIALOG *dlg, const char *s) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->customerId);
  if (s) xdlg->customerId=strdup(s);
  else xdlg->customerId=NULL;
}



const char *EBC_NewKeyFileDialog_GetUrl(const GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->url;
}



void EBC_NewKeyFileDialog_SetUrl(GWEN_DIALOG *dlg, const char *s) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->url);
  if (s) xdlg->url=strdup(s);
  else xdlg->url=NULL;
}



const char *EBC_NewKeyFileDialog_GetEbicsVersion(const GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->ebicsVersion;
}



void EBC_NewKeyFileDialog_SetEbicsVersion(GWEN_DIALOG *dlg, const char *s) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->ebicsVersion);
  if (s) xdlg->ebicsVersion=strdup(s);
  else xdlg->ebicsVersion=NULL;
}



const char *EBC_NewKeyFileDialog_GetSignVersion(const GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->signVersion;
}



void EBC_NewKeyFileDialog_SetSignVersion(GWEN_DIALOG *dlg, const char *s) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->signVersion);
  if (s) xdlg->signVersion=strdup(s);
  else xdlg->signVersion=NULL;
}



const char *EBC_NewKeyFileDialog_GetCryptVersion(const GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->cryptVersion;
}



void EBC_NewKeyFileDialog_SetCryptVersion(GWEN_DIALOG *dlg, const char *s) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->cryptVersion);
  if (s) xdlg->cryptVersion=strdup(s);
  else xdlg->cryptVersion=NULL;
}



const char *EBC_NewKeyFileDialog_GetAuthVersion(const GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->authVersion;
}



void EBC_NewKeyFileDialog_SetAuthVersion(GWEN_DIALOG *dlg, const char *s) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->authVersion);
  if (s) xdlg->authVersion=strdup(s);
  else xdlg->authVersion=NULL;
}



const char *EBC_NewKeyFileDialog_GetHostId(const GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->hostId;
}



void EBC_NewKeyFileDialog_SetHostId(GWEN_DIALOG *dlg, const char *s) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->hostId);
  if (s) xdlg->hostId=strdup(s);
  else xdlg->hostId=NULL;
}




uint32_t EBC_NewKeyFileDialog_GetFlags(const GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->flags;
}



void EBC_NewKeyFileDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags=fl;
}



void EBC_NewKeyFileDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}



void EBC_NewKeyFileDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}






void EBC_NewKeyFileDialog_Init(GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
			      "",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("EBICS Keyfile Setup Wizard"),
			      0);

  /* select first page */
  GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, 0, 0);

  /* setup intro page */
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_begin_label",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("This dialog assists you in setting up a Keyfile User.\n"),
                              0);

  /* setup bank page */
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_bank_label",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("<html>"
                                   "<p>Please select the bank.</p>"
				   "<p>AqBanking has an internal database which "
				   "contains EBICS information about a few banks.<p>"
				   "<p>If there is an entry for your bank this dialog will use the "
                                   "information from the database.</p>"
                                   "</html>"
                                   "Please select the bank.\n"
                                   "AqBanking has an internal database which contains EBICS information\n"
                                   "about a few banks.\n"
                                   "If there is an entry for your bank this dialog will use the\n"
                                   "information from the database."
                                  ),
			      0);

  /* setup user page */
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_user_label",
			      GWEN_DialogProperty_Title,
			      0,
                              I18N("<html>"
                                   "<p>Please enter the necessary information below. You can "
                                   "probably find this information in the letter you received from "
                                   "your bank in response to the application for an EBICS account."
                                   "</p>"
                                   "</html>"
                                   "Please enter the necessary information below. You can probably find\n"
                                   "this information in the letter you received from your bank in response\n"
                                   "to the application for an EBICS account."),
                              0);

  /* setup creation page */
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_create_label",
			      GWEN_DialogProperty_Title,
			      0,
                              I18N("<html>"
                                   "<p>We are now ready to create the user and exchange keys with the server.</p>"
                                   "<p>Click the <i>next</i> button to proceed or <i>abort</i> to abort.</p>"
                                   "</html>"
                                   "We are now ready to create the user and exchange keys with the server.\n"
                                   "Click the \"next\" button to proceed or \"abort\" to abort."),
                              0);

  /* setup extro page */
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_end_label",
			      GWEN_DialogProperty_Title,
			      0,
                              I18N("<html>"
                                   "<p>The user has been successfully created.</p>"
                                   "<p>You must now <b>print</b> the INI and HIA letter (click the button below) "
				   "and <b>send</b> it to the bank.</p> "
                                   "<p>The activation of your account by the bank can take a few days.</p>"
                                   "</html>"
                                   "The user has been successfully created.\n"
                                   "You must now \"print\" the INI and HIA letter (click the button below)\n"
                                   "and \"send\" it to the bank.\n"
                                   "The activation of your account by the bank can take a few days."),
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



void EBC_NewKeyFileDialog_Fini(GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
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



int EBC_NewKeyFileDialog_GetFilePageData(GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_filename_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    EBC_NewKeyFileDialog_SetFileName(dlg, s);
  else {
    DBG_INFO(AQEBICS_LOGDOMAIN, "Missing file name");
    return GWEN_ERROR_NO_DATA;
  }

  return 0;
}



int EBC_NewKeyFileDialog_GetBankPageData(GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_bankcode_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    EBC_NewKeyFileDialog_SetBankCode(dlg, s);
  else {
    DBG_INFO(AQEBICS_LOGDOMAIN, "Missing bank code");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_bankname_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    EBC_NewKeyFileDialog_SetBankName(dlg, s);
  else
    EBC_NewKeyFileDialog_SetBankName(dlg, NULL);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_url_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    EBC_NewKeyFileDialog_SetUrl(dlg, s);
  else {
    DBG_INFO(AQEBICS_LOGDOMAIN, "Missing URL");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_hostid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    EBC_NewKeyFileDialog_SetHostId(dlg, s);
  else {
    DBG_INFO(AQEBICS_LOGDOMAIN, "Missing host id");
    return GWEN_ERROR_NO_DATA;
  }

  return 0;
}



void EBC_NewKeyFileDialog_SetBankPageData(GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  s=xdlg->bankCode;
  GWEN_Dialog_SetCharProperty(dlg, "wiz_bankcode_edit", GWEN_DialogProperty_Value, 0,
                              (s && *s)?s:"", 0);

  s=xdlg->bankName;
  GWEN_Dialog_SetCharProperty(dlg, "wiz_bankname_edit", GWEN_DialogProperty_Value, 0,
                              (s && *s)?s:"", 0);

  s=xdlg->url;
  GWEN_Dialog_SetCharProperty(dlg, "wiz_url_edit", GWEN_DialogProperty_Value, 0,
                              (s && *s)?s:"", 0);

  s=xdlg->hostId;
  GWEN_Dialog_SetCharProperty(dlg, "wiz_hostid_edit", GWEN_DialogProperty_Value, 0,
                              (s && *s)?s:"", 0);
}



int EBC_NewKeyFileDialog_GetUserPageData(GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_username_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    EBC_NewKeyFileDialog_SetUserName(dlg, s);
  else {
    DBG_INFO(AQEBICS_LOGDOMAIN, "Missing user name");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_userid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    EBC_NewKeyFileDialog_SetUserId(dlg, s);
  else {
    DBG_INFO(AQEBICS_LOGDOMAIN, "Missing user id");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_customerid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    EBC_NewKeyFileDialog_SetCustomerId(dlg, s);
  else
    EBC_NewKeyFileDialog_SetCustomerId(dlg, NULL);

  return 0;
}



void EBC_NewKeyFileDialog_SetUserPageData(GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  s=xdlg->userName;
  GWEN_Dialog_SetCharProperty(dlg, "wiz_username_edit", GWEN_DialogProperty_Value, 0,
                              (s && *s)?s:"", 0);

  s=xdlg->userId;
  GWEN_Dialog_SetCharProperty(dlg, "wiz_userid_edit", GWEN_DialogProperty_Value, 0,
                              (s && *s)?s:"", 0);

  s=xdlg->customerId;
  GWEN_Dialog_SetCharProperty(dlg, "wiz_customerid_edit", GWEN_DialogProperty_Value, 0,
                              (s && *s)?s:"", 0);
}



int EBC_NewKeyFileDialog_EnterPage(GWEN_DIALOG *dlg, int page, int forwards) {
  EBC_NEWKEYFILE_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  switch(page) {
  case PAGE_BEGIN:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_FILE:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    rv=EBC_NewKeyFileDialog_GetFilePageData(dlg);
    if (rv<0)
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_BANK:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    rv=EBC_NewKeyFileDialog_GetBankPageData(dlg);
    if (rv<0)
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_USER:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    rv=EBC_NewKeyFileDialog_GetUserPageData(dlg);
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



int EBC_NewKeyFileDialog_DoIt(GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;
  AB_USER *u;
  int rv;
  uint32_t pid;
  AB_PROVIDER *pro;
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *pl;
  GWEN_CRYPT_TOKEN *ct;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  rv=EBC_NewKeyFileDialog_GetFilePageData(dlg);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "No file?");
    // TODO: show error message
    return GWEN_DialogEvent_ResultHandled;
  }

  pro=AB_Banking_GetProvider(xdlg->banking, "aqebics");
  if (pro==NULL) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not find backend, maybe some plugins are not installed?");
    GWEN_Gui_ShowError(I18N("Error"),
		       "%s",
		       I18N("Could not find HBCI backend, maybe some plugins are not installed?"));
    return GWEN_DialogEvent_ResultHandled;
  }

  u=AB_Banking_CreateUser(xdlg->banking, "aqebics");
  if (u==NULL) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not create user, maybe backend missing?");
    GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Could not find HBCI backend, maybe some plugins are not installed?"));
    return GWEN_DialogEvent_ResultHandled;
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

  /* EBICS setup */
  EBC_User_SetTokenType(u, "ohbci");
  EBC_User_SetTokenName(u, EBC_NewKeyFileDialog_GetFileName(dlg));
  EBC_User_SetTokenContextId(u, 1);
  EBC_User_SetStatus(u, EBC_UserStatus_New);
  EBC_User_SetProtoVersion(u, xdlg->ebicsVersion);
  EBC_User_SetSignVersion(u, xdlg->signVersion);
  EBC_User_SetCryptVersion(u, xdlg->cryptVersion);
  EBC_User_SetAuthVersion(u, xdlg->authVersion);
  EBC_User_SetHttpVMajor(u, xdlg->httpVMajor);
  EBC_User_SetHttpVMinor(u, xdlg->httpVMinor);

  EBC_User_SetFlags(u, xdlg->flags);

  /* create CryptToken */
  pm=GWEN_PluginManager_FindPluginManager(GWEN_CRYPT_TOKEN_PLUGIN_TYPENAME);
  if (pm==0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Plugin manager not found");
    GWEN_Gui_ShowError(I18N("Error"),
		       I18N("CryptToken plugin for type %s is not available. Did you install all necessary packages?"),
		       GWEN_CRYPT_TOKEN_PLUGIN_TYPENAME);
    return 3;
  }

  pl=GWEN_PluginManager_GetPlugin(pm, EBC_User_GetTokenType(u));
  if (pl==0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Plugin not found");
    GWEN_Gui_ShowError(I18N("Error"),
		       I18N("CryptToken plugin for type %s is not available. Did you install all necessary packages?"),
		       EBC_User_GetTokenType(u));
    AB_User_free(u);
    return GWEN_DialogEvent_ResultHandled;
  }

  ct=GWEN_Crypt_Token_Plugin_CreateToken(pl, EBC_User_GetTokenName(u));
  if (ct==0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not create crypt token");
    AB_User_free(u);
    return GWEN_DialogEvent_ResultHandled;
  }

  /* create crypt token */
  rv=GWEN_Crypt_Token_Create(ct, 0);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not create token");
    GWEN_Gui_ShowError(I18N("Error"),
		       I18N("The keyfile %s could not be created. Maybe there already is a file of that name (%d)."),
		       GWEN_Crypt_Token_GetTokenName(ct),
		       rv);
    AB_User_free(u);
    return GWEN_DialogEvent_ResultHandled;
  }

  /* close crypt token */
  rv=GWEN_Crypt_Token_Close(ct, 0, 0);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not close token");
    GWEN_Gui_ShowError(I18N("Error"),
		       I18N("The keyfile %s could not be closed. Please check disc space."),
		       GWEN_Crypt_Token_GetTokenName(ct),
		       rv);
    AB_User_free(u);
    unlink(EBC_User_GetTokenName(u));
    return GWEN_DialogEvent_ResultHandled;
  }

  EBC_User_SetServerUrl(u, xdlg->url);
  EBC_User_SetPeerId(u, xdlg->hostId);

  rv=AB_Banking_AddUser(xdlg->banking, u);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not add user (%d)", rv);
    GWEN_Gui_ShowError(I18N("Error"),
		       I18N("Could not add HBCI user, maybe there already is a user of that id (%d)"),
		       rv);
    AB_User_free(u);
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not add user, maybe there already is a user of the same id (%d)?", rv);
    return GWEN_DialogEvent_ResultHandled;
  }

  i=1;
  if (!(EBC_User_GetFlags(u) & EBC_USER_FLAGS_INI))
    i++;
  if (!(EBC_User_GetFlags(u) & EBC_USER_FLAGS_HIA))
    i++;

  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
			     GWEN_GUI_PROGRESS_ALLOW_EMBED |
			     GWEN_GUI_PROGRESS_SHOW_PROGRESS |
			     GWEN_GUI_PROGRESS_SHOW_ABORT,
			     I18N("Setting Up Keyfile User"),
                             I18N("The keys will now be created and sent to the bank."),
                             i, /* mkKeys, sendKeys */
			     0);
  /* lock new user */
  rv=AB_Banking_BeginExclUseUser(xdlg->banking, u);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not lock user (%d)", rv);
    GWEN_Gui_ProgressLog2(pid,
			  GWEN_LoggerLevel_Error,
			  I18N("Unable to lock users (%d)"), rv);
    AB_Banking_DeleteUser(xdlg->banking, u);
    unlink(EBC_NewKeyFileDialog_GetFileName(dlg));
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }

  /* generate keys */
  rv=EBC_Provider_CreateKeys(pro, u, xdlg->cryptAndAuthKeySize, xdlg->signKeySize, 1);
  if (rv<0) {
    AB_Banking_EndExclUseUser(xdlg->banking, u, 1);
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    AB_Banking_DeleteUser(xdlg->banking, u);
    unlink(EBC_NewKeyFileDialog_GetFileName(dlg));
    GWEN_Gui_ProgressLog2(pid,
			  GWEN_LoggerLevel_Error,
			  I18N("Error generating keys: %d"), rv);
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }

  rv=GWEN_Gui_ProgressAdvance(pid, GWEN_GUI_PROGRESS_ONE);
  if (rv==GWEN_ERROR_USER_ABORTED) {
    AB_Banking_EndExclUseUser(xdlg->banking, u, 1);
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    AB_Banking_DeleteUser(xdlg->banking, u);
    unlink(EBC_NewKeyFileDialog_GetFileName(dlg));
    GWEN_Gui_ProgressLog(pid,
			 GWEN_LoggerLevel_Error,
			 I18N("Aborted by user."));
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }

  /* send user keys */
  GWEN_Gui_ProgressLog(pid,
		       GWEN_LoggerLevel_Notice,
		       I18N("Sending user keys"));


  if (!(EBC_User_GetFlags(u) & EBC_USER_FLAGS_INI)) {
    rv=EBC_Provider_Send_INI(pro, u, 0);
    if (rv<0) {
      AB_Banking_EndExclUseUser(xdlg->banking, u, 1);
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      AB_Banking_DeleteUser(xdlg->banking, u);
      unlink(EBC_NewKeyFileDialog_GetFileName(dlg));
      GWEN_Gui_ProgressEnd(pid);
      return GWEN_DialogEvent_ResultHandled;
    }

    rv=GWEN_Gui_ProgressAdvance(pid, GWEN_GUI_PROGRESS_ONE);
    if (rv==GWEN_ERROR_USER_ABORTED) {
      AB_Banking_EndExclUseUser(xdlg->banking, u, 1);
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      AB_Banking_DeleteUser(xdlg->banking, u);
      unlink(EBC_NewKeyFileDialog_GetFileName(dlg));
      GWEN_Gui_ProgressLog(pid,
                           GWEN_LoggerLevel_Error,
                           I18N("Aborted by user."));
      GWEN_Gui_ProgressEnd(pid);
      return GWEN_DialogEvent_ResultHandled;
    }
  }

  if (!(EBC_User_GetFlags(u) & EBC_USER_FLAGS_HIA)) {
    rv=EBC_Provider_Send_HIA(pro, u, 0);
    if (rv<0) {
      AB_Banking_EndExclUseUser(xdlg->banking, u, 1);
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      AB_Banking_DeleteUser(xdlg->banking, u);
      unlink(EBC_NewKeyFileDialog_GetFileName(dlg));
      GWEN_Gui_ProgressEnd(pid);
      return GWEN_DialogEvent_ResultHandled;
    }

    rv=GWEN_Gui_ProgressAdvance(pid, GWEN_GUI_PROGRESS_ONE);
    if (rv==GWEN_ERROR_USER_ABORTED) {
      AB_Banking_EndExclUseUser(xdlg->banking, u, 1);
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      AB_Banking_DeleteUser(xdlg->banking, u);
      unlink(EBC_NewKeyFileDialog_GetFileName(dlg));
      GWEN_Gui_ProgressLog(pid,
                           GWEN_LoggerLevel_Error,
                           I18N("Aborted by user."));
      GWEN_Gui_ProgressEnd(pid);
      return GWEN_DialogEvent_ResultHandled;
    }
  }

  /* unlock user */
  rv=AB_Banking_EndExclUseUser(xdlg->banking, u, 0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN,
	     "Could not unlock customer [%s] (%d)",
	     AB_User_GetCustomerId(u), rv);
    GWEN_Gui_ProgressLog2(pid,
			  GWEN_LoggerLevel_Error,
			  I18N("Could not unlock user %s (%d)"),
			  AB_User_GetUserId(u), rv);
    AB_Banking_EndExclUseUser(xdlg->banking, u, 1);
    AB_Banking_DeleteUser(xdlg->banking, u);
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }

  GWEN_Gui_ProgressEnd(pid);
  EBC_NewKeyFileDialog_EnterPage(dlg, PAGE_END, 1);

  xdlg->user=u;

  return GWEN_DialogEvent_ResultHandled;
}



int EBC_NewKeyFileDialog_Next(GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  if (page==PAGE_CREATE) {
    return EBC_NewKeyFileDialog_DoIt(dlg);
  }
  else if (page<PAGE_END) {
    page++;
    return EBC_NewKeyFileDialog_EnterPage(dlg, page, 1);
  }
  else if (page==PAGE_END)
    return GWEN_DialogEvent_ResultAccept;

  return GWEN_DialogEvent_ResultHandled;
}



int EBC_NewKeyFileDialog_Previous(GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  if (page>PAGE_BEGIN) {
    page--;
    return EBC_NewKeyFileDialog_EnterPage(dlg, page, 0);
  }

  return GWEN_DialogEvent_ResultHandled;
}



int EBC_NewKeyFileDialog_HandleActivatedBankCode(GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  dlg2=AB_SelectBankInfoDialog_new(xdlg->banking, "de", NULL);
  if (dlg2==NULL) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not create dialog");
    GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Could not create dialog, maybe an installation error?"));
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
      while(sv) {
	const char *s;

	s=AB_BankInfoService_GetType(sv);
        if (s && *s && strcasecmp(s, "EBICS")==0)
          break;
        sv=AB_BankInfoService_List_Next(sv);
      }

      if (sv) {
        /* EBICS service found */
        s=AB_BankInfoService_GetAddress(sv);
	GWEN_Dialog_SetCharProperty(dlg,
				    "wiz_url_edit",
				    GWEN_DialogProperty_Value,
				    0,
				    (s && *s)?s:"",
                                    0);
        free(xdlg->ebicsVersion); xdlg->ebicsVersion=NULL;
        free(xdlg->signVersion);  xdlg->signVersion=NULL;
        free(xdlg->cryptVersion); xdlg->cryptVersion=NULL;
        free(xdlg->authVersion);  xdlg->authVersion=NULL;

	s=AB_BankInfoService_GetPversion(sv);
	if (s && *s) {
          if (strcasecmp(s, "H002")==0) {
            xdlg->ebicsVersion=strdup("H002");
            xdlg->signVersion=strdup("A004");
            xdlg->cryptVersion=strdup("E001");
            xdlg->authVersion=strdup("X001");
          }
          else if (strcasecmp(s, "H003")==0) {
            xdlg->ebicsVersion=strdup("H003");
            xdlg->signVersion=strdup("A005");
            xdlg->cryptVersion=strdup("E002");
            xdlg->authVersion=strdup("X002");
          }
        }

        if (xdlg->ebicsVersion==NULL) {
          xdlg->ebicsVersion=strdup("H003");
          xdlg->signVersion=strdup("A005");
          xdlg->cryptVersion=strdup("E002");
          xdlg->authVersion=strdup("X002");
        }
      }
    }
  }

  GWEN_Dialog_free(dlg2);

  if (EBC_NewKeyFileDialog_GetBankPageData(dlg)<0)
    GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
  else
    GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);

  return GWEN_DialogEvent_ResultHandled;
}



int EBC_NewKeyFileDialog_HandleActivatedSpecial(GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  dlg2=EBC_UserSpecialDialog_new(xdlg->banking);
  if (dlg2==NULL) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not create dialog");
    GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Could not create dialog, maybe an installation error?"));
    return GWEN_DialogEvent_ResultHandled;
  }

  EBC_UserSpecialDialog_SetEbicsVersion(dlg2, xdlg->ebicsVersion);
  EBC_UserSpecialDialog_SetSignVersion(dlg2, xdlg->signVersion);
  EBC_UserSpecialDialog_SetCryptVersion(dlg2, xdlg->cryptVersion);
  EBC_UserSpecialDialog_SetAuthVersion(dlg2, xdlg->authVersion);
  EBC_UserSpecialDialog_SetSignKeySize(dlg2, xdlg->signKeySize);
  EBC_UserSpecialDialog_SetCryptAndAuthKeySize(dlg2, xdlg->cryptAndAuthKeySize);

  EBC_UserSpecialDialog_SetHttpVersion(dlg2, xdlg->httpVMajor, xdlg->httpVMinor);

  EBC_UserSpecialDialog_SetFlags(dlg2, xdlg->flags);

  rv=GWEN_Gui_ExecDialog(dlg2, 0);
  if (rv==0) {
    /* rejected */
    GWEN_Dialog_free(dlg2);
    return GWEN_DialogEvent_ResultHandled;
  }
  else {
    s=EBC_UserSpecialDialog_GetEbicsVersion(dlg2);
    EBC_NewKeyFileDialog_SetEbicsVersion(dlg, s);

    s=EBC_UserSpecialDialog_GetSignVersion(dlg2);
    EBC_NewKeyFileDialog_SetSignVersion(dlg, s);

    s=EBC_UserSpecialDialog_GetCryptVersion(dlg2);
    EBC_NewKeyFileDialog_SetCryptVersion(dlg, s);

    s=EBC_UserSpecialDialog_GetAuthVersion(dlg2);
    EBC_NewKeyFileDialog_SetAuthVersion(dlg, s);

    xdlg->signKeySize=EBC_UserSpecialDialog_GetSignKeySize(dlg2);
    xdlg->cryptAndAuthKeySize=EBC_UserSpecialDialog_GetCryptAndAuthKeySize(dlg2);

    xdlg->httpVMajor=EBC_UserSpecialDialog_GetHttpVMajor(dlg2);
    xdlg->httpVMinor=EBC_UserSpecialDialog_GetHttpVMinor(dlg2);
    xdlg->flags=EBC_UserSpecialDialog_GetFlags(dlg2);
  }

  GWEN_Dialog_free(dlg2);

  return GWEN_DialogEvent_ResultHandled;
}



int EBC_NewKeyFileDialog_HandleActivatedFileButton(GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;
  int rv;
  const char *s;
  GWEN_BUFFER *pathBuffer;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  pathBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_filename_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    GWEN_Buffer_AppendString(pathBuffer, s);
  rv=GWEN_Gui_GetFileName(I18N("Create Keyfile"),
			  GWEN_Gui_FileNameType_SaveFileName,
			  0,
			  I18N("All Files (*)\tOHBCI Files (*ohbci;*.medium)"),
			  pathBuffer,
			  GWEN_Dialog_GetGuiId(dlg));
  if (rv==0) {
    GWEN_Dialog_SetCharProperty(dlg,
				"wiz_filename_edit",
				GWEN_DialogProperty_Value,
				0,
				GWEN_Buffer_GetStart(pathBuffer),
				0);
    rv=EBC_NewKeyFileDialog_GetFilePageData(dlg);
    if (rv<0)
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
  }
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "here (%d)", rv);
  }
  GWEN_Buffer_free(pathBuffer);
  return GWEN_DialogEvent_ResultHandled;
}



static int EBC_NewKeyFileDialog_HandleActivatedIniLetter(GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;
  int rv;
  GWEN_BUFFER *tbuf;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  tbuf=GWEN_Buffer_new(0, 1024, 0, 1);

  /* add HTML version of the INI letter */
  //GWEN_Buffer_AppendString(tbuf, "<html>");
  rv=EBC_Provider_GetIniLetterTxt(AB_User_GetProvider(xdlg->user),
				  xdlg->user, 0, tbuf, 0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    // TODO: show error message
    AB_Banking_ClearCryptTokenList(xdlg->banking);
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }
  //GWEN_Buffer_AppendString(tbuf, "</html>");


#if 0
  /* add ASCII version of the INI letter for frontends which don't support HTML */
  rv=EBC_Provider_GetIniLetterTxt(AB_User_GetProvider(xdlg->user),
				  xdlg->user, 0, tbuf, 0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    // TODO: show error message
    AB_Banking_ClearCryptTokenList(xdlg->banking);
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }
#endif

  rv=GWEN_Gui_Print(I18N("INI Letter"),
		    "EBICS-INILETTER",
		    I18N("INI Letter for EBICS"),
		    GWEN_Buffer_GetStart(tbuf),
		    0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    // TODO: show error message
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }

  GWEN_Buffer_free(tbuf);
  return GWEN_DialogEvent_ResultHandled;
}



static int EBC_NewKeyFileDialog_HandleActivatedHiaLetter(GWEN_DIALOG *dlg) {
  EBC_NEWKEYFILE_DIALOG *xdlg;
  int rv;
  GWEN_BUFFER *tbuf;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  tbuf=GWEN_Buffer_new(0, 1024, 0, 1);

  /* add HTML version of the INI letter */
  //GWEN_Buffer_AppendString(tbuf, "<html>");
  rv=EBC_Provider_GetHiaLetterTxt(AB_User_GetProvider(xdlg->user),
				  xdlg->user, 0, tbuf, 0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    // TODO: show error message
    AB_Banking_ClearCryptTokenList(xdlg->banking);
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }
  //GWEN_Buffer_AppendString(tbuf, "</html>");


#if 0
  /* add ASCII version of the HIA letter for frontends which don't support HTML */
  rv=EBC_Provider_GetHIALetterTxt(AB_User_GetProvider(xdlg->user),
				  xdlg->user, 0, tbuf, 0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    // TODO: show error message
    AB_Banking_ClearCryptTokenList(xdlg->banking);
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }
#endif

  rv=GWEN_Gui_Print(I18N("HIA Letter"),
		    "EBICS-HIALETTER",
		    I18N("HIA Letter for EBICS"),
		    GWEN_Buffer_GetStart(tbuf),
		    0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    // TODO: show error message
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }

  GWEN_Buffer_free(tbuf);
  return GWEN_DialogEvent_ResultHandled;
}





int EBC_NewKeyFileDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  if (strcasecmp(sender, "wiz_filename_button")==0)
    return EBC_NewKeyFileDialog_HandleActivatedFileButton(dlg);
  else if (strcasecmp(sender, "wiz_bankcode_button")==0)
    return EBC_NewKeyFileDialog_HandleActivatedBankCode(dlg);
  else if (strcasecmp(sender, "wiz_prev_button")==0)
    return EBC_NewKeyFileDialog_Previous(dlg);
  else if (strcasecmp(sender, "wiz_next_button")==0)
    return EBC_NewKeyFileDialog_Next(dlg);
  else if (strcasecmp(sender, "wiz_abort_button")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "wiz_special_button")==0)
    return EBC_NewKeyFileDialog_HandleActivatedSpecial(dlg);
  else if (strcasecmp(sender, "wiz_iniletter_button")==0)
    return EBC_NewKeyFileDialog_HandleActivatedIniLetter(dlg);
  else if (strcasecmp(sender, "wiz_hialetter_button")==0)
    return EBC_NewKeyFileDialog_HandleActivatedHiaLetter(dlg);
  else if (strcasecmp(sender, "wiz_help_button")==0) {
    /* TODO: open a help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int EBC_NewKeyFileDialog_HandleValueChanged(GWEN_DIALOG *dlg, const char *sender) {
  if (strcasecmp(sender, "wiz_filename_edit")==0 ||
      strcasecmp(sender, "wiz_bankcode_edit")==0 ||
      strcasecmp(sender, "wiz_url_edit")==0 ||
      strcasecmp(sender, "wiz_username_edit")==0 ||
      strcasecmp(sender, "wiz_userid_edit")==0 ||
      strcasecmp(sender, "wiz_customerid_edit")==0) {
    int rv;

    if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_FILE) {
      rv=EBC_NewKeyFileDialog_GetFilePageData(dlg);
      if (rv<0)
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    else if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_BANK) {
      rv=EBC_NewKeyFileDialog_GetBankPageData(dlg);
      if (rv<0)
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    else if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_USER) {
      rv=EBC_NewKeyFileDialog_GetUserPageData(dlg);
      if (rv<0)
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    return GWEN_DialogEvent_ResultHandled;
  }
  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB EBC_NewKeyFileDialog_SignalHandler(GWEN_DIALOG *dlg,
						    GWEN_DIALOG_EVENTTYPE t,
						    const char *sender) {
  EBC_NEWKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_NEWKEYFILE_DIALOG, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    EBC_NewKeyFileDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    EBC_NewKeyFileDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return EBC_NewKeyFileDialog_HandleValueChanged(dlg, sender);

  case GWEN_DialogEvent_TypeActivated:
    return EBC_NewKeyFileDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}




