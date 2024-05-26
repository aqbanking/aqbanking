/***************************************************************************
 begin       : Thu Apr 15 2010
 copyright   : (C) 2024 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_editaccount_p.h"
#include "w_utils.h"
#include "w_accounttypecombo.h"
#include "w_usercombo.h"

#include "aqbanking/i18n_l.h"
#include "aqhbci/banking/provider_l.h"

#include "aqhbci/banking/account.h"
#include "aqhbci/banking/provider.h"
#include "aqhbci/banking/provider_online.h"

#include <aqbanking/backendsupport/account.h>
#include <aqbanking/banking_be.h>
#include <aqbanking/dialogs/dlg_selectbankinfo.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>



/* ------------------------------------------------------------------------------------------------
 * defines, types
 * ------------------------------------------------------------------------------------------------
 */

#define DIALOG_MINWIDTH  400
#define DIALOG_MINHEIGHT 300

#define USER_LIST_MINCOLWIDTH            50
#define TARGET_ACCOUNT_LIST_MINCOLWIDTH 100

/* for improved readability */
#define DLG_WITHPROGRESS 1
#define DLG_UMOUNT       0
#define DLG_DIALOGFILE   "aqbanking/backends/aqhbci/dialogs/dlg_editaccount.dlg"


typedef int (*_DIALOG_SIGNAL_HANDLER_FN)(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
typedef struct _DIALOG_SIGNAL_ENTRY _DIALOG_SIGNAL_ENTRY;
struct _DIALOG_SIGNAL_ENTRY {
  const char *sender;
  GWEN_DIALOG_EVENTTYPE eventType;
  _DIALOG_SIGNAL_HANDLER_FN handlerFn;
};


typedef const char*(*_ACCOUNT_GETCHARVALUE_FN)(const AB_ACCOUNT *acc);
typedef void (*_ACCOUNT_SETCHARVALUE_FN)(AB_ACCOUNT *acc, const char *s);




/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void GWENHYWFAR_CB _freeData(void *bp, void *p);
static int GWENHYWFAR_CB _dlgApi_signalHandler(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static void _createTargetAccountListBoxString(const AB_REFERENCE_ACCOUNT *ra, GWEN_BUFFER *tbuf);

static void _targetAccountListBoxSetup(GWEN_DIALOG *dlg, const char *widgetName);
static void _targetAccountListBoxRebuild(GWEN_DIALOG *dlg);

static void _accountFlagsToGui(GWEN_DIALOG *dlg, uint32_t aflags);
static uint32_t _accountFlagsFromGui(GWEN_DIALOG *dlg);

static void _toGui(GWEN_DIALOG *dlg, const AB_ACCOUNT *account);
static int _fromGui(GWEN_DIALOG *dlg, AB_ACCOUNT *a, int quiet);
static int _handleDialogInit(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleDialogFini(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedBankCode(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedOk(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedReject(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedSepa(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _handleActivatedTargetAcc(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);



/* ------------------------------------------------------------------------------------------------
 * static vars
 * ------------------------------------------------------------------------------------------------
 */

GWEN_INHERIT(GWEN_DIALOG, AH_EDIT_ACCOUNT_DIALOG)

static _DIALOG_SIGNAL_ENTRY _signalMap[]={
  {NULL,                 GWEN_DialogEvent_TypeInit, _handleDialogInit},
  {NULL,                 GWEN_DialogEvent_TypeFini, _handleDialogFini},
  {"bankCodeButton",     GWEN_DialogEvent_TypeActivated, _handleActivatedBankCode},
  {"okButton",           GWEN_DialogEvent_TypeActivated, _handleActivatedOk},
  {"abortButton",        GWEN_DialogEvent_TypeActivated, _handleActivatedReject},
  {"getSepaButton",      GWEN_DialogEvent_TypeActivated, _handleActivatedSepa},
  {"getTargetAccButton", GWEN_DialogEvent_TypeActivated, _handleActivatedTargetAcc},

  {NULL, 0, NULL}
};



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

GWEN_DIALOG *AH_EditAccountDialog_new(AB_PROVIDER *pro, AB_ACCOUNT *a, int doLock)
{
  GWEN_DIALOG *dlg;
  AH_EDIT_ACCOUNT_DIALOG *xdlg;

  dlg=GWEN_Dialog_CreateAndLoadWithPath("ah_edit_account", AB_PM_LIBNAME, AB_PM_DATADIR, DLG_DIALOGFILE);
  if (dlg==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return NULL;
  }

  GWEN_NEW_OBJECT(AH_EDIT_ACCOUNT_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AH_EDIT_ACCOUNT_DIALOG, dlg, xdlg, _freeData);
  GWEN_Dialog_SetSignalHandler(dlg, _dlgApi_signalHandler);

  xdlg->provider=pro;
  xdlg->banking=AB_Provider_GetBanking(pro);
  xdlg->account=a;
  xdlg->doLock=doLock;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB _freeData(void *bp, void *p)
{
  AH_EDIT_ACCOUNT_DIALOG *xdlg;

  xdlg=(AH_EDIT_ACCOUNT_DIALOG *) p;
  GWEN_FREE_OBJECT(xdlg);
}



void _toGui(GWEN_DIALOG *dlg, const AB_ACCOUNT *account)
{
  AH_Widget_AccountToGuiText(dlg, "bankCodeEdit",      account, AB_Account_GetBankCode);
  AH_Widget_AccountToGuiText(dlg, "bankNameEdit",      account, AB_Account_GetBankName);
  AH_Widget_AccountToGuiText(dlg, "bicEdit",           account, AB_Account_GetBic);
  AH_Widget_AccountToGuiText(dlg, "accountNumberEdit", account, AB_Account_GetAccountNumber);
  AH_Widget_AccountToGuiText(dlg, "accountNameEdit",   account, AB_Account_GetAccountName);
  AH_Widget_AccountToGuiText(dlg, "ibanEdit",          account, AB_Account_GetIban);
  AH_Widget_AccountToGuiText(dlg, "ownerNameEdit",     account, AB_Account_GetOwnerName);
  AH_Widget_AccountToGuiText(dlg, "currencyEdit",      account, AB_Account_GetCurrency);
  AH_Widget_AccountToGuiText(dlg, "countryEdit",       account, AB_Account_GetCountry);
  AH_Widget_AccountTypeComboSetCurrent(dlg, "accountTypeCombo", AB_Account_GetAccountType(account));
  AH_Widget_UserComboSetCurrent(dlg, "userCombo", AB_Account_GetUserId(account));
  _accountFlagsToGui(dlg, AH_Account_GetFlags(account));

  _targetAccountListBoxRebuild(dlg);
}



int _fromGui(GWEN_DIALOG *dlg, AB_ACCOUNT *a, int quiet)
{
  AB_ACCOUNT_TYPE t;
  uint32_t uid;

  if (AH_Widget_GuiTextToAccountDeleSpaces(dlg, "accountNumberEdit", a, AB_Account_SetAccountNumber, NULL)<0 ||
      AH_Widget_GuiTextToAccountKeepSpaces(dlg, "accountNameEdit",   a, AB_Account_SetAccountName,   NULL)<0 ||
      AH_Widget_GuiTextToAccountDeleSpaces(dlg, "ibanEdit",          a, AB_Account_SetIban,          NULL)<0 ||
      AH_Widget_GuiTextToAccountKeepSpaces(dlg, "ownerNameEdit",     a, AB_Account_SetOwnerName,
                                           quiet?NULL:I18N("Missing owner name"))<0 ||
      AH_Widget_GuiTextToAccountKeepSpaces(dlg, "currencyEdit",      a, AB_Account_SetCurrency,      NULL)<0 ||
      AH_Widget_GuiTextToAccountKeepSpaces(dlg, "countryEdit",       a, AB_Account_SetCountry,       NULL)<0 ||
      AH_Widget_GuiTextToAccountDeleSpaces(dlg, "bankCodeEdit",      a, AB_Account_SetBankCode,      NULL)<0 ||
      AH_Widget_GuiTextToAccountKeepSpaces(dlg, "bankNameEdit",      a, AB_Account_SetBankName,      NULL)<0 ||
      AH_Widget_GuiTextToAccountDeleSpaces(dlg, "bicEdit",           a, AB_Account_SetBic,           NULL)<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return GWEN_ERROR_INVALID;
  }

  if (a &&
      !quiet &&
      AB_Account_GetIban(a)==NULL &&
      AB_Account_GetAccountNumber(a)==NULL &&
      AB_Account_GetAccountName(a)==NULL) {
    GWEN_Gui_ShowError(I18N("Error on Input"), "%s", I18N("At least one of IBAN, account number or account name required."));
    GWEN_Dialog_SetIntProperty(dlg, "ibanEdit", GWEN_DialogProperty_Focus, 0, 1, 0);
    return GWEN_ERROR_INVALID;
  }

  t=AH_Widget_AccountTypeComboGetCurrent(dlg, "accountTypeCombo");
  if (t==AB_AccountType_Unknown || t==AB_AccountType_Invalid) {
    DBG_ERROR(NULL, "Account type not selected");
    if (!quiet) {
      GWEN_Gui_ShowError(I18N("Error on Input"), "%s", I18N("Please select account type."));
      GWEN_Dialog_SetIntProperty(dlg, "accountTypeCombo", GWEN_DialogProperty_Focus, 0, 1, 0);
      return GWEN_ERROR_INVALID;
    }
  }
  if (a)
    AB_Account_SetAccountType(a, t);

  if (a)
    AH_Account_SetFlags(a, _accountFlagsFromGui(dlg));

  uid=AH_Widget_UserComboGetCurrent(dlg, "userCombo");
  if (uid==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "User not selected");
    if (!quiet) {
      GWEN_Gui_ShowError(I18N("Error on Input"), "%s", I18N("Please select a user for this account"));
      GWEN_Dialog_SetIntProperty(dlg, "userCombo", GWEN_DialogProperty_Focus, 0, 1, 0);
      return GWEN_ERROR_INVALID;
    }
  }
  if (a)
    AB_Account_SetUserId(a, uid);

  return 0;
}



int GWENHYWFAR_CB _dlgApi_signalHandler(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  const _DIALOG_SIGNAL_ENTRY *entry;

  entry=_signalMap;
  while(entry->handlerFn) {
    if (entry->eventType==t && (entry->sender==NULL || (sender && strcasecmp(sender, entry->sender)==0))) {
      return entry->handlerFn(dlg, t, sender);
    }
    entry++;
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int _handleDialogInit(GWEN_DIALOG *dlg, GWEN_UNUSED GWEN_DIALOG_EVENTTYPE t, GWEN_UNUSED const char *sender)
{
  AH_EDIT_ACCOUNT_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_ACCOUNT_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  /* init */
  GWEN_Dialog_SetCharProperty(dlg, "", GWEN_DialogProperty_Title, 0, I18N("Edit Account"), 0);

  AH_Widget_AccountTypeComboSetup(dlg, "accountTypeCombo");
  _targetAccountListBoxSetup(dlg, "targetAccountListBox");
  AH_Widget_UserComboRebuild(dlg, "userCombo", xdlg->provider);

  _toGui(dlg, xdlg->account);

  /* read account column widths */
  GWEN_Dialog_ListReadColumnSettings(dlg, "targetAccountListBox", "target_account_list_", 2, TARGET_ACCOUNT_LIST_MINCOLWIDTH, dbPrefs);

  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);

  return GWEN_DialogEvent_ResultHandled;
}



int _handleDialogFini(GWEN_DIALOG *dlg, GWEN_UNUSED GWEN_DIALOG_EVENTTYPE t, GWEN_UNUSED const char *sender)
{
  AH_EDIT_ACCOUNT_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_ACCOUNT_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  /* store column widths of target account list */
  GWEN_Dialog_ListWriteColumnSettings(dlg, "targetAccountListBox", "target_account_list_", 2, TARGET_ACCOUNT_LIST_MINCOLWIDTH, dbPrefs);

  /* store dialog width */
  i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, -1);
  GWEN_DB_SetIntValue(dbPrefs, GWEN_DB_FLAGS_OVERWRITE_VARS, "dialog_width", i);

  /* store dialog height */
  i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, -1);
  GWEN_DB_SetIntValue(dbPrefs, GWEN_DB_FLAGS_OVERWRITE_VARS, "dialog_height", i);

  return GWEN_DialogEvent_ResultHandled;
}



int _handleActivatedBankCode(GWEN_DIALOG *dlg, GWEN_UNUSED GWEN_DIALOG_EVENTTYPE t, GWEN_UNUSED const char *sender)
{
  AH_EDIT_ACCOUNT_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_ACCOUNT_DIALOG, dlg);
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

      s=AB_BankInfo_GetBankId(bi);
      GWEN_Dialog_SetCharProperty(dlg, "bankCodeEdit", GWEN_DialogProperty_Value, 0, (s && *s)?s:"", 0);

      s=AB_BankInfo_GetBankName(bi);
      GWEN_Dialog_SetCharProperty(dlg, "bankNameEdit", GWEN_DialogProperty_Value, 0, (s && *s)?s:"", 0);

      s=AB_BankInfo_GetBic(bi);
      GWEN_Dialog_SetCharProperty(dlg, "bicEdit", GWEN_DialogProperty_Value, 0, (s && *s)?s:"", 0);
    }
  }
  GWEN_Dialog_free(dlg2);

  return GWEN_DialogEvent_ResultHandled;
}



int _handleActivatedOk(GWEN_DIALOG *dlg, GWEN_UNUSED GWEN_DIALOG_EVENTTYPE t, GWEN_UNUSED const char *sender)
{
  AH_EDIT_ACCOUNT_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_ACCOUNT_DIALOG, dlg);
  assert(xdlg);

  rv=_fromGui(dlg, NULL, 0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Incomplete GUI input");
    return GWEN_DialogEvent_ResultHandled;
  }

  if (xdlg->doLock) {
    int rv;

    rv=AB_Provider_BeginExclUseAccount(xdlg->provider, xdlg->account);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Unable to lock account. Maybe already in use?"));
      return GWEN_DialogEvent_ResultHandled;
    }
  }

  _fromGui(dlg, xdlg->account, 1);

  if (xdlg->doLock) {
    int rv;

    rv=AB_Provider_EndExclUseAccount(xdlg->provider, xdlg->account, 0);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Unable to unlock account."));
      AB_Provider_EndExclUseAccount(xdlg->provider, xdlg->account, 1);
      return GWEN_DialogEvent_ResultHandled;
    }
  }

  rv=AB_Provider_WriteAccountSpecForAccount(xdlg->provider, xdlg->account, 1);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Unable to update account spec."));
  }


  return GWEN_DialogEvent_ResultAccept;
}


int _handleActivatedReject(GWEN_DIALOG *dlg, GWEN_UNUSED GWEN_DIALOG_EVENTTYPE t, GWEN_UNUSED const char *sender)
{
  return GWEN_DialogEvent_ResultReject;
}



int _handleActivatedSepa(GWEN_DIALOG *dlg, GWEN_UNUSED GWEN_DIALOG_EVENTTYPE t, GWEN_UNUSED const char *sender)
{
  AH_EDIT_ACCOUNT_DIALOG *xdlg;
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_ACCOUNT_DIALOG, dlg);
  assert(xdlg);

  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetAccountSepaInfo(xdlg->provider, xdlg->account, ctx, DLG_WITHPROGRESS, DLG_UMOUNT, xdlg->doLock);
  AB_ImExporterContext_free(ctx);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }
  else {
    /* update dialog */
    _toGui(dlg, xdlg->account);
  }

  return GWEN_DialogEvent_ResultHandled;
}



void _accountFlagsToGui(GWEN_DIALOG *dlg, uint32_t aflags)
{
  GWEN_Dialog_SetIntProperty(dlg, "preferSingleTransferCheck", GWEN_DialogProperty_Value, 0,
                             (aflags & AH_BANK_FLAGS_PREFER_SINGLE_TRANSFER)?1:0, 0);
  GWEN_Dialog_SetIntProperty(dlg, "preferSingleDebitNoteCheck", GWEN_DialogProperty_Value, 0,
                             (aflags & AH_BANK_FLAGS_PREFER_SINGLE_DEBITNOTE)?1:0, 0);
  GWEN_Dialog_SetIntProperty(dlg, "sepaPreferSingleTransferCheck", GWEN_DialogProperty_Value, 0,
                             (aflags & AH_BANK_FLAGS_SEPA_PREFER_SINGLE_TRANSFER)?1:0, 0);
  GWEN_Dialog_SetIntProperty(dlg, "sepaPreferSingleDebitNoteCheck", GWEN_DialogProperty_Value, 0,
                             (aflags & AH_BANK_FLAGS_SEPA_PREFER_SINGLE_DEBITNOTE)?1:0, 0);
  GWEN_Dialog_SetIntProperty(dlg, "preferCamtDownloadCheck", GWEN_DialogProperty_Value, 0,
                             (aflags & AH_BANK_FLAGS_PREFER_CAMT_DOWNLOAD)?1:0, 0);
}



uint32_t _accountFlagsFromGui(GWEN_DIALOG *dlg)
{
  uint32_t aflags=0;

  if (GWEN_Dialog_GetIntProperty(dlg, "preferSingleTransferCheck", GWEN_DialogProperty_Value, 0, 0))
    aflags|=AH_BANK_FLAGS_PREFER_SINGLE_TRANSFER;
  if (GWEN_Dialog_GetIntProperty(dlg, "preferSingleDebitNoteCheck", GWEN_DialogProperty_Value, 0, 0))
    aflags|=AH_BANK_FLAGS_PREFER_SINGLE_DEBITNOTE;
  if (GWEN_Dialog_GetIntProperty(dlg, "sepaPreferSingleTransferCheck", GWEN_DialogProperty_Value, 0, 0))
    aflags|=AH_BANK_FLAGS_SEPA_PREFER_SINGLE_TRANSFER;
  if (GWEN_Dialog_GetIntProperty(dlg, "sepaPreferSingleDebitNoteCheck", GWEN_DialogProperty_Value, 0, 0))
    aflags|=AH_BANK_FLAGS_SEPA_PREFER_SINGLE_DEBITNOTE;
  if (GWEN_Dialog_GetIntProperty(dlg, "preferCamtDownloadCheck", GWEN_DialogProperty_Value, 0, 0))
    aflags|=AH_BANK_FLAGS_PREFER_CAMT_DOWNLOAD;
  return aflags;
}



int _handleActivatedTargetAcc(GWEN_DIALOG *dlg, GWEN_UNUSED GWEN_DIALOG_EVENTTYPE t, GWEN_UNUSED const char *sender)
{
  AH_EDIT_ACCOUNT_DIALOG *xdlg;
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_ACCOUNT_DIALOG, dlg);
  assert(xdlg);

  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetTargetAccount(xdlg->provider, xdlg->account, ctx, DLG_WITHPROGRESS, DLG_UMOUNT, xdlg->doLock);
  AB_ImExporterContext_free(ctx);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }
  else {
    /* update target account list */
    _targetAccountListBoxRebuild(dlg);
  }

  return GWEN_DialogEvent_ResultHandled;
}



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *                                                _targetAccountList
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */



void _targetAccountListBoxSetup(GWEN_DIALOG *dlg, const char *widgetName)
{
  GWEN_Dialog_SetCharProperty(dlg, widgetName, GWEN_DialogProperty_Title, 0, I18N("Account Name\tIBAN"), 0);
}



void _targetAccountListBoxRebuild(GWEN_DIALOG *dlg)
{
  AH_EDIT_ACCOUNT_DIALOG *xdlg;
  int i;
  AB_ACCOUNT_SPEC *as=NULL;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_ACCOUNT_DIALOG, dlg);
  assert(xdlg);

  /* target account list */
  i=0;
  GWEN_Dialog_SetIntProperty(dlg, "targetAccountListBox", GWEN_DialogProperty_ClearValues, 0, 0, 0);

  AB_Banking_GetAccountSpecByUniqueId(AB_Provider_GetBanking(xdlg->provider),
                                      AB_Account_GetUniqueId(xdlg->account),
                                      &as);
  if (as) {
    AB_REFERENCE_ACCOUNT_LIST *ral;
    AB_REFERENCE_ACCOUNT *ra;

    ral=AB_AccountSpec_GetRefAccountList(as);
    if (AB_ReferenceAccount_List_GetCount(ral)) {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);

      ra=AB_ReferenceAccount_List_First(ral);
      while (ra) {
        _createTargetAccountListBoxString(ra, tbuf);
        GWEN_Dialog_SetCharProperty(dlg,
                                    "targetAccountListBox",
                                    GWEN_DialogProperty_AddValue,
                                    0,
                                    GWEN_Buffer_GetStart(tbuf),
                                    0);
        i++;
        GWEN_Buffer_Reset(tbuf);

        ra=AB_ReferenceAccount_List_Next(ra);
      }
      GWEN_Buffer_free(tbuf);
    } /* if account list not empty */
    AB_AccountSpec_free(as);
  }

  GWEN_Dialog_SetIntProperty(dlg, "targetAccountListBox", GWEN_DialogProperty_Sort, 0, 0, 0);
  if (i)
    GWEN_Dialog_SetIntProperty(dlg, "targetAccountListBox", GWEN_DialogProperty_Value, 0, 0, 0);
}



void _createTargetAccountListBoxString(const AB_REFERENCE_ACCOUNT *ra, GWEN_BUFFER *tbuf)
{
  const char *s;

  s=AB_ReferenceAccount_GetAccountName(ra);
  GWEN_Buffer_AppendString(tbuf, s?s:"");
  GWEN_Buffer_AppendString(tbuf, "\t");
  s=AB_ReferenceAccount_GetIban(ra);
  GWEN_Buffer_AppendString(tbuf, s?s:"");
}






