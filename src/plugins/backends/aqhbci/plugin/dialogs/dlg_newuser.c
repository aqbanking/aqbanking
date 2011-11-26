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

#include "dlg_pintan_l.h"
#include "dlg_ddvcard_l.h"
#include "dlg_newkeyfile_l.h"

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
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
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
                              I18N("<html>"
                                   "<p>You can now create a new HBCI/FinTS user.</p>"
				   "AqBanking supports the following user types:"
				   "<ul>"
				   " <li>Keyfile-based user</li>"
				   " <li>Chipcard-based user</li>"
				   " <li>PIN/TAN user</li>"
				   "</ul>"
				   "<p>Which type of user you need to setup is determined "
				   "by the bank. The letter from your bank should contain this "
                                   "information.</p>"
                                   "</html>"
                                   "You can now create a new HBCI/FinTS user.\n"
                                   "AqBanking supports the following user types:\n"
				   " - Keyfile-based user\n"
				   " - Chipcard-based user\n"
				   " - PIN/TAN user\n"
				   "Which type of user you need to setup is determined\n"
				   "by the bank. The letter from your bank should contain this\n"
                                   "information."),
			      0);

  /* temporarily disable not-implemented buttons */
  GWEN_Dialog_SetIntProperty(dlg, "importKeyFileButton", GWEN_DialogProperty_Enabled, 0, 0, 0);
  GWEN_Dialog_SetIntProperty(dlg, "initChipcardButton", GWEN_DialogProperty_Enabled, 0, 0, 0);


  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);
}



void AH_NewUserDialog_Fini(GWEN_DIALOG *dlg) {
  int i;
  GWEN_DB_NODE *dbPrefs;

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



static int AH_NewUserDialog_HandleActivatedUseCard(GWEN_DIALOG *dlg) {
  int rv;
  GWEN_BUFFER *mtypeName;
  GWEN_BUFFER *mediumName;
  uint32_t pid;
  GWEN_CRYPT_TOKEN *ct;

  mtypeName=GWEN_Buffer_new(0, 64, 0, 1);
  mediumName=GWEN_Buffer_new(0, 64, 0, 1);

  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS |
			     GWEN_GUI_PROGRESS_SHOW_PROGRESS |
			     GWEN_GUI_PROGRESS_KEEP_OPEN |
			     GWEN_GUI_PROGRESS_SHOW_ABORT,
			     I18N("Checking Chipcard"),
			     I18N("Checking chipcard type, please wait..."),
			     GWEN_GUI_PROGRESS_NONE,
			     0);

  rv=AB_Banking_CheckCryptToken(AB_NewUserDialog_GetBanking(dlg),
				GWEN_Crypt_Token_Device_Card,
				mtypeName,
				mediumName);
  GWEN_Gui_ProgressEnd(pid);
  if (rv<0) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(mediumName);
    GWEN_Buffer_free(mtypeName);
    return GWEN_DialogEvent_ResultHandled;
  }

  rv=AB_Banking_GetCryptToken(AB_NewUserDialog_GetBanking(dlg),
			      GWEN_Buffer_GetStart(mtypeName),
			      GWEN_Buffer_GetStart(mediumName),
			      &ct);
  if (rv<0) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(mediumName);
    GWEN_Buffer_free(mtypeName);
    return GWEN_DialogEvent_ResultHandled;
  }

  if (strcasecmp(GWEN_Buffer_GetStart(mtypeName), "ddvcard")==0) {
    GWEN_DIALOG *dlg2;

    DBG_NOTICE(0, "DDV card");
    dlg2=AH_DdvCardDialog_new(AB_NewUserDialog_GetBanking(dlg), ct);
    if (dlg2==NULL) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (no dialog)");
      GWEN_Buffer_free(mediumName);
      GWEN_Buffer_free(mtypeName);
      return GWEN_DialogEvent_ResultHandled;
    }

    GWEN_Dialog_SetWidgetText(dlg2, "", I18N("Create HBCI/FinTS DDV User"));
  
    rv=GWEN_Gui_ExecDialog(dlg2, 0);
    if (rv==0) {
      /* rejected */
      GWEN_Dialog_free(dlg2);
      AB_Banking_ClearCryptTokenList(AB_NewUserDialog_GetBanking(dlg));
      return GWEN_DialogEvent_ResultHandled;
    }
    AB_NewUserDialog_SetUser(dlg, AH_PinTanDialog_GetUser(dlg2));
    GWEN_Dialog_free(dlg2);
    GWEN_Buffer_free(mediumName);
    GWEN_Buffer_free(mtypeName);
    AB_Banking_ClearCryptTokenList(AB_NewUserDialog_GetBanking(dlg));
    return GWEN_DialogEvent_ResultAccept;
  }
  else if (strcasecmp(GWEN_Buffer_GetStart(mtypeName), "starcoscard")==0) {
    DBG_NOTICE(0, "STARCOS RSA card");
    // TODO
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Card type \"%s\" not yet supported",
	      GWEN_Buffer_GetStart(mtypeName));
  }
  GWEN_Buffer_free(mediumName);
  GWEN_Buffer_free(mtypeName);
  AB_Banking_ClearCryptTokenList(AB_NewUserDialog_GetBanking(dlg));

  return GWEN_DialogEvent_ResultHandled;
}



static int AH_NewUserDialog_HandleActivatedNewKeyFile(GWEN_DIALOG *dlg) {
  GWEN_DIALOG *dlg2;
  int rv;

  dlg2=AH_NewKeyFileDialog_new(AB_NewUserDialog_GetBanking(dlg));
  if (dlg2==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (no dialog)");
    return GWEN_DialogEvent_ResultHandled;
  }

  GWEN_Dialog_SetWidgetText(dlg2, "", I18N("Create HBCI/FinTS Keyfile User"));

  rv=GWEN_Gui_ExecDialog(dlg2, 0);
  if (rv==0) {
    /* rejected */
    GWEN_Dialog_free(dlg2);
    return GWEN_DialogEvent_ResultHandled;
  }
  AB_NewUserDialog_SetUser(dlg, AH_NewKeyFileDialog_GetUser(dlg2));
  GWEN_Dialog_free(dlg2);
  return GWEN_DialogEvent_ResultAccept;
}



int AH_NewUserDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  if (strcasecmp(sender, "abortButton")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "usePinTanButton")==0)
    return AH_NewUserDialog_HandleActivatedPinTan(dlg);
  else if (strcasecmp(sender, "useChipcardButton")==0)
    return AH_NewUserDialog_HandleActivatedUseCard(dlg);
  else if (strcasecmp(sender, "createKeyFileButton")==0)
    return AH_NewUserDialog_HandleActivatedNewKeyFile(dlg);
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






