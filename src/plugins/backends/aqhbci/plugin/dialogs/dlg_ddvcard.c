/***************************************************************************
 begin       : Tue Apr 20 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_ddvcard_p.h"
#include "i18n_l.h"

#include <aqbanking/dlg_selectbankinfo.h>
#include <aqbanking/user.h>
#include <aqbanking/banking_be.h>

#include <aqhbci/user.h>
#include <aqhbci/provider.h>
#include "dlg_ddvcard_special_l.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>


#define PAGE_BEGIN     0
#define PAGE_BANK      1
#define PAGE_USER      2
#define PAGE_CREATE    3
#define PAGE_END       4


#define DIALOG_MINWIDTH  400
#define DIALOG_MINHEIGHT 200


#define MAX_CONTEXT_ID_ENTRIES 64


GWEN_INHERIT(GWEN_DIALOG, AH_DDVCARD_DIALOG)




GWEN_DIALOG *AH_DdvCardDialog_new(AB_BANKING *ab, GWEN_CRYPT_TOKEN *ct) {
  GWEN_DIALOG *dlg;
  AH_DDVCARD_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("ah_setup_ddvcard");
  GWEN_NEW_OBJECT(AH_DDVCARD_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg, xdlg,
		       AH_DdvCardDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, AH_DdvCardDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
			       "aqbanking/backends/aqhbci/dialogs/dlg_ddvcard.dlg",
			       fbuf);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Dialog description file not found (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }

  /* read dialog from dialog description file */
  rv=GWEN_Dialog_ReadXmlFile(dlg, GWEN_Buffer_GetStart(fbuf));
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }
  GWEN_Buffer_free(fbuf);

  xdlg->banking=ab;
  xdlg->cryptToken=ct;
  xdlg->contextList=GWEN_Crypt_Token_Context_List_new();

  if (1) {
    uint32_t idList[MAX_CONTEXT_ID_ENTRIES];
    uint32_t idCount;
    uint32_t i;

    if (!GWEN_Crypt_Token_IsOpen(ct)) {
      rv=GWEN_Crypt_Token_Open(ct, 0, 0);
      if (rv<0) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Error opening token (%d)", rv);
	GWEN_Gui_ShowError(I18N("Error"), I18N("Could not contact card. Maybe removed? (%d)"), rv);
	GWEN_Dialog_free(dlg);
	return NULL;
      }
    }

    idCount=MAX_CONTEXT_ID_ENTRIES;
    rv=GWEN_Crypt_Token_GetContextIdList(ct, idList, &idCount, 0);
    if (rv<0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not read context id list");
      GWEN_Dialog_free(dlg);
      GWEN_Gui_ShowError(I18N("Error"), I18N("Could not read context id list from card (%d)"), rv);
      return NULL;
    }

    for (i=0; i<idCount; i++) {
      const GWEN_CRYPT_TOKEN_CONTEXT *ctx;

      ctx=GWEN_Crypt_Token_GetContext(ct, idList[i], 0);
      if (ctx) {
	GWEN_CRYPT_TOKEN_CONTEXT *nctx;

	nctx=GWEN_Crypt_Token_Context_dup(ctx);
	GWEN_Crypt_Token_Context_List_Add(nctx, xdlg->contextList);
        DBG_INFO(AQHBCI_LOGDOMAIN, "Added context %08x", idList[i]);
      }
    }
  } /* for */

  /* preset */
  xdlg->hbciVersion=210;
  xdlg->flags=0;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB AH_DdvCardDialog_FreeData(void *bp, void *p) {
  AH_DDVCARD_DIALOG *xdlg;

  xdlg=(AH_DDVCARD_DIALOG*) p;
  GWEN_Crypt_Token_Context_List_free(xdlg->contextList);
  free(xdlg->bankCode);
  free(xdlg->bankName);
  free(xdlg->userName);
  free(xdlg->userId);
  free(xdlg->customerId);
  free(xdlg->peerId);
  GWEN_FREE_OBJECT(xdlg);
}



AB_USER *AH_DdvCardDialog_GetUser(const GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->user;
}



GWEN_CRYPT_TOKEN *AH_DdvCardDialog_GetCryptToken(const GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->cryptToken;
}



const char *AH_DdvCardDialog_GetBankCode(const GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->bankCode;
}



void AH_DdvCardDialog_SetBankCode(GWEN_DIALOG *dlg, const char *s) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->bankCode);
  if (s) xdlg->bankCode=strdup(s);
  else xdlg->bankCode=NULL;
}



const char *AH_DdvCardDialog_GetBankName(const GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->bankName;
}



void AH_DdvCardDialog_SetBankName(GWEN_DIALOG *dlg, const char *s) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->bankName);
  if (s) xdlg->bankName=strdup(s);
  else xdlg->bankName=NULL;
}



const char *AH_DdvCardDialog_GetUserName(const GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->userName;
}



void AH_DdvCardDialog_SetUserName(GWEN_DIALOG *dlg, const char *s) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userName);
  if (s) xdlg->userName=strdup(s);
  else xdlg->userName=NULL;
}



const char *AH_DdvCardDialog_GetUserId(const GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->userId;
}



void AH_DdvCardDialog_SetUserId(GWEN_DIALOG *dlg, const char *s) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userId);
  if (s) xdlg->userId=strdup(s);
  else xdlg->userId=NULL;
}



const char *AH_DdvCardDialog_GetCustomerId(const GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->customerId;
}



void AH_DdvCardDialog_SetCustomerId(GWEN_DIALOG *dlg, const char *s) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->customerId);
  if (s) xdlg->customerId=strdup(s);
  else xdlg->customerId=NULL;
}



const char *AH_DdvCardDialog_GetUrl(const GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->url;
}



void AH_DdvCardDialog_SetUrl(GWEN_DIALOG *dlg, const char *s) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->url);
  if (s) xdlg->url=strdup(s);
  else xdlg->url=NULL;
}



const char *AH_DdvCardDialog_GetPeerId(const GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->peerId;
}



void AH_DdvCardDialog_SetPeerId(GWEN_DIALOG *dlg, const char *s) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->peerId);
  if (s) xdlg->peerId=strdup(s);
  else xdlg->peerId=NULL;
}


int AH_DdvCardDialog_GetHbciVersion(const GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->hbciVersion;
}



void AH_DdvCardDialog_SetHbciVersion(GWEN_DIALOG *dlg, int i) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  xdlg->hbciVersion=i;
}



uint32_t AH_DdvCardDialog_GetFlags(const GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->flags;
}



void AH_DdvCardDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags=fl;
}



void AH_DdvCardDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}



void AH_DdvCardDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}






void AH_DdvCardDialog_Init(GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
			      "",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("HBCI DDV-Card Setup Wizard"),
			      0);

  /* select first page */
  GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, 0, 0);

  /* setup intro page */
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_begin_label",
			      GWEN_DialogProperty_Title,
			      0,
                              I18N("<html>"
                                   "<p>This dialog assists you in setting up a DDV Chipcard User.</p>"
				   "<p>Some chipcards contain user information. You can click the button below "
                                   "to read that information from the card.</p>"
                                   "</html>"
                                   "This dialog assists you in setting up a DDV Chipcard User.\n"
                                   "Some chipcards contain user information. You can click the button below\n"
                                   "to read that information from the card."),
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

  GWEN_Dialog_SetIntProperty(dlg, "wiz_context_combo", GWEN_DialogProperty_ClearValues, 0, 0, 0);
  if (1) {
    const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
    int i;

    i=1;
    ctx=GWEN_Crypt_Token_Context_List_First(xdlg->contextList);
    while(ctx) {
      char numbuf[64];
      GWEN_BUFFER *tbuf;
      const char *s;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      snprintf(numbuf, sizeof(numbuf)-1, I18N("Context %d:"), i);
      numbuf[sizeof(numbuf)-1]=0;
      GWEN_Buffer_AppendString(tbuf, numbuf);

      s=GWEN_Crypt_Token_Context_GetServiceId(ctx);
      if (s && *s && strcasecmp(s, "20202020")!=0)
	GWEN_Buffer_AppendString(tbuf, s);
      else
	GWEN_Buffer_AppendString(tbuf, I18N("<no bank code>"));
      GWEN_Buffer_AppendString(tbuf, "-");

      s=GWEN_Crypt_Token_Context_GetUserId(ctx);
      if (s && *s)
	GWEN_Buffer_AppendString(tbuf, s);
      else
	GWEN_Buffer_AppendString(tbuf, I18N("<no user id>"));

      GWEN_Dialog_SetCharProperty(dlg, "wiz_context_combo", GWEN_DialogProperty_AddValue, 0, GWEN_Buffer_GetStart(tbuf), 0);
      GWEN_Buffer_free(tbuf);

      i++;
      ctx=GWEN_Crypt_Token_Context_List_Next(ctx);
    }
  }
  GWEN_Dialog_SetIntProperty(dlg, "wiz_context_combo", GWEN_DialogProperty_Value, 0, 0, 0);
  AH_DdvCardDialog_FromContext(dlg, 0);

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



void AH_DdvCardDialog_Fini(GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
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



int AH_DdvCardDialog_GetBankPageData(GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_bankcode_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AH_DdvCardDialog_SetBankCode(dlg, s);
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Missing bank code");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_bankname_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AH_DdvCardDialog_SetBankName(dlg, s);
  else
    AH_DdvCardDialog_SetBankName(dlg, NULL);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_url_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AH_DdvCardDialog_SetUrl(dlg, s);
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Missing URL");
    return GWEN_ERROR_NO_DATA;
  }

  return 0;
}



int AH_DdvCardDialog_GetUserPageData(GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_username_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AH_DdvCardDialog_SetUserName(dlg, s);
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Missing user name");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_userid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AH_DdvCardDialog_SetUserId(dlg, s);
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Missing user id");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_customerid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AH_DdvCardDialog_SetCustomerId(dlg, s);
  else
    AH_DdvCardDialog_SetCustomerId(dlg, NULL);

  return 0;
}



int AH_DdvCardDialog_EnterPage(GWEN_DIALOG *dlg, int page, int forwards) {
  AH_DDVCARD_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  switch(page) {
  case PAGE_BEGIN:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_BANK:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    rv=AH_DdvCardDialog_GetBankPageData(dlg);
    if (rv<0)
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_USER:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    rv=AH_DdvCardDialog_GetUserPageData(dlg);
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



int AH_DdvCardDialog_DoIt(GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;
  AB_USER *u;
  GWEN_URL *url;
  int rv;
  uint32_t pid;
  AB_IMEXPORTER_CONTEXT *ctx;
  AB_PROVIDER *pro;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  pro=AB_Banking_GetProvider(xdlg->banking, "aqhbci");
  if (pro==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not find backend, maybe some plugins are not installed?");
    GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Could not find backend, maybe some plugins are not installed?"));
    return GWEN_DialogEvent_ResultHandled;
  }

  u=AB_Banking_CreateUser(xdlg->banking, "aqhbci");
  if (u==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create user, maybe backend missing?");
    GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Could not create user, maybe some plugins are not installed?"));
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

  /* HBCI setup */
  AH_User_SetTokenType(u, "ddvcard");
  AH_User_SetTokenName(u, GWEN_Crypt_Token_GetTokenName(xdlg->cryptToken));
  AH_User_SetCryptMode(u, AH_CryptMode_Ddv);
  AH_User_SetTokenContextId(u, 1);
  AH_User_SetStatus(u, AH_UserStatusEnabled);

  url=GWEN_Url_fromString(xdlg->url);
  assert(url);
  GWEN_Url_SetProtocol(url, "hbci");
  if (GWEN_Url_GetPort(url)==0)
    GWEN_Url_SetPort(url, 3000);
  AH_User_SetServerUrl(u, url);
  GWEN_Url_free(url);
  AH_User_SetHbciVersion(u, xdlg->hbciVersion);

  rv=AB_Banking_AddUser(xdlg->banking, u);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not add user (%d)", rv);
    AB_User_free(u);
    GWEN_Gui_ShowError(I18N("Error"), I18N("Could not add user (%d)"), rv);
    return GWEN_DialogEvent_ResultHandled;
  }

  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
			     GWEN_GUI_PROGRESS_ALLOW_EMBED |
			     GWEN_GUI_PROGRESS_SHOW_PROGRESS |
			     GWEN_GUI_PROGRESS_SHOW_ABORT,
			     I18N("Setting Up DDV User"),
			     I18N("The list of accounts will be retrieved."),
			     1,
			     0);
  /* lock new user */
  rv=AB_Banking_BeginExclUseUser(xdlg->banking, u);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not lock user (%d)", rv);
    GWEN_Gui_ProgressLog2(pid,
			  GWEN_LoggerLevel_Error,
			  I18N("Unable to lock users (%d)"), rv);
    AB_Banking_DeleteUser(xdlg->banking, u);
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }

  /* get account list */
  GWEN_Gui_ProgressLog(pid,
		       GWEN_LoggerLevel_Notice,
		       I18N("Retrieving account list"));
  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetAccounts(pro, u, ctx, 0, 1, 0);
  if (rv<0) {
    AB_Banking_EndExclUseUser(xdlg->banking, u, 1);
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_Banking_DeleteUser(xdlg->banking, u);
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }

  rv=GWEN_Gui_ProgressAdvance(pid, GWEN_GUI_PROGRESS_ONE);
  if (rv==GWEN_ERROR_USER_ABORTED) {
    AB_Banking_EndExclUseUser(xdlg->banking, u, 1);
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_Banking_DeleteUser(xdlg->banking, u);
    GWEN_Gui_ProgressLog(pid,
			 GWEN_LoggerLevel_Error,
			 I18N("Aborted by user."));
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }

  /* unlock user */
  rv=AB_Banking_EndExclUseUser(xdlg->banking, u, 0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
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

  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_end_label",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("The user has been successfully setup."),
			      0);
  GWEN_Gui_ProgressEnd(pid);
  AH_DdvCardDialog_EnterPage(dlg, PAGE_END, 1);

  xdlg->user=u;

  return GWEN_DialogEvent_ResultHandled;
}



int AH_DdvCardDialog_Next(GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  if (page==PAGE_CREATE) {
    return AH_DdvCardDialog_DoIt(dlg);
  }
  else if (page<PAGE_END) {
    page++;
    return AH_DdvCardDialog_EnterPage(dlg, page, 1);
  }
  else if (page==PAGE_END)
    return GWEN_DialogEvent_ResultAccept;

  return GWEN_DialogEvent_ResultHandled;
}



int AH_DdvCardDialog_Previous(GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  if (page>PAGE_BEGIN) {
    page--;
    return AH_DdvCardDialog_EnterPage(dlg, page, 0);
  }

  return GWEN_DialogEvent_ResultHandled;
}



int AH_DdvCardDialog_HandleActivatedBankCode(GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  dlg2=AB_SelectBankInfoDialog_new(xdlg->banking, "de", NULL);
  if (dlg2==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create dialog");
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
	if (s && *s && strcasecmp(s, "HBCI")==0) {
	  s=AB_BankInfoService_GetMode(sv);
	  if (s && *s && strcasecmp(s, "ddv")==0)
	    break;
	}
	sv=AB_BankInfoService_List_Next(sv);
      }

      if (sv) {
	/* DDV service found */
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

  if (AH_DdvCardDialog_GetBankPageData(dlg)<0)
    GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
  else
    GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);

  return GWEN_DialogEvent_ResultHandled;
}



int AH_DdvCardDialog_HandleActivatedSpecial(GWEN_DIALOG *dlg) {
  AH_DDVCARD_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  dlg2=AH_DdvCardSpecialDialog_new(xdlg->banking);
  if (dlg2==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create dialog");
    GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Could not create dialog, maybe an installation error?"));
    return GWEN_DialogEvent_ResultHandled;
  }

  AH_DdvCardSpecialDialog_SetHbciVersion(dlg2, xdlg->hbciVersion);
  AH_DdvCardSpecialDialog_SetFlags(dlg2, xdlg->flags);

  rv=GWEN_Gui_ExecDialog(dlg2, 0);
  if (rv==0) {
    /* rejected */
    GWEN_Dialog_free(dlg2);
    return GWEN_DialogEvent_ResultHandled;
  }
  else {
    xdlg->hbciVersion=AH_DdvCardSpecialDialog_GetHbciVersion(dlg2);
    xdlg->flags=AH_DdvCardSpecialDialog_GetFlags(dlg2);
  }

  GWEN_Dialog_free(dlg2);

  return GWEN_DialogEvent_ResultHandled;
}



int AH_DdvCardDialog_FromContext(GWEN_DIALOG *dlg, int i) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  if (i>=0) {
    GWEN_CRYPT_TOKEN_CONTEXT *ctx;

    ctx=GWEN_Crypt_Token_Context_List_First(xdlg->contextList);
    while(ctx && i--)
      ctx=GWEN_Crypt_Token_Context_List_Next(ctx);

    if (ctx) {
      const char *s;

      s=GWEN_Crypt_Token_Context_GetServiceId(ctx);
      if (s && strcasecmp(s, "20202020")==0)
	s=NULL;
      GWEN_Dialog_SetCharProperty(dlg,
				  "wiz_bankcode_edit",
				  GWEN_DialogProperty_Value,
				  0,
				  (s && *s)?s:"",
				  0);

      s=GWEN_Crypt_Token_Context_GetAddress(ctx);
      GWEN_Dialog_SetCharProperty(dlg,
				  "wiz_url_edit",
				  GWEN_DialogProperty_Value,
				  0,
				  (s && *s)?s:"",
				  0);

      s=GWEN_Crypt_Token_Context_GetUserId(ctx);
      GWEN_Dialog_SetCharProperty(dlg,
				  "wiz_userid_edit",
				  GWEN_DialogProperty_Value,
				  0,
				  (s && *s)?s:"",
				  0);
      GWEN_Dialog_SetCharProperty(dlg,
				  "wiz_customerid_edit",
				  GWEN_DialogProperty_Value,
				  0,
				  (s && *s)?s:"",
				  0);

    }
  }

  return GWEN_DialogEvent_ResultHandled;
}



int AH_DdvCardDialog_HandleActivatedContext(GWEN_DIALOG *dlg) {
  int i;

  i=GWEN_Dialog_GetIntProperty(dlg, "wiz_context_combo", GWEN_DialogProperty_Value, 0, -1);
  if (i>=0)
    AH_DdvCardDialog_FromContext(dlg, i);
  return GWEN_DialogEvent_ResultHandled;
}



int AH_DdvCardDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  if (strcasecmp(sender, "wiz_bankcode_button")==0)
    return AH_DdvCardDialog_HandleActivatedBankCode(dlg);
  else if (strcasecmp(sender, "wiz_prev_button")==0)
    return AH_DdvCardDialog_Previous(dlg);
  else if (strcasecmp(sender, "wiz_next_button")==0)
    return AH_DdvCardDialog_Next(dlg);
  else if (strcasecmp(sender, "wiz_abort_button")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "wiz_special_button")==0)
    return AH_DdvCardDialog_HandleActivatedSpecial(dlg);
  else if (strcasecmp(sender, "wiz_help_button")==0) {
    /* TODO: open a help dialog */
  }
  else if (strcasecmp(sender, "wiz_context_combo")==0)
    return AH_DdvCardDialog_HandleActivatedContext(dlg);

  return GWEN_DialogEvent_ResultNotHandled;
}



int AH_DdvCardDialog_HandleValueChanged(GWEN_DIALOG *dlg, const char *sender) {
  if (strcasecmp(sender, "wiz_bankcode_edit")==0 ||
      strcasecmp(sender, "wiz_url_edit")==0 ||
      strcasecmp(sender, "wiz_username_edit")==0 ||
      strcasecmp(sender, "wiz_userid_edit")==0 ||
      strcasecmp(sender, "wiz_customerid_edit")==0) {
    int rv;

    if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_BANK) {
      rv=AH_DdvCardDialog_GetBankPageData(dlg);
      if (rv<0)
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    else if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_USER) {
      rv=AH_DdvCardDialog_GetUserPageData(dlg);
      if (rv<0)
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    return GWEN_DialogEvent_ResultHandled;
  }
  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB AH_DdvCardDialog_SignalHandler(GWEN_DIALOG *dlg,
						GWEN_DIALOG_EVENTTYPE t,
						const char *sender) {
  AH_DDVCARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_DDVCARD_DIALOG, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    AH_DdvCardDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AH_DdvCardDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return AH_DdvCardDialog_HandleValueChanged(dlg, sender);

  case GWEN_DialogEvent_TypeActivated:
    return AH_DdvCardDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}




