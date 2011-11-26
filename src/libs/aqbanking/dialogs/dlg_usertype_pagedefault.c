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



#include "dlg_usertype_pagedefault_p.h"
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




GWEN_DIALOG *AB_UserTypePageDefaultDialog_new(AB_BANKING *ab) {
  GWEN_DIALOG *dlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=AB_UserTypePageDialog_new(ab, "ab_usertype_pagedefault");
  GWEN_Dialog_SetSignalHandler(dlg, AB_UserTypePageDefaultDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
			       "aqbanking/dialogs/dlg_usertype_pagedefault.dlg",
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
					   "aqbanking/dialogs/dialogs");

  /* done */
  return dlg;
}



void AB_UserTypePageDefaultDialog_Init(GWEN_DIALOG *dlg) {
  GWEN_Dialog_SetCharProperty(dlg, "defaultIntroLabel", GWEN_DialogProperty_Title, 0,
                              I18N("<html>"
                                   "<p>Click on the <i>run</i> button below to create the user.</p>"
                                   "</html>"
                                   "Click on the RUN button below to create the user."),
                              0);
}



void AB_UserTypePageDefaultDialog_Fini(GWEN_DIALOG *dlg) {
  DBG_NOTICE(AQBANKING_LOGDOMAIN, "fini called");
  AB_UserTypePageDialog_SetSelectedType(dlg, 0);
}



int AB_UserTypePageDefaultDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  /* nothing for now */
  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB AB_UserTypePageDefaultDialog_SignalHandler(GWEN_DIALOG *dlg,
							     GWEN_DIALOG_EVENTTYPE t,
							     const char *sender) {
  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    AB_UserTypePageDefaultDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AB_UserTypePageDefaultDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeActivated:
    return AB_UserTypePageDefaultDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}






