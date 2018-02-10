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
#include <aqbanking/dlg_editaccount.h>
#include <aqbanking/dlg_edituser.h>

#include <aqbanking/dlg_setup_newuser.h>

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
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
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
  snprintf(numbuf, sizeof(numbuf)-1, "%06d", uid);
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



static void createAccountListBoxString(const AB_ACCOUNT *a, GWEN_BUFFER *tbuf) {
  const char *s;
  char numbuf[32];
  uint32_t uid;
  
  /* column 1 */
  uid=AB_Account_GetUniqueId(a);
  snprintf(numbuf, sizeof(numbuf)-1, "%06d", uid);
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

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  return GWEN_DialogEvent_ResultHandled;
}



int AB_SetupDialog_AccountChanged(GWEN_DIALOG *dlg) {
  AB_SETUP_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  return GWEN_DialogEvent_ResultHandled;
}



void AB_SetupDialog_Reload(GWEN_DIALOG *dlg) {
  AB_SETUP_DIALOG *xdlg;
  AB_USER_LIST2 *ul;
  AB_ACCOUNT_LIST2 *al;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  /* user list */
  i=0;
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
        i++;
        GWEN_Buffer_Reset(tbuf);
	u=AB_User_List2Iterator_Next(it);
      }
      GWEN_Buffer_free(tbuf);

      AB_User_List2Iterator_free(it);
    }
    AB_User_List2_free(ul);
  }
  GWEN_Dialog_SetIntProperty(dlg, "userListBox", GWEN_DialogProperty_Sort, 0, 0, 0);
  if (i)
    GWEN_Dialog_SetIntProperty(dlg, "userListBox", GWEN_DialogProperty_Value, 0, 0, 0);

  /* account list */
  i=0;
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
	i++;
        GWEN_Buffer_Reset(tbuf);
	a=AB_Account_List2Iterator_Next(it);
      }
      GWEN_Buffer_free(tbuf);

      AB_Account_List2Iterator_free(it);
    }
    AB_Account_List2_free(al);
  }
  GWEN_Dialog_SetIntProperty(dlg, "accountListBox", GWEN_DialogProperty_Sort, 0, 0, 0);
  if (i)
    GWEN_Dialog_SetIntProperty(dlg, "accountListBox", GWEN_DialogProperty_Value, 0, 0, 0);


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

  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);

  /* read user column widths */
  for (i=0; i<6; i++) {
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
  GWEN_DB_DeleteVar(dbPrefs, "user_list_columns");
  for (i=0; i<6; i++) {
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
  for (i=0; i<6; i++) {
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
  AB_USER *u;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  u=AB_SetupDialog_GetCurrentUser(dlg);
  if (u) {
    AB_PROVIDER *pro;
    uint32_t flags=0;
    GWEN_DIALOG *dlg2;
    int rv;

    pro=AB_User_GetProvider(u);
    assert(pro);
    flags=AB_Provider_GetFlags(pro);
    if (flags & AB_PROVIDER_FLAGS_HAS_EDITUSER_DIALOG)
      dlg2=AB_Provider_GetEditUserDialog(pro, u);
    else
      dlg2=AB_EditUserDialog_new(xdlg->banking, u, 1);
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
  }
  return GWEN_DialogEvent_ResultHandled;
}



int AB_SetupDialog_AddUser(GWEN_DIALOG *dlg) {
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

      pro=AB_Banking_GetProvider(xdlg->banking, s);
      if (pro==NULL) {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider [%s] not found", s);
	GWEN_Dialog_free(dlg2);
	return GWEN_DialogEvent_ResultHandled;
      }
      selectedType=AB_SetupNewUserDialog_GetSelectedType(dlg2);
      GWEN_Dialog_free(dlg2);

      flags=AB_Provider_GetFlags(pro);
      if (flags & AB_PROVIDER_FLAGS_HAS_NEWUSER_DIALOG) {
	GWEN_DIALOG *dlg3;
	int rv;

	dlg3=AB_Provider_GetNewUserDialog(pro, selectedType);
	if (dlg3==NULL) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not create dialog");
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
	const AB_COUNTRY *c=NULL;
	const char *s;
	int rv;
    
	u=AB_Banking_CreateUser(xdlg->banking, AB_Provider_GetName(pro));
	if (u==NULL) {
	  DBG_INFO(AQBANKING_LOGDOMAIN, "No user created.");
	  AB_User_free(u);
	  return GWEN_DialogEvent_ResultHandled;
	}
    
	s=GWEN_I18N_GetCurrentLocale();
	if (s && *s) {
	  if (strstr(s, "de_"))
	    c=AB_Banking_FindCountryByCode(xdlg->banking, "de");
	  else if (strstr(s, "us_"))
	    c=AB_Banking_FindCountryByCode(xdlg->banking, "us");
	}
	if (c) {
	  AB_User_SetCountry(u, AB_Country_GetCode(c));
	}
    
	dlg3=AB_EditUserDialog_new(xdlg->banking, u, 0);
	if (dlg3==NULL) {
	  DBG_INFO(AQBANKING_LOGDOMAIN, "Could not create dialog");
	  AB_User_free(u);
	  return GWEN_DialogEvent_ResultHandled;
	}
    
	rv=GWEN_Gui_ExecDialog(dlg3, 0);
	if (rv==0) {
	  /* rejected */
	  GWEN_Dialog_free(dlg3);
	  return GWEN_DialogEvent_ResultHandled;
	}
	GWEN_Dialog_free(dlg3);
    
	rv=AB_Banking_AddUser(xdlg->banking, u);
	if (rv<0) {
	  DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
	  AB_User_free(u);
	  return GWEN_DialogEvent_ResultHandled;
	}
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



int AB_SetupDialog_DelUser(GWEN_DIALOG *dlg) {
  AB_SETUP_DIALOG *xdlg;
  AB_USER *u;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  u=AB_SetupDialog_GetCurrentUser(dlg);
  if (u) {
    AB_ACCOUNT *a;
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

    a=AB_Banking_FindFirstAccountOfUser(xdlg->banking, u);
    if (a) {
      int i;

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

      i=0;
      while( (a=AB_Banking_FindFirstAccountOfUser(xdlg->banking, u)) ) {
	rv=AB_Banking_DeleteAccount(xdlg->banking, a);
	if (rv<0) {
	  GWEN_Gui_ShowError(I18N("Error"), I18N("Error deleting account: %d (%d deleted)"), rv, i);
	  AB_SetupDialog_Reload(dlg);
	  return GWEN_DialogEvent_ResultHandled;
	}
	i++;
      }
    }

    /* now delete the user */
    rv=AB_Banking_DeleteUser(xdlg->banking, u);
    if (rv<0) {
      GWEN_Gui_ShowError(I18N("Error"), I18N("Error deleting user: %d"), rv);
      AB_SetupDialog_Reload(dlg);
      return GWEN_DialogEvent_ResultHandled;
    }

    AB_SetupDialog_Reload(dlg);
  }
  return GWEN_DialogEvent_ResultHandled;
}



int AB_SetupDialog_EditAccount(GWEN_DIALOG *dlg) {
  AB_SETUP_DIALOG *xdlg;
  AB_ACCOUNT *a;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  a=AB_SetupDialog_GetCurrentAccount(dlg);
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
      dlg2=AB_EditAccountDialog_new(xdlg->banking, a, 1);
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
  }
  return GWEN_DialogEvent_ResultHandled;
}



int AB_SetupDialog_AddAccount(GWEN_DIALOG *dlg) {
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
  if (flags & AB_PROVIDER_FLAGS_HAS_EDITACCOUNT_DIALOG) {
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
      return GWEN_DialogEvent_ResultHandled;
    }
    GWEN_Dialog_free(dlg2);
    AB_SetupDialog_Reload(dlg);
  }
  else {
    GWEN_DIALOG *dlg2;
    AB_ACCOUNT *a;
    const AB_COUNTRY *c=NULL;
    const char *s;
    int rv;

    a=AB_Banking_CreateAccount(xdlg->banking, AB_Provider_GetName(pro));
    if (a==NULL) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "No account created.");
      AB_Account_free(a);
      return GWEN_DialogEvent_ResultHandled;
    }

    s=GWEN_I18N_GetCurrentLocale();
    if (s && *s) {
      if (strstr(s, "de_"))
	c=AB_Banking_FindCountryByCode(xdlg->banking, "de");
      else if (strstr(s, "us_"))
	c=AB_Banking_FindCountryByCode(xdlg->banking, "us");
    }
    if (c) {
      AB_Account_SetCountry(a, AB_Country_GetCode(c));
      AB_Account_SetCurrency(a, AB_Country_GetCurrencyCode(c));
    }


    dlg2=AB_EditAccountDialog_new(xdlg->banking, a, 0);
    if (dlg2==NULL) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Could not create dialog");
      AB_Account_free(a);
      return GWEN_DialogEvent_ResultHandled;
    }

    rv=GWEN_Gui_ExecDialog(dlg2, 0);
    if (rv==0) {
      /* rejected */
      GWEN_Dialog_free(dlg2);
      return GWEN_DialogEvent_ResultHandled;
    }
    GWEN_Dialog_free(dlg2);

    rv=AB_Banking_AddAccount(xdlg->banking, a);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      AB_Account_free(a);
      return GWEN_DialogEvent_ResultHandled;
    }
    AB_SetupDialog_Reload(dlg);
  }

  return GWEN_DialogEvent_ResultHandled;
}



int AB_SetupDialog_DelAccount(GWEN_DIALOG *dlg) {
  AB_SETUP_DIALOG *xdlg;
  AB_ACCOUNT *a;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_DIALOG, dlg);
  assert(xdlg);

  a=AB_SetupDialog_GetCurrentAccount(dlg);
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

    rv=AB_Banking_DeleteAccount(xdlg->banking, a);
    if (rv<0) {
      GWEN_Gui_ShowError(I18N("Error"), I18N("Error deleting account: %d"), rv);
      AB_SetupDialog_Reload(dlg);
      return GWEN_DialogEvent_ResultHandled;
    }

    AB_SetupDialog_Reload(dlg);
  }
  return GWEN_DialogEvent_ResultHandled;
}



int AB_SetupDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
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






