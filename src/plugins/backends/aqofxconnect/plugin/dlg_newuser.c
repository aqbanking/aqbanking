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



#include "dlg_newuser_p.h"
#include "dlg_ofx_special_l.h"
#include "libofxhome/dlg_getinst.h"
#include "i18n_l.h"

#include <aqbanking/user.h>
#include <aqbanking/banking_be.h>

#include <aqofxconnect/user.h>
#include <aqofxconnect/provider.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/directory.h>


#define PAGE_BEGIN     0
#define PAGE_BANK      1
#define PAGE_USER      2
#define PAGE_APP       3
#define PAGE_CREATE    4
#define PAGE_END       5


#define DIALOG_MINWIDTH  400
#define DIALOG_MINHEIGHT 200



GWEN_INHERIT(GWEN_DIALOG, AO_NEWUSER_DIALOG)





GWEN_DIALOG *AO_NewUserDialog_new(AB_BANKING *ab) {
  GWEN_DIALOG *dlg;
  AO_NEWUSER_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("ao_newuser");
  GWEN_NEW_OBJECT(AO_NEWUSER_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg, xdlg,
		       AO_NewUserDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, AO_NewUserDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
			       "aqbanking/backends/aqofxconnect/dialogs/dlg_newuser.dlg",
			       fbuf);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Dialog description file not found (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }

  /* read dialog from dialog description file */
  rv=GWEN_Dialog_ReadXmlFile(dlg, GWEN_Buffer_GetStart(fbuf));
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }
  GWEN_Buffer_free(fbuf);

  xdlg->banking=ab;

  /* preset */
  xdlg->httpVMajor=1;
  xdlg->httpVMinor=1;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB AO_NewUserDialog_FreeData(void *bp, void *p) {
  AO_NEWUSER_DIALOG *xdlg;

  xdlg=(AO_NEWUSER_DIALOG*) p;
  free(xdlg->userName);
  free(xdlg->userId);
  free(xdlg->url);
  free(xdlg->brokerId);
  free(xdlg->org);
  free(xdlg->fid);
  free(xdlg->appId);
  free(xdlg->appVer);
  free(xdlg->headerVer);
  free(xdlg->clientUid);
  free(xdlg->securityType);

  GWEN_FREE_OBJECT(xdlg);
}



AB_USER *AO_NewUserDialog_GetUser(const GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->user;
}



const char *AO_NewUserDialog_GetBankName(const GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->bankName;
}



void AO_NewUserDialog_SetBankName(GWEN_DIALOG *dlg, const char *s) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->bankName);
  if (s) xdlg->bankName=strdup(s);
  else xdlg->bankName=NULL;
}



const char *AO_NewUserDialog_GetUserName(const GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->userName;
}



void AO_NewUserDialog_SetUserName(GWEN_DIALOG *dlg, const char *s) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userName);
  if (s) xdlg->userName=strdup(s);
  else xdlg->userName=NULL;
}



const char *AO_NewUserDialog_GetUserId(const GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->userId;
}



void AO_NewUserDialog_SetUserId(GWEN_DIALOG *dlg, const char *s) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userId);
  if (s) xdlg->userId=strdup(s);
  else xdlg->userId=NULL;
}



const char *AO_NewUserDialog_GetFid(const GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->fid;
}



void AO_NewUserDialog_SetFid(GWEN_DIALOG *dlg, const char *s) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->fid);
  if (s) xdlg->fid=strdup(s);
  else xdlg->fid=NULL;
}



const char *AO_NewUserDialog_GetOrg(const GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->org;
}



void AO_NewUserDialog_SetOrg(GWEN_DIALOG *dlg, const char *s) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->org);
  if (s) xdlg->org=strdup(s);
  else xdlg->org=NULL;
}



const char *AO_NewUserDialog_GetAppId(const GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->appId;
}



void AO_NewUserDialog_SetAppId(GWEN_DIALOG *dlg, const char *s) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->appId);
  if (s) xdlg->appId=strdup(s);
  else xdlg->appId=NULL;
}



const char *AO_NewUserDialog_GetAppVer(const GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->appVer;
}



void AO_NewUserDialog_SetAppVer(GWEN_DIALOG *dlg, const char *s) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->appVer);
  if (s) xdlg->appVer=strdup(s);
  else xdlg->appVer=NULL;
}



const char *AO_NewUserDialog_GetHeaderVer(const GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->headerVer;
}



void AO_NewUserDialog_SetHeaderVer(GWEN_DIALOG *dlg, const char *s) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->headerVer);
  if (s) xdlg->headerVer=strdup(s);
  else xdlg->headerVer=NULL;
}



const char *AO_NewUserDialog_GetBrokerId(const GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->brokerId;
}



void AO_NewUserDialog_SetBrokerId(GWEN_DIALOG *dlg, const char *s) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->brokerId);
  if (s) xdlg->brokerId=strdup(s);
  else xdlg->brokerId=NULL;
}



const char *AO_NewUserDialog_GetUrl(const GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->url;
}



void AO_NewUserDialog_SetUrl(GWEN_DIALOG *dlg, const char *s) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->url);
  if (s) xdlg->url=strdup(s);
  else xdlg->url=NULL;
}



const char *AO_NewUserDialog_GetClientUid(const GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->clientUid;
}



void AO_NewUserDialog_SetClientUid(GWEN_DIALOG *dlg, const char *s) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->clientUid);
  if (s) xdlg->clientUid=strdup(s);
  else xdlg->clientUid=NULL;
}



int AO_NewUserDialog_GetHttpVMajor(const GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->httpVMajor;
}



int AO_NewUserDialog_GetHttpVMinor(const GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->httpVMinor;
}



void AO_NewUserDialog_SetHttpVersion(GWEN_DIALOG *dlg, int vmajor, int vminor) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  xdlg->httpVMajor=vmajor;
  xdlg->httpVMinor=vminor;
}



uint32_t AO_NewUserDialog_GetFlags(const GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->flags;
}



void AO_NewUserDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags=fl;
}



void AO_NewUserDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}



void AO_NewUserDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}






void AO_NewUserDialog_Init(GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;
  AB_PROVIDER *pro;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  pro=AB_Banking_GetProvider(xdlg->banking, "aqofxconnect");
  if (pro==NULL) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Could not find backend, maybe some plugins are not installed?");
  }

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
			      "",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("OFX DirectConnect Setup Wizard"),
			      0);

  /* select first page */
  GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, 0, 0);

  /* setup intro page */
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_begin_label",
			      GWEN_DialogProperty_Title,
			      0,
                              I18N("<html>"
                                   "<p>This dialog assists you in setting up an OFX DirectConnect User.</p>"
                                   "</html>"
                                   "This dialog assists you in setting up an OFX DirectConnect User."),
                              0);

  /* setup bank page */
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_bank_label",
			      GWEN_DialogProperty_Title,
			      0,
                              I18N("<html>"
                                   "<p>Please enter your bank settings below.</p>"
                                   "<p>Click the <b>Select</b> button to choose from a list of "
                                   "known banks. That will connect to <i>www.ofxhome.com</i> and "
                                   "try to retrieve information about your bank.</p>"
                                   "<p>If you had to manually enter this information because your "
                                   "bank was unknown to <i>www.ofxhome.com</i> you are kindly "
                                   "asked to submit your bank server information there to help "
                                   "the next user.</p>"
                                   "</html>"
                                   "Click the SELECT button to choose from a list of\n"
                                   "known banks. That will connect to \"www.ofxhome.com\" and\n"
                                   "try to retrieve information about your bank.\n"
                                   "If you had to manually enter this information because your\n"
                                   "bank was unknown to \"www.ofxhome.com\" you are kindly\n"
                                   "asked to submit your bank server information there to help\n"
                                   "the next user."),
                              0);

  /* setup user page */
  GWEN_Dialog_SetCharProperty(dlg,
                              "wiz_user_label",
			      GWEN_DialogProperty_Title,
			      0,
                              I18N("<html>"
                                   "<p>Please enter your user settings below.</p>"
                                   "<p><i>User Name</i> is your real name, <i>User Id</i> is "
                                   "assigned to you by the bank after applying for OFX DirectConnect "
                                   "and <i>Client UID</i> is used by some banks only. If you do not have such "
                                   "a value in your documents from the bank just leave it blank.</p>"
                                   "</html>"
                                   "Please enter your user settings below.\n"
                                   "\"User Name\" is your real name, \"User Id\" is\n"
                                   "assigned to you by the bank after applying for OFX DirectConnect\n"
                                   "and \"Client UID\" is used by some banks. If you do not have such\n"
                                   "a value in your documents from the bank just leave it blank."),
                              0);


  /* setup application page */
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_app_label",
			      GWEN_DialogProperty_Title,
			      0,
                              I18N("<html>"
                                   "<p>Please choose the application you want AqBanking to emulate. "
                                   "Not all banks support all applications and versions, you might have "
                                   "to try multiple settings.</p>"
                                   "</html>"
                                   "Please choose the application you want AqBanking to emulate.\n"
                                   "Not all banks support all applications and versions, you might have\n"
                                   "to try multiple settings."),
                              0);

  GWEN_Dialog_SetIntProperty(dlg, "wiz_app_combo", GWEN_DialogProperty_ClearValues, 0, 0, 0);
  GWEN_Dialog_SetCharProperty(dlg, "wiz_app_combo", GWEN_DialogProperty_AddValue, 0, I18N("-- select --"), 0);
  if (pro) {
    const AO_APPINFO *ai;

    ai=AO_Provider_GetAppInfos(pro);
    if (ai) {
      const AO_APPINFO *first;

      first=ai;
      while(ai->appName) {
        GWEN_Dialog_SetCharProperty(dlg, "wiz_app_combo", GWEN_DialogProperty_AddValue, 0, I18N(ai->appName), 0);
        ai++;
      }

      if (first->appName) {
        GWEN_Dialog_SetIntProperty(dlg, "wiz_app_combo", GWEN_DialogProperty_Value, 0, 1, 0);
        if (first->appId)
          GWEN_Dialog_SetCharProperty(dlg, "wiz_appid_edit", GWEN_DialogProperty_Value, 0, first->appId, 0);
        if (first->appVer)
          GWEN_Dialog_SetCharProperty(dlg, "wiz_appver_edit", GWEN_DialogProperty_Value, 0, first->appVer, 0);
      }
    }
  }

  GWEN_Dialog_SetCharProperty(dlg, "wiz_headerver_edit", GWEN_DialogProperty_Value, 0, "102", 0);

  /* setup creation page */
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_create_label",
			      GWEN_DialogProperty_Title,
			      0,
                              I18N("<html>"
                                   "<p>We are now ready to create the user.</p>"
                                   "<p>Click the <i>next</i> button to proceed or <i>abort</i> to abort.</p>"
                                   "</html>"
                                   "We are now ready to create the user.\n"
                                   "Click the NEXT button to proceed or ABORT to abort."),
                              0);

  /* setup extro page */
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_end_label",
			      GWEN_DialogProperty_Title,
			      0,
                              I18N("<html>"
                                   "<p>The user has been successfully setup.</p>"
                                   "<p>You can now try to retrieve the list of accounts the "
                                   "bank allows you to manage via OFX DirectConnect.</p>"
                                   "<p>Please note that not every banks supports this. If your "
                                   "bank does not support account list download you will have to "
                                   "add the account manually.</p>"
                                   "</html>"
                                   "The user has been successfully setup.\n"
                                   "You can now try to retrieve the list of accounts the\n"
                                   "bank allows you to manage via OFX DirectConnect.\n"
                                   "Please note that not every banks supports this. If your\n"
                                   "bank does not support account list download you will have to\n"
                                   "add the account manually."
                                  ),
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



void AO_NewUserDialog_Fini(GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
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



int AO_NewUserDialog_GetBankPageData(GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_bankname_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_NewUserDialog_SetBankName(dlg, s);
  else {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Missing bank name");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_brokerid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_NewUserDialog_SetBrokerId(dlg, s);
  else
    AO_NewUserDialog_SetBrokerId(dlg, NULL);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_fid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_NewUserDialog_SetFid(dlg, s);
  else {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Missing FID");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_org_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_NewUserDialog_SetOrg(dlg, s);
  else {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Missing ORG");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_url_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_NewUserDialog_SetUrl(dlg, s);
  else {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Missing URL");
    return GWEN_ERROR_NO_DATA;
  }

  return 0;
}



int AO_NewUserDialog_GetUserPageData(GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_username_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_NewUserDialog_SetUserName(dlg, s);
  else {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Missing user name");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_userid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_NewUserDialog_SetUserId(dlg, s);
  else {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Missing user id");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_clientuid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_NewUserDialog_SetClientUid(dlg, s);
  else
    AO_NewUserDialog_SetClientUid(dlg, NULL);

  return 0;
}



int AO_NewUserDialog_GetAppPageData(GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_appid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_NewUserDialog_SetAppId(dlg, s);
  else {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Missing application id");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_appver_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_NewUserDialog_SetAppVer(dlg, s);
  else {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Missing application version");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_headerver_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_NewUserDialog_SetHeaderVer(dlg, s);
  else {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Missing application version");
    return GWEN_ERROR_NO_DATA;
  }

  return 0;
}



int AO_NewUserDialog_EnterPage(GWEN_DIALOG *dlg, int page, int forwards) {
  AO_NEWUSER_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  switch(page) {
  case PAGE_BEGIN:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_BANK:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    rv=AO_NewUserDialog_GetBankPageData(dlg);
    if (rv<0)
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_USER:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    rv=AO_NewUserDialog_GetUserPageData(dlg);
    if (rv<0)
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_APP:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    rv=AO_NewUserDialog_GetAppPageData(dlg);
    if (rv<0)
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_CREATE:
    if (!forwards) {
      AO_NewUserDialog_UndoIt(dlg);
      GWEN_Dialog_SetCharProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Title, 0, I18N("Next"), 0);
    }
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_END:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    GWEN_Dialog_SetCharProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Title, 0, I18N("Finish"), 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
#if 0
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_abort_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
#endif
    return GWEN_DialogEvent_ResultHandled;

  default:
    return GWEN_DialogEvent_ResultHandled;
  }

  return GWEN_DialogEvent_ResultHandled;
}



int AO_NewUserDialog_DoIt(GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;
  AB_USER *u;
  int rv;
  uint32_t pid;
  AB_PROVIDER *pro;

  DBG_ERROR(0, "Doit");
  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  pro=AB_Banking_GetProvider(xdlg->banking, "aqofxconnect");
  if (pro==NULL) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Could not find backend, maybe some plugins are not installed?");
    // TODO: show error message
    return GWEN_DialogEvent_ResultHandled;
  }

  DBG_ERROR(0, "Creating user");
  u=AB_Banking_CreateUser(xdlg->banking, "aqofxconnect");
  if (u==NULL) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Could not create user, maybe backend missing?");
    // TODO: show error message
    return GWEN_DialogEvent_ResultHandled;
  }

  /* generic setup */
  AB_User_SetUserName(u, xdlg->userName);
  AB_User_SetUserId(u, xdlg->userId);
  AB_User_SetCustomerId(u, xdlg->userId);
  AB_User_SetCountry(u, "us");
  AO_User_SetBankName(u, xdlg->bankName);
  AB_User_SetBankCode(u, "0000000000");

  AO_User_SetFlags(u, xdlg->flags);
  AO_User_SetBrokerId(u, xdlg->brokerId);
  AO_User_SetOrg(u, xdlg->org);
  AO_User_SetFid(u, xdlg->fid);

  AO_User_SetAppId(u, xdlg->appId);
  AO_User_SetAppVer(u, xdlg->appVer);
  AO_User_SetHeaderVer(u, xdlg->headerVer);
  AO_User_SetClientUid(u, xdlg->clientUid);
  AO_User_SetSecurityType(u, xdlg->securityType);

  AO_User_SetServerAddr(u, xdlg->url);
  AO_User_SetHttpVMajor(u, xdlg->httpVMajor);
  AO_User_SetHttpVMinor(u, xdlg->httpVMinor);

  DBG_ERROR(0, "Adding user");
  rv=AB_Banking_AddUser(xdlg->banking, u);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Could not add user (%d)", rv);
    AB_User_free(u);
    return GWEN_DialogEvent_ResultHandled;
  }

  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
			     GWEN_GUI_PROGRESS_ALLOW_EMBED |
			     GWEN_GUI_PROGRESS_SHOW_PROGRESS |
			     GWEN_GUI_PROGRESS_SHOW_ABORT,
                             I18N("Setting Up OFX DirectConnect User"),
                             I18N("The user will be created and the certificate retrieved."),
                             1,
			     0);
  /* lock new user */
  DBG_ERROR(0, "Locking user");
  rv=AB_Banking_BeginExclUseUser(xdlg->banking, u);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Could not lock user (%d)", rv);
    GWEN_Gui_ProgressLog(pid,
			 GWEN_LoggerLevel_Error,
			 I18N("Unable to lock users"));
    AB_Banking_DeleteUser(xdlg->banking, u);
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }

  GWEN_Gui_ProgressLog(pid,
		       GWEN_LoggerLevel_Notice,
		       I18N("Retrieving SSL certificate"));
  rv=AO_Provider_GetCert(pro, u);
  if (rv<0) {
    AB_Banking_EndExclUseUser(xdlg->banking, u, 1);
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    AB_Banking_DeleteUser(xdlg->banking, u);
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }

  rv=GWEN_Gui_ProgressAdvance(pid, GWEN_GUI_PROGRESS_ONE);
  if (rv==GWEN_ERROR_USER_ABORTED) {
    AB_Banking_EndExclUseUser(xdlg->banking, u, 1);
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    AB_Banking_DeleteUser(xdlg->banking, u);
    GWEN_Gui_ProgressLog(pid,
			 GWEN_LoggerLevel_Error,
			 I18N("Aborted by user."));
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }

  /* unlock user */
  DBG_ERROR(0, "Unlocking user");
  rv=AB_Banking_EndExclUseUser(xdlg->banking, u, 0);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
	     "Could not unlock user [%s] (%d)",
	     AB_User_GetUserId(u), rv);
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
  AO_NewUserDialog_EnterPage(dlg, PAGE_END, 1);

  xdlg->user=u;

  return GWEN_DialogEvent_ResultHandled;
}



int AO_NewUserDialog_UndoIt(GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;
  AB_USER *u;

  DBG_ERROR(0, "UndoIt");
  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  u=xdlg->user;
  if (u) {
    AB_ACCOUNT *a;

    /* delete all accounts created for this user */
    while ( (a=AB_Banking_FindFirstAccountOfUser(xdlg->banking, u)) ) {
      AB_Banking_DeleteAccount(xdlg->banking, a);
    }

    /* delete the user itself */
    AB_Banking_DeleteUser(xdlg->banking, u);
    xdlg->user=NULL;
  }
  return GWEN_DialogEvent_ResultHandled;
}



int AO_NewUserDialog_Next(GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  if (page==PAGE_CREATE) {
    return AO_NewUserDialog_DoIt(dlg);
  }
  else if (page<PAGE_END) {
    page++;
    return AO_NewUserDialog_EnterPage(dlg, page, 1);
  }
  else if (page==PAGE_END)
    return GWEN_DialogEvent_ResultAccept;

  return GWEN_DialogEvent_ResultHandled;
}



int AO_NewUserDialog_Previous(GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  if (page>PAGE_BEGIN) {
    page--;
    return AO_NewUserDialog_EnterPage(dlg, page, 0);
  }

  return GWEN_DialogEvent_ResultHandled;
}



int AO_NewUserDialog_HandleActivatedSpecial(GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  dlg2=AO_OfxSpecialDialog_new(xdlg->banking);
  if (dlg2==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not create dialog");
    return GWEN_DialogEvent_ResultHandled;
  }

  AO_OfxSpecialDialog_SetHttpVersion(dlg2, xdlg->httpVMajor, xdlg->httpVMinor);
  AO_OfxSpecialDialog_SetFlags(dlg2, xdlg->flags);

  AO_OfxSpecialDialog_SetClientUid(dlg2, xdlg->clientUid);
  AO_OfxSpecialDialog_SetSecurityType(dlg2, xdlg->securityType);

  rv=GWEN_Gui_ExecDialog(dlg2, 0);
  if (rv==0) {
    /* rejected */
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Rejected");
    GWEN_Dialog_free(dlg2);
    return GWEN_DialogEvent_ResultHandled;
  }
  else {
    const char *s;

    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Accepted");
    xdlg->httpVMajor=AO_OfxSpecialDialog_GetHttpVMajor(dlg2);
    xdlg->httpVMinor=AO_OfxSpecialDialog_GetHttpVMinor(dlg2);
    xdlg->flags=AO_OfxSpecialDialog_GetFlags(dlg2);

    s=AO_OfxSpecialDialog_GetClientUid(dlg2);
    free(xdlg->clientUid);
    if (s) xdlg->clientUid=strdup(s);
    else xdlg->clientUid=NULL;

    s=AO_OfxSpecialDialog_GetSecurityType(dlg2);
    free(xdlg->securityType);
    if (s) xdlg->securityType=strdup(s);
    else xdlg->securityType=NULL;
  }

  GWEN_Dialog_free(dlg2);
  return GWEN_DialogEvent_ResultHandled;
}



int AO_NewUserDialog_HandleActivatedBankSelect(GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;
  int rv;
  GWEN_DIALOG *dlg2;
  GWEN_BUFFER *tbuf;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  /* get data dir */
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=AB_Banking_GetProviderUserDataDir(xdlg->banking, "aqofxconnect", tbuf);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }
  GWEN_Buffer_AppendString(tbuf, GWEN_DIR_SEPARATOR_S "ofxhome");

  /* possibly create data folder */
  rv=GWEN_Directory_GetPath(GWEN_Buffer_GetStart(tbuf), GWEN_PATH_FLAGS_CHECKROOT);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }
  
  dlg2=OH_GetInstituteDialog_new(GWEN_Buffer_GetStart(tbuf), NULL);
  GWEN_Buffer_free(tbuf);
  if (dlg2==NULL) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Could not create dialog");
    return GWEN_DialogEvent_ResultHandled;
  }
  rv=GWEN_Gui_ExecDialog(dlg2, 0);
  if (rv<=0){
    DBG_DEBUG(AQOFXCONNECT_LOGDOMAIN, "Dialog: rejected (%d)", rv);
    return GWEN_DialogEvent_ResultHandled;
  }
  else {
    const OH_INSTITUTE_DATA *od;
  
    DBG_DEBUG(AQOFXCONNECT_LOGDOMAIN, "Dialog: rejected (%d)", rv);
    od=OH_GetInstituteDialog_GetSelectedInstitute(dlg2);
    if (od) {
      const char *s;
  
      s=OH_InstituteData_GetName(od);
      if (s && *s)
        GWEN_Dialog_SetCharProperty(dlg, "wiz_bankname_edit", GWEN_DialogProperty_Value, 0, s, 0);

      s=OH_InstituteData_GetFid(od);
      if (s && *s)
        GWEN_Dialog_SetCharProperty(dlg, "wiz_fid_edit", GWEN_DialogProperty_Value, 0, s, 0);
      s=OH_InstituteData_GetOrg(od);
      if (s && *s)
        GWEN_Dialog_SetCharProperty(dlg, "wiz_org_edit", GWEN_DialogProperty_Value, 0, s, 0);
      s=OH_InstituteData_GetUrl(od);
      if (s && *s)
        GWEN_Dialog_SetCharProperty(dlg, "wiz_url_edit", GWEN_DialogProperty_Value, 0, s, 0);
      rv=AO_NewUserDialog_GetBankPageData(dlg);
      if (rv<0)
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
  }
  GWEN_Dialog_free(dlg2);
  return GWEN_DialogEvent_ResultHandled;
}



int AO_NewUserDialog_HandleActivatedApp(GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;
  AB_PROVIDER *pro;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  pro=AB_Banking_GetProvider(xdlg->banking, "aqofxconnect");
  if (pro==NULL) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Could not find backend, maybe some plugins are not installed?");
    return GWEN_DialogEvent_ResultHandled;
  }
  else {
    int idx;

    idx=GWEN_Dialog_GetIntProperty(dlg, "wiz_app_combo", GWEN_DialogProperty_Value, 0, -1);
    if (idx>0) {
      const AO_APPINFO *ai;

      ai=AO_Provider_GetAppInfos(pro);
      if (ai) {
        while(ai->appName && --idx) {
          ai++;
        }
        if (ai->appName) {
          if (ai->appId)
            GWEN_Dialog_SetCharProperty(dlg, "wiz_appid_edit", GWEN_DialogProperty_Value, 0, ai->appId, 0);
          if (ai->appVer)
            GWEN_Dialog_SetCharProperty(dlg, "wiz_appver_edit", GWEN_DialogProperty_Value, 0, ai->appVer, 0);
        }
      }
    }
  }

  return GWEN_DialogEvent_ResultHandled;
}



int AO_NewUserDialog_HandleActivatedGetAccounts(GWEN_DIALOG *dlg) {
  AO_NEWUSER_DIALOG *xdlg;
  AB_PROVIDER *pro;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  pro=AB_Banking_GetProvider(xdlg->banking, "aqofxconnect");
  if (pro==NULL) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Could not find backend, maybe some plugins are not installed?");
    return GWEN_DialogEvent_ResultHandled;
  }
  else {
    int rv;

    rv=AO_Provider_RequestAccounts(pro, xdlg->user, 1);
    if (rv<0) {
      DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here");
    }
  }

  return GWEN_DialogEvent_ResultHandled;
}




int AO_NewUserDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  DBG_ERROR(0, "Activated: %s", sender);
  if (strcasecmp(sender, "wiz_prev_button")==0)
    return AO_NewUserDialog_Previous(dlg);
  else if (strcasecmp(sender, "wiz_next_button")==0)
    return AO_NewUserDialog_Next(dlg);
  else if (strcasecmp(sender, "wiz_abort_button")==0) {
    AO_NewUserDialog_UndoIt(dlg);
    return GWEN_DialogEvent_ResultReject;
  }
  else if (strcasecmp(sender, "wiz_bank_button")==0)
    return AO_NewUserDialog_HandleActivatedBankSelect(dlg);
  else if (strcasecmp(sender, "wiz_app_combo")==0)
    return AO_NewUserDialog_HandleActivatedApp(dlg);
  else if (strcasecmp(sender, "wiz_special_button")==0)
    return AO_NewUserDialog_HandleActivatedSpecial(dlg);
  else if (strcasecmp(sender, "wiz_getaccounts_button")==0)
    return AO_NewUserDialog_HandleActivatedGetAccounts(dlg);
  else if (strcasecmp(sender, "wiz_help_button")==0) {
    /* TODO: open a help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int AO_NewUserDialog_HandleValueChanged(GWEN_DIALOG *dlg, const char *sender) {
  if (strcasecmp(sender, "wiz_username_edit")==0 ||
      strcasecmp(sender, "wiz_userid_edit")==0 ||
      strcasecmp(sender, "wiz_url_edit")==0 ||
      strcasecmp(sender, "wiz_brokerid_edit")==0 ||
      strcasecmp(sender, "wiz_fid_edit")==0 ||
      strcasecmp(sender, "wiz_org_edit")==0 ||
      strcasecmp(sender, "wiz_appid_edit")==0 ||
      strcasecmp(sender, "wiz_appver_edit")==0 ||
      strcasecmp(sender, "wiz_headerver_edit")==0) {
    int rv;

    if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_BANK) {
      rv=AO_NewUserDialog_GetBankPageData(dlg);
      if (rv<0)
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    else if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_USER) {
      rv=AO_NewUserDialog_GetUserPageData(dlg);
      if (rv<0)
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    else if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_APP) {
      rv=AO_NewUserDialog_GetAppPageData(dlg);
      if (rv<0)
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    return GWEN_DialogEvent_ResultHandled;
  }
  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB AO_NewUserDialog_SignalHandler(GWEN_DIALOG *dlg,
                                                 GWEN_DIALOG_EVENTTYPE t,
                                                 const char *sender) {
  AO_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    AO_NewUserDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AO_NewUserDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return AO_NewUserDialog_HandleValueChanged(dlg, sender);

  case GWEN_DialogEvent_TypeActivated:
    return AO_NewUserDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}




