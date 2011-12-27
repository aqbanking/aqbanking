/***************************************************************************
 begin       : Mon Apr 12 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_pintan_p.h"
#include "i18n_l.h"

#include <aqbanking/dlg_selectbankinfo.h>
#include <aqbanking/user.h>
#include <aqbanking/banking_be.h>

#include <aqhbci/user.h>
#include <aqhbci/provider.h>
#include "dlg_pintan_special_l.h"

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




GWEN_DIALOG *AH_PinTanDialog_new(AB_BANKING *ab) {
  GWEN_DIALOG *dlg;
  AH_PINTAN_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("ah_setup_pintan");
  GWEN_NEW_OBJECT(AH_PINTAN_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg, xdlg,
		       AH_PinTanDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, AH_PinTanDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
			       "aqbanking/backends/aqhbci/dialogs/dlg_pintan.dlg",
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

  /* preset */
  xdlg->hbciVersion=300;
  xdlg->httpVMajor=1;
  xdlg->httpVMinor=1;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB AH_PinTanDialog_FreeData(void *bp, void *p) {
  AH_PINTAN_DIALOG *xdlg;

  xdlg=(AH_PINTAN_DIALOG*) p;
  free(xdlg->bankCode);
  free(xdlg->bankName);
  free(xdlg->userName);
  free(xdlg->userId);
  free(xdlg->customerId);
  free(xdlg->tanMediumId);

  GWEN_FREE_OBJECT(xdlg);
}



AB_USER *AH_PinTanDialog_GetUser(const GWEN_DIALOG *dlg) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  return xdlg->user;
}


const char *AH_PinTanDialog_GetBankCode(const GWEN_DIALOG *dlg) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  return xdlg->bankCode;
}



void AH_PinTanDialog_SetBankCode(GWEN_DIALOG *dlg, const char *s) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->bankCode);
  if (s) xdlg->bankCode=strdup(s);
  else xdlg->bankCode=NULL;
}



const char *AH_PinTanDialog_GetBankName(const GWEN_DIALOG *dlg) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  return xdlg->bankName;
}



void AH_PinTanDialog_SetBankName(GWEN_DIALOG *dlg, const char *s) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->bankName);
  if (s) xdlg->bankName=strdup(s);
  else xdlg->bankName=NULL;
}



const char *AH_PinTanDialog_GetUserName(const GWEN_DIALOG *dlg) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  return xdlg->userName;
}



void AH_PinTanDialog_SetUserName(GWEN_DIALOG *dlg, const char *s) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userName);
  if (s) xdlg->userName=strdup(s);
  else xdlg->userName=NULL;
}



const char *AH_PinTanDialog_GetUserId(const GWEN_DIALOG *dlg) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  return xdlg->userId;
}



void AH_PinTanDialog_SetUserId(GWEN_DIALOG *dlg, const char *s) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userId);
  if (s) xdlg->userId=strdup(s);
  else xdlg->userId=NULL;
}



const char *AH_PinTanDialog_GetCustomerId(const GWEN_DIALOG *dlg) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  return xdlg->customerId;
}



void AH_PinTanDialog_SetCustomerId(GWEN_DIALOG *dlg, const char *s) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->customerId);
  if (s) xdlg->customerId=strdup(s);
  else xdlg->customerId=NULL;
}



const char *AH_PinTanDialog_GetUrl(const GWEN_DIALOG *dlg) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  return xdlg->url;
}



void AH_PinTanDialog_SetUrl(GWEN_DIALOG *dlg, const char *s) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->url);
  if (s) xdlg->url=strdup(s);
  else xdlg->url=NULL;
}



int AH_PinTanDialog_GetHttpVMajor(const GWEN_DIALOG *dlg) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  return xdlg->httpVMajor;
}



int AH_PinTanDialog_GetHttpVMinor(const GWEN_DIALOG *dlg) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  return xdlg->httpVMinor;
}



void AH_PinTanDialog_SetHttpVersion(GWEN_DIALOG *dlg, int vmajor, int vminor) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  xdlg->httpVMajor=vmajor;
  xdlg->httpVMinor=vminor;
}



int AH_PinTanDialog_GetHbciVersion(const GWEN_DIALOG *dlg) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  return xdlg->hbciVersion;
}



void AH_PinTanDialog_SetHbciVersion(GWEN_DIALOG *dlg, int i) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  xdlg->hbciVersion=i;
}



uint32_t AH_PinTanDialog_GetFlags(const GWEN_DIALOG *dlg) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  return xdlg->flags;
}



void AH_PinTanDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags=fl;
}



void AH_PinTanDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}



void AH_PinTanDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}



const char *AH_PinTanDialog_GetTanMediumId(const GWEN_DIALOG *dlg) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  return xdlg->tanMediumId;
}



void AH_PinTanDialog_SetTanMediumId(GWEN_DIALOG *dlg, const char *s) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->tanMediumId);
  if (s) xdlg->tanMediumId=strdup(s);
  else xdlg->tanMediumId=NULL;
}









void AH_PinTanDialog_Init(GWEN_DIALOG *dlg) {
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



void AH_PinTanDialog_Fini(GWEN_DIALOG *dlg) {
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



static void removeAllSpaces(uint8_t *s) {
  uint8_t *d;

  d=s;
  while(*s) {
    if (*s>33)
      *(d++)=*s;
    s++;
  }
  *d=0;
}



int AH_PinTanDialog_GetBankPageData(GWEN_DIALOG *dlg) {
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
    AH_PinTanDialog_SetBankCode(dlg, GWEN_Buffer_GetStart(tbuf));
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
    AH_PinTanDialog_SetBankName(dlg, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else
    AH_PinTanDialog_SetBankName(dlg, NULL);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_url_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    removeAllSpaces((uint8_t*)GWEN_Buffer_GetStart(tbuf));
    AH_PinTanDialog_SetUrl(dlg, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Missing URL");
    return GWEN_ERROR_NO_DATA;
  }

  return 0;
}



int AH_PinTanDialog_GetUserPageData(GWEN_DIALOG *dlg) {
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
    AH_PinTanDialog_SetUserName(dlg, GWEN_Buffer_GetStart(tbuf));
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
    AH_PinTanDialog_SetUserId(dlg, GWEN_Buffer_GetStart(tbuf));
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
    AH_PinTanDialog_SetCustomerId(dlg, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else
    AH_PinTanDialog_SetCustomerId(dlg, NULL);

  return 0;
}



int AH_PinTanDialog_EnterPage(GWEN_DIALOG *dlg, int page, int forwards) {
  AH_PINTAN_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  switch(page) {
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



int AH_PinTanDialog_DoIt(GWEN_DIALOG *dlg) {
  AH_PINTAN_DIALOG *xdlg;
  AB_USER *u;
  GWEN_URL *url;
  int rv;
  uint32_t pid;
  AB_IMEXPORTER_CONTEXT *ctx;
  AB_PROVIDER *pro;

  DBG_NOTICE(0, "Doit");
  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  pro=AB_Banking_GetProvider(xdlg->banking, "aqhbci");
  if (pro==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not find backend, maybe some plugins are not installed?");
    // TODO: show error message
    return GWEN_DialogEvent_ResultHandled;
  }

  DBG_NOTICE(0, "Creating user");
  u=AB_Banking_CreateUser(xdlg->banking, "aqhbci");
  if (u==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create user, maybe backend missing?");
    // TODO: show error message
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

  DBG_NOTICE(0, "Adding user");
  rv=AB_Banking_AddUser(xdlg->banking, u);
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
			     3,
			     0);
  /* lock new user */
  DBG_NOTICE(0, "Locking user");
  rv=AB_Banking_BeginExclUseUser(xdlg->banking, u);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not lock user (%d)", rv);
    GWEN_Gui_ProgressLog(pid,
			 GWEN_LoggerLevel_Error,
			 I18N("Unable to lock users"));
    AB_Banking_DeleteUser(xdlg->banking, u);
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }

  /* get certificate */
  DBG_NOTICE(0, "Getting certs (%08x)", AH_User_GetFlags(u));
  GWEN_Gui_ProgressLog(pid,
		       GWEN_LoggerLevel_Notice,
		       I18N("Retrieving SSL certificate"));
  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetCert(pro, u, 0, 1, 0);
  if (rv<0) {
    // TODO: retry with SSLv3 if necessary
    AB_Banking_EndExclUseUser(xdlg->banking, u, 1);
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
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

  /* get system id */
  DBG_NOTICE(0, "Getting sysid");
  GWEN_Gui_ProgressLog(pid,
		       GWEN_LoggerLevel_Notice,
		       I18N("Retrieving system id"));
  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetSysId(pro, u, ctx, 0, 1, 0);
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

  /* get account list */
  DBG_NOTICE(0, "Getting account list");
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
  DBG_NOTICE(0, "Unlocking user");
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
  AH_PinTanDialog_EnterPage(dlg, PAGE_END, 1);

  xdlg->user=u;

  return GWEN_DialogEvent_ResultHandled;
}



int AH_PinTanDialog_Next(GWEN_DIALOG *dlg) {
  AH_PINTAN_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  if (page==PAGE_CREATE) {
    return AH_PinTanDialog_DoIt(dlg);
  }
  else if (page<PAGE_END) {
    page++;
    return AH_PinTanDialog_EnterPage(dlg, page, 1);
  }
  else if (page==PAGE_END)
    return GWEN_DialogEvent_ResultAccept;

  return GWEN_DialogEvent_ResultHandled;
}



int AH_PinTanDialog_Previous(GWEN_DIALOG *dlg) {
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



int AH_PinTanDialog_HandleActivatedBankCode(GWEN_DIALOG *dlg) {
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
      while(sv) {
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



int AH_PinTanDialog_HandleActivatedSpecial(GWEN_DIALOG *dlg) {
  AH_PINTAN_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  dlg2=AH_PinTanSpecialDialog_new(xdlg->banking);
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
    AH_PinTanDialog_SetTanMediumId(dlg, AH_PinTanSpecialDialog_GetTanMediumId(dlg2));
  }

  GWEN_Dialog_free(dlg2);

  return GWEN_DialogEvent_ResultHandled;
}



int AH_PinTanDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  DBG_NOTICE(0, "Activated: %s", sender);
  if (strcasecmp(sender, "wiz_bankcode_button")==0)
    return AH_PinTanDialog_HandleActivatedBankCode(dlg);
  else if (strcasecmp(sender, "wiz_prev_button")==0)
    return AH_PinTanDialog_Previous(dlg);
  else if (strcasecmp(sender, "wiz_next_button")==0)
    return AH_PinTanDialog_Next(dlg);
  else if (strcasecmp(sender, "wiz_abort_button")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "wiz_special_button")==0)
    return AH_PinTanDialog_HandleActivatedSpecial(dlg);
  else if (strcasecmp(sender, "wiz_help_button")==0) {
    /* TODO: open a help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int AH_PinTanDialog_HandleValueChanged(GWEN_DIALOG *dlg, const char *sender) {
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



int GWENHYWFAR_CB AH_PinTanDialog_SignalHandler(GWEN_DIALOG *dlg,
						GWEN_DIALOG_EVENTTYPE t,
						const char *sender) {
  AH_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_DIALOG, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    AH_PinTanDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AH_PinTanDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return AH_PinTanDialog_HandleValueChanged(dlg, sender);

  case GWEN_DialogEvent_TypeActivated:
    return AH_PinTanDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}




