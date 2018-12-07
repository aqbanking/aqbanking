/***************************************************************************
 begin       : Thu Jul 08 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_edituser_p.h"

#include <aqebics/user.h>
#include <aqebics/provider.h>

#include <aqbanking/user.h>
#include <aqbanking/banking_be.h>
#include <aqbanking/dlg_selectbankinfo.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/i18n.h>


#define DIALOG_MINWIDTH  200
#define DIALOG_MINHEIGHT 200


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)
#define I18N_NOOP(msg) msg
#define I18S(msg) msg



GWEN_INHERIT(GWEN_DIALOG, EBC_EDIT_USER_DIALOG)




GWEN_DIALOG *EBC_EditUserDialog_new(AB_BANKING *ab, AB_USER *u, int doLock) {
  GWEN_DIALOG *dlg;
  EBC_EDIT_USER_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("ebc_edit_user");
  GWEN_NEW_OBJECT(EBC_EDIT_USER_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, EBC_EDIT_USER_DIALOG, dlg, xdlg,
		       EBC_EditUserDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, EBC_EditUserDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(GWEN_PM_LIBNAME, GWEN_PM_SYSDATADIR,
                               "aqbanking/backends/aqebics/dialogs/dlg_edituser.dlg",
                               fbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "Dialog description file not found (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }

  /* read dialog from dialog description file */
  rv=GWEN_Dialog_ReadXmlFile(dlg, GWEN_Buffer_GetStart(fbuf));
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }
  GWEN_Buffer_free(fbuf);

  /* preset */
  xdlg->banking=ab;
  xdlg->user=u;
  xdlg->doLock=doLock;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB EBC_EditUserDialog_FreeData(void *bp, void *p) {
  EBC_EDIT_USER_DIALOG *xdlg;

  xdlg=(EBC_EDIT_USER_DIALOG*) p;
  GWEN_FREE_OBJECT(xdlg);
}



static int createCountryString(const AB_COUNTRY *c, GWEN_BUFFER *tbuf) {
  const char *s;

  s=AB_Country_GetLocalName(c);
  if (s && *s) {
    GWEN_Buffer_AppendString(tbuf, s);
    s=AB_Country_GetCode(c);
    if (s && *s) {
      GWEN_Buffer_AppendString(tbuf, " (");
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ")");
    }
    return 0;
  }
  DBG_INFO(AQEBICS_LOGDOMAIN, "No local name");
  return GWEN_ERROR_NO_DATA;
}



const AB_COUNTRY *EBC_EditUserDialog_GetCurrentCountry(GWEN_DIALOG *dlg) {
  EBC_EDIT_USER_DIALOG *xdlg;
  int idx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_EDIT_USER_DIALOG, dlg);
  assert(xdlg);

  idx=GWEN_Dialog_GetIntProperty(dlg, "countryCombo", GWEN_DialogProperty_Value, 0, -1);
  if (idx>=0) {
    const char *currentText;

    currentText=GWEN_Dialog_GetCharProperty(dlg, "countryCombo", GWEN_DialogProperty_Value, idx, NULL);
    if (currentText && *currentText && xdlg->countryList) {
      AB_COUNTRY_CONSTLIST2_ITERATOR *it;

      it=AB_Country_ConstList2_First(xdlg->countryList);
      if (it) {
	const AB_COUNTRY *c;
	GWEN_BUFFER *tbuf;

	tbuf=GWEN_Buffer_new(0, 256, 0, 1);
	c=AB_Country_ConstList2Iterator_Data(it);
	while(c) {
	  if (createCountryString(c, tbuf)==0 &&
	      strcasecmp(GWEN_Buffer_GetStart(tbuf), currentText)==0) {
	    GWEN_Buffer_free(tbuf);
	    AB_Country_ConstList2Iterator_free(it);
            return c;
	  }
	  GWEN_Buffer_Reset(tbuf);
	  c=AB_Country_ConstList2Iterator_Next(it);
	}
	GWEN_Buffer_free(tbuf);
	AB_Country_ConstList2Iterator_free(it);
      }
    }
  }

  return NULL;
}



void EBC_EditUserDialog_Init(GWEN_DIALOG *dlg) {
  EBC_EDIT_USER_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_EDIT_USER_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  /* init */
  xdlg->countryList=AB_Banking_ListCountriesByName(xdlg->banking, "*");

  GWEN_Dialog_SetCharProperty(dlg,
			      "",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("Edit User"),
			      0);

  /* setup country */
  if (xdlg->countryList) {
    AB_COUNTRY_CONSTLIST2_ITERATOR *it;
    int idx=-1;
    const char *selectedCountry;

    selectedCountry=AB_User_GetCountry(xdlg->user);
    it=AB_Country_ConstList2_First(xdlg->countryList);
    if (it) {
      const AB_COUNTRY *c;
      GWEN_BUFFER *tbuf;
      GWEN_STRINGLIST *sl;
      GWEN_STRINGLISTENTRY *se;
      int i=0;

      sl=GWEN_StringList_new();
      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      c=AB_Country_ConstList2Iterator_Data(it);
      while(c) {
	GWEN_Buffer_AppendByte(tbuf, '1');
	if (createCountryString(c, tbuf)==0) {
	  const char *s;

          s=AB_Country_GetCode(c);
	  if (idx==-1 && selectedCountry && s && strcasecmp(s, selectedCountry)==0) {
	    char *p;

	    p=GWEN_Buffer_GetStart(tbuf);
	    if (p)
	      *p='0';
	    idx=i;
	  }
	  GWEN_StringList_AppendString(sl, GWEN_Buffer_GetStart(tbuf), 0, 1);
          i++;
	}
        GWEN_Buffer_Reset(tbuf);
	c=AB_Country_ConstList2Iterator_Next(it);
      }
      GWEN_Buffer_free(tbuf);
      AB_Country_ConstList2Iterator_free(it);

      GWEN_StringList_Sort(sl, 1, GWEN_StringList_SortModeNoCase);
      idx=-1;
      i=0;
      se=GWEN_StringList_FirstEntry(sl);
      while(se) {
	const char *s;

	s=GWEN_StringListEntry_Data(se);
	if (*s=='0')
          idx=i;
	GWEN_Dialog_SetCharProperty(dlg, "countryCombo", GWEN_DialogProperty_AddValue, 0, s+1, 0);
        i++;
	se=GWEN_StringListEntry_Next(se);
      }
      GWEN_StringList_free(sl);
    }
    if (idx>=0)
      /* chooses selected entry in combo box */
      GWEN_Dialog_SetIntProperty(dlg, "countryCombo", GWEN_DialogProperty_Value, 0, idx, 0);
  }

  s=AB_User_GetUserName(xdlg->user);
  if (s && *s)
    GWEN_Dialog_SetCharProperty(dlg, "userNameEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AB_User_GetBankCode(xdlg->user);
  if (s && *s)
    GWEN_Dialog_SetCharProperty(dlg, "bankCodeEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AB_User_GetUserId(xdlg->user);
  if (s && *s)
    GWEN_Dialog_SetCharProperty(dlg, "userIdEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AB_User_GetCustomerId(xdlg->user);
  GWEN_Dialog_SetCharProperty(dlg, "customerIdEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=EBC_User_GetServerUrl(xdlg->user);
  if (s && *s)
    GWEN_Dialog_SetCharProperty(dlg, "urlEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=EBC_User_GetPeerId(xdlg->user);
  if (s && *s)
    GWEN_Dialog_SetCharProperty(dlg, "hostIdEdit", GWEN_DialogProperty_Value, 0, s, 0);

  GWEN_Dialog_SetCharProperty(dlg, "ebicsVersionCombo", GWEN_DialogProperty_AddValue, 0, "2.3 (H002)", 0);
  GWEN_Dialog_SetCharProperty(dlg, "ebicsVersionCombo", GWEN_DialogProperty_AddValue, 0, "2.4 (H003)", 0);

  GWEN_Dialog_SetCharProperty(dlg, "signVersionCombo", GWEN_DialogProperty_AddValue, 0, "A004", 0);
  GWEN_Dialog_SetCharProperty(dlg, "signVersionCombo", GWEN_DialogProperty_AddValue, 0, "A005", 0);

  GWEN_Dialog_SetCharProperty(dlg, "cryptVersionCombo", GWEN_DialogProperty_AddValue, 0, "E001", 0);
  GWEN_Dialog_SetCharProperty(dlg, "cryptVersionCombo", GWEN_DialogProperty_AddValue, 0, "E002", 0);

  GWEN_Dialog_SetCharProperty(dlg, "authVersionCombo", GWEN_DialogProperty_AddValue, 0, "X001", 0);
  GWEN_Dialog_SetCharProperty(dlg, "authVersionCombo", GWEN_DialogProperty_AddValue, 0, "X002", 0);

  GWEN_Dialog_SetCharProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_AddValue, 0, "1.0", 0);
  GWEN_Dialog_SetCharProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_AddValue, 0, "1.1", 0);

  GWEN_Dialog_SetCharProperty(dlg, "statusCombo", GWEN_DialogProperty_AddValue, 0, I18N("EBICSUserStatus|new"), 0);
  GWEN_Dialog_SetCharProperty(dlg, "statusCombo", GWEN_DialogProperty_AddValue, 0, I18N("EBICSUserStatus|init1"), 0);
  GWEN_Dialog_SetCharProperty(dlg, "statusCombo", GWEN_DialogProperty_AddValue, 0, I18N("EBICSUserStatus|init2"), 0);
  GWEN_Dialog_SetCharProperty(dlg, "statusCombo", GWEN_DialogProperty_AddValue, 0, I18N("EBICSUserStatus|enabled"), 0);
  GWEN_Dialog_SetCharProperty(dlg, "statusCombo", GWEN_DialogProperty_AddValue, 0, I18N("EBICSUserStatus|disabled"), 0);

  /* toGui */
  EBC_EditUserDialog_toGui(dlg);


  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);
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



void EBC_EditUserDialog_toGui(GWEN_DIALOG *dlg) {
  EBC_EDIT_USER_DIALOG *xdlg;
  uint32_t flags;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_EDIT_USER_DIALOG, dlg);
  assert(xdlg);

  /* protocol version */
  s=EBC_User_GetProtoVersion(xdlg->user);
  if (! (s && *s))
    s="H003";
  if (strcasecmp(s, "H002")==0)
    GWEN_Dialog_SetIntProperty(dlg, "ebicsVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0);
  else if (strcasecmp(s, "H003")==0)
    GWEN_Dialog_SetIntProperty(dlg, "ebicsVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0);

  /* signature version */
  s=EBC_User_GetSignVersion(xdlg->user);
  if (! (s && *s))
    s="A005";
  if (strcasecmp(s, "A004")==0)
    GWEN_Dialog_SetIntProperty(dlg, "signVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0);
  else if (strcasecmp(s, "A005")==0)
    GWEN_Dialog_SetIntProperty(dlg, "signVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0);

  /* crypt version */
  s=EBC_User_GetCryptVersion(xdlg->user);
  if (! (s && *s))
    s="E002";
  if (strcasecmp(s, "E001")==0)
    GWEN_Dialog_SetIntProperty(dlg, "cryptVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0);
  else if (strcasecmp(s, "E002")==0)
    GWEN_Dialog_SetIntProperty(dlg, "cryptVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0);

  /* auth version */
  s=EBC_User_GetAuthVersion(xdlg->user);
  if (! (s && *s))
    s="X002";
  if (strcasecmp(s, "X001")==0)
    GWEN_Dialog_SetIntProperty(dlg, "authVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0);
  else if (strcasecmp(s, "X002")==0)
    GWEN_Dialog_SetIntProperty(dlg, "authVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0);

  /* status */
  switch(EBC_User_GetStatus(xdlg->user)) {
  case EBC_UserStatus_New:      GWEN_Dialog_SetIntProperty(dlg, "statusCombo", GWEN_DialogProperty_Value, 0, 0, 0); break;
  case EBC_UserStatus_Init1:    GWEN_Dialog_SetIntProperty(dlg, "statusCombo", GWEN_DialogProperty_Value, 0, 1, 0); break;
  case EBC_UserStatus_Init2:    GWEN_Dialog_SetIntProperty(dlg, "statusCombo", GWEN_DialogProperty_Value, 0, 2, 0); break;
  case EBC_UserStatus_Enabled:  GWEN_Dialog_SetIntProperty(dlg, "statusCombo", GWEN_DialogProperty_Value, 0, 3, 0); break;
  case EBC_UserStatus_Disabled: GWEN_Dialog_SetIntProperty(dlg, "statusCombo", GWEN_DialogProperty_Value, 0, 4, 0); break;
  default:  break;
  }

  /* http version */
  switch(((EBC_User_GetHttpVMajor(xdlg->user))<<8)+EBC_User_GetHttpVMinor(xdlg->user)) {
  case 0x0100: GWEN_Dialog_SetIntProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0); break;
  case 0x0101: GWEN_Dialog_SetIntProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0); break;
  default:     break;
  }


  /* flags */
  flags=EBC_User_GetFlags(xdlg->user);
  GWEN_Dialog_SetIntProperty(dlg, "forceSslv3Check", GWEN_DialogProperty_Value, 0,
			     (flags & EBC_USER_FLAGS_FORCE_SSLV3)?1:0,
			     0);
  GWEN_Dialog_SetIntProperty(dlg, "useIzlCheck", GWEN_DialogProperty_Value, 0,
			     (flags & EBC_USER_FLAGS_USE_IZL)?1:0,
			     0);
  GWEN_Dialog_SetIntProperty(dlg, "noEuCheck", GWEN_DialogProperty_Value, 0,
			     (flags & EBC_USER_FLAGS_NO_EU)?1:0,
			     0);
}



int EBC_EditUserDialog_fromGui(GWEN_DIALOG *dlg, AB_USER *u, int quiet) {
  EBC_EDIT_USER_DIALOG *xdlg;
  const char *s;
  const AB_COUNTRY *c;
  int i;
  uint32_t flags;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_EDIT_USER_DIALOG, dlg);
  assert(xdlg);

  /* fromGui */
  s=GWEN_Dialog_GetCharProperty(dlg, "userNameEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    if (u)
      AB_User_SetUserName(u, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "bankCodeEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    removeAllSpaces((uint8_t*)GWEN_Buffer_GetStart(tbuf));
    if (u)
      AB_User_SetBankCode(u, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "userIdEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    if (u)
      AB_User_SetUserId(u, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "customerIdEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    if (u)
      AB_User_SetCustomerId(u, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "urlEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    if (u)
      EBC_User_SetServerUrl(u, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "hostIdEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    if (u)
      EBC_User_SetPeerId(u, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  /*  get country */
  c=EBC_EditUserDialog_GetCurrentCountry(dlg);
  if (c) {
    if (u)
      AB_User_SetCountry(u, AB_Country_GetCode(c));
  }

  i=GWEN_Dialog_GetIntProperty(dlg, "ebicsVersionCombo", GWEN_DialogProperty_Value, 0, -1);
  switch(i) {
  case 0: EBC_User_SetProtoVersion(xdlg->user, "H002"); break;
  default:
  case 1: EBC_User_SetProtoVersion(xdlg->user, "H003"); break;
  }

  i=GWEN_Dialog_GetIntProperty(dlg, "signVersionCombo", GWEN_DialogProperty_Value, 0, -1);
  switch(i) {
  case 0: EBC_User_SetSignVersion(xdlg->user, "A004"); break;
  default:
  case 1: EBC_User_SetSignVersion(xdlg->user, "A005"); break;
  }

  i=GWEN_Dialog_GetIntProperty(dlg, "cryptVersionCombo", GWEN_DialogProperty_Value, 0, -1);
  switch(i) {
  case 0: EBC_User_SetCryptVersion(xdlg->user, "E001"); break;
  default:
  case 1: EBC_User_SetCryptVersion(xdlg->user, "E002"); break;
  }

  i=GWEN_Dialog_GetIntProperty(dlg, "authVersionCombo", GWEN_DialogProperty_Value, 0, -1);
  switch(i) {
  case 0: EBC_User_SetAuthVersion(xdlg->user, "X001"); break;
  default:
  case 1: EBC_User_SetAuthVersion(xdlg->user, "X002"); break;
  }

  i=GWEN_Dialog_GetIntProperty(dlg, "statusCombo", GWEN_DialogProperty_Value, 0, -1);
  switch(i) {
  case 0: EBC_User_SetStatus(xdlg->user, EBC_UserStatus_New); break;
  case 1: EBC_User_SetStatus(xdlg->user, EBC_UserStatus_Init1); break;
  case 2: EBC_User_SetStatus(xdlg->user, EBC_UserStatus_Init2); break;
  case 3: EBC_User_SetStatus(xdlg->user, EBC_UserStatus_Enabled); break;
  case 4: EBC_User_SetStatus(xdlg->user, EBC_UserStatus_Disabled); break;
  default: break;
  }

  i=GWEN_Dialog_GetIntProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_Value, 0, -1);
  switch(i) {
  case 0:
    EBC_User_SetHttpVMajor(xdlg->user, 1);
    EBC_User_SetHttpVMinor(xdlg->user, 0);
    break;
  default:
  case 1:
    EBC_User_SetHttpVMajor(xdlg->user, 1);
    EBC_User_SetHttpVMinor(xdlg->user, 1);
    break;
  }

  flags=0;
  if (GWEN_Dialog_GetIntProperty(dlg, "forceSslv3Check", GWEN_DialogProperty_Value, 0, 0))
    flags|=EBC_USER_FLAGS_FORCE_SSLV3;
  if (GWEN_Dialog_GetIntProperty(dlg, "useIzlCheck", GWEN_DialogProperty_Value, 0, 0))
    flags|=EBC_USER_FLAGS_USE_IZL;
  if (GWEN_Dialog_GetIntProperty(dlg, "noEuCheck", GWEN_DialogProperty_Value, 0, 0))
    flags|=EBC_USER_FLAGS_NO_EU;
  EBC_User_SetFlags(xdlg->user, flags);

  return 0;
}



void EBC_EditUserDialog_Fini(GWEN_DIALOG *dlg) {
  EBC_EDIT_USER_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_EDIT_USER_DIALOG, dlg);
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



int EBC_EditUserDialog_HandleActivatedBankCode(GWEN_DIALOG *dlg) {
  EBC_EDIT_USER_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_EDIT_USER_DIALOG, dlg);
  assert(xdlg);

  dlg2=AB_SelectBankInfoDialog_new(xdlg->banking, "de", NULL);
  if (dlg2==NULL) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not create dialog");
    GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Could create dialog, maybe incomplete installation?"));
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
    }
  }
  GWEN_Dialog_free(dlg2);

  return GWEN_DialogEvent_ResultHandled;
}



int EBC_EditUserDialog_HandleActivatedOk(GWEN_DIALOG *dlg) {
  EBC_EDIT_USER_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_EDIT_USER_DIALOG, dlg);
  assert(xdlg);

  rv=EBC_EditUserDialog_fromGui(dlg, NULL, 0);
  if (rv<0) {
    return GWEN_DialogEvent_ResultHandled;
  }

  if (xdlg->doLock) {
    int rv;

    rv=AB_Banking_BeginExclUseUser(xdlg->banking, xdlg->user);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
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

  EBC_EditUserDialog_fromGui(dlg, xdlg->user, 1);

  if (xdlg->doLock) {
    int rv;

    rv=AB_Banking_EndExclUseUser(xdlg->banking, xdlg->user, 0);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
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



static int EBC_EditUserDialog_HandleActivatedGetBankKeys(GWEN_DIALOG *dlg) {
  EBC_EDIT_USER_DIALOG *xdlg;
  int rv;
  uint32_t guiid;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_EDIT_USER_DIALOG, dlg);
  assert(xdlg);

  guiid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS |
			       GWEN_GUI_PROGRESS_SHOW_PROGRESS |
			       GWEN_GUI_PROGRESS_SHOW_LOG |
			       GWEN_GUI_PROGRESS_ALWAYS_SHOW_LOG |
			       GWEN_GUI_PROGRESS_KEEP_OPEN |
			       GWEN_GUI_PROGRESS_SHOW_ABORT,
			       I18N("Executing Request"),
			       I18N("Now the request is send "
				    "to the credit institute."),
			       GWEN_GUI_PROGRESS_NONE,
			       0);
  rv=EBC_Provider_Send_HPB(AB_User_GetProvider(xdlg->user), xdlg->user, 1);
  EBC_EditUserDialog_toGui(dlg);
  GWEN_Gui_ProgressEnd(guiid);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Error sending key request (%d)", rv);
    return GWEN_DialogEvent_ResultHandled;
  }


  return GWEN_DialogEvent_ResultHandled;
}



static int EBC_EditUserDialog_HandleActivatedGetAccounts(GWEN_DIALOG *dlg) {
  EBC_EDIT_USER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_EDIT_USER_DIALOG, dlg);
  assert(xdlg);

  if ((EBC_User_GetStatus(xdlg->user)==EBC_UserStatus_Enabled) &&
      (EBC_User_GetFlags(xdlg->user) & EBC_USER_FLAGS_CLIENT_DATA_DOWNLOAD_SPP)) {
    uint32_t guiid;
    int rv1;
    int rv2;

    guiid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS |
				 GWEN_GUI_PROGRESS_SHOW_PROGRESS |
				 GWEN_GUI_PROGRESS_SHOW_LOG |
				 GWEN_GUI_PROGRESS_ALWAYS_SHOW_LOG |
				 GWEN_GUI_PROGRESS_KEEP_OPEN |
				 GWEN_GUI_PROGRESS_SHOW_ABORT,
				 I18N("Executing Request"),
				 I18N("Now the request is send "
				      "to the credit institute."),
				 GWEN_GUI_PROGRESS_NONE,
				 0);

    rv1=EBC_Provider_Send_HKD(AB_User_GetProvider(xdlg->user), xdlg->user, 1);
    DBG_INFO(AQEBICS_LOGDOMAIN, "Retrieving user information");
    rv2=EBC_Provider_Send_HTD(AB_User_GetProvider(xdlg->user), xdlg->user, 1);
    GWEN_Gui_ProgressEnd(guiid);
    if ((rv1<0) && (rv2<0)) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Couldn't send HKD or HTD request (%d, %d)", rv1, rv2);
    }
  }
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "The bank does not support download of account information");
  }

  return GWEN_DialogEvent_ResultHandled;
}




static int EBC_EditUserDialog_HandleActivatedIniLetter(GWEN_DIALOG *dlg) {
  EBC_EDIT_USER_DIALOG *xdlg;
  int rv;
  GWEN_BUFFER *tbuf;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_EDIT_USER_DIALOG, dlg);
  assert(xdlg);

  tbuf=GWEN_Buffer_new(0, 1024, 0, 1);

  /* add HTML version of the INI letter */
  //GWEN_Buffer_AppendString(tbuf, "<html>");
  rv=EBC_Provider_GetIniLetterTxt(AB_User_GetProvider(xdlg->user),
				  xdlg->user, 0, tbuf, 0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    // TODO: show error message
    AB_Banking_ClearCryptTokenList(xdlg->banking);
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }
  //GWEN_Buffer_AppendString(tbuf, "</html>");


#if 0
  /* add ASCII version of the INI letter for frontends which don't support HTML */
  rv=EBC_Provider_GetIniLetterTxt(AB_User_GetProvider(xdlg->user),
				  xdlg->user, 0, tbuf, 0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    // TODO: show error message
    AB_Banking_ClearCryptTokenList(xdlg->banking);
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }
#endif

  rv=GWEN_Gui_Print(I18N("INI Letter"),
		    "EBICS-INILETTER",
		    I18N("INI Letter for EBICS"),
		    GWEN_Buffer_GetStart(tbuf),
		    0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    // TODO: show error message
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }

  GWEN_Buffer_free(tbuf);
  return GWEN_DialogEvent_ResultHandled;
}



static int EBC_EditUserDialog_HandleActivatedHiaLetter(GWEN_DIALOG *dlg) {
  EBC_EDIT_USER_DIALOG *xdlg;
  int rv;
  GWEN_BUFFER *tbuf;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_EDIT_USER_DIALOG, dlg);
  assert(xdlg);

  tbuf=GWEN_Buffer_new(0, 1024, 0, 1);

  /* add HTML version of the INI letter */
  //GWEN_Buffer_AppendString(tbuf, "<html>");
  rv=EBC_Provider_GetHiaLetterTxt(AB_User_GetProvider(xdlg->user),
				  xdlg->user, 0, tbuf, 0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    // TODO: show error message
    AB_Banking_ClearCryptTokenList(xdlg->banking);
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }
  //GWEN_Buffer_AppendString(tbuf, "</html>");


#if 0
  /* add ASCII version of the HIA letter for frontends which don't support HTML */
  rv=EBC_Provider_GetHIALetterTxt(AB_User_GetProvider(xdlg->user),
				  xdlg->user, 0, tbuf, 0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    // TODO: show error message
    AB_Banking_ClearCryptTokenList(xdlg->banking);
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }
#endif

  rv=GWEN_Gui_Print(I18N("HIA Letter"),
		    "EBICS-HIALETTER",
		    I18N("HIA Letter for EBICS"),
		    GWEN_Buffer_GetStart(tbuf),
		    0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    // TODO: show error message
    GWEN_Buffer_free(tbuf);
    return GWEN_DialogEvent_ResultHandled;
  }

  GWEN_Buffer_free(tbuf);
  return GWEN_DialogEvent_ResultHandled;
}



int EBC_EditUserDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  if (strcasecmp(sender, "bankCodeButton")==0)
    return EBC_EditUserDialog_HandleActivatedBankCode(dlg);
  else if (strcasecmp(sender, "getBankKeysButton")==0)
    return EBC_EditUserDialog_HandleActivatedGetBankKeys(dlg);
  else if (strcasecmp(sender, "getAccountsButton")==0)
    return EBC_EditUserDialog_HandleActivatedGetAccounts(dlg);
  else if (strcasecmp(sender, "iniLetterButton")==0)
    return EBC_EditUserDialog_HandleActivatedIniLetter(dlg);
  else if (strcasecmp(sender, "hiaLetterButton")==0)
    return EBC_EditUserDialog_HandleActivatedHiaLetter(dlg);
  else if (strcasecmp(sender, "ebicsVersionCombo")==0) {
    int i;


    i=GWEN_Dialog_GetIntProperty(dlg, "ebicsVersionCombo", GWEN_DialogProperty_Value, 0, -1);
    switch(i) {
    case 0: /* H002 */
      GWEN_Dialog_SetIntProperty(dlg, "signVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0);
      GWEN_Dialog_SetIntProperty(dlg, "cryptVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0);
      GWEN_Dialog_SetIntProperty(dlg, "authVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0);
      break;

    default:
    case 1: /* H003 */
      GWEN_Dialog_SetIntProperty(dlg, "signVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0);
      GWEN_Dialog_SetIntProperty(dlg, "cryptVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0);
      GWEN_Dialog_SetIntProperty(dlg, "authVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0);
      break;
    }
    return GWEN_DialogEvent_ResultHandled;
  }
  else if (strcasecmp(sender, "okButton")==0)
    return EBC_EditUserDialog_HandleActivatedOk(dlg);
  else if (strcasecmp(sender, "abortButton")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "helpButton")==0) {
    /* TODO: open u help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB EBC_EditUserDialog_SignalHandler(GWEN_DIALOG *dlg,
						   GWEN_DIALOG_EVENTTYPE t,
						   const char *sender) {
  EBC_EDIT_USER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_EDIT_USER_DIALOG, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    EBC_EditUserDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    EBC_EditUserDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return GWEN_DialogEvent_ResultNotHandled;

  case GWEN_DialogEvent_TypeActivated:
    return EBC_EditUserDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}




