/***************************************************************************
 begin       : Fri Jul 30 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_choose_usertype_p.h"
#include "i18n_l.h"

#include <aqhbci/provider.h>

#include <aqbanking/user.h>
#include <aqbanking/banking_be.h>
#include <aqbanking/dlg_usertype_page_be.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>




GWEN_DIALOG *AH_ChooseUserTypeDialog_new(AB_BANKING *ab) {
  GWEN_DIALOG *dlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=AB_UserTypePageDialog_new(ab, "ah_choose_usertype");
  GWEN_Dialog_SetSignalHandler(dlg, AH_ChooseUserTypeDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
			       "aqbanking/backends/aqhbci/dialogs/dlg_choose_usertype.dlg",
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

  /* add media paths for icons */
  GWEN_Dialog_AddMediaPathsFromPathManager(dlg,
					   GWEN_PM_LIBNAME,
					   GWEN_PM_SYSDATADIR,
					   "aqbanking/backends/aqhbci/dialogs");

  /* done */
  return dlg;
}



void AH_ChooseUserTypeDialog_Init(GWEN_DIALOG *dlg) {
  GWEN_Dialog_SetCharProperty(dlg, "hbciIntroLabel", GWEN_DialogProperty_Title, 0,
                              I18N("<html>"
                                   "<p>The HBCI module supports a broad range of security "
				   "media. Please choose the user setup mode from the following "
                                   "list.</p>"
                                   "<p>Click on the <i>run</i> button below to create the user.</p>"
                                   "</html>"
                                   "The HBCI module supports a broad range of security\n"
                                   "media. Please choose the user setup mode from the following\n"
                                   "list.\n"
                                   "Click on the RUN button below to create the user."
                                  ),
			      0);

  switch(AB_UserTypePageDialog_GetSelectedType(dlg)) {
  case AqHBCI_NewUserDialog_CodeGeneric:
  case AqHBCI_NewUserDialog_CodeExistingPinTan:
    GWEN_Dialog_SetIntProperty(dlg, "hbciPinTanRadio", GWEN_DialogProperty_Value, 0, 1, 0);
    break;

  case AqHBCI_NewUserDialog_CodeCreateKeyFile:
    GWEN_Dialog_SetIntProperty(dlg, "hbciCreateKeyFileRadio", GWEN_DialogProperty_Value, 0, 1, 0);
    break;
  case AqHBCI_NewUserDialog_CodeExistingKeyFile:
    GWEN_Dialog_SetIntProperty(dlg, "hbciImportKeyFileRadio", GWEN_DialogProperty_Value, 0, 1, 0);
    break;
  case AqHBCI_NewUserDialog_CodeCreateChipcard:
    GWEN_Dialog_SetIntProperty(dlg, "hbciInitChipcardRadio", GWEN_DialogProperty_Value, 0, 1, 0);
    break;
  case AqHBCI_NewUserDialog_CodeExistingChipcard:
    GWEN_Dialog_SetIntProperty(dlg, "hbciUseChipcardRadio", GWEN_DialogProperty_Value, 0, 1, 0);
    break;
  }

  /* temporarily disable not-implemented buttons */
  GWEN_Dialog_SetIntProperty(dlg, "hbciInitChipcardRadio", GWEN_DialogProperty_Enabled, 0, 0, 0);
}



void AH_ChooseUserTypeDialog_Fini(GWEN_DIALOG *dlg) {
  int i;

  if (GWEN_Dialog_GetIntProperty(dlg, "hbciPinTanRadio", GWEN_DialogProperty_Value, 0, 0)!=0)
    i=AqHBCI_NewUserDialog_CodeExistingPinTan;
  else if (GWEN_Dialog_GetIntProperty(dlg, "hbciUseChipcardRadio", GWEN_DialogProperty_Value, 0, 0)!=0)
    i=AqHBCI_NewUserDialog_CodeExistingChipcard;
  else if (GWEN_Dialog_GetIntProperty(dlg, "hbciCreateKeyFileRadio", GWEN_DialogProperty_Value, 0, 0)!=0)
    i=AqHBCI_NewUserDialog_CodeCreateKeyFile;
  else if (GWEN_Dialog_GetIntProperty(dlg, "hbciImportKeyFileRadio", GWEN_DialogProperty_Value, 0, 0)!=0)
    i=AqHBCI_NewUserDialog_CodeExistingKeyFile;
  else if (GWEN_Dialog_GetIntProperty(dlg, "hbciInitChipcardRadio", GWEN_DialogProperty_Value, 0, 0)!=0)
    i=AqHBCI_NewUserDialog_CodeCreateChipcard;
  else
    i=AqHBCI_NewUserDialog_CodeGeneric;
  DBG_NOTICE(0, "Setting selected type to %d", i);
  AB_UserTypePageDialog_SetSelectedType(dlg, i);
}



int AH_ChooseUserTypeDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  /* nothing for now */
  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB AH_ChooseUserTypeDialog_SignalHandler(GWEN_DIALOG *dlg,
							GWEN_DIALOG_EVENTTYPE t,
							const char *sender) {
  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    AH_ChooseUserTypeDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AH_ChooseUserTypeDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeActivated:
    return AH_ChooseUserTypeDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}






