/***************************************************************************
 begin       : Wed Apr 14 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_setup_p.h"
#include "i18n_l.h"

#include <aqbanking/user.h>
#include <aqbanking/banking_be.h>
#include <aqbanking/dlg_selectbackend.h>

#include <aqhbci/user.h>
#include <aqhbci/provider.h>

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




GWEN_DIALOG *AB_SetupDialog_new(AB_BANKING *ab) {
  GWEN_DIALOG *dlg;
  AB_SETUP_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("ab_setup");
  GWEN_NEW_OBJECT(AB_SETUP_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg, xdlg,
		       AB_SetupDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, AB_SetupDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(GWEN_PM_LIBNAME, GWEN_PM_SYSDATADIR,
			       "aqbanking/dialogs/dlg_setup.dlg",
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

  /* done */
  return dlg;
}



void GWENHYWFAR_CB AB_SetupDialog_FreeData(void *bp, void *p) {
  AB_SETUP_DIALOG *xdlg;

  xdlg=(AB_SETUP_DIALOG*) p;
  GWEN_FREE_OBJECT(xdlg);
}



static void createUserListBoxString(const AB_USER *u, GWEN_BUFFER *tbuf) {
  const char *s;
  char numbuf[32];
  uint32_t uid;
  
  /* column 1 */
  uid=AB_User_GetUniqueId(u);
  snprintf(numbuf, sizeof(numbuf)-1, "%d", uid);
  numbuf[sizeof(numbuf)-1]=0;
  GWEN_Buffer_AppendString(tbuf, numbuf);
  GWEN_Buffer_AppendString(tbuf, "\t");
  
  /* column 2 */
  s=AB_User_GetUserId(u);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "\t");
  
  /* column 3 */
  s=AB_User_GetCustomerId(u);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "\t");
  
  /* column 4 */
  s=AB_User_GetUserName(u);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "\t");
  
  /* column 5 */
  s=AB_User_GetBackendName(u);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
}



static void createAccountListBoxString(const AB_ACCOUNT *a, GWEN_BUFFER *tbuf) {
  const char *s;
  char numbuf[32];
  uint32_t uid;
  
  /* column 1 */
  uid=AB_Account_GetUniqueId(a);
  snprintf(numbuf, sizeof(numbuf)-1, "%d", uid);
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



AB_USER *AB_SetupDialog_GetCurrentUser(GWEN_DIALOG *dlg) {
  AB_SETUP_DIALOG *xdlg;
  AB_USER_LIST2 *ul;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  /* user list */
  ul=AB_Banking_GetUsers(xdlg->banking);
  if (ul) {
    int idx;

    idx=GWEN_Dialog_GetIntProperty(dlg, "userListBox", GWEN_DialogProperty_Value, 0, -1);
    if (idx>=0) {
      const char *currentText;
  
      currentText=GWEN_Dialog_GetCharProperty(dlg, "userListBox", GWEN_DialogProperty_Value, idx, NULL);
      if (currentText && *currentText) {
	AB_USER_LIST2_ITERATOR *it;
    
	it=AB_User_List2_First(ul);
	if (it) {
	  AB_USER *u;
	  GWEN_BUFFER *tbuf;
    
	  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
	  u=AB_User_List2Iterator_Data(it);
	  while(u) {
	    createUserListBoxString(u, tbuf);
	    if (strcasecmp(currentText, GWEN_Buffer_GetStart(tbuf))==0) {
	      GWEN_Buffer_free(tbuf);
	      AB_User_List2Iterator_free(it);
	      AB_User_List2_free(ul);
	      return u;
	    }
	    GWEN_Buffer_Reset(tbuf);
	    u=AB_User_List2Iterator_Next(it);
	  }
	  GWEN_Buffer_free(tbuf);

	  AB_User_List2Iterator_free(it);
	}
	AB_User_List2_free(ul);
      }
    }
  }

  return NULL;
}



AB_ACCOUNT *AB_SetupDialog_GetCurrentAccount(GWEN_DIALOG *dlg) {
  AB_SETUP_DIALOG *xdlg;
  AB_ACCOUNT_LIST2 *al;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  /* user list */
  al=AB_Banking_GetAccounts(xdlg->banking);
  if (al) {
    int idx;

    idx=GWEN_Dialog_GetIntProperty(dlg, "accountListBox", GWEN_DialogProperty_Value, 0, -1);
    if (idx>=0) {
      const char *currentText;
  
      currentText=GWEN_Dialog_GetCharProperty(dlg, "accountListBox", GWEN_DialogProperty_Value, idx, NULL);
      if (currentText && *currentText) {
	AB_ACCOUNT_LIST2_ITERATOR *it;
    
	it=AB_Account_List2_First(al);
	if (it) {
	  AB_ACCOUNT *a;
	  GWEN_BUFFER *tbuf;
    
	  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
	  a=AB_Account_List2Iterator_Data(it);
	  while(a) {
	    createAccountListBoxString(a, tbuf);
	    if (strcasecmp(currentText, GWEN_Buffer_GetStart(tbuf))==0) {
	      GWEN_Buffer_free(tbuf);
	      AB_Account_List2Iterator_free(it);
	      AB_Account_List2_free(al);
	      return a;
	    }
	    GWEN_Buffer_Reset(tbuf);
	    a=AB_Account_List2Iterator_Next(it);
	  }
	  GWEN_Buffer_free(tbuf);

	  AB_Account_List2Iterator_free(it);
	}
	AB_Account_List2_free(al);
      }
    }
  }

  return NULL;
}



int AB_SetupDialog_UserChanged(GWEN_DIALOG *dlg) {
  AB_SETUP_DIALOG *xdlg;
  AB_USER *u;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  u=AB_SetupDialog_GetCurrentUser(dlg);
  if (u) {
    AB_PROVIDER *pro;
    uint32_t flags=0;

    pro=AB_User_GetProvider(u);
    assert(pro);
    flags=AB_Provider_GetFlags(pro);
    GWEN_Dialog_SetIntProperty(dlg, "editUserButton", GWEN_DialogProperty_Enabled, 0,
			       (flags & AB_PROVIDER_FLAGS_HAS_EDITUSER_DIALOG)?1:0,
			       0);
  }
  else {
    GWEN_Dialog_SetIntProperty(dlg, "editUserButton", GWEN_DialogProperty_Enabled, 0, 0, 0);
  }

  return GWEN_DialogEvent_ResultHandled;
}



int AB_SetupDialog_AccountChanged(GWEN_DIALOG *dlg) {
  AB_SETUP_DIALOG *xdlg;
  AB_ACCOUNT *a;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  a=AB_SetupDialog_GetCurrentAccount(dlg);
  if (a) {
    AB_PROVIDER *pro;
    uint32_t flags=0;

    pro=AB_Account_GetProvider(a);
    assert(pro);
    flags=AB_Provider_GetFlags(pro);
    GWEN_Dialog_SetIntProperty(dlg, "editAccountButton", GWEN_DialogProperty_Enabled, 0,
			       (flags & AB_PROVIDER_FLAGS_HAS_EDITACCOUNT_DIALOG)?1:0,
			       0);
  }
  else {
    GWEN_Dialog_SetIntProperty(dlg, "editAccountButton", GWEN_DialogProperty_Enabled, 0, 0, 0);
  }

  return GWEN_DialogEvent_ResultHandled;
}



void AB_SetupDialog_Reload(GWEN_DIALOG *dlg) {
  AB_SETUP_DIALOG *xdlg;
  AB_USER_LIST2 *ul;
  AB_ACCOUNT_LIST2 *al;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  /* user list */
  GWEN_Dialog_SetIntProperty(dlg, "userListBox", GWEN_DialogProperty_ClearValues, 0, 0, 0);
  ul=AB_Banking_GetUsers(xdlg->banking);
  if (ul) {
    AB_USER_LIST2_ITERATOR *it;

    it=AB_User_List2_First(ul);
    if (it) {
      AB_USER *u;
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      u=AB_User_List2Iterator_Data(it);
      while(u) {
	createUserListBoxString(u, tbuf);
	GWEN_Dialog_SetCharProperty(dlg,
				    "userListBox",
				    GWEN_DialogProperty_AddValue,
				    0,
				    GWEN_Buffer_GetStart(tbuf),
				    0);
        GWEN_Buffer_Reset(tbuf);
	u=AB_User_List2Iterator_Next(it);
      }
      GWEN_Buffer_free(tbuf);

      AB_User_List2Iterator_free(it);
    }
    AB_User_List2_free(ul);
  }

  /* account list */
  GWEN_Dialog_SetIntProperty(dlg, "accountListBox", GWEN_DialogProperty_ClearValues, 0, 0, 0);
  al=AB_Banking_GetAccounts(xdlg->banking);
  if (al) {
    AB_ACCOUNT_LIST2_ITERATOR *it;

    it=AB_Account_List2_First(al);
    if (it) {
      AB_ACCOUNT *a;
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      a=AB_Account_List2Iterator_Data(it);
      while(a) {
	createAccountListBoxString(a, tbuf);
	GWEN_Dialog_SetCharProperty(dlg,
				    "accountListBox",
				    GWEN_DialogProperty_AddValue,
				    0,
				    GWEN_Buffer_GetStart(tbuf),
				    0);

        GWEN_Buffer_Reset(tbuf);
	a=AB_Account_List2Iterator_Next(it);
      }
      GWEN_Buffer_free(tbuf);

      AB_Account_List2Iterator_free(it);
    }
    AB_Account_List2_free(al);
  }


  AB_SetupDialog_UserChanged(dlg);
  AB_SetupDialog_AccountChanged(dlg);
}



void AB_SetupDialog_Init(GWEN_DIALOG *dlg) {
  AB_SETUP_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;
  int j;

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
			      I18N("Id\tUser Id\tCustomer Id\tUser Name\tModule"),
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

  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i<DIALOG_MINWIDTH)
    i=DIALOG_MINWIDTH;
  GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i<DIALOG_MINHEIGHT)
    i=DIALOG_MINHEIGHT;
  GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);

  /* read user column widths */
  for (i=0; i<5; i++) {
    j=GWEN_DB_GetIntValue(dbPrefs, "user_list_columns", i, -1);
    if (j<USER_LIST_MINCOLWIDTH)
      j=USER_LIST_MINCOLWIDTH;
    GWEN_Dialog_SetIntProperty(dlg, "userListBox", GWEN_DialogProperty_ColumnWidth, i, j, 0);
  }
  /* get sort column */
  i=GWEN_DB_GetIntValue(dbPrefs, "user_list_sortbycolumn", 0, -1);
  j=GWEN_DB_GetIntValue(dbPrefs, "user_list_sortdir", 0, -1);
  if (i>=0 && j>=0)
    GWEN_Dialog_SetIntProperty(dlg, "userListBox", GWEN_DialogProperty_SortDirection, i, j, 0);

  /* read account column widths */
  for (i=0; i<7; i++) {
    j=GWEN_DB_GetIntValue(dbPrefs, "account_list_columns", i, -1);
    if (j<ACCOUNT_LIST_MINCOLWIDTH)
      j=ACCOUNT_LIST_MINCOLWIDTH;
    GWEN_Dialog_SetIntProperty(dlg, "accountListBox", GWEN_DialogProperty_ColumnWidth, i, j, 0);
  }
  /* get sort column */
  i=GWEN_DB_GetIntValue(dbPrefs, "account_list_sortbycolumn", 0, -1);
  j=GWEN_DB_GetIntValue(dbPrefs, "account_list_sortdir", 0, -1);
  if (i>=0 && j>=0)
    GWEN_Dialog_SetIntProperty(dlg, "accountListBox", GWEN_DialogProperty_SortDirection, i, j, 0);

  AB_SetupDialog_Reload(dlg);
}



void AB_SetupDialog_Fini(GWEN_DIALOG *dlg) {
  AB_SETUP_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  /* store dialog width */
  i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, -1);
  if (i<DIALOG_MINWIDTH)
    i=DIALOG_MINWIDTH;
  GWEN_DB_SetIntValue(dbPrefs,
		      GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "dialog_width",
		      i);

  /* store dialog height */
  i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, -1);
  if (i<DIALOG_MINHEIGHT)
    i=DIALOG_MINHEIGHT;
  GWEN_DB_SetIntValue(dbPrefs,
		      GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "dialog_height",
		      i);

  /* store column widths of user list */
  GWEN_DB_DeleteVar(dbPrefs, "user_list_columns");
  for (i=0; i<5; i++) {
    int j;

    j=GWEN_Dialog_GetIntProperty(dlg, "userListBox", GWEN_DialogProperty_ColumnWidth, i, -1);
    if (j<USER_LIST_MINCOLWIDTH)
      j=USER_LIST_MINCOLWIDTH;
    GWEN_DB_SetIntValue(dbPrefs,
			GWEN_DB_FLAGS_DEFAULT,
			"user_list_columns",
			j);
  }
  /* store column sorting of user list */
  GWEN_DB_SetIntValue(dbPrefs,
		      GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "user_list_sortbycolumn",
		      -1);
  for (i=0; i<5; i++) {
    int j;

    j=GWEN_Dialog_GetIntProperty(dlg, "userListBox", GWEN_DialogProperty_SortDirection, i,
				 GWEN_DialogSortDirection_None);
    if (j!=GWEN_DialogSortDirection_None) {
      GWEN_DB_SetIntValue(dbPrefs,
			  GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "user_list_sortbycolumn",
			  i);
      GWEN_DB_SetIntValue(dbPrefs,
			  GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "user_list_sortdir",
			  (j==GWEN_DialogSortDirection_Up)?1:0);
      break;
    }
  }

  /* store column widths of account list */
  GWEN_DB_DeleteVar(dbPrefs, "account_list_columns");
  for (i=0; i<7; i++) {
    int j;

    j=GWEN_Dialog_GetIntProperty(dlg, "accountListBox", GWEN_DialogProperty_ColumnWidth, i, -1);
    if (j<ACCOUNT_LIST_MINCOLWIDTH)
      j=ACCOUNT_LIST_MINCOLWIDTH;
    GWEN_DB_SetIntValue(dbPrefs,
			GWEN_DB_FLAGS_DEFAULT,
			"account_list_columns",
			j);
  }
  /* store column sorting */
  GWEN_DB_SetIntValue(dbPrefs,
		      GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "account_list_sortbycolumn",
		      -1);
  for (i=0; i<7; i++) {
    int j;

    j=GWEN_Dialog_GetIntProperty(dlg, "accountListBox", GWEN_DialogProperty_SortDirection, i,
				 GWEN_DialogSortDirection_None);
    if (j!=GWEN_DialogSortDirection_None) {
      GWEN_DB_SetIntValue(dbPrefs,
			  GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "account_list_sortbycolumn",
			  i);
      GWEN_DB_SetIntValue(dbPrefs,
			  GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "account_list_sortdir",
			  (j==GWEN_DialogSortDirection_Up)?1:0);
      break;
    }
  }
}



int AB_SetupDialog_EditUser(GWEN_DIALOG *dlg) {
  AB_SETUP_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  return GWEN_DialogEvent_ResultNotHandled;
}



int AB_SetupDialog_AddUser(GWEN_DIALOG *dlg) {
  AB_SETUP_DIALOG *xdlg;
  AB_PROVIDER *pro;
  const char *s;
  const char *initialProvider=NULL;

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
			    "user is to be created for."));
  if (pro==NULL) {
    DBG_ERROR(0, "No provider selected.");
    return GWEN_DialogEvent_ResultHandled;
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int AB_SetupDialog_DelUser(GWEN_DIALOG *dlg) {
  AB_SETUP_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  return GWEN_DialogEvent_ResultNotHandled;
}



int AB_SetupDialog_EditAccount(GWEN_DIALOG *dlg) {
  AB_SETUP_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  return GWEN_DialogEvent_ResultNotHandled;
}



int AB_SetupDialog_AddAccount(GWEN_DIALOG *dlg) {
  AB_SETUP_DIALOG *xdlg;
  AB_PROVIDER *pro;
  const char *s;
  const char *initialProvider=NULL;

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

  return GWEN_DialogEvent_ResultNotHandled;
}



int AB_SetupDialog_DelAccount(GWEN_DIALOG *dlg) {
  AB_SETUP_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  return GWEN_DialogEvent_ResultNotHandled;
}



int AB_SetupDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  DBG_ERROR(0, "Activated: %s", sender);
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
					       const char *sender) {
  AB_SETUP_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  switch(t) {
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
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}






