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

#include "dlg_setup_newuser_p.h"
#include "dlg_usertype_pagedefault_l.h"
#include "dlg_usertype_page_be.h"
#include "i18n_l.h"

#include <aqbanking/banking_be.h>
#include <aqbanking/provider_be.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <assert.h>



#define PAGE_BEGIN     0
#define PAGE_BACKEND   1

#define DIALOG_MINWIDTH  400
#define DIALOG_MINHEIGHT 400



GWEN_INHERIT(GWEN_DIALOG, AB_SETUP_NEWUSER_DIALOG)






GWEN_DIALOG *AB_SetupNewUserDialog_new(AB_BANKING *ab) {
  GWEN_DIALOG *dlg;
  AB_SETUP_NEWUSER_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;
  GWEN_XMLNODE *n;
  GWEN_XMLNODE *nDialog;
  GWEN_PLUGIN_DESCRIPTION_LIST2 *providers;
  GWEN_DIALOG_LIST *subDialogs=NULL;

  dlg=GWEN_Dialog_new("ab_setup_newuser");
  GWEN_NEW_OBJECT(AB_SETUP_NEWUSER_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AB_SETUP_NEWUSER_DIALOG, dlg, xdlg,
		       AB_SetupNewUserDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, AB_SetupNewUserDialog_SignalHandler);

  xdlg->backendDialogs=GWEN_Dialog_List2_new();
  xdlg->backendRadioNames=GWEN_StringList_new();
  xdlg->backendNames=GWEN_StringList_new();

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
			       "aqbanking/dialogs/dlg_setup_newuser.dlg",
			       fbuf);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Dialog description file not found (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }

  /* read XML file, extend it */
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  rv=GWEN_XML_ReadFile(n, GWEN_Buffer_GetStart(fbuf),
		       GWEN_XML_FLAGS_DEFAULT |
		       GWEN_XML_FLAGS_HANDLE_HEADERS);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_XMLNode_free(n);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }

  nDialog=GWEN_XMLNode_FindFirstTag(n, "dialog", NULL, NULL);
  if (nDialog==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Dialog element not found in XML file [%s]", GWEN_Buffer_GetStart(fbuf));
    GWEN_XMLNode_free(n);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }

  providers=AB_Banking_GetProviderDescrs(ab);
  if (providers) {
    GWEN_XMLNODE *nBackendRadioWidget;

    nBackendRadioWidget=GWEN_XMLNode_FindFirstTag(nDialog, "widget", "name", "dialogVLayout");
    if (nBackendRadioWidget)
      nBackendRadioWidget=GWEN_XMLNode_FindFirstTag(nBackendRadioWidget, "widget", "name", "wiz_stack");
    if (nBackendRadioWidget)
      nBackendRadioWidget=GWEN_XMLNode_FindFirstTag(nBackendRadioWidget, "widget", "name", "backendRadioWidget");
    subDialogs=GWEN_Dialog_List_new();
    if (nBackendRadioWidget) {
      GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *pit;
  
      pit=GWEN_PluginDescription_List2_First(providers);
      if (pit) {
	GWEN_PLUGIN_DESCRIPTION *pd;
  
	pd=GWEN_PluginDescription_List2Iterator_Data(pit);
	while(pd) {
	  const char *name;
  
	  name=GWEN_PluginDescription_GetName(pd);
	  if (name && *name) {
	    AB_PROVIDER *pro;

	    pro=AB_Banking_GetProvider(ab, name);
	    if (pro) {
	      GWEN_DIALOG *cdlg;
	      GWEN_BUFFER *tbuf;
	      GWEN_XMLNODE *wn;
	      const char *s;

	      if (AB_Provider_GetFlags(pro) & AB_PROVIDER_FLAGS_HAS_USERTYPE_DIALOG) {
		cdlg=AB_ProviderGetUserTypeDialog(pro);
		if (cdlg==NULL) {
		  DBG_WARN(AQBANKING_LOGDOMAIN, "Backend [%s] does not return a userType dialog, using default", name);
		  cdlg=AB_UserTypePageDefaultDialog_new(ab);
		}
	      }
	      else
		cdlg=AB_UserTypePageDefaultDialog_new(ab);

	      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
	      GWEN_Buffer_AppendString(tbuf, "backend_");
	      GWEN_Buffer_AppendString(tbuf, name);
	      GWEN_Buffer_AppendString(tbuf, "_radio");
    
	      wn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "widget");
	      GWEN_XMLNode_SetProperty(wn, "type", "radioButton");
	      GWEN_XMLNode_SetProperty(wn, "name", GWEN_Buffer_GetStart(tbuf));
	      GWEN_StringList_AppendString(xdlg->backendRadioNames, GWEN_Buffer_GetStart(tbuf), 0, 0);
	      GWEN_StringList_AppendString(xdlg->backendNames, name, 0, 0);
    
	      s=GWEN_PluginDescription_GetShortDescr(pd);
	      if (!(s && *s))
		s=name;
	      GWEN_XMLNode_SetProperty(wn, "text", I18N(s));
    
	      GWEN_XMLNode_SetProperty(wn, "groupId", "999999");
	      GWEN_XMLNode_SetProperty(wn, "flags", "fillX justifyLeft");
    
	      GWEN_XMLNode_AddChild(nBackendRadioWidget, wn);
	      GWEN_Dialog_List_Add(cdlg, subDialogs);
	    }
	    else {
	      DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not activate backend [%s]", name);
	    }
	  }
  
	  pd=GWEN_PluginDescription_List2Iterator_Next(pit);
	}
	GWEN_PluginDescription_List2Iterator_free(pit);
      }
    }
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Dialog description does not contain path [dialogVLayout/wiz_stack/backendRadioWidget]");
      GWEN_XMLNode_Dump(nDialog, 2);
      GWEN_XMLNode_free(n);
      GWEN_Buffer_free(fbuf);
      GWEN_Dialog_free(dlg);
      return NULL;
    }
  }

  /* read dialog from dialog description file */
  rv=GWEN_Dialog_ReadXml(dlg, nDialog);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d).", rv);
    GWEN_Dialog_List_free(subDialogs);
    GWEN_XMLNode_free(n);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }
  GWEN_XMLNode_free(n);
  GWEN_Buffer_free(fbuf);

  /* insert pages for each backend */
  if (GWEN_Dialog_List_GetCount(subDialogs)) {
    GWEN_DIALOG *cdlg;

    while ( (cdlg=GWEN_Dialog_List_First(subDialogs)) ) {
      GWEN_Dialog_List_Del(cdlg);
      DBG_NOTICE(AQBANKING_LOGDOMAIN, "Adding dialog %s", GWEN_Dialog_GetId(cdlg));
      rv=GWEN_Dialog_AddSubDialog(dlg, "wiz_stack", cdlg);
      if (rv<0) {
	DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
        GWEN_Dialog_List_free(subDialogs);
	GWEN_Dialog_free(cdlg);
	GWEN_Dialog_free(dlg);
	return NULL;
      }
      GWEN_Dialog_List2_PushBack(xdlg->backendDialogs, cdlg);
    }
  }
  GWEN_Dialog_List_free(subDialogs);

  xdlg->banking=ab;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB AB_SetupNewUserDialog_FreeData(void *bp, void *p) {
  AB_SETUP_NEWUSER_DIALOG *xdlg;

  xdlg=(AB_SETUP_NEWUSER_DIALOG*) p;
  GWEN_Dialog_List2_free(xdlg->backendDialogs); /* don't use _freeAll here! */
  GWEN_StringList_free(xdlg->backendRadioNames);
  GWEN_StringList_free(xdlg->backendNames);

  GWEN_FREE_OBJECT(xdlg);
}



void AB_SetupNewUserDialog_Init(GWEN_DIALOG *dlg) {
  AB_SETUP_NEWUSER_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
			      "",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("New User Wizard"),
			      0);

  /* select first page */
  GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, 0, 0);

  /* setup intro page */
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_begin_label",
			      GWEN_DialogProperty_Title,
			      0,
                              I18N("<html>"
                                   "<p>This dialog assists you in creating an online banking user."
				   "The following steps are:</p>"
                                   "<ul>"
				   "<li>select the banking protocol</li>"
				   "<li>select the type of user to create</li>"
                                   "</ul>"
                                   "</html>"
                                   "This dialog assists you in creating an online banking user.\n"
                                   "The following steps are:\n"
                                   " - select the banking protocol\n"
				   " - select the type of user to create\n"),
			      0);

  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
      GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
      GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);

  /* disable next and previous buttons */
  GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
  GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
}



void AB_SetupNewUserDialog_Fini(GWEN_DIALOG *dlg) {
  AB_SETUP_NEWUSER_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  xdlg->selectedType=0;
  free(xdlg->selectedBackend);
  xdlg->selectedBackend=NULL;
  i=AB_SetupNewUserDialog_DetermineBackendIndex(dlg);
  if (i>=0) {
    const char *s;
    GWEN_DIALOG_LIST2_ITERATOR *it;

    s=GWEN_StringList_StringAt(xdlg->backendNames, i);
    if (s && *s)
      xdlg->selectedBackend=strdup(s);

    it=GWEN_Dialog_List2_First(xdlg->backendDialogs);
    if (it) {
      int j=i;
      GWEN_DIALOG *cdlg;

      cdlg=GWEN_Dialog_List2Iterator_Data(it);
      while(cdlg && j>0) {
        j--;
	cdlg=GWEN_Dialog_List2Iterator_Next(it);
      }
      GWEN_Dialog_List2Iterator_free(it);

      if (cdlg)
	xdlg->selectedType=AB_UserTypePageDialog_GetSelectedType(cdlg);
    }
  }

  /* save dialog settings */
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



int AB_SetupNewUserDialog_EnterPage(GWEN_DIALOG *dlg, int page, int forwards) {
  AB_SETUP_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  switch(page) {
  case PAGE_BEGIN:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_BACKEND:
    GWEN_Dialog_SetCharProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Title, 0, I18N("Next"), 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  default:
    if (forwards){
      DBG_NOTICE(0, "About to enter page %d", page);
      GWEN_Dialog_SetCharProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Title, 0, I18N("Run"), 0);
      GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
      return GWEN_DialogEvent_ResultHandled;
    }
    return GWEN_DialogEvent_ResultHandled;
  }

  return GWEN_DialogEvent_ResultHandled;
}



int AB_SetupNewUserDialog_DetermineBackendIndex(GWEN_DIALOG *dlg) {
  AB_SETUP_NEWUSER_DIALOG *xdlg;
  GWEN_STRINGLISTENTRY *se;
  int i=0;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  se=GWEN_StringList_FirstEntry(xdlg->backendRadioNames);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    if (GWEN_Dialog_GetIntProperty(dlg, s, GWEN_DialogProperty_Value, 0, 0))
      break;
    i++;
    se=GWEN_StringListEntry_Next(se);
  }

  if (se)
    return i;

  return -1;
}



int AB_SetupNewUserDialog_Next(GWEN_DIALOG *dlg) {
  AB_SETUP_NEWUSER_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  DBG_NOTICE(0, "Value of wiz_stack: %d", page);

  if (page==PAGE_BEGIN) {
    DBG_NOTICE(0, "First page");
    page++;
    return AB_SetupNewUserDialog_EnterPage(dlg, page, 1);
  }
  if (page==PAGE_BACKEND) {
    int idx;

    DBG_NOTICE(0, "Backend page");
    idx=AB_SetupNewUserDialog_DetermineBackendIndex(dlg);
    if (idx!=-1) {
      page=idx+PAGE_BACKEND+1;
      return AB_SetupNewUserDialog_EnterPage(dlg, page, 1);
    }
  }
  else if (page>PAGE_BACKEND)
    return GWEN_DialogEvent_ResultAccept;

  return GWEN_DialogEvent_ResultHandled;
}



int AB_SetupNewUserDialog_Previous(GWEN_DIALOG *dlg) {
  AB_SETUP_NEWUSER_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  if (page>PAGE_BACKEND) {
    page=PAGE_BACKEND;
    return AB_SetupNewUserDialog_EnterPage(dlg, page, 0);
  }
  if (page==PAGE_BACKEND) {
    page=PAGE_BEGIN;
    return AB_SetupNewUserDialog_EnterPage(dlg, page, 0);
  }

  return GWEN_DialogEvent_ResultHandled;
}



int AB_SetupNewUserDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  DBG_NOTICE(0, "Activated: %s", sender);
  if (strcasecmp(sender, "wiz_prev_button")==0)
    return AB_SetupNewUserDialog_Previous(dlg);
  else if (strcasecmp(sender, "wiz_next_button")==0)
    return AB_SetupNewUserDialog_Next(dlg);
  else if (strcasecmp(sender, "wiz_abort_button")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "wiz_help_button")==0) {
    /* TODO: open a help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB AB_SetupNewUserDialog_SignalHandler(GWEN_DIALOG *dlg,
						      GWEN_DIALOG_EVENTTYPE t,
						      const char *sender) {
  AB_SETUP_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    AB_SetupNewUserDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AB_SetupNewUserDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeActivated:
    return AB_SetupNewUserDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeValueChanged:
  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}



const char *AB_SetupNewUserDialog_GetSelectedBackend(const GWEN_DIALOG *dlg) {
  AB_SETUP_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->selectedBackend;
}



int AB_SetupNewUserDialog_GetSelectedType(const GWEN_DIALOG *dlg) {
  AB_SETUP_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SETUP_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->selectedType;
}






