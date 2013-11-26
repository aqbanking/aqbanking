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



#include "dlg_user_special_p.h"

#include <aqbanking/user.h>
#include <aqbanking/banking_be.h>

#include <aqebics/user.h>
#include <aqebics/provider.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/i18n.h>


#define DIALOG_MINWIDTH  200
#define DIALOG_MINHEIGHT 100


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)
#define I18N_NOOP(msg) msg
#define I18S(msg) msg



GWEN_INHERIT(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG)




GWEN_DIALOG *EBC_UserSpecialDialog_new(AB_BANKING *ab) {
  GWEN_DIALOG *dlg;
  EBC_USER_SPECIAL_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("ah_setup_pintan_special");
  GWEN_NEW_OBJECT(EBC_USER_SPECIAL_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg, xdlg,
		       EBC_UserSpecialDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, EBC_UserSpecialDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(GWEN_PM_LIBNAME, GWEN_PM_SYSDATADIR,
			       "aqbanking/backends/aqebics/dialogs/dlg_user_special.dlg",
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

  xdlg->banking=ab;

  /* preset */
  xdlg->httpVMajor=1;
  xdlg->httpVMinor=1;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB EBC_UserSpecialDialog_FreeData(void *bp, void *p) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  xdlg=(EBC_USER_SPECIAL_DIALOG*) p;
  GWEN_FREE_OBJECT(xdlg);
}



int EBC_UserSpecialDialog_GetHttpVMajor(const GWEN_DIALOG *dlg) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  return xdlg->httpVMajor;
}



int EBC_UserSpecialDialog_GetHttpVMinor(const GWEN_DIALOG *dlg) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  return xdlg->httpVMinor;
}



void EBC_UserSpecialDialog_SetHttpVersion(GWEN_DIALOG *dlg, int vmajor, int vminor) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  xdlg->httpVMajor=vmajor;
  xdlg->httpVMinor=vminor;
}



uint32_t EBC_UserSpecialDialog_GetFlags(const GWEN_DIALOG *dlg) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  return xdlg->flags;
}



void EBC_UserSpecialDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags=fl;
}



void EBC_UserSpecialDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}



void EBC_UserSpecialDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}



const char *EBC_UserSpecialDialog_GetEbicsVersion(const GWEN_DIALOG *dlg) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  return xdlg->ebicsVersion;
}



void EBC_UserSpecialDialog_SetEbicsVersion(GWEN_DIALOG *dlg, const char *s) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->ebicsVersion);
  if (s) xdlg->ebicsVersion=strdup(s);
  else xdlg->ebicsVersion=NULL;
}



const char *EBC_UserSpecialDialog_GetSignVersion(const GWEN_DIALOG *dlg) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  return xdlg->signVersion;
}



void EBC_UserSpecialDialog_SetSignVersion(GWEN_DIALOG *dlg, const char *s) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->signVersion);
  if (s) xdlg->signVersion=strdup(s);
  else xdlg->signVersion=NULL;
}



const char *EBC_UserSpecialDialog_GetCryptVersion(const GWEN_DIALOG *dlg) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  return xdlg->cryptVersion;
}



void EBC_UserSpecialDialog_SetCryptVersion(GWEN_DIALOG *dlg, const char *s) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->cryptVersion);
  if (s) xdlg->cryptVersion=strdup(s);
  else xdlg->cryptVersion=NULL;
}



const char *EBC_UserSpecialDialog_GetAuthVersion(const GWEN_DIALOG *dlg) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  return xdlg->authVersion;
}



void EBC_UserSpecialDialog_SetAuthVersion(GWEN_DIALOG *dlg, const char *s) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->authVersion);
  if (s) xdlg->authVersion=strdup(s);
  else xdlg->authVersion=NULL;
}



int EBC_UserSpecialDialog_GetSignKeySize(const GWEN_DIALOG *dlg) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  return xdlg->signKeySize;
}



void EBC_UserSpecialDialog_SetSignKeySize(GWEN_DIALOG *dlg, int i) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  xdlg->signKeySize=i;
}



int EBC_UserSpecialDialog_GetCryptAndAuthKeySize(const GWEN_DIALOG *dlg) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  return xdlg->cryptAndAuthKeySize;
}



void EBC_UserSpecialDialog_SetCryptAndAuthKeySize(GWEN_DIALOG *dlg, int i) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  xdlg->cryptAndAuthKeySize=i;
}





void EBC_UserSpecialDialog_Init(GWEN_DIALOG *dlg) {
  EBC_USER_SPECIAL_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
			      "",
			      GWEN_DialogProperty_Title,
                              0,
                              I18N("EBICS Special Settings"),
			      0);

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

  GWEN_Dialog_SetCharProperty(dlg, "signKeySizeCombo", GWEN_DialogProperty_AddValue, 0, "1024", 0);
  GWEN_Dialog_SetCharProperty(dlg, "signKeySizeCombo", GWEN_DialogProperty_AddValue, 0, "2048", 0);
  GWEN_Dialog_SetCharProperty(dlg, "signKeySizeCombo", GWEN_DialogProperty_AddValue, 0, "4096", 0);
  GWEN_Dialog_SetCharProperty(dlg, "signKeySizeCombo", GWEN_DialogProperty_AddValue, 0, "8192", 0);

  GWEN_Dialog_SetCharProperty(dlg, "cryptAndAuthKeySizeCombo", GWEN_DialogProperty_AddValue, 0, "1024", 0);
  GWEN_Dialog_SetCharProperty(dlg, "cryptAndAuthKeySizeCombo", GWEN_DialogProperty_AddValue, 0, "2048", 0);
  GWEN_Dialog_SetCharProperty(dlg, "cryptAndAuthKeySizeCombo", GWEN_DialogProperty_AddValue, 0, "4096", 0);
  GWEN_Dialog_SetCharProperty(dlg, "cryptAndAuthKeySizeCombo", GWEN_DialogProperty_AddValue, 0, "8192", 0);

  /* toGui */

  /* protocol version */
  s=xdlg->ebicsVersion;
  if (! (s && *s))
    s="H003";
  if (strcasecmp(s, "H002")==0)
    GWEN_Dialog_SetIntProperty(dlg, "ebicsVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0);
  else if (strcasecmp(s, "H003")==0)
    GWEN_Dialog_SetIntProperty(dlg, "ebicsVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0);

  /* signature version */
  s=xdlg->signVersion;
  if (! (s && *s))
    s="A005";
  if (strcasecmp(s, "A004")==0)
    GWEN_Dialog_SetIntProperty(dlg, "signVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0);
  else if (strcasecmp(s, "A005")==0)
    GWEN_Dialog_SetIntProperty(dlg, "signVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0);

  /* crypt version */
  s=xdlg->cryptVersion;
  if (! (s && *s))
    s="E002";
  if (strcasecmp(s, "E001")==0)
    GWEN_Dialog_SetIntProperty(dlg, "cryptVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0);
  else if (strcasecmp(s, "E002")==0)
    GWEN_Dialog_SetIntProperty(dlg, "cryptVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0);

  /* auth version */
  s=xdlg->authVersion;
  if (! (s && *s))
    s="X002";
  if (strcasecmp(s, "X001")==0)
    GWEN_Dialog_SetIntProperty(dlg, "authVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0);
  else if (strcasecmp(s, "X002")==0)
    GWEN_Dialog_SetIntProperty(dlg, "authVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0);

  /* http version */
  switch(((xdlg->httpVMajor)<<8)+xdlg->httpVMinor) {
  case 0x0100: GWEN_Dialog_SetIntProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0); break;
  case 0x0101: GWEN_Dialog_SetIntProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0); break;
  default:     break;
  }

  switch(xdlg->signKeySize) {
  case 128:
    GWEN_Dialog_SetIntProperty(dlg, "signKeySizeCombo", GWEN_DialogProperty_Value, 0, 0, 0);
    break;
  case 256:
  default:
    GWEN_Dialog_SetIntProperty(dlg, "signKeySizeCombo", GWEN_DialogProperty_Value, 0, 1, 0);
    break;
  case 512:
    GWEN_Dialog_SetIntProperty(dlg, "signKeySizeCombo", GWEN_DialogProperty_Value, 0, 2, 0);
    break;
  case 1024:
    GWEN_Dialog_SetIntProperty(dlg, "signKeySizeCombo", GWEN_DialogProperty_Value, 0, 3, 0);
    break;
  }

  switch(xdlg->cryptAndAuthKeySize) {
  case 128:
    GWEN_Dialog_SetIntProperty(dlg, "cryptAndAuthKeySizeCombo", GWEN_DialogProperty_Value, 0, 0, 0);
    break;
  case 256:
  default:
    GWEN_Dialog_SetIntProperty(dlg, "cryptAndAuthKeySizeCombo", GWEN_DialogProperty_Value, 0, 1, 0);
    break;
  case 512:
    GWEN_Dialog_SetIntProperty(dlg, "cryptAndAuthKeySizeCombo", GWEN_DialogProperty_Value, 0, 2, 0);
    break;
  case 1024:
    GWEN_Dialog_SetIntProperty(dlg, "cryptAndAuthKeySizeCombo", GWEN_DialogProperty_Value, 0, 3, 0);
    break;
  }


  GWEN_Dialog_SetIntProperty(dlg, "forceSslv3Check", GWEN_DialogProperty_Value, 0,
                             (xdlg->flags & EBC_USER_FLAGS_FORCE_SSLV3)?1:0,
			     0);
  GWEN_Dialog_SetIntProperty(dlg, "useIzlCheck", GWEN_DialogProperty_Value, 0,
			     (xdlg->flags & EBC_USER_FLAGS_USE_IZL)?1:0,
			     0);
  GWEN_Dialog_SetIntProperty(dlg, "noEuCheck", GWEN_DialogProperty_Value, 0,
			     (xdlg->flags & EBC_USER_FLAGS_NO_EU)?1:0,
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






void EBC_UserSpecialDialog_Fini(GWEN_DIALOG *dlg) {
  EBC_USER_SPECIAL_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;
  uint32_t flags;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  /* fromGui */
  i=GWEN_Dialog_GetIntProperty(dlg, "ebicsVersionCombo", GWEN_DialogProperty_Value, 0, -1);
  switch(i) {
  case 0: EBC_UserSpecialDialog_SetEbicsVersion(dlg, "H002"); break;
  default:
  case 1: EBC_UserSpecialDialog_SetEbicsVersion(dlg, "H003"); break;
  }

  i=GWEN_Dialog_GetIntProperty(dlg, "signVersionCombo", GWEN_DialogProperty_Value, 0, -1);
  switch(i) {
  case 0: EBC_UserSpecialDialog_SetSignVersion(dlg, "A004"); break;
  default:
  case 1: EBC_UserSpecialDialog_SetSignVersion(dlg, "A005"); break;
  }

  i=GWEN_Dialog_GetIntProperty(dlg, "cryptVersionCombo", GWEN_DialogProperty_Value, 0, -1);
  switch(i) {
  case 0: EBC_UserSpecialDialog_SetCryptVersion(dlg, "E001"); break;
  default:
  case 1: EBC_UserSpecialDialog_SetCryptVersion(dlg, "E002"); break;
  }

  i=GWEN_Dialog_GetIntProperty(dlg, "authVersionCombo", GWEN_DialogProperty_Value, 0, -1);
  switch(i) {
  case 0: EBC_UserSpecialDialog_SetAuthVersion(dlg, "X001"); break;
  default:
  case 1: EBC_UserSpecialDialog_SetAuthVersion(dlg, "X002"); break;
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

  i=GWEN_Dialog_GetIntProperty(dlg, "signKeySizeCombo", GWEN_DialogProperty_Value, 0, -1);
  switch(i) {
  case 0: xdlg->signKeySize=128;  break;
  default:
  case 1: xdlg->signKeySize=256;  break;
  case 2: xdlg->signKeySize=512;  break;
  case 3: xdlg->signKeySize=1024; break;
  }

  i=GWEN_Dialog_GetIntProperty(dlg, "cryptAndAuthKeySizeCombo", GWEN_DialogProperty_Value, 0, -1);
  switch(i) {
  case 0: xdlg->cryptAndAuthKeySize=128;  break;
  default:
  case 1: xdlg->cryptAndAuthKeySize=256;  break;
  case 2: xdlg->cryptAndAuthKeySize=512;  break;
  case 3: xdlg->cryptAndAuthKeySize=1024; break;
  }

  flags=0;
  if (GWEN_Dialog_GetIntProperty(dlg, "forceSslv3Check", GWEN_DialogProperty_Value, 0, 0))
    flags|=EBC_USER_FLAGS_FORCE_SSLV3;
  if (GWEN_Dialog_GetIntProperty(dlg, "useIzlCheck", GWEN_DialogProperty_Value, 0, 0))
    flags|=EBC_USER_FLAGS_USE_IZL;
  if (GWEN_Dialog_GetIntProperty(dlg, "noEuCheck", GWEN_DialogProperty_Value, 0, 0))
    flags|=EBC_USER_FLAGS_NO_EU;
  xdlg->flags=flags;


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



int EBC_UserSpecialDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  DBG_ERROR(0, "Activated: %s", sender);
  if (strcasecmp(sender, "okButton")==0)
    return GWEN_DialogEvent_ResultAccept;
  else if (strcasecmp(sender, "abortButton")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "helpButton")==0) {
    /* TODO: open a help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB EBC_UserSpecialDialog_SignalHandler(GWEN_DIALOG *dlg,
                                                      GWEN_DIALOG_EVENTTYPE t,
                                                      const char *sender) {
  EBC_USER_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, EBC_USER_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    EBC_UserSpecialDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    EBC_UserSpecialDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeActivated:
    return EBC_UserSpecialDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}






