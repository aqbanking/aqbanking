/***************************************************************************
 begin       : Thu Aug 19 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_ofx_special_p.h"
#include "i18n_l.h"

#include <aqbanking/user.h>
#include <aqbanking/banking_be.h>

#include <aqofxconnect/user.h>
#include <aqofxconnect/provider.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>


#define DIALOG_MINWIDTH  200
#define DIALOG_MINHEIGHT 100



GWEN_INHERIT(GWEN_DIALOG, AO_OFX_SPECIAL_DIALOG)




GWEN_DIALOG *AO_OfxSpecialDialog_new(AB_BANKING *ab) {
  GWEN_DIALOG *dlg;
  AO_OFX_SPECIAL_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("ao_ofx_special");
  GWEN_NEW_OBJECT(AO_OFX_SPECIAL_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AO_OFX_SPECIAL_DIALOG, dlg, xdlg,
		       AO_OfxSpecialDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, AO_OfxSpecialDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
                               "aqbanking/backends/aqofxconnect/dialogs/dlg_ofx_special.dlg",
                               fbuf);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Dialog description file not found (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }

  /* read dialog from dialog description file */
  rv=GWEN_Dialog_ReadXmlFile(dlg, GWEN_Buffer_GetStart(fbuf));
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d).", rv);
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



void GWENHYWFAR_CB AO_OfxSpecialDialog_FreeData(void *bp, void *p) {
  AO_OFX_SPECIAL_DIALOG *xdlg;

  xdlg=(AO_OFX_SPECIAL_DIALOG*) p;
  free(xdlg->clientUid);
  free(xdlg->securityType);

  GWEN_FREE_OBJECT(xdlg);
}



int AO_OfxSpecialDialog_GetHttpVMajor(const GWEN_DIALOG *dlg) {
  AO_OFX_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_OFX_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  return xdlg->httpVMajor;
}



int AO_OfxSpecialDialog_GetHttpVMinor(const GWEN_DIALOG *dlg) {
  AO_OFX_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_OFX_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  return xdlg->httpVMinor;
}



void AO_OfxSpecialDialog_SetHttpVersion(GWEN_DIALOG *dlg, int vmajor, int vminor) {
  AO_OFX_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_OFX_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  xdlg->httpVMajor=vmajor;
  xdlg->httpVMinor=vminor;
}



uint32_t AO_OfxSpecialDialog_GetFlags(const GWEN_DIALOG *dlg) {
  AO_OFX_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_OFX_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  return xdlg->flags;
}



void AO_OfxSpecialDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AO_OFX_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_OFX_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags=fl;
}



void AO_OfxSpecialDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AO_OFX_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_OFX_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}



void AO_OfxSpecialDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl) {
  AO_OFX_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_OFX_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  xdlg->flags&=~fl;
}



void AO_OfxSpecialDialog_SetSecurityType(GWEN_DIALOG *dlg, const char *s) {
  AO_OFX_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_OFX_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->securityType);
  if (s) xdlg->securityType=strdup(s);
  else xdlg->securityType=NULL;
}



const char *AO_OfxSpecialDialog_GetSecurityType(const GWEN_DIALOG *dlg) {
  AO_OFX_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_OFX_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  return xdlg->securityType;
}



void AO_OfxSpecialDialog_SetClientUid(GWEN_DIALOG *dlg, const char *s) {
  AO_OFX_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_OFX_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->clientUid);
  if (s) xdlg->clientUid=strdup(s);
  else xdlg->clientUid=NULL;
}



const char *AO_OfxSpecialDialog_GetClientUid(const GWEN_DIALOG *dlg) {
  AO_OFX_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_OFX_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  return xdlg->clientUid;
}



void AO_OfxSpecialDialog_Init(GWEN_DIALOG *dlg) {
  AO_OFX_SPECIAL_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_OFX_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
			      "",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("OFX DirectConnect Special Settings"),
			      0);

  GWEN_Dialog_SetCharProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_AddValue, 0, "1.0", 0);
  GWEN_Dialog_SetCharProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_AddValue, 0, "1.1", 0);

  /* toGui */
  switch(((xdlg->httpVMajor)<<8)+xdlg->httpVMinor) {
  case 0x0100: GWEN_Dialog_SetIntProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_Value, 0, 0, 0); break;
  case 0x0101: GWEN_Dialog_SetIntProperty(dlg, "httpVersionCombo", GWEN_DialogProperty_Value, 0, 1, 0); break;
  default:     break;
  }

  GWEN_Dialog_SetIntProperty(dlg, "forceSslv3Check", GWEN_DialogProperty_Value, 0,
			     (xdlg->flags & AO_USER_FLAGS_FORCE_SSL3)?1:0,
			     0);

  GWEN_Dialog_SetIntProperty(dlg, "emptyBankIdCheck", GWEN_DialogProperty_Value, 0,
			     (xdlg->flags & AO_USER_FLAGS_EMPTY_BANKID)?1:0,
			     0);

  GWEN_Dialog_SetIntProperty(dlg, "emptyFidCheck", GWEN_DialogProperty_Value, 0,
                             (xdlg->flags & AO_USER_FLAGS_EMPTY_FID)?1:0,
			     0);

  GWEN_Dialog_SetIntProperty(dlg, "shortDateCheck", GWEN_DialogProperty_Value, 0,
                             (xdlg->flags & AO_USER_FLAGS_SEND_SHORT_DATE)?1:0,
			     0);
  if (xdlg->clientUid)
    GWEN_Dialog_SetCharProperty(dlg, "clientUidEdit", GWEN_DialogProperty_Value, 0,
                                xdlg->clientUid,
                                0);

  if (xdlg->securityType)
    GWEN_Dialog_SetCharProperty(dlg, "securityTypeEdit", GWEN_DialogProperty_Value, 0,
                                xdlg->securityType,
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



void AO_OfxSpecialDialog_Fini(GWEN_DIALOG *dlg) {
  AO_OFX_SPECIAL_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;
  uint32_t flags;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_OFX_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  /* fromGui */
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
    flags|=AO_USER_FLAGS_FORCE_SSL3;
  if (GWEN_Dialog_GetIntProperty(dlg, "emptyBankIdCheck", GWEN_DialogProperty_Value, 0, 0))
    flags|=AO_USER_FLAGS_EMPTY_BANKID;
  if (GWEN_Dialog_GetIntProperty(dlg, "emptyFidCheck", GWEN_DialogProperty_Value, 0, 0))
    flags|=AO_USER_FLAGS_EMPTY_FID;
  if (GWEN_Dialog_GetIntProperty(dlg, "shortDateCheck", GWEN_DialogProperty_Value, 0, 0))
    flags|=AO_USER_FLAGS_SEND_SHORT_DATE;
  xdlg->flags=flags;

  s=GWEN_Dialog_GetCharProperty(dlg, "clientUidEdit", GWEN_DialogProperty_Value, 0, NULL);
  AO_OfxSpecialDialog_SetClientUid(dlg, s);

  s=GWEN_Dialog_GetCharProperty(dlg, "securityTypeEdit", GWEN_DialogProperty_Value, 0, NULL);
  AO_OfxSpecialDialog_SetSecurityType(dlg, s);

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



int AO_OfxSpecialDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
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



int GWENHYWFAR_CB AO_OfxSpecialDialog_SignalHandler(GWEN_DIALOG *dlg,
                                                    GWEN_DIALOG_EVENTTYPE t,
                                                    const char *sender) {
  AO_OFX_SPECIAL_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AO_OFX_SPECIAL_DIALOG, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    AO_OfxSpecialDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AO_OfxSpecialDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeActivated:
    return AO_OfxSpecialDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}






