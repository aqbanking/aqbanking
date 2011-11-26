/***************************************************************************
 begin       : Wed Apr 14 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "dlg_selectbackend_p.h"
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



GWEN_INHERIT(GWEN_DIALOG, AB_SELECTBACKEND_DIALOG)




GWEN_DIALOG *AB_SelectBackendDialog_new(AB_BANKING *ab, const char *text) {
  GWEN_DIALOG *dlg;
  AB_SELECTBACKEND_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("ab_selectbackend");
  GWEN_NEW_OBJECT(AB_SELECTBACKEND_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AB_SELECTBACKEND_DIALOG, dlg, xdlg,
		       AB_SelectBackendDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, AB_SelectBackendDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
			       "aqbanking/dialogs/dlg_selectbackend.dlg",
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
  if (text)
    xdlg->text=strdup(text);

  /* done */
  return dlg;
}



void GWENHYWFAR_CB AB_SelectBackendDialog_FreeData(void *bp, void *p) {
  AB_SELECTBACKEND_DIALOG *xdlg;

  xdlg=(AB_SELECTBACKEND_DIALOG*) p;
  free(xdlg->selectedProvider);
  free(xdlg->text);
  GWEN_FREE_OBJECT(xdlg);
}



const char *AB_SelectBackendDialog_GetSelectedProvider(const GWEN_DIALOG *dlg) {
  AB_SELECTBACKEND_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SELECTBACKEND_DIALOG, dlg);
  assert(xdlg);

  return xdlg->selectedProvider;
}



void AB_SelectBackendDialog_SetSelectedProvider(GWEN_DIALOG *dlg, const char *s) {
  AB_SELECTBACKEND_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SELECTBACKEND_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->selectedProvider);
  if (s) xdlg->selectedProvider=strdup(s);
  else xdlg->selectedProvider=NULL;
}



void AB_SelectBackendDialog_DetermineBackend(GWEN_DIALOG *dlg) {
  int idx;
  AB_SELECTBACKEND_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SELECTBACKEND_DIALOG, dlg);
  assert(xdlg);

  AB_SelectBackendDialog_SetSelectedProvider(dlg, NULL);
  idx=GWEN_Dialog_GetIntProperty(dlg, "backendCombo", GWEN_DialogProperty_Value, 0, -1);
  if (idx>=0 && xdlg->pluginDescrList) {
    GWEN_PLUGIN_DESCRIPTION *d;

    d=GWEN_PluginDescription_List_First(xdlg->pluginDescrList);
    while(d && idx>0) {
      idx--;
      d=GWEN_PluginDescription_List_Next(d);
    }
    if (d)
      AB_SelectBackendDialog_SetSelectedProvider(dlg, GWEN_PluginDescription_GetName(d));
  }
}



int AB_SelectBackendDialog_BackendChanged(GWEN_DIALOG *dlg) {
  AB_SELECTBACKEND_DIALOG *xdlg;
  int idx;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SELECTBACKEND_DIALOG, dlg);
  assert(xdlg);

  idx=GWEN_Dialog_GetIntProperty(dlg, "backendCombo", GWEN_DialogProperty_Value, 0, -1);
  if (idx>=0 && xdlg->pluginDescrList) {
    GWEN_PLUGIN_DESCRIPTION *d;

    d=GWEN_PluginDescription_List_First(xdlg->pluginDescrList);
    while(d && idx>0) {
      idx--;
      d=GWEN_PluginDescription_List_Next(d);
    }
    if (d) {
      const char *s;

      s=GWEN_PluginDescription_GetLongDescr(d);
      GWEN_Dialog_SetCharProperty(dlg, "descrLabel", GWEN_DialogProperty_Title, 0, s, 0);
    }
  }

  return GWEN_DialogEvent_ResultHandled;
}



void AB_SelectBackendDialog_Reload(GWEN_DIALOG *dlg) {
  AB_SELECTBACKEND_DIALOG *xdlg;
  GWEN_PLUGIN_DESCRIPTION_LIST2 *ll;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SELECTBACKEND_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->selectedProvider==NULL)
    AB_SelectBackendDialog_DetermineBackend(dlg);

  GWEN_Dialog_SetIntProperty(dlg, "backendCombo", GWEN_DialogProperty_ClearValues, 0, 0, 0);
  if (xdlg->pluginDescrList)
    GWEN_PluginDescription_List_Clear(xdlg->pluginDescrList);
  else
    xdlg->pluginDescrList=GWEN_PluginDescription_List_new();

  ll=AB_Banking_GetProviderDescrs(xdlg->banking);
  if (ll) {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *it;

    it=GWEN_PluginDescription_List2_First(ll);
    if (it) {
      GWEN_PLUGIN_DESCRIPTION *d;
      GWEN_BUFFER *tbuf;
      int idx=-1;
      int i=0;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      d=GWEN_PluginDescription_List2Iterator_Data(it);
      while(d) {
	const char *s;
    
	s=GWEN_PluginDescription_GetName(d);
	if (s && *s) {
          GWEN_Buffer_AppendString(tbuf, s);
	  if (idx==-1 && xdlg->selectedProvider && strcasecmp(xdlg->selectedProvider, s)==0)
            idx=i;
	  s=GWEN_PluginDescription_GetShortDescr(d);
	  if (s && *s) {
	    GWEN_Buffer_AppendString(tbuf, " - ");
	    GWEN_Buffer_AppendString(tbuf, s);
	  }

	  GWEN_PluginDescription_Attach(d);
	  GWEN_PluginDescription_List_Add(d, xdlg->pluginDescrList);

	  GWEN_Dialog_SetCharProperty(dlg,
				      "backendCombo",
				      GWEN_DialogProperty_AddValue,
				      0,
				      GWEN_Buffer_GetStart(tbuf),
				      0);
	  GWEN_Buffer_Reset(tbuf);

	  i++;
	}

	d=GWEN_PluginDescription_List2Iterator_Next(it);
      }
      if (idx!=-1)
	GWEN_Dialog_SetIntProperty(dlg, "backendCombo", GWEN_DialogProperty_Value, 0, idx, 0);

      GWEN_Buffer_free(tbuf);
      GWEN_PluginDescription_List2Iterator_free(it);
    }

    GWEN_PluginDescription_List2_freeAll(ll);
  }

  AB_SelectBackendDialog_BackendChanged(dlg);
}



void AB_SelectBackendDialog_Init(GWEN_DIALOG *dlg) {
  AB_SELECTBACKEND_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SELECTBACKEND_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
			      "",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("Select Backend"),
			      0);

  if (xdlg->text && *(xdlg->text))
    GWEN_Dialog_SetCharProperty(dlg,
				"introLabel",
				GWEN_DialogProperty_Title,
				0,
                                xdlg->text,
				0);
  else
    GWEN_Dialog_SetCharProperty(dlg,
				"introLabel",
				GWEN_DialogProperty_Title,
				0,
				I18N("Select a backend."),
				0);

  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);

  AB_SelectBackendDialog_Reload(dlg);
}



void AB_SelectBackendDialog_Fini(GWEN_DIALOG *dlg) {
  AB_SELECTBACKEND_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SELECTBACKEND_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  AB_SelectBackendDialog_DetermineBackend(dlg);

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



int AB_SelectBackendDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  DBG_NOTICE(0, "Activated: %s", sender);
  if (strcasecmp(sender, "okButton")==0)
    return GWEN_DialogEvent_ResultAccept;
  else if (strcasecmp(sender, "abortButton")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "backendCombo")==0)
    return AB_SelectBackendDialog_BackendChanged(dlg);
  else if (strcasecmp(sender, "helpButton")==0) {
    /* TODO: open a help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB AB_SelectBackendDialog_SignalHandler(GWEN_DIALOG *dlg,
						       GWEN_DIALOG_EVENTTYPE t,
						       const char *sender) {
  AB_SELECTBACKEND_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SELECTBACKEND_DIALOG, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    AB_SelectBackendDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AB_SelectBackendDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeActivated:
    return AB_SelectBackendDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}



AB_PROVIDER *AB_SelectBackend(AB_BANKING *ab, const char *initial, const char *text) {
  GWEN_DIALOG *dlg;
  int rv;

  dlg=AB_SelectBackendDialog_new(ab, text);
  if (dlg==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not create dialog");
    return NULL;
  }

  AB_SelectBackendDialog_SetSelectedProvider(dlg, initial);

  rv=GWEN_Gui_ExecDialog(dlg, 0);
  if (rv==0) {
    /* rejected */
    GWEN_Dialog_free(dlg);
    return NULL;
  }
  else {
    const char *s;

    s=AB_SelectBackendDialog_GetSelectedProvider(dlg);
    if (s && *s) {
      AB_PROVIDER *pro;

      pro=AB_Banking_GetProvider(ab, s);
      if (pro==NULL) {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider [%s] not found", s);
	GWEN_Dialog_free(dlg);
	return NULL;
      }
      GWEN_Dialog_free(dlg);
      return pro;
    }
    GWEN_Dialog_free(dlg);
    return NULL;
  }
}





