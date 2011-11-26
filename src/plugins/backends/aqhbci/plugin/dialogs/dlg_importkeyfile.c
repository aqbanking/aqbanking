/***************************************************************************
 begin       : Sat Aug 07 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_importkeyfile_p.h"
#include "i18n_l.h"

#include <aqbanking/dlg_selectbankinfo.h>
#include <aqbanking/user.h>
#include <aqbanking/banking_be.h>

#include <aqhbci/user.h>
#include <aqhbci/provider.h>
#include "dlg_rdh_special_l.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/ctplugin.h>

#include <unistd.h>


#define PAGE_BEGIN     0
#define PAGE_FILE      1
#define PAGE_BANK      2
#define PAGE_USER      3
#define PAGE_CREATE    4
#define PAGE_END       5


#define DIALOG_MINWIDTH  400
#define DIALOG_MINHEIGHT 200


#define MAX_CONTEXT_ID_ENTRIES 64


GWEN_INHERIT(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG)




GWEN_DIALOG *AH_ImportKeyFileDialog_new(AB_BANKING *ab) {
  GWEN_DIALOG *dlg;
  AH_IMPORTKEYFILE_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("ah_setup_importkeyfile");
  GWEN_NEW_OBJECT(AH_IMPORTKEYFILE_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg, xdlg,
		       AH_ImportKeyFileDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, AH_ImportKeyFileDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
			       "aqbanking/backends/aqhbci/dialogs/dlg_importkeyfile.dlg",
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
  xdlg->contextList=GWEN_Crypt_Token_Context_List_new();

  /* preset */
  xdlg->hbciVersion=210;
  xdlg->rdhVersion=0;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB AH_ImportKeyFileDialog_FreeData(void *bp, void *p) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  xdlg=(AH_IMPORTKEYFILE_DIALOG*) p;
  GWEN_Crypt_Token_Context_List_free(xdlg->contextList);
  free(xdlg->fileName);
  free(xdlg->bankCode);
  free(xdlg->bankName);
  free(xdlg->userName);
  free(xdlg->userId);
  free(xdlg->customerId);
  GWEN_FREE_OBJECT(xdlg);
}



AB_USER *AH_ImportKeyFileDialog_GetUser(const GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->user;
}



const char *AH_ImportKeyFileDialog_GetFileName(const GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->fileName;
}



void AH_ImportKeyFileDialog_SetFileName(GWEN_DIALOG *dlg, const char *s) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->fileName);
  if (s) xdlg->fileName=strdup(s);
  else xdlg->fileName=NULL;
}



const char *AH_ImportKeyFileDialog_GetBankCode(const GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->bankCode;
}



void AH_ImportKeyFileDialog_SetBankCode(GWEN_DIALOG *dlg, const char *s) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->bankCode);
  if (s) xdlg->bankCode=strdup(s);
  else xdlg->bankCode=NULL;
}



const char *AH_ImportKeyFileDialog_GetBankName(const GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->bankName;
}



void AH_ImportKeyFileDialog_SetBankName(GWEN_DIALOG *dlg, const char *s) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->bankName);
  if (s) xdlg->bankName=strdup(s);
  else xdlg->bankName=NULL;
}



const char *AH_ImportKeyFileDialog_GetUserName(const GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->userName;
}



void AH_ImportKeyFileDialog_SetUserName(GWEN_DIALOG *dlg, const char *s) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userName);
  if (s) xdlg->userName=strdup(s);
  else xdlg->userName=NULL;
}



const char *AH_ImportKeyFileDialog_GetUserId(const GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->userId;
}



void AH_ImportKeyFileDialog_SetUserId(GWEN_DIALOG *dlg, const char *s) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userId);
  if (s) xdlg->userId=strdup(s);
  else xdlg->userId=NULL;
}



const char *AH_ImportKeyFileDialog_GetCustomerId(const GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->customerId;
}



void AH_ImportKeyFileDialog_SetCustomerId(GWEN_DIALOG *dlg, const char *s) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->customerId);
  if (s) xdlg->customerId=strdup(s);
  else xdlg->customerId=NULL;
}



const char *AH_ImportKeyFileDialog_GetUrl(const GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->url;
}



void AH_ImportKeyFileDialog_SetUrl(GWEN_DIALOG *dlg, const char *s) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->url);
  if (s) xdlg->url=strdup(s);
  else xdlg->url=NULL;
}



int AH_ImportKeyFileDialog_GetHbciVersion(const GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->hbciVersion;
}



void AH_ImportKeyFileDialog_SetHbciVersion(GWEN_DIALOG *dlg, int i) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  xdlg->hbciVersion=i;
}



int AH_ImportKeyFileDialog_GetRdhVersion(const GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->rdhVersion;
}



void AH_ImportKeyFileDialog_SetRdhVersion(GWEN_DIALOG *dlg, int i) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  xdlg->rdhVersion=i;
}



uint32_t AH_ImportKeyFileDialog_GetFlags(const GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->flags;
}



void AH_ImportKeyFileDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags=fl;
}



void AH_ImportKeyFileDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}



void AH_ImportKeyFileDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}






void AH_ImportKeyFileDialog_Init(GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
			      "",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("HBCI Keyfile Import Wizard"),
			      0);

  /* select first page */
  GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, 0, 0);

  /* setup intro page */
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_begin_label",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("This dialog assists you in importing a Keyfile User.\n"),
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
                                   "<p>We are now ready to create the user and exchange keys with the server.</p>"
                                   "<p>Click the <i>next</i> button to proceed or <i>abort</i> to abort.</p>"
                                   "</html>"
                                   "We are now ready to create the user and exchange keys with the server.\n"
                                   "Click the NEXT button to proceed or ABORT to abort."),
			      0);

  /* setup extro page */
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_end_label",
			      GWEN_DialogProperty_Title,
			      0,
                              I18N("<html>"
                                   "<p>The user has been successfully created.</p>"
                                   "</html>"
                                   "The user has been successfully created."),
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



void AH_ImportKeyFileDialog_Fini(GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
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



int AH_ImportKeyFileDialog_GetFilePageData(GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_filename_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AH_ImportKeyFileDialog_SetFileName(dlg, s);
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Missing file name");
    return GWEN_ERROR_NO_DATA;
  }

  return 0;
}



int AH_ImportKeyFileDialog_GetBankPageData(GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_bankcode_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AH_ImportKeyFileDialog_SetBankCode(dlg, s);
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Missing bank code");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_bankname_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AH_ImportKeyFileDialog_SetBankName(dlg, s);
  else
    AH_ImportKeyFileDialog_SetBankName(dlg, NULL);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_url_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AH_ImportKeyFileDialog_SetUrl(dlg, s);
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Missing URL");
    return GWEN_ERROR_NO_DATA;
  }

  return 0;
}



int AH_ImportKeyFileDialog_GetUserPageData(GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_username_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AH_ImportKeyFileDialog_SetUserName(dlg, s);
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Missing user name");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_userid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AH_ImportKeyFileDialog_SetUserId(dlg, s);
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Missing user id");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_customerid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AH_ImportKeyFileDialog_SetCustomerId(dlg, s);
  else
    AH_ImportKeyFileDialog_SetCustomerId(dlg, NULL);

  return 0;
}



int AH_ImportKeyFileDialog_CheckFileType(GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *pl;
  GWEN_BUFFER *tnBuf;
  GWEN_BUFFER *ttBuf;
  GWEN_CRYPT_TOKEN *ct;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  GWEN_Crypt_Token_Context_List_Clear(xdlg->contextList);

  /* create CryptToken */
  pm=GWEN_PluginManager_FindPluginManager(GWEN_CRYPT_TOKEN_PLUGIN_TYPENAME);
  if (pm==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Plugin manager not found");
    GWEN_Gui_ShowError(I18N("Error"),
		       I18N("CryptToken plugin for type %s is not available. Did you install all necessary packages?"),
		       GWEN_CRYPT_TOKEN_PLUGIN_TYPENAME);
    return GWEN_ERROR_INTERNAL;
  }

  tnBuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(tnBuf, xdlg->fileName);
  ttBuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_Crypt_Token_PluginManager_CheckToken(pm, GWEN_Crypt_Token_Device_File, ttBuf, tnBuf, 0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(ttBuf);
    GWEN_Buffer_free(tnBuf);
    return rv;
  }

  pl=GWEN_PluginManager_GetPlugin(pm, GWEN_Buffer_GetStart(ttBuf));
  if (pl==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Plugin not found");
    GWEN_Gui_ShowError(I18N("Error"),
		       I18N("CryptToken plugin for type %s is not available. Did you install all necessary packages?"),
                       GWEN_Buffer_GetStart(ttBuf));
    GWEN_Buffer_free(ttBuf);
    GWEN_Buffer_free(tnBuf);
    return GWEN_ERROR_NOT_SUPPORTED;
  }
  DBG_INFO(AQHBCI_LOGDOMAIN, "Plugin found");

  ct=GWEN_Crypt_Token_Plugin_CreateToken(pl, GWEN_Buffer_GetStart(tnBuf));
  if (ct==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create crypt token");
    GWEN_Buffer_free(ttBuf);
    GWEN_Buffer_free(tnBuf);
    return GWEN_ERROR_INTERNAL;
  }
  GWEN_Buffer_free(ttBuf);
  GWEN_Buffer_free(tnBuf);

  /* create crypt token */
  rv=GWEN_Crypt_Token_Open(ct, 0, 0);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not open token");
    GWEN_Gui_ShowError(I18N("Error"),
		       I18N("The keyfile %s could not be opened. Please check permissions (%d)."),
		       GWEN_Crypt_Token_GetTokenName(ct),
		       rv);
    GWEN_Crypt_Token_free(ct);
    return rv;
  }

  GWEN_Dialog_SetIntProperty(dlg, "wiz_context_combo", GWEN_DialogProperty_ClearValues, 0, 0, 0);
  GWEN_Dialog_SetCharProperty(dlg, "wiz_context_combo", GWEN_DialogProperty_AddValue, 0, I18N("-- custom --"), 0);
  if (1) {
    uint32_t idList[MAX_CONTEXT_ID_ENTRIES];
    uint32_t idCount;
    uint32_t i;

    idCount=MAX_CONTEXT_ID_ENTRIES;
    rv=GWEN_Crypt_Token_GetContextIdList(ct, idList, &idCount, 0);
    if (rv<0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not read context id list");
      GWEN_Gui_ShowError(I18N("Error"),
			 I18N("Could not read context id list (%d)."),
			 rv);
      GWEN_Crypt_Token_Close(ct, 1, 0);
      GWEN_Crypt_Token_free(ct);
      return rv;
    }

    for (i=0; i<idCount; i++) {
      const GWEN_CRYPT_TOKEN_CONTEXT *ctx;

      ctx=GWEN_Crypt_Token_GetContext(ct, idList[i], 0);
      if (ctx) {
	GWEN_CRYPT_TOKEN_CONTEXT *nctx;
        char numbuf[64];
	GWEN_BUFFER *tbuf;
        const char *s;

	nctx=GWEN_Crypt_Token_Context_dup(ctx);
	GWEN_Crypt_Token_Context_List_Add(nctx, xdlg->contextList);

	tbuf=GWEN_Buffer_new(0, 256, 0, 1);
	snprintf(numbuf, sizeof(numbuf)-1, I18N("Context %d:"), i+1);
	numbuf[sizeof(numbuf)-1]=0;
	GWEN_Buffer_AppendString(tbuf, numbuf);

	s=GWEN_Crypt_Token_Context_GetServiceId(nctx);
	if (s && *s && strcasecmp(s, "20202020")!=0)
	  GWEN_Buffer_AppendString(tbuf, s);
        else
	  GWEN_Buffer_AppendString(tbuf, I18N("<no bank code>"));
	GWEN_Buffer_AppendString(tbuf, "-");

	s=GWEN_Crypt_Token_Context_GetUserId(nctx);
	if (s && *s)
	  GWEN_Buffer_AppendString(tbuf, s);
	else
	  GWEN_Buffer_AppendString(tbuf, I18N("<no user id>"));
	GWEN_Dialog_SetCharProperty(dlg, "wiz_context_combo", GWEN_DialogProperty_AddValue, 0, GWEN_Buffer_GetStart(tbuf), 0);
        GWEN_Buffer_free(tbuf);
        DBG_INFO(AQHBCI_LOGDOMAIN, "Added context %08x", idList[i]);
      }
    }
  } /* for */

  /* close crypt token */
  rv=GWEN_Crypt_Token_Close(ct, 0, 0);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not close token");
    GWEN_Gui_ShowError(I18N("Error"),
		       I18N("The keyfile %s could not be closed. Please check disc space."),
		       GWEN_Crypt_Token_GetTokenName(ct),
		       rv);
    GWEN_Crypt_Token_free(ct);
    return rv;
  }

  GWEN_Crypt_Token_free(ct);

  return 0;
}



int AH_ImportKeyFileDialog_EnterPage(GWEN_DIALOG *dlg, int page, int forwards) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  switch(page) {
  case PAGE_BEGIN:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_FILE:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    rv=AH_ImportKeyFileDialog_GetFilePageData(dlg);
    if (rv<0)
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_BANK:
    if (forwards) {
      /* leaving FILE page, check whether we can open the file */
      rv=AH_ImportKeyFileDialog_GetFilePageData(dlg);
      if (rv<0) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	return GWEN_DialogEvent_ResultHandled;
      }
      rv=AH_ImportKeyFileDialog_CheckFileType(dlg);
      if (rv<0) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	return GWEN_DialogEvent_ResultHandled;
      }
    }
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    rv=AH_ImportKeyFileDialog_GetBankPageData(dlg);
    if (rv<0)
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_USER:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    rv=AH_ImportKeyFileDialog_GetUserPageData(dlg);
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



int AH_ImportKeyFileDialog_DoIt(GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;
  AB_USER *u;
  GWEN_URL *url;
  int rv;
  uint32_t pid;
  AB_IMEXPORTER_CONTEXT *ctx;
  AB_PROVIDER *pro;
  int contextId=1;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  rv=AH_ImportKeyFileDialog_GetFilePageData(dlg);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No file?");
    // TODO: show error message
    return GWEN_DialogEvent_ResultHandled;
  }

  pro=AB_Banking_GetProvider(xdlg->banking, "aqhbci");
  if (pro==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not find backend, maybe some plugins are not installed?");
    GWEN_Gui_ShowError(I18N("Error"),
                       "%s",
		       I18N("Could not find HBCI backend, maybe some plugins are not installed?"));
    return GWEN_DialogEvent_ResultHandled;
  }

  i=GWEN_Dialog_GetIntProperty(dlg, "wiz_context_combo", GWEN_DialogProperty_Value, 0, -1);
  if (i>0) {
    GWEN_CRYPT_TOKEN_CONTEXT *tctx;

    tctx=GWEN_Crypt_Token_Context_List_First(xdlg->contextList);
    while(tctx && --i)
      tctx=GWEN_Crypt_Token_Context_List_Next(tctx);

    if (tctx)
      contextId=GWEN_Crypt_Token_Context_GetId(tctx);
  }

  DBG_NOTICE(0, "Creating user");
  u=AB_Banking_CreateUser(xdlg->banking, "aqhbci");
  if (u==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create user, maybe backend missing?");
    GWEN_Gui_ShowError(I18N("Error"),
		       "%s",
		       I18N("Could not create HBCI user (internal error)"));
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
  AH_User_SetTokenType(u, "ohbci");
  AH_User_SetTokenName(u, AH_ImportKeyFileDialog_GetFileName(dlg));
  AH_User_SetTokenContextId(u, contextId);
  AH_User_SetCryptMode(u, AH_CryptMode_Rdh);
  AH_User_SetStatus(u, AH_UserStatusPending);
  AH_User_SetHbciVersion(u, xdlg->hbciVersion);
  AH_User_SetRdhType(u, xdlg->rdhVersion);
  AH_User_SetFlags(u, xdlg->flags);

  url=GWEN_Url_fromString(xdlg->url);
  assert(url);
  GWEN_Url_SetProtocol(url, "hbci");
  if (GWEN_Url_GetPort(url)==0)
    GWEN_Url_SetPort(url, 3000);
  AH_User_SetServerUrl(u, url);
  GWEN_Url_free(url);

  rv=AB_Banking_AddUser(xdlg->banking, u);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not add user (%d)", rv);
    AB_User_free(u);
    GWEN_Gui_ShowError(I18N("Error"),
		       I18N("Could not add HBCI user, maybe there already is a user of that id (%d)"),
		       rv);
    return GWEN_DialogEvent_ResultHandled;
  }

  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
			     GWEN_GUI_PROGRESS_ALLOW_EMBED |
			     GWEN_GUI_PROGRESS_SHOW_PROGRESS |
			     GWEN_GUI_PROGRESS_SHOW_ABORT,
			     I18N("Setting Up Keyfile User"),
			     I18N("The server keys and system id will now be retrieved."),
			     2, /* getkeys, getsysid */
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

  /* get server keys id */
  GWEN_Gui_ProgressLog(pid,
		       GWEN_LoggerLevel_Notice,
		       I18N("Retrieving server keys"));
  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetServerKeys(pro, u, ctx, 0, 1, 0);
  if (rv<0) {
    AB_Banking_EndExclUseUser(xdlg->banking, u, 1);
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_Banking_DeleteUser(xdlg->banking, u);
    GWEN_Gui_ProgressEnd(pid);
    return GWEN_DialogEvent_ResultHandled;
  }

  /* TODO: show bank key hash */

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

  /* get sysid keys id */
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

  GWEN_Gui_ProgressEnd(pid);
  AH_ImportKeyFileDialog_EnterPage(dlg, PAGE_END, 1);

  xdlg->user=u;

  return GWEN_DialogEvent_ResultHandled;
}



int AH_ImportKeyFileDialog_Next(GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  if (page==PAGE_CREATE) {
    return AH_ImportKeyFileDialog_DoIt(dlg);
  }
  else if (page<PAGE_END) {
    page++;
    return AH_ImportKeyFileDialog_EnterPage(dlg, page, 1);
  }
  else if (page==PAGE_END)
    return GWEN_DialogEvent_ResultAccept;

  return GWEN_DialogEvent_ResultHandled;
}



int AH_ImportKeyFileDialog_Previous(GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  if (page>PAGE_BEGIN) {
    page--;
    return AH_ImportKeyFileDialog_EnterPage(dlg, page, 0);
  }

  return GWEN_DialogEvent_ResultHandled;
}



int AH_ImportKeyFileDialog_HandleActivatedBankCode(GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
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
	  if (s && *s && strncasecmp(s, "RDH", 3)==0)
	    break;
	}
	sv=AB_BankInfoService_List_Next(sv);
      }

      if (sv) {
	/* RDH service found */
	s=AB_BankInfoService_GetMode(sv);
	if (s && *s) {
	  if (strcasecmp(s, "RDH1")==0)
	    xdlg->rdhVersion=1;
	  else if (strcasecmp(s, "RDH2")==0)
	    xdlg->rdhVersion=2;
	  else if (strcasecmp(s, "RDH3")==0)
	    xdlg->rdhVersion=3;
	  else if (strcasecmp(s, "RDH4")==0)
	    xdlg->rdhVersion=4;
	  else if (strcasecmp(s, "RDH5")==0)
	    xdlg->rdhVersion=5;
	  else if (strcasecmp(s, "RDH6")==0)
	    xdlg->rdhVersion=6;
	  else if (strcasecmp(s, "RDH7")==0)
	    xdlg->rdhVersion=7;
	  else if (strcasecmp(s, "RDH8")==0)
	    xdlg->rdhVersion=8;
	  else if (strcasecmp(s, "RDH9")==0)
	    xdlg->rdhVersion=9;
	  else if (strcasecmp(s, "RDH10")==0)
	    xdlg->rdhVersion=10;
	  else if (strcasecmp(s, "RDH")==0)
	    xdlg->rdhVersion=1;
	}

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

  if (AH_ImportKeyFileDialog_GetBankPageData(dlg)<0)
    GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
  else
    GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);

  return GWEN_DialogEvent_ResultHandled;
}



int AH_ImportKeyFileDialog_HandleActivatedSpecial(GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  dlg2=AH_RdhSpecialDialog_new(xdlg->banking);
  if (dlg2==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create dialog");
    GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Could not create dialog, maybe an installation error?"));
    return GWEN_DialogEvent_ResultHandled;
  }

  AH_RdhSpecialDialog_SetFlags(dlg2, xdlg->flags);

  rv=GWEN_Gui_ExecDialog(dlg2, 0);
  if (rv==0) {
    /* rejected */
    GWEN_Dialog_free(dlg2);
    return GWEN_DialogEvent_ResultHandled;
  }
  else {
    xdlg->hbciVersion=AH_RdhSpecialDialog_GetHbciVersion(dlg2);
    xdlg->rdhVersion=AH_RdhSpecialDialog_GetRdhVersion(dlg2);
    xdlg->flags=AH_RdhSpecialDialog_GetFlags(dlg2);
  }

  GWEN_Dialog_free(dlg2);

  return GWEN_DialogEvent_ResultHandled;
}



int AH_ImportKeyFileDialog_HandleActivatedFileButton(GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;
  int rv;
  const char *s;
  GWEN_BUFFER *pathBuffer;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  pathBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_filename_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    GWEN_Buffer_AppendString(pathBuffer, s);
  rv=GWEN_Gui_GetFileName(I18N("Select Keyfile"),
			  GWEN_Gui_FileNameType_OpenFileName,
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
    rv=AH_ImportKeyFileDialog_GetFilePageData(dlg);
    if (rv<0)
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
  }
  else {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }
  GWEN_Buffer_free(pathBuffer);
  return GWEN_DialogEvent_ResultHandled;
}



static int AH_ImportKeyFileDialog_HandleActivatedIniLetter(GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;
  int rv;
  GWEN_BUFFER *tbuf;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  tbuf=GWEN_Buffer_new(0, 1024, 0, 1);

  /* add HTML version of the INI letter */
  GWEN_Buffer_AppendString(tbuf, "<html>");
  rv=AH_Provider_GetIniLetterHtml(AB_User_GetProvider(xdlg->user),
				  xdlg->user,
				  0,
				  0,
				  tbuf,
				  1);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    // TODO: show error message
    AB_Banking_ClearCryptTokenList(xdlg->banking);
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }
  GWEN_Buffer_AppendString(tbuf, "</html>");


  /* add ASCII version of the INI letter for frontends which don't support HTML */
  rv=AH_Provider_GetIniLetterTxt(AB_User_GetProvider(xdlg->user),
				 xdlg->user,
				 0,
				 0,
				 tbuf,
				 0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    // TODO: show error message
    AB_Banking_ClearCryptTokenList(xdlg->banking);
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }

  rv=GWEN_Gui_Print(I18N("INI Letter"),
		    "HBCI-INILETTER",
		    I18N("INI Letter for HBCI"),
		    GWEN_Buffer_GetStart(tbuf),
		    0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    // TODO: show error message
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }

  GWEN_Buffer_free(tbuf);
  return GWEN_DialogEvent_ResultHandled;
}



int AH_ImportKeyFileDialog_HandleActivatedContext(GWEN_DIALOG *dlg) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  i=GWEN_Dialog_GetIntProperty(dlg, "wiz_context_combo", GWEN_DialogProperty_Value, 0, -1);
  if (i>0) {
    GWEN_CRYPT_TOKEN_CONTEXT *ctx;

    ctx=GWEN_Crypt_Token_Context_List_First(xdlg->contextList);
    while(ctx && --i)
      ctx=GWEN_Crypt_Token_Context_List_Next(ctx);

    if (ctx) {
      const char *s;

      s=GWEN_Crypt_Token_Context_GetServiceId(ctx);
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



int AH_ImportKeyFileDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  if (strcasecmp(sender, "wiz_filename_button")==0)
    return AH_ImportKeyFileDialog_HandleActivatedFileButton(dlg);
  else if (strcasecmp(sender, "wiz_bankcode_button")==0)
    return AH_ImportKeyFileDialog_HandleActivatedBankCode(dlg);
  else if (strcasecmp(sender, "wiz_prev_button")==0)
    return AH_ImportKeyFileDialog_Previous(dlg);
  else if (strcasecmp(sender, "wiz_next_button")==0)
    return AH_ImportKeyFileDialog_Next(dlg);
  else if (strcasecmp(sender, "wiz_abort_button")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "wiz_special_button")==0)
    return AH_ImportKeyFileDialog_HandleActivatedSpecial(dlg);
  else if (strcasecmp(sender, "wiz_iniletter_button")==0)
    return AH_ImportKeyFileDialog_HandleActivatedIniLetter(dlg);
  else if (strcasecmp(sender, "wiz_help_button")==0) {
    /* TODO: open a help dialog */
  }
  else if (strcasecmp(sender, "wiz_context_combo")==0)
    return AH_ImportKeyFileDialog_HandleActivatedContext(dlg);

  return GWEN_DialogEvent_ResultNotHandled;
}



int AH_ImportKeyFileDialog_HandleValueChanged(GWEN_DIALOG *dlg, const char *sender) {
  if (strcasecmp(sender, "wiz_filename_edit")==0 ||
      strcasecmp(sender, "wiz_bankcode_edit")==0 ||
      strcasecmp(sender, "wiz_url_edit")==0 ||
      strcasecmp(sender, "wiz_username_edit")==0 ||
      strcasecmp(sender, "wiz_userid_edit")==0 ||
      strcasecmp(sender, "wiz_customerid_edit")==0) {
    int rv;

    if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_FILE) {
      rv=AH_ImportKeyFileDialog_GetFilePageData(dlg);
      if (rv<0)
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    else if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_BANK) {
      rv=AH_ImportKeyFileDialog_GetBankPageData(dlg);
      if (rv<0)
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    else if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_USER) {
      rv=AH_ImportKeyFileDialog_GetUserPageData(dlg);
      if (rv<0)
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
	GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    return GWEN_DialogEvent_ResultHandled;
  }
  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB AH_ImportKeyFileDialog_SignalHandler(GWEN_DIALOG *dlg,
						       GWEN_DIALOG_EVENTTYPE t,
						       const char *sender) {
  AH_IMPORTKEYFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_IMPORTKEYFILE_DIALOG, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    AH_ImportKeyFileDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AH_ImportKeyFileDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return AH_ImportKeyFileDialog_HandleValueChanged(dlg, sender);

  case GWEN_DialogEvent_TypeActivated:
    return AH_ImportKeyFileDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}




