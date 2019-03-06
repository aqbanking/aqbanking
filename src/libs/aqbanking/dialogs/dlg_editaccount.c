/***************************************************************************
 begin       : Thu Apr 15 2010
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_editaccount_p.h"

#include "aqbanking/i18n_l.h"
#include "aqbanking/backendsupport/account.h"
#include "aqbanking/backendsupport/user.h"
#include "aqbanking/backendsupport/provider_be.h"
#include "aqbanking/dialogs/dlg_selectbankinfo.h"

#include <aqbanking/banking_be.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>


#define DIALOG_MINWIDTH  400
#define DIALOG_MINHEIGHT 300

#define USER_LIST_MINCOLWIDTH 50



GWEN_INHERIT(GWEN_DIALOG, AB_EDIT_ACCOUNT_DIALOG)




GWEN_DIALOG *AB_EditAccountDialog_new(AB_PROVIDER *pro, AB_ACCOUNT *a, int doLock)
{
  GWEN_DIALOG *dlg;
  AB_EDIT_ACCOUNT_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("ab_edit_account");
  GWEN_NEW_OBJECT(AB_EDIT_ACCOUNT_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AB_EDIT_ACCOUNT_DIALOG, dlg, xdlg,
                       AB_EditAccountDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, AB_EditAccountDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
                               "aqbanking/dialogs/dlg_editaccount.dlg",
                               fbuf);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Dialog description file not found (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }

  /* read dialog from dialog description file */
  rv=GWEN_Dialog_ReadXmlFile(dlg, GWEN_Buffer_GetStart(fbuf));
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }
  GWEN_Buffer_free(fbuf);

  xdlg->provider=pro;
  xdlg->banking=AB_Provider_GetBanking(pro);
  xdlg->account=a;
  xdlg->doLock=doLock;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB AB_EditAccountDialog_FreeData(void *bp, void *p)
{
  AB_EDIT_ACCOUNT_DIALOG *xdlg;

  xdlg=(AB_EDIT_ACCOUNT_DIALOG *) p;
  GWEN_FREE_OBJECT(xdlg);
}



static void createUserString(const AB_USER *u, GWEN_BUFFER *tbuf)
{
  const char *s;
  char numbuf[32];
  uint32_t uid;

  /* column 1 */
  uid=AB_User_GetUniqueId(u);
  snprintf(numbuf, sizeof(numbuf)-1, "%d", uid);
  numbuf[sizeof(numbuf)-1]=0;

  s=AB_User_GetUserName(u);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "-");

  s=AB_User_GetBankCode(u);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "-");

  s=AB_User_GetCustomerId(u);
  if (!(s && *s))
    s=AB_User_GetUserId(u);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);

  GWEN_Buffer_AppendString(tbuf, " (");
  GWEN_Buffer_AppendString(tbuf, numbuf);
  GWEN_Buffer_AppendString(tbuf, ")");

}



uint32_t AB_EditAccountDialog_GetCurrentUserId(GWEN_DIALOG *dlg)
{
  int idx;

  idx=GWEN_Dialog_GetIntProperty(dlg, "userCombo",  GWEN_DialogProperty_Value, 0, -1);
  if (idx>=0) {
    const char *currentText;

    currentText=GWEN_Dialog_GetCharProperty(dlg, "userCombo", GWEN_DialogProperty_Value, idx, NULL);
    if (currentText && *currentText) {
      unsigned long int id;

      if (sscanf(currentText, "%lu", &id)==1) {
        return (uint32_t) id;
      }
    }
  }

  return 0;
}



int AB_EditAccountDialog_FindUserEntry(GWEN_DIALOG *dlg, uint32_t userId)
{
  int i;
  int num;

  /* user list */
  num=GWEN_Dialog_GetIntProperty(dlg, "userCombo", GWEN_DialogProperty_ValueCount, 0, 0);
  for (i=0; i<num; i++) {
    const char *t;

    t=GWEN_Dialog_GetCharProperty(dlg, "userCombo", GWEN_DialogProperty_Value, i, NULL);
    if (t && *t) {
      unsigned long int id;

      if (sscanf(t, "%lu", &id)==1) {
        return i;
      }
    }
  } /* for i */

  return -1;
}



void AB_EditAccountDialog_RebuildUserLists(GWEN_DIALOG *dlg)
{
  AB_EDIT_ACCOUNT_DIALOG *xdlg;
  AB_USER_LIST *users;
  GWEN_STRINGLIST *sl;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_EDIT_ACCOUNT_DIALOG, dlg);
  assert(xdlg);

  GWEN_Dialog_SetIntProperty(dlg, "userCombo", GWEN_DialogProperty_ClearValues, 0, 0, 0);
  GWEN_Dialog_SetCharProperty(dlg,
                              "userCombo",
                              GWEN_DialogProperty_AddValue,
                              0,
                              I18N("-- select --"),
                              0);

  /* setup lists of available and selected users */
  sl=GWEN_StringList_new();
  users=AB_User_List_new();
  rv=AB_Provider_ReadUsers(xdlg->provider, users);
  if (rv<0) {

  }
  else {
    GWEN_BUFFER *tbuf;
    AB_USER *u;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);

    u=AB_User_List_First(users);
    while (u) {
      createUserString(u, tbuf);
      GWEN_StringList_AppendString(sl, GWEN_Buffer_GetStart(tbuf), 0, 1);
      GWEN_Buffer_Reset(tbuf);
      u=AB_User_List_Next(u);
    }
    GWEN_Buffer_free(tbuf);
  }
  AB_User_List_free(users);

  if (GWEN_StringList_Count(sl)) {
    GWEN_STRINGLISTENTRY *se;

    /* sort user list */
    GWEN_StringList_Sort(sl, 1, GWEN_StringList_SortModeNoCase);
    se=GWEN_StringList_FirstEntry(sl);
    while (se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s && *s)
        GWEN_Dialog_SetCharProperty(dlg,
                                    "userCombo",
                                    GWEN_DialogProperty_AddValue,
                                    0,
                                    s,
                                    0);
      se=GWEN_StringListEntry_Next(se);
    }
  }
  GWEN_StringList_free(sl);
}



void AB_EditAccountDialog_Init(GWEN_DIALOG *dlg)
{
  AB_EDIT_ACCOUNT_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;
  const char *s;
  AB_ACCOUNT_TYPE t;
  uint32_t uid;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_EDIT_ACCOUNT_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  /* init */
  GWEN_Dialog_SetCharProperty(dlg,
                              "",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("Edit Account"),
                              0);

  s=AB_Account_GetBankCode(xdlg->account);
  GWEN_Dialog_SetCharProperty(dlg, "bankCodeEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AB_Account_GetBankName(xdlg->account);
  GWEN_Dialog_SetCharProperty(dlg, "bankNameEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AB_Account_GetBic(xdlg->account);
  GWEN_Dialog_SetCharProperty(dlg, "bicEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AB_Account_GetAccountNumber(xdlg->account);
  GWEN_Dialog_SetCharProperty(dlg, "accountNumberEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AB_Account_GetAccountName(xdlg->account);
  GWEN_Dialog_SetCharProperty(dlg, "accountNameEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AB_Account_GetIban(xdlg->account);
  GWEN_Dialog_SetCharProperty(dlg, "ibanEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AB_Account_GetOwnerName(xdlg->account);
  GWEN_Dialog_SetCharProperty(dlg, "ownerNameEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AB_Account_GetCurrency(xdlg->account);
  GWEN_Dialog_SetCharProperty(dlg, "currencyEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AB_Account_GetCountry(xdlg->account);
  GWEN_Dialog_SetCharProperty(dlg, "countryEdit", GWEN_DialogProperty_Value, 0, s, 0);

  /* setup account type */
  GWEN_Dialog_SetCharProperty(dlg, "accountTypeCombo", GWEN_DialogProperty_AddValue, 0,
                              I18N("unknown"),
                              0);
  GWEN_Dialog_SetCharProperty(dlg, "accountTypeCombo", GWEN_DialogProperty_AddValue, 0,
                              I18N("Bank Account"),
                              0);
  GWEN_Dialog_SetCharProperty(dlg, "accountTypeCombo", GWEN_DialogProperty_AddValue, 0,
                              I18N("Credit Card Account"),
                              0);
  GWEN_Dialog_SetCharProperty(dlg, "accountTypeCombo", GWEN_DialogProperty_AddValue, 0,
                              I18N("Checking Account"),
                              0);
  GWEN_Dialog_SetCharProperty(dlg, "accountTypeCombo", GWEN_DialogProperty_AddValue, 0,
                              I18N("Savings Account"),
                              0);
  GWEN_Dialog_SetCharProperty(dlg, "accountTypeCombo", GWEN_DialogProperty_AddValue, 0,
                              I18N("Investment Account"),
                              0);
  GWEN_Dialog_SetCharProperty(dlg, "accountTypeCombo", GWEN_DialogProperty_AddValue, 0,
                              I18N("Cash Account"),
                              0);
  GWEN_Dialog_SetCharProperty(dlg, "accountTypeCombo", GWEN_DialogProperty_AddValue, 0,
                              I18N("Moneymarket Account"),
                              0);

  t=AB_Account_GetAccountType(xdlg->account);
  if (t<AB_AccountType_MoneyMarket)
    GWEN_Dialog_SetIntProperty(dlg, "accountTypeCombo", GWEN_DialogProperty_Value, 0, t, 0);

  AB_EditAccountDialog_RebuildUserLists(dlg);
  uid=AB_Account_GetUserId(xdlg->account);
  if (uid) {
    int idx;

    idx=AB_EditAccountDialog_FindUserEntry(dlg, uid);
    if (idx>=0)
      GWEN_Dialog_SetIntProperty(dlg, "userCombo", GWEN_DialogProperty_Value, 0, idx, 0);
  }


  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);
}



static void removeAllSpaces(uint8_t *s)
{
  uint8_t *d;

  d=s;
  while (*s) {
    if (*s>33)
      *(d++)=*s;
    s++;
  }
  *d=0;
}



int AB_EditAccountDialog_fromGui(GWEN_DIALOG *dlg, AB_ACCOUNT *a, int quiet)
{
  AB_EDIT_ACCOUNT_DIALOG *xdlg;
  int i;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_EDIT_ACCOUNT_DIALOG, dlg);
  assert(xdlg);

  /* fromGui */
  s=GWEN_Dialog_GetCharProperty(dlg, "accountNumberEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    removeAllSpaces((uint8_t *)GWEN_Buffer_GetStart(tbuf));
    s=GWEN_Buffer_GetStart(tbuf);
    if (a)
      AB_Account_SetAccountNumber(a, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "accountNameEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    if (a)
      AB_Account_SetAccountName(a, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "ibanEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    removeAllSpaces((uint8_t *)GWEN_Buffer_GetStart(tbuf));
    if (a)
      AB_Account_SetIban(a, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "ownerNameEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    if (a)
      AB_Account_SetOwnerName(a, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  /* get currency */
  s=GWEN_Dialog_GetCharProperty(dlg, "currencyEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (a && s && *s)
    AB_Account_SetCurrency(a, s);


  i=GWEN_Dialog_GetIntProperty(dlg, "accountTypeCombo", GWEN_DialogProperty_Value, 0, 0);
  if (a)
    AB_Account_SetAccountType(a, i);

  /*  get country */
  s=GWEN_Dialog_GetCharProperty(dlg, "countryEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (a && s && *s)
    AB_Account_SetCountry(a, s);

  s=GWEN_Dialog_GetCharProperty(dlg, "bankCodeEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    removeAllSpaces((uint8_t *)GWEN_Buffer_GetStart(tbuf));
    if (a)
      AB_Account_SetBankCode(a, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "bankNameEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    if (a)
      AB_Account_SetBankName(a, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "bicEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    removeAllSpaces((uint8_t *)GWEN_Buffer_GetStart(tbuf));
    if (a)
      AB_Account_SetBic(a, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  if (a) {
    uint32_t uid;

    uid=AB_EditAccountDialog_GetCurrentUserId(dlg);
    if (uid)
      AB_Account_SetUserId(a, uid);
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "No user selected.");
      return GWEN_ERROR_INVALID;
    }
  }

  return 0;
}



void AB_EditAccountDialog_Fini(GWEN_DIALOG *dlg)
{
  AB_EDIT_ACCOUNT_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_EDIT_ACCOUNT_DIALOG, dlg);
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



int AB_EditAccountDialog_HandleActivatedBankCode(GWEN_DIALOG *dlg)
{
  AB_EDIT_ACCOUNT_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_EDIT_ACCOUNT_DIALOG, dlg);
  assert(xdlg);

  dlg2=AB_SelectBankInfoDialog_new(xdlg->banking, "de", NULL);
  if (dlg2==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not create dialog");
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
      GWEN_Dialog_SetCharProperty(dlg,
                                  "bankCodeEdit",
                                  GWEN_DialogProperty_Value,
                                  0,
                                  (s && *s)?s:"",
                                  0);

      s=AB_BankInfo_GetBankName(bi);
      GWEN_Dialog_SetCharProperty(dlg,
                                  "bankNameEdit",
                                  GWEN_DialogProperty_Value,
                                  0,
                                  (s && *s)?s:"",
                                  0);

      s=AB_BankInfo_GetBic(bi);
      GWEN_Dialog_SetCharProperty(dlg,
                                  "bicEdit",
                                  GWEN_DialogProperty_Value,
                                  0,
                                  (s && *s)?s:"",
                                  0);
    }
  }
  GWEN_Dialog_free(dlg2);

  return GWEN_DialogEvent_ResultHandled;
}



int AB_EditAccountDialog_HandleActivatedOk(GWEN_DIALOG *dlg)
{
  AB_EDIT_ACCOUNT_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_EDIT_ACCOUNT_DIALOG, dlg);
  assert(xdlg);

  rv=AB_EditAccountDialog_fromGui(dlg, NULL, 0);
  if (rv<0) {
    return GWEN_DialogEvent_ResultHandled;
  }

  if (xdlg->doLock) {
    int rv;

    rv=AB_Provider_BeginExclUseAccount(xdlg->provider, xdlg->account);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL |
                          GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
                          GWEN_GUI_MSG_FLAGS_CONFIRM_B1,
                          I18N("Error"),
                          I18N("Unable to lock account. Maybe already in use?"),
                          I18N("Dismiss"),
                          NULL,
                          NULL,
                          0);
      return GWEN_DialogEvent_ResultHandled;
    }
  }

  AB_EditAccountDialog_fromGui(dlg, xdlg->account, 1);

  if (xdlg->doLock) {
    int rv;

    rv=AB_Provider_EndExclUseAccount(xdlg->provider, xdlg->account, 0);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL |
                          GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
                          GWEN_GUI_MSG_FLAGS_CONFIRM_B1,
                          I18N("Error"),
                          I18N("Unable to unlock account."),
                          I18N("Dismiss"),
                          NULL,
                          NULL,
                          0);
      AB_Provider_EndExclUseAccount(xdlg->provider, xdlg->account, 1);
      return GWEN_DialogEvent_ResultHandled;
    }
  }

  return GWEN_DialogEvent_ResultAccept;
}


int AB_EditAccountDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender)
{
  if (strcasecmp(sender, "bankCodeButton")==0)
    return AB_EditAccountDialog_HandleActivatedBankCode(dlg);
  else if (strcasecmp(sender, "okButton")==0)
    return AB_EditAccountDialog_HandleActivatedOk(dlg);
  else if (strcasecmp(sender, "abortButton")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "helpButton")==0) {
    /* TODO: open a help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB AB_EditAccountDialog_SignalHandler(GWEN_DIALOG *dlg,
                                                     GWEN_DIALOG_EVENTTYPE t,
                                                     const char *sender)
{
  AB_EDIT_ACCOUNT_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_EDIT_ACCOUNT_DIALOG, dlg);
  assert(xdlg);

  switch (t) {
  case GWEN_DialogEvent_TypeInit:
    AB_EditAccountDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AB_EditAccountDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    DBG_NOTICE(0, "ValueChanged: %s", sender);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeActivated:
    return AB_EditAccountDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}






