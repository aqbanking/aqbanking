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



#include "dlg_edituserpintan_p.h"
#include "i18n_l.h"

#include <aqhbci/user.h>
#include <aqhbci/provider.h>

#include <aqbanking/user.h>
#include <aqbanking/banking_be.h>
#include <aqbanking/dlg_selectbankinfo.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>


#define DIALOG_MINWIDTH  200
#define DIALOG_MINHEIGHT 200



GWEN_INHERIT(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG)




GWEN_DIALOG *AH_EditUserPinTanDialog_new(AB_BANKING *ab, AB_USER *u, int doLock) {
  GWEN_DIALOG *dlg;
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("ah_edit_user_pintan");
  GWEN_NEW_OBJECT(AH_EDIT_USER_PINTAN_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg, xdlg,
		       AH_EditUserPinTanDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, AH_EditUserPinTanDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
			       "aqbanking/backends/aqhbci/dialogs/dlg_edituserpintan.dlg",
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

  /* preset */
  xdlg->banking=ab;
  xdlg->user=u;
  xdlg->doLock=doLock;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB AH_EditUserPinTanDialog_FreeData(void *bp, void *p) {
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;

  xdlg=(AH_EDIT_USER_PINTAN_DIALOG*) p;
  AH_TanMethod_List_free(xdlg->tanMethodList);

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
  DBG_INFO(AQHBCI_LOGDOMAIN, "No local name");
  return GWEN_ERROR_NO_DATA;
}



static int createTanMethodString(const AH_TAN_METHOD *tm, GWEN_BUFFER *tbuf) {
  const char *s;
  char numbuf[32];

  snprintf(numbuf, sizeof(numbuf)-1, "%d", AH_TanMethod_GetFunction(tm));
  numbuf[sizeof(numbuf)-1]=0;
  GWEN_Buffer_AppendString(tbuf, numbuf);

  s=AH_TanMethod_GetMethodName(tm);
  if (!(s && *s))
    s=AH_TanMethod_GetMethodId(tm);
  if (s && *s) {
    GWEN_Buffer_AppendString(tbuf, " - ");
    GWEN_Buffer_AppendString(tbuf, s);
  }

  /* add HKTAN version */
  GWEN_Buffer_AppendString(tbuf, " (Version ");
  snprintf(numbuf, sizeof(numbuf)-1, "%d", AH_TanMethod_GetGvVersion(tm));
  numbuf[sizeof(numbuf)-1]=0;
  GWEN_Buffer_AppendString(tbuf, numbuf);
  GWEN_Buffer_AppendString(tbuf, ")");

  return 0;
}



const AB_COUNTRY *AH_EditUserPinTanDialog_GetCurrentCountry(GWEN_DIALOG *dlg) {
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  int idx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
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



const AH_TAN_METHOD *AH_EditUserPinTanDialog_GetCurrentTanMethod(GWEN_DIALOG *dlg) {
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  int idx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
  assert(xdlg);

  idx=GWEN_Dialog_GetIntProperty(dlg, "tanMethodCombo", GWEN_DialogProperty_Value, 0, -1);
  if (idx>=0) {
    const char *currentText;

    currentText=GWEN_Dialog_GetCharProperty(dlg, "tanMethodCombo", GWEN_DialogProperty_Value, idx, NULL);
    if (currentText && *currentText && xdlg->tanMethodList) {
      AH_TAN_METHOD *tm;
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      tm=AH_TanMethod_List_First(xdlg->tanMethodList);
      while(tm) {
	if (createTanMethodString(tm, tbuf)==0 &&
	    strcasecmp(GWEN_Buffer_GetStart(tbuf), currentText)==0) {
	  GWEN_Buffer_free(tbuf);
	  return tm;
	}
	GWEN_Buffer_Reset(tbuf);

        tm=AH_TanMethod_List_Next(tm);
      }
      GWEN_Buffer_free(tbuf);
    }
  }

  return NULL;
}



static void AH_EditUserPinTanDialog_UpdateTanMethods(GWEN_DIALOG *dlg) {
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  const AH_TAN_METHOD_LIST *ctl;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->tanMethodList) {
    AH_TanMethod_List_free(xdlg->tanMethodList);
    xdlg->tanMethodList=NULL;
  }
  ctl=AH_User_GetTanMethodDescriptions(xdlg->user);
  if (ctl)
    xdlg->tanMethodList=AH_TanMethod_List_dup(ctl);

  /* setup tanmethod combo */
  GWEN_Dialog_SetIntProperty(dlg, "tanMethodCombo", GWEN_DialogProperty_ClearValues, 0, 0, 0);
  GWEN_Dialog_SetCharProperty(dlg, "tanMethodCombo", GWEN_DialogProperty_AddValue, 0, I18N("-- select --"), 0);
  if (xdlg->tanMethodList) {
    AH_TAN_METHOD *tm;
    GWEN_BUFFER *tbuf;
    int i;
    int idx;
    int selectedMethod;
    int tjv;
    int tfn;

    selectedMethod=AH_User_GetSelectedTanMethod(xdlg->user);
    tjv=selectedMethod / 1000;
    tfn=selectedMethod % 1000;
    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    idx=-1;
    i=1;
    tm=AH_TanMethod_List_First(xdlg->tanMethodList);
    while(tm) {
      if (createTanMethodString(tm, tbuf)==0) {
	if (AH_TanMethod_GetFunction(tm)==tfn && AH_TanMethod_GetGvVersion(tm)==tjv)
          idx=i;
	GWEN_Dialog_SetCharProperty(dlg, "tanMethodCombo", GWEN_DialogProperty_AddValue, 0, GWEN_Buffer_GetStart(tbuf), 0);
        i++;
      }
      GWEN_Buffer_Reset(tbuf);

      tm=AH_TanMethod_List_Next(tm);
    }
    GWEN_Buffer_free(tbuf);
    if (idx>=0)
      /* chooses selected entry in combo box */
      GWEN_Dialog_SetIntProperty(dlg, "tanMethodCombo", GWEN_DialogProperty_Value, 0, idx, 0);
  }
}



void AH_EditUserPinTanDialog_Init(GWEN_DIALOG *dlg) {
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;
  const char *s;
  uint32_t flags;
  const GWEN_URL *gu;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
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

  /* also selects currently selected TAN method */
  AH_EditUserPinTanDialog_UpdateTanMethods(dlg);

  s=AB_User_GetUserName(xdlg->user);
  GWEN_Dialog_SetCharProperty(dlg, "userNameEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AB_User_GetBankCode(xdlg->user);
  GWEN_Dialog_SetCharProperty(dlg, "bankCodeEdit", GWEN_DialogProperty_Value, 0, s, 0);

  gu=AH_User_GetServerUrl(xdlg->user);
  if (gu) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Url_toString(gu, tbuf);
    GWEN_Dialog_SetCharProperty(dlg, "urlEdit", GWEN_DialogProperty_Value, 0, GWEN_Buffer_GetStart(tbuf), 0);
    GWEN_Buffer_free(tbuf);
  }

  s=AB_User_GetUserId(xdlg->user);
  GWEN_Dialog_SetCharProperty(dlg, "userIdEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=AB_User_GetCustomerId(xdlg->user);
  GWEN_Dialog_SetCharProperty(dlg, "customerIdEdit", GWEN_DialogProperty_Value, 0, s, 0);

  GWEN_Dialog_SetCharProperty(dlg, "hbciVersionCombo", GWEN_DialogProperty_AddValue, 0, "2.20", 0);
  GWEN_Dialog_SetCharProperty(dlg, "hbciVersionCombo", GWEN_DialogProperty_AddValue, 0, "3.0", 0);

  GWEN_Dialog_SetCharProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_AddValue, 0, "1.0", 0);
  GWEN_Dialog_SetCharProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_AddValue, 0, "1.1", 0);

  /* toGui */
  switch(((AH_User_GetHttpVMajor(xdlg->user))<<8)+AH_User_GetHttpVMinor(xdlg->user)) {
  case 0x0100: GWEN_Dialog_SetIntProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0); break;
  case 0x0101: GWEN_Dialog_SetIntProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0); break;
  default:     break;
  }

  switch(AH_User_GetHbciVersion(xdlg->user)) {
  case 220: GWEN_Dialog_SetIntProperty(dlg, "hbciVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0); break;
  case 300: GWEN_Dialog_SetIntProperty(dlg, "hbciVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0); break;
  default:  break;
  }

  /* TAN medium id */
  s=AH_User_GetTanMediumId(xdlg->user);
  GWEN_Dialog_SetCharProperty(dlg, "tanMediumIdEdit", GWEN_DialogProperty_Value, 0, s, 0);
  GWEN_Dialog_SetCharProperty(dlg, "tanMediumIdEdit", GWEN_DialogProperty_ToolTip, 0,
                              I18N("For smsTAN or mTAN this is your mobile phone number. "
                                   "Please ask your bank for the necessary format of this number."),
                              0);

  flags=AH_User_GetFlags(xdlg->user);
  GWEN_Dialog_SetIntProperty(dlg, "forceSslv3Check", GWEN_DialogProperty_Value, 0,
			     (flags & AH_USER_FLAGS_FORCE_SSL3)?1:0,
			     0);

  GWEN_Dialog_SetIntProperty(dlg, "noBase64Check", GWEN_DialogProperty_Value, 0,
			     (flags & AH_USER_FLAGS_NO_BASE64)?1:0,
			     0);

  GWEN_Dialog_SetIntProperty(dlg, "omitSmsAccountCheck", GWEN_DialogProperty_Value, 0,
                             (flags & AH_USER_FLAGS_TAN_OMIT_SMS_ACCOUNT)?1:0,
			     0);


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



int AH_EditUserPinTanDialog_fromGui(GWEN_DIALOG *dlg, AB_USER *u, int quiet) {
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  const char *s;
  const AB_COUNTRY *c;
  int i;
  uint32_t flags;
  const AH_TAN_METHOD *tm;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
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

  /*  get country */
  c=AH_EditUserPinTanDialog_GetCurrentCountry(dlg);
  if (c) {
    if (u)
      AB_User_SetCountry(u, AB_Country_GetCode(c));
  }

  i=GWEN_Dialog_GetIntProperty(dlg, "hbciVersionCombo", GWEN_DialogProperty_Value, 0, -1);
  switch(i) {
  case 0: AH_User_SetHbciVersion(xdlg->user, 220); break;
  default:
  case 1: AH_User_SetHbciVersion(xdlg->user, 300); break;
  }

  i=GWEN_Dialog_GetIntProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_Value, 0, -1);
  switch(i) {
  case 0:
    AH_User_SetHttpVMajor(xdlg->user, 1);
    AH_User_SetHttpVMinor(xdlg->user, 0);
    break;
  default:
  case 1:
    AH_User_SetHttpVMajor(xdlg->user, 1);
    AH_User_SetHttpVMinor(xdlg->user, 1);
    break;
  }

  tm=AH_EditUserPinTanDialog_GetCurrentTanMethod(dlg);
  if (tm) {
    int fn;

    fn=(AH_TanMethod_GetGvVersion(tm)*1000)+AH_TanMethod_GetFunction(tm);
    AH_User_SetSelectedTanMethod(xdlg->user, fn);
  }

  /* handle tan medium id */
  s=GWEN_Dialog_GetCharProperty(dlg, "tanMediumIdEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    if (u)
      AH_User_SetTanMediumId(u, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "urlEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;
    GWEN_URL *gu;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    removeAllSpaces((uint8_t*)GWEN_Buffer_GetStart(tbuf));
    gu=GWEN_Url_fromString(GWEN_Buffer_GetStart(tbuf));
    if (gu==NULL) {
      if (!quiet) {
	GWEN_Gui_ShowError(I18N("Error"), "%s", I18N("Invalid URL"));
      }
      GWEN_Buffer_free(tbuf);
      return GWEN_ERROR_BAD_DATA;
    }
    if (u)
      AH_User_SetServerUrl(u, gu);
    GWEN_Url_free(gu);
    GWEN_Buffer_free(tbuf);
  }

  flags=0;
  if (GWEN_Dialog_GetIntProperty(dlg, "forceSslv3Check", GWEN_DialogProperty_Value, 0, 0))
    flags|=AH_USER_FLAGS_FORCE_SSL3;
  if (GWEN_Dialog_GetIntProperty(dlg, "noBase64Check", GWEN_DialogProperty_Value, 0, 0))
    flags|=AH_USER_FLAGS_NO_BASE64;
  if (GWEN_Dialog_GetIntProperty(dlg, "omitSmsAccountCheck", GWEN_DialogProperty_Value, 0, 0))
    flags|=AH_USER_FLAGS_TAN_OMIT_SMS_ACCOUNT;
  AH_User_SetFlags(xdlg->user, flags);

  return 0;
}



void AH_EditUserPinTanDialog_Fini(GWEN_DIALOG *dlg) {
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
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



int AH_EditUserPinTanDialog_HandleActivatedBankCode(GWEN_DIALOG *dlg) {
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  GWEN_DIALOG *dlg2;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
  assert(xdlg);

  dlg2=AB_SelectBankInfoDialog_new(xdlg->banking, "de", NULL);
  if (dlg2==NULL) {
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



int AH_EditUserPinTanDialog_HandleActivatedOk(GWEN_DIALOG *dlg) {
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
  assert(xdlg);

  rv=AH_EditUserPinTanDialog_fromGui(dlg, NULL, 0);
  if (rv<0) {
    return GWEN_DialogEvent_ResultHandled;
  }

  if (xdlg->doLock) {
    int rv;

    rv=AB_Banking_BeginExclUseUser(xdlg->banking, xdlg->user);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
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

  AH_EditUserPinTanDialog_fromGui(dlg, xdlg->user, 1);

  if (xdlg->doLock) {
    int rv;

    rv=AB_Banking_EndExclUseUser(xdlg->banking, xdlg->user, 0);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
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



static int AH_EditUserPinTanDialog_HandleActivatedGetCert(GWEN_DIALOG *dlg) {
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
  assert(xdlg);

  rv=AH_Provider_GetCert(AB_User_GetProvider(xdlg->user),
			 xdlg->user,
			 1,   /* withProgress */
			 0,   /* nounmount */
			 xdlg->doLock);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }

  return GWEN_DialogEvent_ResultHandled;
}



static int AH_EditUserPinTanDialog_HandleActivatedGetSysId(GWEN_DIALOG *dlg) {
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
  assert(xdlg);

  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetSysId(AB_User_GetProvider(xdlg->user),
			  xdlg->user,
                          ctx,
			  1,   /* withProgress */
			  0,   /* nounmount */
			  xdlg->doLock);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }

  AH_EditUserPinTanDialog_UpdateTanMethods(dlg);

  AB_ImExporterContext_free(ctx);
  return GWEN_DialogEvent_ResultHandled;
}



static int AH_EditUserPinTanDialog_HandleActivatedGetItanModes(GWEN_DIALOG *dlg) {
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
  assert(xdlg);

  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetItanModes(AB_User_GetProvider(xdlg->user),
			      xdlg->user,
			      ctx,
			      1,   /* withProgress */
			      0,   /* nounmount */
			      xdlg->doLock);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }

  AH_EditUserPinTanDialog_UpdateTanMethods(dlg);

  AB_ImExporterContext_free(ctx);
  return GWEN_DialogEvent_ResultHandled;
}



static int AH_EditUserPinTanDialog_HandleActivatedGetAccounts(GWEN_DIALOG *dlg) {
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
  assert(xdlg);

  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetAccounts(AB_User_GetProvider(xdlg->user),
			     xdlg->user,
			     ctx,
			     1,   /* withProgress */
			     0,   /* nounmount */
			     xdlg->doLock);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }

  AH_EditUserPinTanDialog_UpdateTanMethods(dlg);

  AB_ImExporterContext_free(ctx);
  return GWEN_DialogEvent_ResultHandled;
}



int AH_EditUserPinTanDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  if (strcasecmp(sender, "bankCodeButton")==0)
    return AH_EditUserPinTanDialog_HandleActivatedBankCode(dlg);
  else if (strcasecmp(sender, "getCertButton")==0)
    return AH_EditUserPinTanDialog_HandleActivatedGetCert(dlg);
  else if (strcasecmp(sender, "getSysIdButton")==0)
    return AH_EditUserPinTanDialog_HandleActivatedGetSysId(dlg);
  else if (strcasecmp(sender, "getItanModesButton")==0)
    return AH_EditUserPinTanDialog_HandleActivatedGetItanModes(dlg);
  else if (strcasecmp(sender, "getAccountsButton")==0)
    return AH_EditUserPinTanDialog_HandleActivatedGetAccounts(dlg);
  else if (strcasecmp(sender, "okButton")==0)
    return AH_EditUserPinTanDialog_HandleActivatedOk(dlg);
  else if (strcasecmp(sender, "abortButton")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "helpButton")==0) {
    /* TODO: open u help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB AH_EditUserPinTanDialog_SignalHandler(GWEN_DIALOG *dlg,
							GWEN_DIALOG_EVENTTYPE t,
							const char *sender) {
  AH_EDIT_USER_PINTAN_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_EDIT_USER_PINTAN_DIALOG, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    AH_EditUserPinTanDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AH_EditUserPinTanDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeActivated:
    return AH_EditUserPinTanDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}




