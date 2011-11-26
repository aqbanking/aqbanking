/***************************************************************************
 begin       : Wed Aug 18 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "dlg_getinst_p.h"
#include "i18n_l.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>
#include <aqbanking/banking.h>


#define DIALOG_MINWIDTH  400
#define DIALOG_MINHEIGHT 200
#define LIST_MINCOLWIDTH 50




GWEN_INHERIT(GWEN_DIALOG, OH_GETINST_DIALOG)





GWEN_DIALOG *OH_GetInstituteDialog_new(const char *dataFolder, const char *name) {
  GWEN_DIALOG *dlg;
  OH_GETINST_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("oh_getinst");
  GWEN_NEW_OBJECT(OH_GETINST_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, OH_GETINST_DIALOG, dlg, xdlg,
		       OH_GetInstituteDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, OH_GetInstituteDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
			       "aqbanking/backends/aqofxconnect/dialogs/dlg_getinst.dlg",
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

  xdlg->ofxHome=OfxHome_new(dataFolder);
  xdlg->matchingSpecList=OH_InstituteSpec_List_new();

  if (name)
    xdlg->name=strdup(name);

  /* done */
  return dlg;
}



void GWENHYWFAR_CB OH_GetInstituteDialog_FreeData(void *bp, void *p) {
  OH_GETINST_DIALOG *xdlg;

  xdlg=(OH_GETINST_DIALOG*) p;
  OH_InstituteSpec_List_free(xdlg->matchingSpecList);
  OH_InstituteData_free(xdlg->selectedData);
  free(xdlg->name);
  OfxHome_free(xdlg->ofxHome);
  GWEN_FREE_OBJECT(xdlg);
}



static void createListBoxString(const OH_INSTITUTE_SPEC *os, GWEN_BUFFER *tbuf) {
  const char *s;
  char numbuf[32];

  s=OH_InstituteSpec_GetName(os);
  if (s && *s) {
    GWEN_Buffer_AppendString(tbuf, s);
    snprintf(numbuf, sizeof(numbuf)-1, " (%d)", OH_InstituteSpec_GetId(os));
    numbuf[sizeof(numbuf)-1]=0;
    GWEN_Buffer_AppendString(tbuf, numbuf);
  }
  else {
    snprintf(numbuf, sizeof(numbuf)-1, "%d", OH_InstituteSpec_GetId(os));
    numbuf[sizeof(numbuf)-1]=0;
    GWEN_Buffer_AppendString(tbuf, numbuf);
  }
}



const OH_INSTITUTE_DATA *OH_GetInstituteDialog_GetSelectedInstitute(GWEN_DIALOG *dlg) {
  OH_GETINST_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, OH_GETINST_DIALOG, dlg);
  assert(xdlg);

  return xdlg->selectedData;
}



OH_INSTITUTE_SPEC *OH_GetInstituteDialog_DetermineSelectedBankInfo(GWEN_DIALOG *dlg) {
  OH_GETINST_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, OH_GETINST_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->matchingSpecList) {
    int idx;

    idx=GWEN_Dialog_GetIntProperty(dlg, "listBox", GWEN_DialogProperty_Value, 0, -1);
    if (idx>=0) {
      const char *currentText;

      currentText=GWEN_Dialog_GetCharProperty(dlg, "listBox", GWEN_DialogProperty_Value, idx, NULL);
      if (currentText && *currentText) {
        OH_INSTITUTE_SPEC *os;
        GWEN_BUFFER *tbuf;

        tbuf=GWEN_Buffer_new(0, 256, 0, 1);
        os=OH_InstituteSpec_List_First(xdlg->matchingSpecList);
        while(os) {
          createListBoxString(os, tbuf);
          if (strcasecmp(currentText, GWEN_Buffer_GetStart(tbuf))==0) {
            GWEN_Buffer_free(tbuf);
            return os;
          }

          GWEN_Buffer_Reset(tbuf);
          os=OH_InstituteSpec_List_Next(os);
        }

        GWEN_Buffer_free(tbuf);
      }
    }
  }
  return NULL;
}



void OH_GetInstituteDialog_UpdateList(GWEN_DIALOG *dlg) {
  OH_GETINST_DIALOG *xdlg;
  const OH_INSTITUTE_SPEC_LIST *sl;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, OH_GETINST_DIALOG, dlg);
  assert(xdlg);

  /* clear bank info list */
  GWEN_Dialog_SetIntProperty(dlg, "listBox", GWEN_DialogProperty_ClearValues, 0, 0, 0);
  OH_InstituteSpec_List_Clear(xdlg->matchingSpecList);
  OH_InstituteData_free(xdlg->selectedData);
  xdlg->selectedData=NULL;

  sl=OfxHome_GetSpecs(xdlg->ofxHome);
  if (sl) {
    GWEN_BUFFER *tbuf;
    const OH_INSTITUTE_SPEC *os;
    const char *s;

    s=GWEN_Dialog_GetCharProperty(dlg, "nameEdit", GWEN_DialogProperty_Value, 0, NULL);
    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    os=OH_InstituteSpec_List_First(sl);
    while(os) {
      const char *bname;

      bname=OH_InstituteSpec_GetName(os);
      /* only add matching entries */
      if ( (s && bname && GWEN_Text_StrCaseStr(bname, s)!=NULL) ||
          !(s && *s)) {
        OH_InstituteSpec_List_Add(OH_InstituteSpec_dup(os), xdlg->matchingSpecList);
        createListBoxString(os, tbuf);

        GWEN_Dialog_SetCharProperty(dlg,
                                    "listBox",
                                    GWEN_DialogProperty_AddValue,
                                    0,
                                    GWEN_Buffer_GetStart(tbuf),
                                    0);
        GWEN_Buffer_Reset(tbuf);
      }
      os=OH_InstituteSpec_List_Next(os);
    }

    GWEN_Buffer_free(tbuf);
  }

  if (GWEN_Dialog_GetIntProperty(dlg, "listBox", GWEN_DialogProperty_Value, 0, -1)>=0)
    GWEN_Dialog_SetIntProperty(dlg, "okButton", GWEN_DialogProperty_Enabled, 0, 1, 0);
  else
    GWEN_Dialog_SetIntProperty(dlg, "okButton", GWEN_DialogProperty_Enabled, 0, 0, 0);
}



void OH_GetInstituteDialog_Init(GWEN_DIALOG *dlg) {
  OH_GETINST_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;
  int j;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, OH_GETINST_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
			      "",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("Select a Bank"),
			      0);

  GWEN_Dialog_SetCharProperty(dlg,
			      "listBox",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("Bank Name"),
			      0);
  GWEN_Dialog_SetIntProperty(dlg,
			     "listBox",
			     GWEN_DialogProperty_SelectionMode,
			     0,
			     GWEN_Dialog_SelectionMode_Single,
			     0);

  GWEN_Dialog_SetCharProperty(dlg,
			      "infoLabel",
			      GWEN_DialogProperty_Title,
			      0,
                              I18N("<html>"
                                   "<p>Please start typing in the name of your bank. The list "
                                   "below will be updated to show matching banks.</p>"
                                   "<p>Choose the bank from list below and click <b>ok</b>.</p>"
                                   "</html>"
                                   "Please start typing in the name of your bank. The list\n"
                                   "below will be updated to show matching banks.\n"
                                   "Choose the bank from list below and click OK."),
                              0);

  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);

  /* read bank column widths */
  for (i=0; i<1; i++) {
    j=GWEN_DB_GetIntValue(dbPrefs, "bank_list_columns", i, -1);
    if (j<LIST_MINCOLWIDTH)
      j=LIST_MINCOLWIDTH;
    GWEN_Dialog_SetIntProperty(dlg, "listBox", GWEN_DialogProperty_ColumnWidth, i, j, 0);
  }
  /* get sort column */
  i=GWEN_DB_GetIntValue(dbPrefs, "bank_list_sortbycolumn", 0, -1);
  j=GWEN_DB_GetIntValue(dbPrefs, "bank_list_sortdir", 0, -1);
  if (i>=0 && j>=0)
    GWEN_Dialog_SetIntProperty(dlg, "listBox", GWEN_DialogProperty_SortDirection, i, j, 0);

  /* disable ok button */
  GWEN_Dialog_SetIntProperty(dlg, "okButton", GWEN_DialogProperty_Enabled, 0, 0, 0);
}



void OH_GetInstituteDialog_Fini(GWEN_DIALOG *dlg) {
  OH_GETINST_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, OH_GETINST_DIALOG, dlg);
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


  /* store column widths of importer list */
  GWEN_DB_DeleteVar(dbPrefs, "bank_list_columns");
  for (i=0; i<1; i++) {
    int j;

    j=GWEN_Dialog_GetIntProperty(dlg, "listBox", GWEN_DialogProperty_ColumnWidth, i, -1);
    if (j<LIST_MINCOLWIDTH)
      j=LIST_MINCOLWIDTH;
    GWEN_DB_SetIntValue(dbPrefs,
			GWEN_DB_FLAGS_DEFAULT,
			"bank_list_columns",
			j);
  }
  /* store column sorting */
  GWEN_DB_SetIntValue(dbPrefs,
		      GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "bank_list_sortbycolumn",
		      -1);
  for (i=0; i<1; i++) {
    int j;

    j=GWEN_Dialog_GetIntProperty(dlg, "listBox", GWEN_DialogProperty_SortDirection, i,
				 GWEN_DialogSortDirection_None);
    if (j!=GWEN_DialogSortDirection_None) {
      GWEN_DB_SetIntValue(dbPrefs,
			  GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "bank_list_sortbycolumn",
			  i);
      GWEN_DB_SetIntValue(dbPrefs,
			  GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "bank_list_sortdir",
			  (j==GWEN_DialogSortDirection_Up)?1:0);
      break;
    }
  }
}



int OH_GetInstituteDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  OH_GETINST_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, OH_GETINST_DIALOG, dlg);
  assert(xdlg);

  DBG_ERROR(0, "Activated: %s", sender);
  if (strcasecmp(sender, "listBox")==0) {
    OH_INSTITUTE_SPEC *os;

    os=OH_GetInstituteDialog_DetermineSelectedBankInfo(dlg);
    GWEN_Dialog_SetIntProperty(dlg, "okButton", GWEN_DialogProperty_Enabled, 0, os?1:0, 0);

    return GWEN_DialogEvent_ResultHandled;
  }
  else if (strcasecmp(sender, "nameEdit")==0) {
    OH_GetInstituteDialog_UpdateList(dlg);
    return GWEN_DialogEvent_ResultHandled;
  }
  else if (strcasecmp(sender, "okButton")==0) {
    OH_INSTITUTE_SPEC *os;

    os=OH_GetInstituteDialog_DetermineSelectedBankInfo(dlg);
    if (os) {
      const OH_INSTITUTE_DATA *od;

      od=OfxHome_GetData(xdlg->ofxHome, OH_InstituteSpec_GetId(os));
      if (od) {
        OH_InstituteData_free(xdlg->selectedData);
        xdlg->selectedData=OH_InstituteData_dup(od);
        return GWEN_DialogEvent_ResultAccept;
      }
      else {
        DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "No institute data for id=%d",
                  OH_InstituteSpec_GetId(os));
      }
    }
    return GWEN_DialogEvent_ResultHandled;
  }
  else if (strcasecmp(sender, "abortButton")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "wiz_help_button")==0) {
    /* TODO: open a help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int OH_GetInstituteDialog_HandleValueChanged(GWEN_DIALOG *dlg, const char *sender) {
  DBG_ERROR(0, "Changed %s", sender);
  if (strcasecmp(sender, "nameEdit")==0) {
    OH_GetInstituteDialog_UpdateList(dlg);
    return GWEN_DialogEvent_ResultHandled;
  }
  else if (strcasecmp(sender, "listBox")==0) {
    return GWEN_DialogEvent_ResultHandled;
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB OH_GetInstituteDialog_SignalHandler(GWEN_DIALOG *dlg,
                                                      GWEN_DIALOG_EVENTTYPE t,
                                                      const char *sender) {
  OH_GETINST_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, OH_GETINST_DIALOG, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    OH_GetInstituteDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    OH_GetInstituteDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return OH_GetInstituteDialog_HandleValueChanged(dlg, sender);

  case GWEN_DialogEvent_TypeActivated:
    return OH_GetInstituteDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}







