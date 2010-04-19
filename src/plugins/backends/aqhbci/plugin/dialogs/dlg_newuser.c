/***************************************************************************
 begin       : Mon Apr 19 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_newuser_p.h"
#include "i18n_l.h"

#include <aqhbci/dlg_pintan.h>

#include <aqbanking/user.h>
#include <aqbanking/banking_be.h>
#include <aqbanking/dlg_newuser_be.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>


#define DIALOG_MINWIDTH  200
#define DIALOG_MINHEIGHT 200




GWEN_DIALOG *AH_NewUserDialog_new(AB_BANKING *ab) {
  GWEN_DIALOG *dlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=AB_NewUserDialog_new(ab, "ah_new_user");
  GWEN_Dialog_SetSignalHandler(dlg, AH_NewUserDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(GWEN_PM_LIBNAME, GWEN_PM_SYSDATADIR,
			       "aqbanking/backends/aqhbci/dialogs/dlg_newuser.dlg",
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



void AH_NewUserDialog_Init(GWEN_DIALOG *dlg) {
  GWEN_DB_NODE *dbPrefs;
  int i;

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
			      "",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("Create a New User"),
			      0);

  GWEN_Dialog_SetCharProperty(dlg,
			      "introLabel",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("<p>You can now create a new HBCI/FinTS user.</p>"
				   "AqBanking supports the following user types:"
				   "<ul>"
				   " <li>Keyfile-based user</li>"
				   " <li>Chipcard-based user</li>"
				   " <li>PIN/TAN user</li>"
				   "</ul>"
				   "<p>Which type of user you need to setup is determined "
				   "by the bank. The letter from your bank should contain this "
				   "information.</p>"),
			      0);

  /* temporarily disable not-implemented buttons */
  GWEN_Dialog_SetIntProperty(dlg, "createKeyFileButton", GWEN_DialogProperty_Enabled, 0, 0, 0);
  GWEN_Dialog_SetIntProperty(dlg, "importKeyFileButton", GWEN_DialogProperty_Enabled, 0, 0, 0);
  GWEN_Dialog_SetIntProperty(dlg, "initChipcardButton", GWEN_DialogProperty_Enabled, 0, 0, 0);
  GWEN_Dialog_SetIntProperty(dlg, "useChipcardButton", GWEN_DialogProperty_Enabled, 0, 0, 0);


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
}



void AH_NewUserDialog_Fini(GWEN_DIALOG *dlg) {
  int i;
  GWEN_DB_NODE *dbPrefs;

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
}



static int AH_NewUserDialog_HandleActivatedPinTan(GWEN_DIALOG *dlg) {
  GWEN_DIALOG *dlg2;
  int rv;

  dlg2=AH_PinTanDialog_new(AB_NewUserDialog_GetBanking(dlg));
  if (dlg2==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (no dialog)");
    return GWEN_DialogEvent_ResultHandled;
  }

  GWEN_Dialog_SetWidgetText(dlg2, "", I18N("Create HBCI/FinTS PIN/TAN User"));

  rv=GWEN_Gui_ExecDialog(dlg2, 0);
  if (rv==0) {
    /* rejected */
    GWEN_Dialog_free(dlg2);
    return GWEN_DialogEvent_ResultHandled;
  }
  AB_NewUserDialog_SetUser(dlg, AH_PinTanDialog_GetUser(dlg2));
  GWEN_Dialog_free(dlg2);
  return GWEN_DialogEvent_ResultAccept;
}



int AH_NewUserDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  if (strcasecmp(sender, "abortButton")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "usePinTanButton")==0)
    return AH_NewUserDialog_HandleActivatedPinTan(dlg);
  else if (strcasecmp(sender, "helpButton")==0) {
    /* TODO: open u help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB AH_NewUserDialog_SignalHandler(GWEN_DIALOG *dlg,
						 GWEN_DIALOG_EVENTTYPE t,
						 const char *sender) {
  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    AH_NewUserDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AH_NewUserDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeActivated:
    return AH_NewUserDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}






