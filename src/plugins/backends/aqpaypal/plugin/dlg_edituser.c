/***************************************************************************
 begin       : Tue Aug 03 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "provider_l.h"
#include "dlg_edituser_p.h"
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
#include <gwenhywfar/text.h>


#define DIALOG_MINWIDTH  400
#define DIALOG_MINHEIGHT 200



GWEN_INHERIT(GWEN_DIALOG, APY_EDITUSER_DIALOG)




GWEN_DIALOG *APY_EditUserDialog_new(AB_BANKING *ab, AB_USER *u, int doLock) {
  GWEN_DIALOG *dlg;
  APY_EDITUSER_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;
  const char *s;

  dlg=GWEN_Dialog_new("apy_edituser");
  GWEN_NEW_OBJECT(APY_EDITUSER_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg, xdlg,
		       APY_EditUserDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, APY_EditUserDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
			       "aqbanking/backends/aqpaypal/dialogs/dlg_edituser.dlg",
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

  /* preset */
  xdlg->doLock=doLock;
  xdlg->user=u;

  s=AB_User_GetUserName(u);
  if (s && *s) xdlg->userName=strdup(s);
  else xdlg->userName=NULL;

  s=AB_User_GetUserId(u);
  if (s && *s) xdlg->userId=strdup(s);
  else xdlg->userId=NULL;

  s=AB_User_GetCustomerId(u);
  if (s && *s) xdlg->customerId=strdup(s);
  else xdlg->customerId=NULL;

  s=APY_User_GetServerUrl(u);
  if (!(s && *s)) xdlg->url=strdup("https://api-3t.paypal.com/nvp");
  else xdlg->url=strdup(s);


  /* done */
  return dlg;
}



void GWENHYWFAR_CB APY_EditUserDialog_FreeData(void *bp, void *p) {
  APY_EDITUSER_DIALOG *xdlg;

  xdlg=(APY_EDITUSER_DIALOG*) p;
  free(xdlg->apiUserId);
  free(xdlg->apiPassword);
  free(xdlg->apiSignature);
  free(xdlg->userName);
  free(xdlg->userId);
  free(xdlg->url);
  GWEN_FREE_OBJECT(xdlg);
}



AB_USER *APY_EditUserDialog_GetUser(const GWEN_DIALOG *dlg) {
  APY_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->user;
}



const char *APY_EditUserDialog_GetApiUserId(const GWEN_DIALOG *dlg) {
  APY_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->apiUserId;
}



void APY_EditUserDialog_SetApiUserId(GWEN_DIALOG *dlg, const char *s) {
  APY_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->apiUserId);
  if (s) xdlg->apiUserId=strdup(s);
  else xdlg->apiUserId=NULL;
}



const char *APY_EditUserDialog_GetApiPassword(const GWEN_DIALOG *dlg) {
  APY_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->apiPassword;
}



void APY_EditUserDialog_SetApiPassword(GWEN_DIALOG *dlg, const char *s) {
  APY_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->apiPassword);
  if (s) xdlg->apiPassword=strdup(s);
  else xdlg->apiPassword=NULL;
}



const char *APY_EditUserDialog_GetApiSignature(const GWEN_DIALOG *dlg) {
  APY_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->apiSignature;
}



void APY_EditUserDialog_SetApiSignature(GWEN_DIALOG *dlg, const char *s) {
  APY_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->apiSignature);
  if (s) xdlg->apiSignature=strdup(s);
  else xdlg->apiSignature=NULL;
}



const char *APY_EditUserDialog_GetUserName(const GWEN_DIALOG *dlg) {
  APY_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->userName;
}



void APY_EditUserDialog_SetUserName(GWEN_DIALOG *dlg, const char *s) {
  APY_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userName);
  if (s) xdlg->userName=strdup(s);
  else xdlg->userName=NULL;
}



const char *APY_EditUserDialog_GetUserId(const GWEN_DIALOG *dlg) {
  APY_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->userId;
}



void APY_EditUserDialog_SetUserId(GWEN_DIALOG *dlg, const char *s) {
  APY_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userId);
  if (s) xdlg->userId=strdup(s);
  else xdlg->userId=NULL;
}



const char *APY_EditUserDialog_GetUrl(const GWEN_DIALOG *dlg) {
  APY_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->url;
}



void APY_EditUserDialog_SetUrl(GWEN_DIALOG *dlg, const char *s) {
  APY_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->url);
  if (s) xdlg->url=strdup(s);
  else xdlg->url=NULL;
}



uint32_t APY_EditUserDialog_GetFlags(const GWEN_DIALOG *dlg) {
  APY_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->flags;
}



void APY_EditUserDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  APY_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags=fl;
}



void APY_EditUserDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  APY_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}



void APY_EditUserDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  APY_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}






void APY_EditUserDialog_Init(GWEN_DIALOG *dlg) {
  APY_EDITUSER_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
			      "",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("Edit Paypal User"),
			      0);

  if (xdlg->userName)
    GWEN_Dialog_SetCharProperty(dlg,
				"wiz_username_edit",
				GWEN_DialogProperty_Value,
				0,
				xdlg->userName,
				0);

  if (xdlg->userId)
    GWEN_Dialog_SetCharProperty(dlg,
				"wiz_userid_edit",
				GWEN_DialogProperty_Value,
				0,
				xdlg->userId,
				0);

  if (xdlg->url)
    GWEN_Dialog_SetCharProperty(dlg,
				"wiz_url_edit",
				GWEN_DialogProperty_Value,
				0,
                                xdlg->url,
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



void APY_EditUserDialog_Fini(GWEN_DIALOG *dlg) {
  APY_EDITUSER_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
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



int APY_EditUserDialog_fromGui(GWEN_DIALOG *dlg, AB_USER *u, int quiet) {
  APY_EDITUSER_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_username_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    if (u)
      APY_EditUserDialog_SetUserName(dlg, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "Missing user name");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_userid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    if (u)
      APY_EditUserDialog_SetUserId(dlg, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "Missing user id");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_url_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    if (u)
      APY_EditUserDialog_SetUrl(dlg, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "Missing URL");
    return GWEN_ERROR_NO_DATA;
  }

  return 0;
}



int APY_EditUserDialog_HandleActivatedOk(GWEN_DIALOG *dlg) {
  APY_EDITUSER_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  rv=APY_EditUserDialog_fromGui(dlg, NULL, 0);
  if (rv<0) {
    return GWEN_DialogEvent_ResultHandled;
  }

  if (xdlg->doLock) {
    int rv;

    rv=AB_Banking_BeginExclUseUser(xdlg->banking, xdlg->user);
    if (rv<0) {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
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

  APY_EditUserDialog_fromGui(dlg, xdlg->user, 1);
  AB_User_SetUserName(xdlg->user, xdlg->userName);
  AB_User_SetUserId(xdlg->user, xdlg->userId);
  AB_User_SetCustomerId(xdlg->user, xdlg->userId);
  AB_User_SetCountry(xdlg->user, "de");
  AB_User_SetBankCode(xdlg->user, "PAYPAL");

  if (xdlg->doLock) {
    int rv;

    rv=AB_Banking_EndExclUseUser(xdlg->banking, xdlg->user, 0);
    if (rv<0) {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
      GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL |
			  GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
			  GWEN_GUI_MSG_FLAGS_CONFIRM_B1,
			  I18N("Error"),
			  I18N("Unable to unlock user."),
			  I18N("Dismiss"),
			  NULL,
			  NULL,
			  0);
      return GWEN_DialogEvent_ResultHandled;
    }
  }

  return GWEN_DialogEvent_ResultAccept;
}


int APY_EditUserDialog_HandleActivatedSecret(GWEN_DIALOG *dlg) {
  APY_EDITUSER_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;
  AB_USER *u;
  GWEN_BUFFER *xbuf, *tbuf;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  u=xdlg->user;

  dlg2=APY_EditSecretDialog_new(xdlg->banking);
  if (dlg2==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not create dialog");
    return GWEN_DialogEvent_ResultHandled;
  }

  xbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=APY_Provider_ReadUserApiSecrets(AB_User_GetProvider(u), u, xbuf);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    return rv;
  }
  else {
    char *t;
    char *t2=NULL;
    GWEN_BUFFER *sbuf1;
    GWEN_BUFFER *sbuf2;
    GWEN_BUFFER *sbuf3;

    t=strchr(GWEN_Buffer_GetStart(xbuf), ':');
    if (t) {
      *(t++)=0;
      t2=strchr(t, ':');
      if (t2) {
	*(t2++)=0;
      }
    }

    sbuf1=GWEN_Buffer_new(0, 256, 0, 1);
    sbuf2=GWEN_Buffer_new(0, 256, 0, 1);
    sbuf3=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Text_UnescapeToBufferTolerant(GWEN_Buffer_GetStart(xbuf), sbuf1);
    if (t) {
      GWEN_Text_UnescapeToBufferTolerant(t, sbuf2);
      t=GWEN_Buffer_GetStart(sbuf2);
      if (t2) {
	GWEN_Text_UnescapeToBufferTolerant(t2, sbuf3);
      }
    }
    APY_EditSecretDialog_SetApiUserId(dlg2,    GWEN_Buffer_GetStart(sbuf3));
    APY_EditSecretDialog_SetApiPassword(dlg2,  GWEN_Buffer_GetStart(sbuf1));
    APY_EditSecretDialog_SetApiSignature(dlg2, GWEN_Buffer_GetStart(sbuf2));

    GWEN_Buffer_free(xbuf);
    GWEN_Buffer_free(sbuf3);
    GWEN_Buffer_free(sbuf2);
    GWEN_Buffer_free(sbuf1);
  }

  rv=GWEN_Gui_ExecDialog(dlg2, 0);
  if (rv==0) {
    /* rejected */
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "Rejected");
    GWEN_Dialog_free(dlg2);
    return GWEN_DialogEvent_ResultHandled;
  }
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "Accepted");
    
    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Text_EscapeToBuffer(APY_EditSecretDialog_GetApiPassword(dlg2), tbuf);
    GWEN_Buffer_AppendByte(tbuf, ':');
    GWEN_Text_EscapeToBuffer(APY_EditSecretDialog_GetApiSignature(dlg2), tbuf);
    GWEN_Buffer_AppendByte(tbuf, ':');
    GWEN_Text_EscapeToBuffer(APY_EditSecretDialog_GetApiUserId(dlg2), tbuf);
    rv=APY_Provider_WriteUserApiSecrets(AB_User_GetProvider(u), u,
					GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
    if (rv<0) {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  GWEN_Dialog_free(dlg2);
  return GWEN_DialogEvent_ResultHandled;
}


int APY_EditUserDialog_GetSecretPageData(GWEN_DIALOG *dlg) {
  APY_EDITUSER_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  fprintf(stderr, "\n>>>> GetSecretPageData #1\n");
  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_apiuserid_edit", GWEN_DialogProperty_Value, 0, NULL);
  fprintf(stderr, "\n>>>> GetSecretPageData #2%s\n", s);
  if (s && *s)
    APY_EditUserDialog_SetApiUserId(dlg, s);
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "Missing API User ID");
    return GWEN_ERROR_NO_DATA;
  }
  fprintf(stderr, "\n>>>> GetSecretPageData #3\n");

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_apipass_edit", GWEN_DialogProperty_Value, 0, NULL);
  fprintf(stderr, "\n>>>> GetSecretPageData #4\n");
  if (s && *s)
    APY_EditUserDialog_SetApiPassword(dlg, s);
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "Missing API Password");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_apisig_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    APY_EditUserDialog_SetApiSignature(dlg, s);
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "Missing API Signature");
    return GWEN_ERROR_NO_DATA;
  }

  return 0;
}



int APY_EditUserDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  DBG_ERROR(0, "Activated: %s", sender);
  if (strcasecmp(sender, "okButton")==0)
    return APY_EditUserDialog_HandleActivatedOk(dlg);
  else if (strcasecmp(sender, "abortButton")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "wiz_secret_button")==0) {
    APY_EditUserDialog_HandleActivatedSecret(dlg);
  }
  else if (strcasecmp(sender, "wiz_help_button")==0) {
    /* TODO: open a help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB APY_EditUserDialog_SignalHandler(GWEN_DIALOG *dlg,
						   GWEN_DIALOG_EVENTTYPE t,
						   const char *sender) {
  APY_EDITUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, APY_EDITUSER_DIALOG, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    APY_EditUserDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;

  case GWEN_DialogEvent_TypeFini:
    APY_EditUserDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;

  case GWEN_DialogEvent_TypeValueChanged:
    return GWEN_DialogEvent_ResultHandled;

  case GWEN_DialogEvent_TypeActivated:
    return APY_EditUserDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}




