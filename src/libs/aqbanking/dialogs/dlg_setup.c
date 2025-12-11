/***************************************************************************
 begin       : Wed Apr 14 2010
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_setup_p.h"

#include "aqbanking/i18n_l.h"
#include "aqbanking/backendsupport/user.h"
#include "aqbanking/dialogs/dlg_selectbackend.h"
#include "aqbanking/dialogs/dlg_editaccount.h"
#include "aqbanking/dialogs/dlg_edituser.h"
#include "aqbanking/dialogs/dlg_setup_newuser.h"

#include <aqbanking/banking_be.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>


#define DIALOG_MINWIDTH  400
#define DIALOG_MINHEIGHT 100

#define USER_LIST_MINCOLWIDTH    50
#define ACCOUNT_LIST_MINCOLWIDTH 50


GWEN_INHERIT(GWEN_DIALOG, AB_SETUP_DIALOG)




GWEN_DIALOG *AB_SetupDialog_new(AB_BANKING *ab)
{
  GWEN_DIALOG *dlg;
  AB_SETUP_DIALOG *xdlg;

  dlg=GWEN_Dialog_CreateAndLoadWithPath("ab_setup", AB_PM_LIBNAME, AB_PM_DATADIR, "aqbanking/dialogs/dlg_setup.dlg");
  if (dlg==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not create dialog \"ab_setup\".");
    return NULL;
  }

  GWEN_NEW_OBJECT(AB_SETUP_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg, xdlg, AB_SetupDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, AB_SetupDialog_SignalHandler);

  xdlg->banking=ab;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB AB_SetupDialog_FreeData(void *bp, void *p)
{
  AB_SETUP_DIALOG *xdlg;

  xdlg=(AB_SETUP_DIALOG *) p;

  AB_User_List_free(xdlg->currentUserList);
  AB_Account_List_free(xdlg->currentAccountList);
  AB_Provider_List2_free(xdlg->providersInUse);

  GWEN_FREE_OBJECT(xdlg);
}



static void createUserListBoxString(const AB_USER *u, GWEN_BUFFER *tbuf)
{
  const char *s;
  char numbuf[32];
  uint32_t uid;

  /* column 1 */
  uid=AB_User_GetUniqueId(u);
  snprintf(numbuf, sizeof(numbuf)-1, "%09d", uid);
  numbuf[sizeof(numbuf)-1]=0;
  GWEN_Buffer_AppendString(tbuf, numbuf);
  GWEN_Buffer_AppendString(tbuf, "\t");

  /* column 2 */
  s=AB_User_GetBankCode(u);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "\t");

  /* column 3 */
  s=AB_User_GetUserId(u);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "\t");

  /* column 4 */
  s=AB_User_GetCustomerId(u);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "\t");

  /* column 5 */
  s=AB_User_GetUserName(u);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "\t");

  /* column 6 */
  s=AB_User_GetBackendName(u);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
}



static void createAccountListBoxString(const AB_ACCOUNT *a, GWEN_BUFFER *tbuf)
{
  const char *s;
  char numbuf[32];
  uint32_t uid;

  /* column 1 */
  uid=AB_Account_GetUniqueId(a);
  snprintf(numbuf, sizeof(numbuf)-1, "%09d", uid);
  numbuf[sizeof(numbuf)-1]=0;
  GWEN_Buffer_AppendString(tbuf, numbuf);
  GWEN_Buffer_AppendString(tbuf, "\t");

  /* column 2 */
  s=AB_Account_GetBankCode(a);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "\t");

  /* column 3 */
  s=AB_Account_GetBankName(a);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "\t");

  /* column 4 */
  s=AB_Account_GetAccountNumber(a);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "\t");

  /* column 5 */
  s=AB_Account_GetAccountName(a);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "\t");

  /* column 6 */
  s=AB_Account_GetOwnerName(a);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "\t");

  /* column 7 */
  s=AB_Account_GetBackendName(a);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
}



uint32_t AB_SetupDialog_GetCurrentId(GWEN_DIALOG *dlg, const char *comboBoxName)
{
  int idx;

  idx=GWEN_Dialog_GetIntProperty(dlg, comboBoxName, GWEN_DialogProperty_Value, 0, -1);
  if (idx>=0) {
    const char *currentText;

    currentText=GWEN_Dialog_GetCharProperty(dlg, comboBoxName, GWEN_DialogProperty_Value, idx, NULL);
    if (currentText && *currentText) {
      unsigned long int uid=0;

      if (1==sscanf(currentText, "%09lu", &uid)) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Got id \"%lu id from \"%s\"", uid, currentText);
        return (uint32_t) uid;
      }
      else {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "No id found in \"%s\"", currentText);
      }
    }
  }

  return 0;
}


uint32_t AB_SetupDialog_GetCurrentUserId(GWEN_DIALOG *dlg)
{
  return AB_SetupDialog_GetCurrentId(dlg, "userListBox");
}



uint32_t AB_SetupDialog_GetCurrentAccountId(GWEN_DIALOG *dlg)
{
  return AB_SetupDialog_GetCurrentId(dlg, "accountListBox");
}



int AB_SetupDialog_UserChanged(GWEN_DIALOG *dlg)
{
  AB_SETUP_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  return GWEN_DialogEvent_ResultHandled;
}



int AB_SetupDialog_AccountChanged(GWEN_DIALOG *dlg)
{
  AB_SETUP_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  return GWEN_DialogEvent_ResultHandled;
}



static void AB_SetupDialog_ActivateProviders(GWEN_DIALOG *dlg)
{
  AB_SETUP_DIALOG *xdlg;
  GWEN_PLUGIN_DESCRIPTION_LIST2 *pdl;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->providersInUse)
    AB_Provider_List2_free(xdlg->providersInUse);
  xdlg->providersInUse=AB_Provider_List2_new();

  pdl=AB_Banking_GetProviderDescrs(xdlg->banking);
  if (pdl) {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *it;

    it=GWEN_PluginDescription_List2_First(pdl);
    if (it) {
      GWEN_PLUGIN_DESCRIPTION *pd;

      pd=GWEN_PluginDescription_List2Iterator_Data(it);
      while (pd) {
        const char *pName;

        pName=GWEN_PluginDescription_GetName(pd);
        if (pName && *pName) {
          AB_PROVIDER *pro;

          pro=AB_Banking_BeginUseProvider(xdlg->banking, pName);
          if (pro) {
            DBG_INFO(AQBANKING_LOGDOMAIN, "Adding provider %s", pName);
            AB_Provider_List2_PushBack(xdlg->providersInUse, pro);
          }
          else {
            DBG_INFO(AQBANKING_LOGDOMAIN, "Provider %s not available", pName);
          }
        }
        pd=GWEN_PluginDescription_List2Iterator_Next(it);
      }
      GWEN_PluginDescription_List2Iterator_free(it);
    }
    GWEN_PluginDescription_List2_freeAll(pdl);
  }
}



static void AB_SetupDialog_DeactivateProviders(GWEN_DIALOG *dlg)
{
  AB_SETUP_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->providersInUse) {
    AB_PROVIDER_LIST2_ITERATOR *it;

    it=AB_Provider_List2_First(xdlg->providersInUse);
    if (it) {
      AB_PROVIDER *pro;

      pro=AB_Provider_List2Iterator_Data(it);
      while (pro) {
        AB_Banking_EndUseProvider(xdlg->banking, pro);
        pro=AB_Provider_List2Iterator_Next(it);
      }
      AB_Provider_List2Iterator_free(it);
    }
    AB_Provider_List2_free(xdlg->providersInUse);
    xdlg->providersInUse=NULL;
  }
}



static AB_PROVIDER *AB_SetupDialog_GetProviderByName(GWEN_DIALOG *dlg, const char *pName)
{
  AB_SETUP_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->providersInUse) {
    AB_PROVIDER_LIST2_ITERATOR *it;

    it=AB_Provider_List2_First(xdlg->providersInUse);
    if (it) {
      AB_PROVIDER *pro;

      pro=AB_Provider_List2Iterator_Data(it);
      while (pro) {
        const char *s;

        s=AB_Provider_GetName(pro);
        if (s && *s && strcasecmp(pName, s)==0) {
          AB_Provider_List2Iterator_free(it);
          return pro;
        }
        pro=AB_Provider_List2Iterator_Next(it);
      }
      AB_Provider_List2Iterator_free(it);
    }
  }

  return NULL;
}



static void AB_SetupDialog_LoadUsers(GWEN_DIALOG *dlg, AB_USER_LIST *ul)
{
  AB_SETUP_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->providersInUse) {
    AB_PROVIDER_LIST2_ITERATOR *it;

    it=AB_Provider_List2_First(xdlg->providersInUse);
    if (it) {
      AB_PROVIDER *pro;

      pro=AB_Provider_List2Iterator_Data(it);
      while (pro) {
        int rv;

        rv=AB_Provider_ReadUsers(pro, ul);
        if (rv<0) {
          DBG_INFO(AQBANKING_LOGDOMAIN, "Error reading users from backends \"%s\": %d", AB_Provider_GetName(pro), rv);
        }
        pro=AB_Provider_List2Iterator_Next(it);
      }
      AB_Provider_List2Iterator_free(it);
    }
  }
}



static void AB_SetupDialog_LoadAccounts(GWEN_DIALOG *dlg, AB_ACCOUNT_LIST *al)
{
  AB_SETUP_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->providersInUse) {
    AB_PROVIDER_LIST2_ITERATOR *it;

    it=AB_Provider_List2_First(xdlg->providersInUse);
    if (it) {
      AB_PROVIDER *pro;

      pro=AB_Provider_List2Iterator_Data(it);
      while (pro) {
        int rv;

        rv=AB_Provider_ReadAccounts(pro, al);
        if (rv<0) {
          DBG_INFO(AQBANKING_LOGDOMAIN, "Error reading accounts from backends \"%s\": %d", AB_Provider_GetName(pro), rv);
        }
        pro=AB_Provider_List2Iterator_Next(it);
      }
      AB_Provider_List2Iterator_free(it);
    }
  }
}



static void AB_SetupDialog_Reload(GWEN_DIALOG *dlg)
{
  AB_SETUP_DIALOG *xdlg;
  AB_USER *u;
  AB_ACCOUNT *a;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  /* user list */
  i=0;
  GWEN_Dialog_SetIntProperty(dlg, "userListBox", GWEN_DialogProperty_ClearValues, 0, 0, 0);
  if (xdlg->currentUserList)
    AB_User_List_free(xdlg->currentUserList);
  xdlg->currentUserList=AB_User_List_new();
  AB_SetupDialog_LoadUsers(dlg, xdlg->currentUserList);
  if (AB_User_List_GetCount(xdlg->currentUserList)) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);

    u=AB_User_List_First(xdlg->currentUserList);
    while (u) {
      createUserListBoxString(u, tbuf);
      GWEN_Dialog_SetCharProperty(dlg,
                                  "userListBox",
                                  GWEN_DialogProperty_AddValue,
                                  0,
                                  GWEN_Buffer_GetStart(tbuf),
                                  0);
      i++;
      GWEN_Buffer_Reset(tbuf);

      u=AB_User_List_Next(u);
    }
    GWEN_Buffer_free(tbuf);
  } /* if user list not empty */
  GWEN_Dialog_SetIntProperty(dlg, "userListBox", GWEN_DialogProperty_Sort, 0, 0, 0);
  if (i)
    GWEN_Dialog_SetIntProperty(dlg, "userListBox", GWEN_DialogProperty_Value, 0, 0, 0);

  /* account list */
  i=0;
  GWEN_Dialog_SetIntProperty(dlg, "accountListBox", GWEN_DialogProperty_ClearValues, 0, 0, 0);
  if (xdlg->currentAccountList)
    AB_Account_List_free(xdlg->currentAccountList);
  xdlg->currentAccountList=AB_Account_List_new();
  AB_SetupDialog_LoadAccounts(dlg, xdlg->currentAccountList);
  if (AB_Account_List_GetCount(xdlg->currentAccountList)) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);

    a=AB_Account_List_First(xdlg->currentAccountList);
    while (a) {
      createAccountListBoxString(a, tbuf);
      GWEN_Dialog_SetCharProperty(dlg,
                                  "accountListBox",
                                  GWEN_DialogProperty_AddValue,
                                  0,
                                  GWEN_Buffer_GetStart(tbuf),
                                  0);
      i++;
      GWEN_Buffer_Reset(tbuf);

      a=AB_Account_List_Next(a);
    }
    GWEN_Buffer_free(tbuf);
  } /* if account list not empty */
  GWEN_Dialog_SetIntProperty(dlg, "accountListBox", GWEN_DialogProperty_Sort, 0, 0, 0);
  if (i)
    GWEN_Dialog_SetIntProperty(dlg, "accountListBox", GWEN_DialogProperty_Value, 0, 0, 0);

  AB_SetupDialog_UserChanged(dlg);
  AB_SetupDialog_AccountChanged(dlg);
}

void AB_SetupDialog_InfoPage(GWEN_DIALOG *dlg)
{
  AB_SETUP_DIALOG *xdlg;
  GWEN_BUFFER *buf;
  int rv;
  int vmajor, vminor, vpatchLevel, vbuild;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  buf=GWEN_Buffer_new(0, 1024, 0, 1);

  AB_Banking_GetVersion(&vmajor, &vminor, &vpatchLevel, &vbuild);
  sprintf(GWEN_Buffer_GetStart(buf), "%d.%d.%d.%d\n", vmajor, vminor, vpatchLevel, vbuild);
  GWEN_Dialog_SetCharProperty(dlg,
                              "aqVersion",
                              GWEN_DialogProperty_Value,
                              0,
                              GWEN_Buffer_GetStart(buf),
                              0);

  GWEN_Version(&vmajor, &vminor, &vpatchLevel, &vbuild);
  sprintf(GWEN_Buffer_GetStart(buf), "%d.%d.%d.%d\n", vmajor, vminor, vpatchLevel, vbuild);
  GWEN_Dialog_SetCharProperty(dlg,
                              "gwenVersion",
                              GWEN_DialogProperty_Value,
                              0,
                              GWEN_Buffer_GetStart(buf),
                              0);

  rv=AB_Banking_GetUserDataDir(xdlg->banking, buf);
  if (rv>=0) {
    GWEN_Dialog_SetCharProperty(dlg,
                                "configPath",
                                GWEN_DialogProperty_Value,
                                0,
                                GWEN_Buffer_GetStart(buf),
                                0);
  }

  GWEN_Buffer_free(buf);
}

void AB_SetupDialog_Init(GWEN_DIALOG *dlg)
{
  AB_SETUP_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
                              "",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("AqBanking Setup"),
                              0);

  /* user list */
  GWEN_Dialog_SetCharProperty(dlg,
                              "userListBox",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("Id\tBank Code\tUser Id\tCustomer Id\tUser Name\tModule"),
                              0);
  GWEN_Dialog_SetIntProperty(dlg,
                             "userListBox",
                             GWEN_DialogProperty_SelectionMode,
                             0,
                             GWEN_Dialog_SelectionMode_Single,
                             0);

  /* account list */
  GWEN_Dialog_SetCharProperty(dlg,
                              "accountListBox",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("Id\tBank Code\tBank Name\tAccount Number\tAccount Name\tOwner Name\tModule"),
                              0);
  GWEN_Dialog_SetIntProperty(dlg,
                             "accountListBox",
                             GWEN_DialogProperty_SelectionMode,
                             0,
                             GWEN_Dialog_SelectionMode_Single,
                             0);
  /* info */
  AB_SetupDialog_InfoPage(dlg);
  
  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);

  /* read user column widths */
  GWEN_Dialog_ListReadColumnSettings(dlg, "userListBox", "user_list_", 6, USER_LIST_MINCOLWIDTH, dbPrefs);

  /* read account column widths */
  GWEN_Dialog_ListReadColumnSettings(dlg, "accountListBox", "account_list_", 7, ACCOUNT_LIST_MINCOLWIDTH, dbPrefs);

  /* activate providers */
  AB_SetupDialog_ActivateProviders(dlg);

  /* reload accounts and users */
  AB_SetupDialog_Reload(dlg);
}

void AB_SetupDialog_Fini(GWEN_DIALOG *dlg)
{
  AB_SETUP_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  /* deactivate providers */
  AB_SetupDialog_DeactivateProviders(dlg);

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

  /* store column widths of user list */
  GWEN_Dialog_ListWriteColumnSettings(dlg, "userListBox", "user_list_", 6, USER_LIST_MINCOLWIDTH, dbPrefs);

  /* write account column widths */
  GWEN_Dialog_ListWriteColumnSettings(dlg, "accountListBox", "account_list_", 7, ACCOUNT_LIST_MINCOLWIDTH, dbPrefs);
}



int AB_SetupDialog_EditUser(GWEN_DIALOG *dlg)
{
  AB_SETUP_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->currentUserList) {
    uint32_t uid;

    uid=AB_SetupDialog_GetCurrentUserId(dlg);
    if (uid) {
      AB_USER *u;

      u=AB_User_List_GetByUniqueId(xdlg->currentUserList, uid);
      if (u) {
        AB_PROVIDER *pro;
        uint32_t flags=0;
        GWEN_DIALOG *dlg2;
        int rv;

        pro=AB_User_GetProvider(u);
        assert(pro);

        /* get EditUser dialog */
        flags=AB_Provider_GetFlags(pro);
        if (flags & AB_PROVIDER_FLAGS_HAS_EDITUSER_DIALOG) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Letting backend \"%s\" create dialog", AB_Provider_GetName(pro));
          dlg2=AB_Provider_GetEditUserDialog(pro, u);
        }
        else {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Letting AqBanking create dialog");
          dlg2=AB_EditUserDialog_new(pro, u, 1);
        }
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
        GWEN_Dialog_free(dlg2);

        /* reload */
        AB_SetupDialog_Reload(dlg);
      } /* if u */
    } /* if uid */
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "No current user");
    }
  } /* if xdlg->currentUserList */
  return GWEN_DialogEvent_ResultHandled;
}



int AB_SetupDialog_AddUser(GWEN_DIALOG *dlg)
{
  AB_SETUP_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  dlg2=AB_SetupNewUserDialog_new(xdlg->banking);
  if (dlg2==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not create dialog");
    return GWEN_DialogEvent_ResultHandled;
  }

  rv=GWEN_Gui_ExecDialog(dlg2, 0);
  if (rv==0) {
    /* rejected */
    GWEN_Dialog_free(dlg2);
  }
  else {
    const char *s;

    s=AB_SetupNewUserDialog_GetSelectedBackend(dlg2);
    if (s && *s) {
      AB_PROVIDER *pro;
      int selectedType;
      uint32_t flags;

      DBG_INFO(AQBANKING_LOGDOMAIN, "Selected provider [%s]", s);
      pro=AB_SetupDialog_GetProviderByName(dlg, s);
      if (pro==NULL) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider [%s] not found", s);
        GWEN_Dialog_free(dlg2);
        return GWEN_DialogEvent_ResultHandled;
      }
      selectedType=AB_SetupNewUserDialog_GetSelectedType(dlg2);
      GWEN_Dialog_free(dlg2);
      DBG_INFO(AQBANKING_LOGDOMAIN, "Selected type is %d", selectedType);

      flags=AB_Provider_GetFlags(pro);
      if (flags & AB_PROVIDER_FLAGS_HAS_NEWUSER_DIALOG) {
        GWEN_DIALOG *dlg3;
        int rv;

        dlg3=AB_Provider_GetNewUserDialog(pro, selectedType);
        if (dlg3==NULL) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not create dialog (type=%d)", selectedType);
          return GWEN_DialogEvent_ResultHandled;
        }

        rv=GWEN_Gui_ExecDialog(dlg3, 0);
        if (rv==0) {
          /* rejected */
          GWEN_Dialog_free(dlg3);
          return GWEN_DialogEvent_ResultHandled;
        }
        GWEN_Dialog_free(dlg3);
        AB_SetupDialog_Reload(dlg);
      }
      else {
        GWEN_DIALOG *dlg3;
        AB_USER *u;
        const char *s;
        int rv;

        u=AB_Provider_CreateUserObject(pro);
        if (u==NULL) {
          DBG_INFO(AQBANKING_LOGDOMAIN, "No user created.");
          return GWEN_DialogEvent_ResultHandled;
        }

        s=GWEN_I18N_GetCurrentLocale();
        if (s && *s) {
          if (strstr(s, "de_"))
            AB_User_SetCountry(u, "de");
          else if (strstr(s, "us_"))
            AB_User_SetCountry(u, "us");
        }

        dlg3=AB_EditUserDialog_new(pro, u, 0);
        if (dlg3==NULL) {
          DBG_INFO(AQBANKING_LOGDOMAIN, "Could not create dialog");
          AB_User_free(u);
          return GWEN_DialogEvent_ResultHandled;
        }

        rv=GWEN_Gui_ExecDialog(dlg3, 0);
        if (rv==0) {
          /* rejected */
          GWEN_Dialog_free(dlg3);
          AB_User_free(u);
          return GWEN_DialogEvent_ResultHandled;
        }
        GWEN_Dialog_free(dlg3);

        rv=AB_Provider_AddUser(pro, u);
        if (rv<0) {
          DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
          AB_User_free(u);
          return GWEN_DialogEvent_ResultHandled;
        }
        AB_User_free(u);

        AB_SetupDialog_Reload(dlg);
      }
    }
    else {
      GWEN_Dialog_free(dlg2);
      return GWEN_DialogEvent_ResultHandled;
    }
  }

  return GWEN_DialogEvent_ResultHandled;
}



int AB_SetupDialog_DelUser(GWEN_DIALOG *dlg)
{
  AB_SETUP_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->currentUserList) {
    uint32_t uid;

    uid=AB_SetupDialog_GetCurrentUserId(dlg);
    if (uid) {
      AB_USER *u;

      u=AB_User_List_GetByUniqueId(xdlg->currentUserList, uid);

      if (u) {
        AB_ACCOUNT *a;
        uint32_t aid;
        int rv;
        char nbuf[512];

        snprintf(nbuf, sizeof(nbuf)-1,
                 I18N("<html>"
                      "<p>Do you really want to delete the user <i>%s</i>?"
                      "</html>"
                      "Do you really want to delete the user \"%s\"?"),
                 AB_User_GetUserId(u), AB_User_GetUserId(u));
        nbuf[sizeof(nbuf)-1]=0;

        rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_WARN |
                               GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS,
                               I18N("Delete User"),
                               nbuf,
                               I18N("Yes"),
                               I18N("No"),
                               NULL,
                               0);
        if (rv!=1) {
          DBG_INFO(AQBANKING_LOGDOMAIN, "Aborted by user");
          return GWEN_DialogEvent_ResultHandled;
        }

        xdlg->currentAccountList=AB_Account_List_new();
        AB_SetupDialog_LoadAccounts(dlg, xdlg->currentAccountList);
        if (AB_Account_List_GetCount(xdlg->currentAccountList)) {
          a=AB_Account_List_First(xdlg->currentAccountList);
          while (a) {
            if (AB_Account_GetUserId(a) == uid) {
              aid=AB_Account_GetUniqueId(a);
              rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
                                     GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS,
                                     I18N("Error"),
                                     I18N("<html>"
                                          "<p>There is at least one account assigned to the selected user.</p>"
                                          "<p>Do you want to remove the account(s) and continue removing the user?</p>"
                                          "</html>"
                                          "There is at least one account assigned to the selected user.\n"
                                          "Do you want to remove the account(s) and continue removing the user?"),
                                     I18N("Yes"),
                                     I18N("No"),
                                     NULL,
                                     0);
              if (rv!=1) {
                DBG_INFO(AQBANKING_LOGDOMAIN, "Aborted by user");
                return GWEN_DialogEvent_ResultHandled;
              }

              rv=AB_Provider_DeleteAccount(AB_Account_GetProvider(a), aid);
              if (rv<0) {
                GWEN_Gui_ShowError(I18N("Error"), I18N("Error deleting account: %d (%d deleted)"), rv, aid);
                AB_SetupDialog_Reload(dlg);
                return GWEN_DialogEvent_ResultHandled;
              }
            }
            a=AB_Account_List_Next(a);
          }
        }

        /* now delete the user */
        rv=AB_Provider_DeleteUser(AB_User_GetProvider(u), uid);
        if (rv<0) {
          GWEN_Gui_ShowError(I18N("Error"), I18N("Error deleting user: %d"), rv);
          AB_SetupDialog_Reload(dlg);
          return GWEN_DialogEvent_ResultHandled;
        }
      } /* if u */
    } /* if uid */
    AB_SetupDialog_Reload(dlg);
  } /* if currentUserList */
  return GWEN_DialogEvent_ResultHandled;
}



int AB_SetupDialog_EditAccount(GWEN_DIALOG *dlg)
{
  AB_SETUP_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->currentAccountList) {
    uint32_t aid;

    aid=AB_SetupDialog_GetCurrentAccountId(dlg);
    if (aid) {
      AB_ACCOUNT *a;

      a=AB_Account_List_GetByUniqueId(xdlg->currentAccountList, aid);
      if (a) {
        AB_PROVIDER *pro;
        uint32_t flags=0;
        GWEN_DIALOG *dlg2;
        int rv;

        pro=AB_Account_GetProvider(a);
        assert(pro);
        flags=AB_Provider_GetFlags(pro);
        if (flags & AB_PROVIDER_FLAGS_HAS_EDITACCOUNT_DIALOG)
          dlg2=AB_Provider_GetEditAccountDialog(pro, a);
        else
          dlg2=AB_EditAccountDialog_new(pro, a, 1);
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
        GWEN_Dialog_free(dlg2);
        AB_SetupDialog_Reload(dlg);
      } /* if a */
    } /* if aid */
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "No current account");
    }
  } /* if xdlg->currentAccountList */
  return GWEN_DialogEvent_ResultHandled;
}



int AB_SetupDialog_AddAccount(GWEN_DIALOG *dlg)
{
  AB_SETUP_DIALOG *xdlg;
  AB_PROVIDER *pro;
  const char *s;
  const char *initialProvider=NULL;
  uint32_t flags;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  s=GWEN_I18N_GetCurrentLocale();
  if (s && *s) {
    if (strstr(s, "de_"))
      initialProvider="aqhbci";
    else
      initialProvider="aqofxconnect";
  }
  pro=AB_SelectBackend(xdlg->banking,
                       initialProvider,
                       I18N("Please select the online banking backend the new "
                            "account is to be created for."));
  if (pro==NULL) {
    DBG_ERROR(0, "No provider selected.");
    return GWEN_DialogEvent_ResultHandled;
  }

  flags=AB_Provider_GetFlags(pro);
  if (flags & AB_PROVIDER_FLAGS_HAS_NEWACCOUNT_DIALOG) {
    GWEN_DIALOG *dlg2;
    int rv;

    dlg2=AB_Provider_GetNewAccountDialog(pro);
    if (dlg2==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not create dialog");
      return GWEN_DialogEvent_ResultHandled;
    }

    rv=GWEN_Gui_ExecDialog(dlg2, 0);
    if (rv==0) {
      /* rejected */
      GWEN_Dialog_free(dlg2);
      AB_Banking_EndUseProvider(xdlg->banking, pro);
      return GWEN_DialogEvent_ResultHandled;
    }
    GWEN_Dialog_free(dlg2);
    AB_Banking_EndUseProvider(xdlg->banking, pro);
    AB_SetupDialog_Reload(dlg);
  }
  else {
    GWEN_DIALOG *dlg2;
    AB_ACCOUNT *a;
    const char *s;
    int rv;

    a=AB_Provider_CreateAccountObject(pro);
    if (a==NULL) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "No account created.");
      AB_Banking_EndUseProvider(xdlg->banking, pro);
      return GWEN_DialogEvent_ResultHandled;
    }

    s=GWEN_I18N_GetCurrentLocale();
    if (s && *s) {
      if (strstr(s, "de_")) {
        AB_Account_SetCountry(a, "de");
        AB_Account_SetCurrency(a, "EUR");
      }
      else if (strstr(s, "us_")) {
        AB_Account_SetCountry(a, "us");
        AB_Account_SetCurrency(a, "USD");
      }
    }

    dlg2=AB_EditAccountDialog_new(pro, a, 0);
    if (dlg2==NULL) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Could not create dialog");
      AB_Account_free(a);
      AB_Banking_EndUseProvider(xdlg->banking, pro);
      return GWEN_DialogEvent_ResultHandled;
    }

    rv=GWEN_Gui_ExecDialog(dlg2, 0);
    if (rv==0) {
      /* rejected */
      GWEN_Dialog_free(dlg2);
      AB_Account_free(a);
      AB_Banking_EndUseProvider(xdlg->banking, pro);
      return GWEN_DialogEvent_ResultHandled;
    }
    GWEN_Dialog_free(dlg2);

    rv=AB_Provider_AddAccount(pro, a, 1); /* do lock corresponding user */
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      AB_Account_free(a);
      return GWEN_DialogEvent_ResultHandled;
    }
    AB_Account_free(a);

    AB_Banking_EndUseProvider(xdlg->banking, pro);

    AB_SetupDialog_Reload(dlg);
  }

  return GWEN_DialogEvent_ResultHandled;
}



int AB_SetupDialog_DelAccount(GWEN_DIALOG *dlg)
{
  AB_SETUP_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->currentAccountList) {
    uint32_t aid;

    aid=AB_SetupDialog_GetCurrentAccountId(dlg);
    if (aid) {
      AB_ACCOUNT *a;

      a=AB_Account_List_GetByUniqueId(xdlg->currentAccountList, aid);
      if (a) {
        int rv;
        char nbuf[512];
        char ibuf[32];
        const char *an;

        an=AB_Account_GetAccountName(a);
        if (!(an && *an))
          an=AB_Account_GetAccountNumber(a);
        if (!(an && *an)) {
          snprintf(ibuf, sizeof(ibuf)-1, "%d", (int) AB_Account_GetUniqueId(a));
          ibuf[sizeof(ibuf)-1]=0;
          an=ibuf;
        }

        snprintf(nbuf, sizeof(nbuf)-1,
                 I18N("<html>"
                      "<p>Do you really want to delete the account <i>%s</i>?"
                      "</html>"
                      "Do you really want to delete the account \"%s\"?"),
                 an, an);
        nbuf[sizeof(nbuf)-1]=0;

        rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_WARN |
                               GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS,
                               I18N("Delete Account"),
                               nbuf,
                               I18N("Yes"),
                               I18N("No"),
                               NULL,
                               0);
        if (rv!=1) {
          DBG_INFO(AQBANKING_LOGDOMAIN, "Aborted by user");
          return GWEN_DialogEvent_ResultHandled;
        }

        rv=AB_Provider_DeleteAccount(AB_Account_GetProvider(a), aid);
        if (rv<0) {
          GWEN_Gui_ShowError(I18N("Error"), I18N("Error deleting account: %d"), rv);
          AB_SetupDialog_Reload(dlg);
          return GWEN_DialogEvent_ResultHandled;
        }

        AB_SetupDialog_Reload(dlg);
      } /* if a */
    } /* if aid */
  } /* if xdlg->currentAccountList */
  return GWEN_DialogEvent_ResultHandled;
}



int AB_SetupDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender)
{
  DBG_NOTICE(0, "Activated: %s", sender);
  if (strcasecmp(sender, "closeButton")==0)
    return GWEN_DialogEvent_ResultAccept;
  else if (strcasecmp(sender, "editUserButton")==0)
    return AB_SetupDialog_EditUser(dlg);
  else if (strcasecmp(sender, "addUserButton")==0)
    return AB_SetupDialog_AddUser(dlg);
  else if (strcasecmp(sender, "delUserButton")==0)
    return AB_SetupDialog_DelUser(dlg);
  else if (strcasecmp(sender, "editAccountButton")==0)
    return AB_SetupDialog_EditAccount(dlg);
  else if (strcasecmp(sender, "addAccountButton")==0)
    return AB_SetupDialog_AddAccount(dlg);
  else if (strcasecmp(sender, "delAccountButton")==0)
    return AB_SetupDialog_DelAccount(dlg);
  else if (strcasecmp(sender, "userListBox")==0)
    return AB_SetupDialog_UserChanged(dlg);
  else if (strcasecmp(sender, "accountListBox")==0)
    return AB_SetupDialog_AccountChanged(dlg);
  else if (strcasecmp(sender, "helpButton")==0) {
    /* TODO: open a help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB AB_SetupDialog_SignalHandler(GWEN_DIALOG *dlg,
                                               GWEN_DIALOG_EVENTTYPE t,
                                               const char *sender)
{
  AB_SETUP_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  switch (t) {
  case GWEN_DialogEvent_TypeInit:
    AB_SetupDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AB_SetupDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeActivated:
    return AB_SetupDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
  default:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}






