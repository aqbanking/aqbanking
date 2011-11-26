/***************************************************************************
 begin       : Tue Aug 24 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_edituser_p.h"
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



GWEN_INHERIT(GWEN_DIALOG, AO_EDITUSER_DIALOG)





GWEN_DIALOG *AO_EditUserDialog_new(AB_BANKING *ab, AB_USER *u, int doLock) {
  GWEN_DIALOG *dlg;
  AO_EDITUSER_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("ao_newuser");
  GWEN_NEW_OBJECT(AO_EDITUSER_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg, xdlg,
		       AO_EditUserDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, AO_EditUserDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
			       "aqbanking/backends/aqofxconnect/dialogs/dlg_edituser.dlg",
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
  xdlg->banking=ab;
  xdlg->user=u;
  xdlg->doLock=doLock;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB AO_EditUserDialog_FreeData(void *bp, void *p) {
  AO_EDITUSER_DIALOG *xdlg;

  xdlg=(AO_EDITUSER_DIALOG*) p;
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



AB_USER *AO_EditUserDialog_GetUser(const GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->user;
}



const char *AO_EditUserDialog_GetBankName(const GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->bankName;
}



void AO_EditUserDialog_SetBankName(GWEN_DIALOG *dlg, const char *s) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->bankName);
  if (s) xdlg->bankName=strdup(s);
  else xdlg->bankName=NULL;
}



const char *AO_EditUserDialog_GetUserName(const GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->userName;
}



void AO_EditUserDialog_SetUserName(GWEN_DIALOG *dlg, const char *s) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userName);
  if (s) xdlg->userName=strdup(s);
  else xdlg->userName=NULL;
}



const char *AO_EditUserDialog_GetUserId(const GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->userId;
}



void AO_EditUserDialog_SetUserId(GWEN_DIALOG *dlg, const char *s) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userId);
  if (s) xdlg->userId=strdup(s);
  else xdlg->userId=NULL;
}



const char *AO_EditUserDialog_GetFid(const GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->fid;
}



void AO_EditUserDialog_SetFid(GWEN_DIALOG *dlg, const char *s) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->fid);
  if (s) xdlg->fid=strdup(s);
  else xdlg->fid=NULL;
}



const char *AO_EditUserDialog_GetOrg(const GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->org;
}



void AO_EditUserDialog_SetOrg(GWEN_DIALOG *dlg, const char *s) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->org);
  if (s) xdlg->org=strdup(s);
  else xdlg->org=NULL;
}



const char *AO_EditUserDialog_GetAppId(const GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->appId;
}



void AO_EditUserDialog_SetAppId(GWEN_DIALOG *dlg, const char *s) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->appId);
  if (s) xdlg->appId=strdup(s);
  else xdlg->appId=NULL;
}



const char *AO_EditUserDialog_GetAppVer(const GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->appVer;
}



void AO_EditUserDialog_SetAppVer(GWEN_DIALOG *dlg, const char *s) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->appVer);
  if (s) xdlg->appVer=strdup(s);
  else xdlg->appVer=NULL;
}



const char *AO_EditUserDialog_GetHeaderVer(const GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->headerVer;
}



void AO_EditUserDialog_SetHeaderVer(GWEN_DIALOG *dlg, const char *s) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->headerVer);
  if (s) xdlg->headerVer=strdup(s);
  else xdlg->headerVer=NULL;
}



const char *AO_EditUserDialog_GetBrokerId(const GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->brokerId;
}



void AO_EditUserDialog_SetBrokerId(GWEN_DIALOG *dlg, const char *s) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->brokerId);
  if (s) xdlg->brokerId=strdup(s);
  else xdlg->brokerId=NULL;
}



const char *AO_EditUserDialog_GetUrl(const GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->url;
}



void AO_EditUserDialog_SetUrl(GWEN_DIALOG *dlg, const char *s) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->url);
  if (s) xdlg->url=strdup(s);
  else xdlg->url=NULL;
}



const char *AO_EditUserDialog_GetClientUid(const GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->clientUid;
}



void AO_EditUserDialog_SetClientUid(GWEN_DIALOG *dlg, const char *s) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->clientUid);
  if (s) xdlg->clientUid=strdup(s);
  else xdlg->clientUid=NULL;
}



int AO_EditUserDialog_GetHttpVMajor(const GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->httpVMajor;
}



int AO_EditUserDialog_GetHttpVMinor(const GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->httpVMinor;
}



void AO_EditUserDialog_SetHttpVersion(GWEN_DIALOG *dlg, int vmajor, int vminor) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  xdlg->httpVMajor=vmajor;
  xdlg->httpVMinor=vminor;
}



uint32_t AO_EditUserDialog_GetFlags(const GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->flags;
}



void AO_EditUserDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags=fl;
}



void AO_EditUserDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}



void AO_EditUserDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}






void AO_EditUserDialog_Init(GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;
  AB_PROVIDER *pro;
  GWEN_DB_NODE *dbPrefs;
  const char *s;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
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
			      I18N("OFX DirectConnect User Setup"),
			      0);

  /* select first page */
  GWEN_Dialog_SetIntProperty(dlg, "wiz_tab_book", GWEN_DialogProperty_Value, 0, 0, 0);

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

  s=AB_User_GetUserName(xdlg->user);
  GWEN_Dialog_SetCharProperty(dlg, "wiz_username_edit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AB_User_GetUserId(xdlg->user);
  GWEN_Dialog_SetCharProperty(dlg, "wiz_userid_edit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AO_User_GetClientUid(xdlg->user);
  GWEN_Dialog_SetCharProperty(dlg, "wiz_clientuid_edit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AO_User_GetBankName(xdlg->user);
  GWEN_Dialog_SetCharProperty(dlg, "wiz_bankname_edit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AO_User_GetBrokerId(xdlg->user);
  GWEN_Dialog_SetCharProperty(dlg, "wiz_brokerid_edit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AO_User_GetFid(xdlg->user);
  GWEN_Dialog_SetCharProperty(dlg, "wiz_fid_edit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AO_User_GetOrg(xdlg->user);
  GWEN_Dialog_SetCharProperty(dlg, "wiz_org_edit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AO_User_GetServerAddr(xdlg->user);
  GWEN_Dialog_SetCharProperty(dlg, "wiz_url_edit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AO_User_GetAppId(xdlg->user);
  if (s && *s) {
    GWEN_Dialog_SetIntProperty(dlg, "wiz_app_combo", GWEN_DialogProperty_Value, 0, 0, 0);
    GWEN_Dialog_SetCharProperty(dlg, "wiz_appid_edit", GWEN_DialogProperty_Value, 0, s, 0);
  }

  s=AO_User_GetAppVer(xdlg->user);
  if (s && *s) {
    GWEN_Dialog_SetIntProperty(dlg, "wiz_app_combo", GWEN_DialogProperty_Value, 0, 0, 0);
    GWEN_Dialog_SetCharProperty(dlg, "wiz_appver_edit", GWEN_DialogProperty_Value, 0, s, 0);
  }

  s=AO_User_GetHeaderVer(xdlg->user);
  if (!(s && *s))
    s="102";
  GWEN_Dialog_SetCharProperty(dlg, "wiz_headerver_edit", GWEN_DialogProperty_Value, 0, s, 0);

  xdlg->flags=AO_User_GetFlags(xdlg->user);

  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);
}



void AO_EditUserDialog_Fini(GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
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



int AO_EditUserDialog_GetBankPageData(GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_bankname_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_EditUserDialog_SetBankName(dlg, s);
  else {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Missing bank name");
    /* switch to correct page, show error message, set focus */
    GWEN_Dialog_SetIntProperty(dlg, "wiz_tab_book", GWEN_DialogProperty_Value, 0, 1, 0);
    GWEN_Gui_ShowError(I18N("Missing Input"), I18N("Please enter the name of your bank."));
    GWEN_Dialog_SetIntProperty(dlg, "wiz_bankname_edit", GWEN_DialogProperty_Focus, 0, 1, 0);
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_brokerid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_EditUserDialog_SetBrokerId(dlg, s);
  else
    AO_EditUserDialog_SetBrokerId(dlg, NULL);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_fid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_EditUserDialog_SetFid(dlg, s);
  else {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Missing FID");
    /* switch to correct page, show error message, set focus */
    GWEN_Dialog_SetIntProperty(dlg, "wiz_tab_book", GWEN_DialogProperty_Value, 0, 1, 0);
    GWEN_Gui_ShowError(I18N("Missing Input"), I18N("Please enter a valid FID code."));
    GWEN_Dialog_SetIntProperty(dlg, "wiz_fid_edit", GWEN_DialogProperty_Focus, 0, 1, 0);
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_org_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_EditUserDialog_SetOrg(dlg, s);
  else {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Missing ORG");
    /* switch to correct page, show error message, set focus */
    GWEN_Dialog_SetIntProperty(dlg, "wiz_tab_book", GWEN_DialogProperty_Value, 0, 1, 0);
    GWEN_Gui_ShowError(I18N("Missing Input"), I18N("Please enter a valid ORG code."));
    GWEN_Dialog_SetIntProperty(dlg, "wiz_org_edit", GWEN_DialogProperty_Focus, 0, 1, 0);
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_url_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_EditUserDialog_SetUrl(dlg, s);
  else {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Missing URL");
    /* switch to correct page, show error message, set focus */
    GWEN_Dialog_SetIntProperty(dlg, "wiz_tab_book", GWEN_DialogProperty_Value, 0, 1, 0);
    GWEN_Gui_ShowError(I18N("Missing Input"), I18N("Please enter a server address."));
    GWEN_Dialog_SetIntProperty(dlg, "wiz_url_edit", GWEN_DialogProperty_Focus, 0, 1, 0);
    return GWEN_ERROR_NO_DATA;
  }

  return 0;
}



int AO_EditUserDialog_GetUserPageData(GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_username_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_EditUserDialog_SetUserName(dlg, s);
  else {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Missing user name");
    /* switch to correct page, show error message, set focus */
    GWEN_Dialog_SetIntProperty(dlg, "wiz_tab_book", GWEN_DialogProperty_Value, 0, 0, 0);
    GWEN_Gui_ShowError(I18N("Missing Input"), I18N("Please enter your name."));
    GWEN_Dialog_SetIntProperty(dlg, "wiz_username_edit", GWEN_DialogProperty_Focus, 0, 1, 0);
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_userid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_EditUserDialog_SetUserId(dlg, s);
  else {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Missing user id");
    /* switch to correct page, show error message, set focus */
    GWEN_Dialog_SetIntProperty(dlg, "wiz_tab_book", GWEN_DialogProperty_Value, 0, 0, 0);
    GWEN_Gui_ShowError(I18N("Missing Input"), I18N("Please enter your User ID."));
    GWEN_Dialog_SetIntProperty(dlg, "wiz_userid_edit", GWEN_DialogProperty_Focus, 0, 1, 0);
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_clientuid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_EditUserDialog_SetClientUid(dlg, s);
  else
    AO_EditUserDialog_SetClientUid(dlg, NULL);

  return 0;
}



int AO_EditUserDialog_GetAppPageData(GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_appid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_EditUserDialog_SetAppId(dlg, s);
  else {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Missing application id");
    /* switch to correct page, show error message, set focus */
    GWEN_Dialog_SetIntProperty(dlg, "wiz_tab_book", GWEN_DialogProperty_Value, 0, 2, 0);
    GWEN_Gui_ShowError(I18N("Missing Input"), I18N("Please select a valid application to emulate."));
    GWEN_Dialog_SetIntProperty(dlg, "wiz_app_combo", GWEN_DialogProperty_Focus, 0, 1, 0);
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_appver_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_EditUserDialog_SetAppVer(dlg, s);
  else {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Missing application version");
    /* switch to correct page, show error message, set focus */
    GWEN_Dialog_SetIntProperty(dlg, "wiz_tab_book", GWEN_DialogProperty_Value, 0, 2, 0);
    GWEN_Gui_ShowError(I18N("Missing Input"), I18N("Please select a valid application to emulate."));
    GWEN_Dialog_SetIntProperty(dlg, "wiz_app_combo", GWEN_DialogProperty_Focus, 0, 1, 0);
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_headerver_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AO_EditUserDialog_SetHeaderVer(dlg, s);
  else {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Missing application version");
    /* switch to correct page, show error message, set focus */
    GWEN_Dialog_SetIntProperty(dlg, "wiz_tab_book", GWEN_DialogProperty_Value, 0, 2, 0);
    GWEN_Gui_ShowError(I18N("Missing Input"), I18N("Please enter a correct header version (default is 102)."));
    GWEN_Dialog_SetIntProperty(dlg, "wiz_headerver_edit", GWEN_DialogProperty_Focus, 0, 1, 0);
    return GWEN_ERROR_NO_DATA;
  }

  return 0;
}



int AO_EditUserDialog_FromGui(GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  rv=AO_EditUserDialog_GetBankPageData(dlg);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    return GWEN_ERROR_BAD_DATA;
  }

  rv=AO_EditUserDialog_GetUserPageData(dlg);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    return GWEN_ERROR_BAD_DATA;
  }

  rv=AO_EditUserDialog_GetAppPageData(dlg);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    return GWEN_ERROR_BAD_DATA;
  }

  /* lock new user */
  if (xdlg->doLock) {
    DBG_ERROR(0, "Locking user");
    rv=AB_Banking_BeginExclUseUser(xdlg->banking, xdlg->user);
    if (rv<0) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Could not lock user (%d)", rv);
      return rv;
    }
  }

  /* generic setup */
  AB_User_SetUserName(xdlg->user, xdlg->userName);
  AB_User_SetUserId(xdlg->user, xdlg->userId);
  AB_User_SetCustomerId(xdlg->user, xdlg->userId);
  AB_User_SetCountry(xdlg->user, "us");
  AB_User_SetBankCode(xdlg->user, "0000000000");

  AO_User_SetFlags(xdlg->user, xdlg->flags);
  AO_User_SetBankName(xdlg->user, xdlg->bankName);
  AO_User_SetBrokerId(xdlg->user, xdlg->brokerId);
  AO_User_SetOrg(xdlg->user, xdlg->org);
  AO_User_SetFid(xdlg->user, xdlg->fid);

  AO_User_SetAppId(xdlg->user, xdlg->appId);
  AO_User_SetAppVer(xdlg->user, xdlg->appVer);
  AO_User_SetHeaderVer(xdlg->user, xdlg->headerVer);
  AO_User_SetClientUid(xdlg->user, xdlg->clientUid);
  AO_User_SetSecurityType(xdlg->user, xdlg->securityType);

  AO_User_SetServerAddr(xdlg->user, xdlg->url);
  AO_User_SetHttpVMajor(xdlg->user, xdlg->httpVMajor);
  AO_User_SetHttpVMinor(xdlg->user, xdlg->httpVMinor);

  if (xdlg->doLock) {
    /* unlock user */
    DBG_ERROR(0, "Unlocking user");
    rv=AB_Banking_EndExclUseUser(xdlg->banking, xdlg->user, 0);
    if (rv<0) {
      DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
	       "Could not unlock user [%s] (%d)",
	       AB_User_GetUserId(xdlg->user), rv);
      AB_Banking_EndExclUseUser(xdlg->banking, xdlg->user, 1);
      return rv;
    }
  }

  return 0;
}




int AO_EditUserDialog_HandleActivatedSpecial(GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
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



int AO_EditUserDialog_HandleActivatedBankSelect(GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;
  int rv;
  GWEN_DIALOG *dlg2;
  GWEN_BUFFER *tbuf;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
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
      rv=AO_EditUserDialog_GetBankPageData(dlg);
      if (rv<0)
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
  }
  GWEN_Dialog_free(dlg2);
  return GWEN_DialogEvent_ResultHandled;
}



int AO_EditUserDialog_HandleActivatedApp(GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;
  AB_PROVIDER *pro;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
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



int AO_EditUserDialog_HandleActivatedGetAccounts(GWEN_DIALOG *dlg) {
  AO_EDITUSER_DIALOG *xdlg;
  AB_PROVIDER *pro;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
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




int AO_EditUserDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  DBG_ERROR(0, "Activated: %s", sender);
  if (strcasecmp(sender, "abortButton")==0) {
    return GWEN_DialogEvent_ResultReject;
  }
  else if (strcasecmp(sender, "okButton")==0) {
    int rv;

    rv=AO_EditUserDialog_GetBankPageData(dlg);
    if (rv<0) {
      DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
      return GWEN_DialogEvent_ResultHandled;
    }

    rv=AO_EditUserDialog_GetUserPageData(dlg);
    if (rv<0) {
      DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
      return GWEN_DialogEvent_ResultHandled;
    }

    rv=AO_EditUserDialog_GetAppPageData(dlg);
    if (rv<0) {
      DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
      return GWEN_DialogEvent_ResultHandled;
    }

    rv=AO_EditUserDialog_FromGui(dlg);
    if (rv<0) {
      DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
      return GWEN_DialogEvent_ResultHandled;
    }
    return GWEN_DialogEvent_ResultAccept;
  }
  else if (strcasecmp(sender, "wiz_bank_button")==0)
    return AO_EditUserDialog_HandleActivatedBankSelect(dlg);
  else if (strcasecmp(sender, "wiz_app_combo")==0)
    return AO_EditUserDialog_HandleActivatedApp(dlg);
  else if (strcasecmp(sender, "wiz_special_button")==0)
    return AO_EditUserDialog_HandleActivatedSpecial(dlg);
  else if (strcasecmp(sender, "wiz_getaccounts_button")==0)
    return AO_EditUserDialog_HandleActivatedGetAccounts(dlg);
  else if (strcasecmp(sender, "wiz_help_button")==0) {
    /* TODO: open a help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int AO_EditUserDialog_HandleValueChanged(GWEN_DIALOG *dlg, const char *sender) {
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
      rv=AO_EditUserDialog_GetBankPageData(dlg);
      if (rv<0)
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    else if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_USER) {
      rv=AO_EditUserDialog_GetUserPageData(dlg);
      if (rv<0)
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    else if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_APP) {
      rv=AO_EditUserDialog_GetAppPageData(dlg);
      if (rv<0)
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    return GWEN_DialogEvent_ResultHandled;
  }
  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB AO_EditUserDialog_SignalHandler(GWEN_DIALOG *dlg,
                                                 GWEN_DIALOG_EVENTTYPE t,
                                                 const char *sender) {
  AO_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    AO_EditUserDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AO_EditUserDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return AO_EditUserDialog_HandleValueChanged(dlg, sender);

  case GWEN_DialogEvent_TypeActivated:
    return AO_EditUserDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}

