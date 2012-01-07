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



#include "dlg_pintan_special_p.h"
#include "i18n_l.h"

#include <aqbanking/user.h>
#include <aqbanking/banking_be.h>

#include <aqhbci/user.h>
#include <aqhbci/provider.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>


#define DIALOG_MINWIDTH  200
#define DIALOG_MINHEIGHT 100



GWEN_INHERIT(GWEN_DIALOG, AH_PINTAN_SPECIAL_DIALOG)




GWEN_DIALOG *AH_PinTanSpecialDialog_new(AB_BANKING *ab) {
  GWEN_DIALOG *dlg;
  AH_PINTAN_SPECIAL_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("ah_setup_pintan_special");
  GWEN_NEW_OBJECT(AH_PINTAN_SPECIAL_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AH_PINTAN_SPECIAL_DIALOG, dlg, xdlg,
		       AH_PinTanSpecialDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, AH_PinTanSpecialDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
			       "aqbanking/backends/aqhbci/dialogs/dlg_pintan_special.dlg",
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



void GWENHYWFAR_CB AH_PinTanSpecialDialog_FreeData(void *bp, void *p) {
  AH_PINTAN_SPECIAL_DIALOG *xdlg;

  xdlg=(AH_PINTAN_SPECIAL_DIALOG*) p;

  free(xdlg->tanMediumId);

  GWEN_FREE_OBJECT(xdlg);
}



int AH_PinTanSpecialDialog_GetHttpVMajor(const GWEN_DIALOG *dlg) {
  AH_PINTAN_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  return xdlg->httpVMajor;
}



int AH_PinTanSpecialDialog_GetHttpVMinor(const GWEN_DIALOG *dlg) {
  AH_PINTAN_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  return xdlg->httpVMinor;
}



void AH_PinTanSpecialDialog_SetHttpVersion(GWEN_DIALOG *dlg, int vmajor, int vminor) {
  AH_PINTAN_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  xdlg->httpVMajor=vmajor;
  xdlg->httpVMinor=vminor;
}



int AH_PinTanSpecialDialog_GetHbciVersion(const GWEN_DIALOG *dlg) {
  AH_PINTAN_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  return xdlg->hbciVersion;
}



void AH_PinTanSpecialDialog_SetHbciVersion(GWEN_DIALOG *dlg, int i) {
  AH_PINTAN_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  xdlg->hbciVersion=i;
}



uint32_t AH_PinTanSpecialDialog_GetFlags(const GWEN_DIALOG *dlg) {
  AH_PINTAN_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  return xdlg->flags;
}



void AH_PinTanSpecialDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AH_PINTAN_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags=fl;
}



void AH_PinTanSpecialDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AH_PINTAN_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}



void AH_PinTanSpecialDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AH_PINTAN_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}



const char *AH_PinTanSpecialDialog_GetTanMediumId(const GWEN_DIALOG *dlg) {
  AH_PINTAN_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  return xdlg->tanMediumId;
}



void AH_PinTanSpecialDialog_SetTanMediumId(GWEN_DIALOG *dlg, const char *s) {
  AH_PINTAN_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->tanMediumId);
  if (s && *s)
    xdlg->tanMediumId=strdup(s);
  else
    xdlg->tanMediumId=NULL;
}



void AH_PinTanSpecialDialog_Init(GWEN_DIALOG *dlg) {
  AH_PINTAN_SPECIAL_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
			      "",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("HBCI PIN/TAN Special Settings"),
			      0);

  GWEN_Dialog_SetCharProperty(dlg, "hbciVersionCombo", GWEN_DialogProperty_AddValue, 0, "2.20", 0);
  GWEN_Dialog_SetCharProperty(dlg, "hbciVersionCombo", GWEN_DialogProperty_AddValue, 0, "3.0", 0);

  GWEN_Dialog_SetCharProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_AddValue, 0, "1.0", 0);
  GWEN_Dialog_SetCharProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_AddValue, 0, "1.1", 0);

  /* toGui */
  switch(((xdlg->httpVMajor)<<8)+xdlg->httpVMinor) {
  case 0x0100: GWEN_Dialog_SetIntProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0); break;
  case 0x0101: GWEN_Dialog_SetIntProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0); break;
  default:     break;
  }

  switch(xdlg->hbciVersion) {
  case 220: GWEN_Dialog_SetIntProperty(dlg, "hbciVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0); break;
  case 300: GWEN_Dialog_SetIntProperty(dlg, "hbciVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0); break;
  default:  break;
  }

  GWEN_Dialog_SetIntProperty(dlg, "forceSslv3Check", GWEN_DialogProperty_Value, 0,
			     (xdlg->flags & AH_USER_FLAGS_FORCE_SSL3)?1:0,
			     0);

  GWEN_Dialog_SetIntProperty(dlg, "noBase64Check", GWEN_DialogProperty_Value, 0,
			     (xdlg->flags & AH_USER_FLAGS_NO_BASE64)?1:0,
			     0);

  GWEN_Dialog_SetIntProperty(dlg, "omitSmsAccountCheck", GWEN_DialogProperty_Value, 0,
			     (xdlg->flags & AH_USER_FLAGS_TAN_OMIT_SMS_ACCOUNT)?1:0,
			     0);

  if (xdlg->tanMediumId)
    GWEN_Dialog_SetCharProperty(dlg, "tanMediumIdEdit", GWEN_DialogProperty_Value, 0, xdlg->tanMediumId, 0);
  /* set tooltip */
  GWEN_Dialog_SetCharProperty(dlg, "tanMediumIdEdit", GWEN_DialogProperty_ToolTip, 0,
                              I18N("For smsTAN or mTAN this is your mobile phone number. "
                                   "Please ask your bank for the necessary format of this number."),
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



void AH_PinTanSpecialDialog_Fini(GWEN_DIALOG *dlg) {
  AH_PINTAN_SPECIAL_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;
  uint32_t flags;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  /* fromGui */
  i=GWEN_Dialog_GetIntProperty(dlg, "hbciVersionCombo", GWEN_DialogProperty_Value, 0, -1);
  switch(i) {
  case 0: xdlg->hbciVersion=220; break;
  default:
  case 1: xdlg->hbciVersion=300; break;
  }

  i=GWEN_Dialog_GetIntProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_Value, 0, -1);
  switch(i) {
  case 0:
    xdlg->httpVMajor=1;
    xdlg->httpVMinor=0;
    break;
  default:
  case 1:
    xdlg->httpVMajor=1;
    xdlg->httpVMinor=1;
    break;
  }

  flags=0;
  if (GWEN_Dialog_GetIntProperty(dlg, "forceSslv3Check", GWEN_DialogProperty_Value, 0, 0))
    flags|=AH_USER_FLAGS_FORCE_SSL3;
  if (GWEN_Dialog_GetIntProperty(dlg, "noBase64Check", GWEN_DialogProperty_Value, 0, 0))
    flags|=AH_USER_FLAGS_NO_BASE64;
  if (GWEN_Dialog_GetIntProperty(dlg, "omitSmsAccountCheck", GWEN_DialogProperty_Value, 0, 0))
    flags|=AH_USER_FLAGS_TAN_OMIT_SMS_ACCOUNT;
  xdlg->flags=flags;

  s=GWEN_Dialog_GetCharProperty(dlg, "tanMediumIdEdit", GWEN_DialogProperty_Value, 0, NULL);
  AH_PinTanSpecialDialog_SetTanMediumId(dlg, s);

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



int AH_PinTanSpecialDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  DBG_NOTICE(0, "Activated: %s", sender);
  if (strcasecmp(sender, "okButton")==0)
    return GWEN_DialogEvent_ResultAccept;
  else if (strcasecmp(sender, "abortButton")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "helpButton")==0) {
    /* TODO: open a help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB AH_PinTanSpecialDialog_SignalHandler(GWEN_DIALOG *dlg,
						       GWEN_DIALOG_EVENTTYPE t,
						       const char *sender) {
  AH_PINTAN_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_PINTAN_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    AH_PinTanSpecialDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AH_PinTanSpecialDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeActivated:
    return AH_PinTanSpecialDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}






