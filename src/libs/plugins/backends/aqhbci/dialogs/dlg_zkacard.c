/***************************************************************************
 begin       : Tue Apr 20 2010
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_zkacard_p.h"

#include "aqhbci/banking/provider_l.h"
#include "aqhbci/banking/user.h"
#include "aqhbci/banking/provider.h"
#include "aqhbci/banking/provider_online.h"
#include "dlg_rdh_special_l.h"

#include "aqbanking/i18n_l.h"
#include <aqbanking/dialogs/dlg_selectbankinfo.h>
#include <aqbanking/backendsupport/user.h>
#include <aqbanking/banking_be.h>

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


GWEN_INHERIT(GWEN_DIALOG, AH_ZKACARD_DIALOG)




GWEN_DIALOG *AH_ZkaCardDialog_new(AB_PROVIDER *pro, GWEN_CRYPT_TOKEN *ct)
{
  GWEN_DIALOG *dlg;
  AH_ZKACARD_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("ah_setup_zkacard");
  GWEN_NEW_OBJECT(AH_ZKACARD_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg, xdlg,
                       AH_ZkaCardDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, AH_ZkaCardDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
                               "aqbanking/backends/aqhbci/dialogs/dlg_zkacard.dlg",
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

  xdlg->provider=pro;
  xdlg->banking=AB_Provider_GetBanking(pro);
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
  xdlg->hbciVersion=300;
  xdlg->rdhVersion=9;
  xdlg->cryptMode = AH_CryptMode_Rdh;
  xdlg->flags= AH_USER_FLAGS_BANK_DOESNT_SIGN | AH_USER_FLAGS_BANK_USES_SIGNSEQ;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB AH_ZkaCardDialog_FreeData(void *bp, void *p)
{
  AH_ZKACARD_DIALOG *xdlg;

  xdlg=(AH_ZKACARD_DIALOG *) p;
  GWEN_Crypt_Token_Context_List_free(xdlg->contextList);
  free(xdlg->bankCode);
  free(xdlg->bankName);
  free(xdlg->userName);
  free(xdlg->userId);
  free(xdlg->customerId);
  free(xdlg->peerId);
  GWEN_FREE_OBJECT(xdlg);
}



AB_USER *AH_ZkaCardDialog_GetUser(const GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->user;
}



GWEN_CRYPT_TOKEN *AH_ZkaCardDialog_GetCryptToken(const GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->cryptToken;
}



const char *AH_ZkaCardDialog_GetBankCode(const GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->bankCode;
}



void AH_ZkaCardDialog_SetBankCode(GWEN_DIALOG *dlg, const char *s)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->bankCode);
  if (s)
    xdlg->bankCode=strdup(s);
  else
    xdlg->bankCode=NULL;
}



const char *AH_ZkaCardDialog_GetBankName(const GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->bankName;
}



void AH_ZkaCardDialog_SetBankName(GWEN_DIALOG *dlg, const char *s)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->bankName);
  if (s)
    xdlg->bankName=strdup(s);
  else
    xdlg->bankName=NULL;
}



const char *AH_ZkaCardDialog_GetUserName(const GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->userName;
}



void AH_ZkaCardDialog_SetUserName(GWEN_DIALOG *dlg, const char *s)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userName);
  if (s)
    xdlg->userName=strdup(s);
  else
    xdlg->userName=NULL;
}



const char *AH_ZkaCardDialog_GetUserId(const GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->userId;
}



void AH_ZkaCardDialog_SetUserId(GWEN_DIALOG *dlg, const char *s)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->userId);
  if (s)
    xdlg->userId=strdup(s);
  else
    xdlg->userId=NULL;
}



const char *AH_ZkaCardDialog_GetCustomerId(const GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->customerId;
}



void AH_ZkaCardDialog_SetCustomerId(GWEN_DIALOG *dlg, const char *s)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->customerId);
  if (s)
    xdlg->customerId=strdup(s);
  else
    xdlg->customerId=NULL;
}



const char *AH_ZkaCardDialog_GetUrl(const GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->url;
}



void AH_ZkaCardDialog_SetUrl(GWEN_DIALOG *dlg, const char *s)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->url);
  if (s)
    xdlg->url=strdup(s);
  else
    xdlg->url=NULL;
}



const char *AH_ZkaCardDialog_GetPeerId(const GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->peerId;
}



void AH_ZkaCardDialog_SetPeerId(GWEN_DIALOG *dlg, const char *s)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->peerId);
  if (s)
    xdlg->peerId=strdup(s);
  else
    xdlg->peerId=NULL;
}


int AH_ZkaCardDialog_GetHbciVersion(const GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->hbciVersion;
}



void AH_ZkaCardDialog_SetHbciVersion(GWEN_DIALOG *dlg, int i)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  xdlg->hbciVersion=i;
}



uint32_t AH_ZkaCardDialog_GetFlags(const GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  return xdlg->flags;
}



void AH_ZkaCardDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags=fl;
}



void AH_ZkaCardDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}



void AH_ZkaCardDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}






void AH_ZkaCardDialog_Init(GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
                              "",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("HBCI ZKA-Card Setup Wizard"),
                              0);

  /* select first page */
  GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, 0, 0);

  /* setup intro page */
  GWEN_Dialog_SetCharProperty(dlg,
                              "wiz_begin_label",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("<html>"
                                   "<p>This dialog assists you in setting up a ZKA Chipcard User.</p>"
                                   "<p>Some chipcards contain user information. You can click the button below "
                                   "to read that information from the card.</p>"
                                   "</html>"
                                   "This dialog assists you in setting up a ZKA Chipcard User.\n"
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
    while (ctx) {
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
  AH_ZkaCardDialog_FromContext(dlg, 0);

  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);

  /* enable next and disable previous buttons */
  GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
  GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
}



void AH_ZkaCardDialog_Fini(GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
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



int AH_ZkaCardDialog_GetBankPageData(GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_bankcode_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AH_ZkaCardDialog_SetBankCode(dlg, s);
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Missing bank code");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_bankname_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AH_ZkaCardDialog_SetBankName(dlg, s);
  else
    AH_ZkaCardDialog_SetBankName(dlg, NULL);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_url_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AH_ZkaCardDialog_SetUrl(dlg, s);
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Missing URL");
    return GWEN_ERROR_NO_DATA;
  }

  return 0;
}



int AH_ZkaCardDialog_GetUserPageData(GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_username_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AH_ZkaCardDialog_SetUserName(dlg, s);
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Missing user name");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_userid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AH_ZkaCardDialog_SetUserId(dlg, s);
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Missing user id");
    return GWEN_ERROR_NO_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_customerid_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    AH_ZkaCardDialog_SetCustomerId(dlg, s);
  else
    AH_ZkaCardDialog_SetCustomerId(dlg, NULL);

  return 0;
}



int AH_ZkaCardDialog_EnterPage(GWEN_DIALOG *dlg, int page, int forwards)
{
  AH_ZKACARD_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  switch (page) {
  case PAGE_BEGIN:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_BANK:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    rv=AH_ZkaCardDialog_GetBankPageData(dlg);
    if (rv<0)
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_USER:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    rv=AH_ZkaCardDialog_GetUserPageData(dlg);
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
    //GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_abort_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    return GWEN_DialogEvent_ResultHandled;

  default:
    return GWEN_DialogEvent_ResultHandled;
  }

  return GWEN_DialogEvent_ResultHandled;
}



int AH_ZkaCardDialog_DoIt(GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;
  AB_USER *u;
  GWEN_URL *url;
  int rv;
  uint32_t pid;
  AB_IMEXPORTER_CONTEXT *ctx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  u=AB_Provider_CreateUserObject(xdlg->provider);
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
  AH_User_SetTokenType(u, "zkacard");
  AH_User_SetTokenName(u, GWEN_Crypt_Token_GetTokenName(xdlg->cryptToken));
  AH_User_SetCryptMode(u, AH_CryptMode_Rdh);
  AH_User_SetTokenContextId(u, xdlg->contextId);
  AH_User_SetStatus(u, AH_UserStatusEnabled);

  url=GWEN_Url_fromString(xdlg->url);
  assert(url);
  GWEN_Url_SetProtocol(url, "hbci");
  if (GWEN_Url_GetPort(url)==0)
    GWEN_Url_SetPort(url, 3000);
  AH_User_SetServerUrl(u, url);
  GWEN_Url_free(url);
  AH_User_SetHbciVersion(u, xdlg->hbciVersion);
  AH_User_SetRdhType(u, xdlg->rdhVersion);
  AH_User_AddFlags(u, xdlg->flags);

  rv=AB_Provider_AddUser(xdlg->provider, u);
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
                             I18N("Setting Up ZKA User"),
                             I18N("The list of accounts will be retrieved."),
                             1,
                             0);

  /* get public bank server key */
  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetServerKeys(xdlg->provider, u, ctx, 1, 0, 1);
  AB_ImExporterContext_free(ctx);
  if (rv) {
    AB_Provider_EndExclUseUser(xdlg->provider, u, 1);
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error getting server keys (%d)", rv);
    AB_Provider_DeleteUser(xdlg->provider, AB_User_GetUniqueId(u));
    GWEN_Gui_ProgressEnd(pid);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    return GWEN_DialogEvent_ResultHandled;
  }

  /* send user keys */
  if (xdlg->keyStatus & 0x01) {
    int withAuthKey=0;
    if (xdlg->rdhVersion == 7) {
      withAuthKey=1;
    }
    ctx=AB_ImExporterContext_new();
    rv=AH_Provider_SendUserKeys2(xdlg->provider, u, ctx, withAuthKey, 1, 0, 1);
    AB_ImExporterContext_free(ctx);
    if (rv) {
      AB_Provider_EndExclUseUser(xdlg->provider, u, 1);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error sending user keys (%d)", rv);
      AB_Provider_DeleteUser(xdlg->provider, AB_User_GetUniqueId(u));
      GWEN_Gui_ProgressEnd(pid);
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      return GWEN_DialogEvent_ResultHandled;
    }
  }
  /* get Kundensystem-ID */
  /* FIXME the SysID of the zkacard is the CID, so this should not be necessary */

  if (xdlg->rdhVersion != 7) {
    ctx=AB_ImExporterContext_new();
    rv=AH_Provider_GetSysId(xdlg->provider, u, ctx, 1, 0, 1);
    AB_ImExporterContext_free(ctx);
    if (rv) {
      AB_Provider_EndExclUseUser(xdlg->provider, u, 1);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error getting Kundensystem ID (%d)", rv);
      AB_Provider_DeleteUser(xdlg->provider, AB_User_GetUniqueId(u));
      GWEN_Gui_ProgressEnd(pid);
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      return GWEN_DialogEvent_ResultHandled;
    }
  }

  /* lock new user */
  rv=AB_Provider_BeginExclUseUser(xdlg->provider, u);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not lock user (%d)", rv);
    GWEN_Gui_ProgressLog2(pid,
                          GWEN_LoggerLevel_Error,
                          I18N("Unable to lock users (%d)"), rv);
    AB_Provider_DeleteUser(xdlg->provider, AB_User_GetUniqueId(u));
    GWEN_Gui_ProgressEnd(pid);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    return GWEN_DialogEvent_ResultHandled;
  }

  /* get account list */
  GWEN_Gui_ProgressLog(pid,
                       GWEN_LoggerLevel_Notice,
                       I18N("Retrieving account list"));
  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetAccounts(xdlg->provider, u, ctx, 0, 1, 0);
  AB_ImExporterContext_free(ctx);
  if (rv<0) {
    AB_Provider_EndExclUseUser(xdlg->provider, u, 1);
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error getting accounts (%d)", rv);
    AB_Provider_DeleteUser(xdlg->provider, AB_User_GetUniqueId(u));
    GWEN_Gui_ProgressEnd(pid);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    return GWEN_DialogEvent_ResultHandled;
  }

  rv=GWEN_Gui_ProgressAdvance(pid, GWEN_GUI_PROGRESS_ONE);
  if (rv==GWEN_ERROR_USER_ABORTED) {
    AB_Provider_EndExclUseUser(xdlg->provider, u, 1);
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_Provider_DeleteUser(xdlg->provider, AB_User_GetUniqueId(u));
    GWEN_Gui_ProgressLog(pid,
                         GWEN_LoggerLevel_Error,
                         I18N("Aborted by user."));
    GWEN_Gui_ProgressEnd(pid);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    return GWEN_DialogEvent_ResultHandled;
  }

  /* unlock user */
  rv=AB_Provider_EndExclUseUser(xdlg->provider, u, 0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not unlock customer [%s] (%d)",
             AB_User_GetCustomerId(u), rv);
    GWEN_Gui_ProgressLog2(pid,
                          GWEN_LoggerLevel_Error,
                          I18N("Could not unlock user %s (%d)"),
                          AB_User_GetUserId(u), rv);
    AB_Provider_EndExclUseUser(xdlg->provider, u, 1);
    AB_Provider_DeleteUser(xdlg->provider, AB_User_GetUniqueId(u));
    GWEN_Gui_ProgressEnd(pid);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    return GWEN_DialogEvent_ResultHandled;
  }

  GWEN_Dialog_SetCharProperty(dlg,
                              "wiz_end_label",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("The user has been successfully setup."),
                              0);
  GWEN_Gui_ProgressEnd(pid);
  AH_ZkaCardDialog_EnterPage(dlg, PAGE_END, 1);

  xdlg->user=u;

  return GWEN_DialogEvent_ResultHandled;
}



int AH_ZkaCardDialog_Next(GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  if (page==PAGE_CREATE) {
    return AH_ZkaCardDialog_DoIt(dlg);
  }
  else if (page<PAGE_END) {
    page++;
    return AH_ZkaCardDialog_EnterPage(dlg, page, 1);
  }
  else if (page==PAGE_END)
    return GWEN_DialogEvent_ResultAccept;

  return GWEN_DialogEvent_ResultHandled;
}



int AH_ZkaCardDialog_Previous(GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  if (page>PAGE_BEGIN) {
    page--;
    return AH_ZkaCardDialog_EnterPage(dlg, page, 0);
  }

  return GWEN_DialogEvent_ResultHandled;
}



int AH_ZkaCardDialog_HandleActivatedBankCode(GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  dlg2=AB_SelectBankInfoDialog_new(xdlg->banking, "de", GWEN_Dialog_GetCharProperty(dlg, "wiz_bankcode_edit",
                                   GWEN_DialogProperty_Value, 0, NULL));
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
      while (sv) {
        const char *s;

        s=AB_BankInfoService_GetType(sv);
        if (s && *s && strcasecmp(s, "HBCI")==0) {
          s=AB_BankInfoService_GetMode(sv);
          if (s && *s && strcasecmp(s, "zka")==0)
            break;
        }
        sv=AB_BankInfoService_List_Next(sv);
      }

      if (sv) {
        /* ZKA service found */
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

  if (AH_ZkaCardDialog_GetBankPageData(dlg)<0)
    GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
  else
    GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);

  return GWEN_DialogEvent_ResultHandled;
}



int AH_ZkaCardDialog_HandleActivatedSpecial(GWEN_DIALOG *dlg)
{
  AH_ZKACARD_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  dlg2=AH_RdhSpecialDialog_new(xdlg->provider);
  if (dlg2==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create dialog");
    GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Could not create dialog, maybe an installation error?"));
    return GWEN_DialogEvent_ResultHandled;
  }

  AH_RdhSpecialDialog_SetHbciVersion(dlg2, xdlg->hbciVersion);
  AH_RdhSpecialDialog_SetRdhVersion(dlg2, xdlg->rdhVersion);
  AH_RdhSpecialDialog_SetCryptMode(dlg2, xdlg->cryptMode);
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
    xdlg->cryptMode=AH_RdhSpecialDialog_GetCryptMode(dlg2);
    xdlg->flags=AH_RdhSpecialDialog_GetFlags(dlg2);
  }

  GWEN_Dialog_free(dlg2);

  return GWEN_DialogEvent_ResultHandled;
}



int AH_ZkaCardDialog_FromContext(GWEN_DIALOG *dlg, int i)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  xdlg->contextId = i+1;  // Real contextId on the card

  if (i>=0) {
    GWEN_CRYPT_TOKEN_CONTEXT *ctx;

    ctx=GWEN_Crypt_Token_Context_List_First(xdlg->contextList);
    while (ctx && i--)
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
      s=GWEN_Crypt_Token_Context_GetCustomerId(ctx);
      GWEN_Dialog_SetCharProperty(dlg,
                                  "wiz_customerid_edit",
                                  GWEN_DialogProperty_Value,
                                  0,
                                  (s && *s)?s:"",
                                  0);

      xdlg->rdhVersion = GWEN_Crypt_Token_Context_GetProtocolVersion(ctx);
      xdlg->cryptMode = AH_CryptMode_Rdh;
      xdlg->keyStatus  = GWEN_Crypt_Token_Context_GetKeyStatus(ctx);
    }
  }

  return GWEN_DialogEvent_ResultHandled;
}



int AH_ZkaCardDialog_HandleActivatedContext(GWEN_DIALOG *dlg)
{
  int i;

  i=GWEN_Dialog_GetIntProperty(dlg, "wiz_context_combo", GWEN_DialogProperty_Value, 0, -1);
  if (i>=0)
    AH_ZkaCardDialog_FromContext(dlg, i);
  return GWEN_DialogEvent_ResultHandled;
}



int AH_ZkaCardDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender)
{
  if (strcasecmp(sender, "wiz_bankcode_button")==0)
    return AH_ZkaCardDialog_HandleActivatedBankCode(dlg);
  else if (strcasecmp(sender, "wiz_prev_button")==0)
    return AH_ZkaCardDialog_Previous(dlg);
  else if (strcasecmp(sender, "wiz_next_button")==0)
    return AH_ZkaCardDialog_Next(dlg);
  else if (strcasecmp(sender, "wiz_abort_button")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "wiz_special_button")==0)
    return AH_ZkaCardDialog_HandleActivatedSpecial(dlg);
  else if (strcasecmp(sender, "wiz_help_button")==0) {
    /* TODO: open a help dialog */
  }
  else if (strcasecmp(sender, "wiz_context_combo")==0)
    return AH_ZkaCardDialog_HandleActivatedContext(dlg);

  return GWEN_DialogEvent_ResultNotHandled;
}



int AH_ZkaCardDialog_HandleValueChanged(GWEN_DIALOG *dlg, const char *sender)
{
  if (strcasecmp(sender, "wiz_bankcode_edit")==0 ||
      strcasecmp(sender, "wiz_url_edit")==0 ||
      strcasecmp(sender, "wiz_username_edit")==0 ||
      strcasecmp(sender, "wiz_userid_edit")==0 ||
      strcasecmp(sender, "wiz_customerid_edit")==0) {
    int rv;

    if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_BANK) {
      rv=AH_ZkaCardDialog_GetBankPageData(dlg);
      if (rv<0)
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    else if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_USER) {
      rv=AH_ZkaCardDialog_GetUserPageData(dlg);
      if (rv<0)
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    return GWEN_DialogEvent_ResultHandled;
  }
  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB AH_ZkaCardDialog_SignalHandler(GWEN_DIALOG *dlg,
                                                 GWEN_DIALOG_EVENTTYPE t,
                                                 const char *sender)
{
  AH_ZKACARD_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_ZKACARD_DIALOG, dlg);
  assert(xdlg);

  switch (t) {
  case GWEN_DialogEvent_TypeInit:
    AH_ZkaCardDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AH_ZkaCardDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return AH_ZkaCardDialog_HandleValueChanged(dlg, sender);

  case GWEN_DialogEvent_TypeActivated:
    return AH_ZkaCardDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  default:
    break;
  }

  return GWEN_DialogEvent_ResultNotHandled;
}




