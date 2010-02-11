/***************************************************************************
 begin       : Tue Feb 10 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "dlg_importer_p.h"
#include "i18n_l.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>

#include <assert.h>



#define PAGE_BEGIN     0
#define PAGE_FILE      1
#define PAGE_IMPORTER  2
#define PAGE_PROFILE   3
#define PAGE_END       4

#define IMPORTER_LIST_MINCOLWIDTH 50
#define PROFILE_LIST_MINCOLWIDTH  50

#define DIALOG_MINWIDTH  400
#define DIALOG_MINHEIGHT 400


GWEN_INHERIT(GWEN_DIALOG, AB_IMPORTER_DIALOG)






GWEN_DIALOG *AB_ImporterDialog_new(AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *ctx) {
  GWEN_DIALOG *dlg;
  AB_IMPORTER_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("ab_importwizard");
  GWEN_NEW_OBJECT(AB_IMPORTER_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg, xdlg,
		       AB_ImporterDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, AB_ImporterDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(GWEN_PM_LIBNAME, GWEN_PM_SYSDATADIR,
			       "aqbanking/dialogs/dlg_importer.dlg",
			       fbuf);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Dialog description file not found (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }

  /* read dialog from dialog description file */
  rv=GWEN_Dialog_ReadXmlFile(dlg, GWEN_Buffer_GetStart(fbuf));
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }
  GWEN_Buffer_free(fbuf);

  xdlg->banking=ab;
  xdlg->context=ctx;

  /* done */
  return dlg;
}



void AB_ImporterDialog_FreeData(void *bp, void *p) {
  AB_IMPORTER_DIALOG *xdlg;

  xdlg=(AB_IMPORTER_DIALOG*) p;
  free(xdlg->fileName);
  free(xdlg->importerName);
  free(xdlg->profileName);
  GWEN_DB_Group_free(xdlg->dbParams);
  GWEN_FREE_OBJECT(xdlg);
}



const char *AB_ImporterDialog_GetFileName(const GWEN_DIALOG *dlg) {
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->fileName;
}



void AB_ImporterDialog_SetFileName(GWEN_DIALOG *dlg, const char *s) {
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->fileName);
  if (s) xdlg->fileName=strdup(s);
  else xdlg->fileName=NULL;
}



const char *AB_ImporterDialog_GetImporterName(const GWEN_DIALOG *dlg) {
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->importerName;
}



void AB_ImporterDialog_SetImporterName(GWEN_DIALOG *dlg, const char *s) {
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->importerName);
  if (s) xdlg->importerName=strdup(s);
  else xdlg->importerName=NULL;
}



const char *AB_ImporterDialog_GetProfileName(const GWEN_DIALOG *dlg) {
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->profileName;
}



void AB_ImporterDialog_SetProfileName(GWEN_DIALOG *dlg, const char *s) {
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->profileName);
  if (s) xdlg->profileName=strdup(s);
  else xdlg->profileName=NULL;
}




void AB_ImporterDialog_Init(GWEN_DIALOG *dlg) {
  AB_IMPORTER_DIALOG *xdlg;
  int rv;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  GWEN_DB_Group_free(xdlg->dbParams);
  rv=AB_Banking_LoadSharedConfig(xdlg->banking,
				 GWEN_Dialog_GetId(dlg),
				 &(xdlg->dbParams),
				 0);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    xdlg->dbParams=GWEN_DB_Group_new("params");
  }

  GWEN_Dialog_SetCharProperty(dlg,
			      "",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("File Import Wizard"),
			      0);

  /* select first page */
  GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, 0, 0);

  /* setup intro page */
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_begin_label",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("This dialog assists you in importing files.\n"
				   "The following steps are:\n"
				   "- select file to import\n"
				   "- select importer module\n"
				   "- select importer profile\n"),
			      0);

  /* setup file page */
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_file_label",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("Please select the file to import."),
			      0);

  /* setup importer page */
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_importer_label",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("Please select the import module for the file."),
			      0);
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_importer_list",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("Name\tDescription"),
			      0);
  GWEN_Dialog_SetIntProperty(dlg,
			     "wiz_importer_list",
			     GWEN_DialogProperty_SelectionMode,
			     0,
			     GWEN_Dialog_SelectionMode_Single,
			     0);

  /* setup profile page */
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_profile_label",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("Please select the import profile for the file."),
			      0);
  GWEN_Dialog_SetCharProperty(dlg,
			      "wiz_profile_list",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("Name\tDescription"),
			      0);
  GWEN_Dialog_SetIntProperty(dlg,
			     "wiz_profile_list",
			     GWEN_DialogProperty_SelectionMode,
			     0,
			     GWEN_Dialog_SelectionMode_Single,
			     0);

  /* read width */
  i=GWEN_DB_GetIntValue(xdlg->dbParams, "dialog_width", 0, -1);
  if (i<DIALOG_MINWIDTH)
    i=DIALOG_MINWIDTH;
  GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(xdlg->dbParams, "dialog_height", 0, -1);
  if (i<DIALOG_MINHEIGHT)
    i=DIALOG_MINHEIGHT;
  GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);

  /* read importer column widths */
  for (i=0; i<2; i++) {
    int j;

    j=GWEN_DB_GetIntValue(xdlg->dbParams, "importer_list_columns", i, -1);
    if (j<IMPORTER_LIST_MINCOLWIDTH)
      j=IMPORTER_LIST_MINCOLWIDTH;
    GWEN_Dialog_SetIntProperty(dlg, "wiz_importer_list", GWEN_DialogProperty_ColumnWidth, i, j, 0);
  }

  /* read profile column widths */
  for (i=0; i<2; i++) {
    int j;

    j=GWEN_DB_GetIntValue(xdlg->dbParams, "profile_list_columns", i, -1);
    if (j<PROFILE_LIST_MINCOLWIDTH)
      j=PROFILE_LIST_MINCOLWIDTH;
    GWEN_Dialog_SetIntProperty(dlg, "wiz_profile_list", GWEN_DialogProperty_ColumnWidth, i, j, 0);
  }

  /* disable next and previous buttons */
  GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
  GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
}



void AB_ImporterDialog_Fini(GWEN_DIALOG *dlg) {
  AB_IMPORTER_DIALOG *xdlg;
  int rv;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  /* store dialog width */
  i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, -1);
  if (i<DIALOG_MINWIDTH)
    i=DIALOG_MINWIDTH;
  GWEN_DB_SetIntValue(xdlg->dbParams,
		      GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "dialog_width",
		      i);

  /* store dialog height */
  i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, -1);
  if (i<DIALOG_MINHEIGHT)
    i=DIALOG_MINHEIGHT;
  GWEN_DB_SetIntValue(xdlg->dbParams,
		      GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "dialog_height",
		      i);


  /* store column widths of importer list */
  GWEN_DB_DeleteVar(xdlg->dbParams, "importer_list_columns");
  for (i=0; i<2; i++) {
    int j;

    j=GWEN_Dialog_GetIntProperty(dlg, "wiz_importer_list", GWEN_DialogProperty_ColumnWidth, i, -1);
    if (j<IMPORTER_LIST_MINCOLWIDTH)
      j=IMPORTER_LIST_MINCOLWIDTH;
    GWEN_DB_SetIntValue(xdlg->dbParams,
			GWEN_DB_FLAGS_DEFAULT,
			"importer_list_columns",
			j);
  }

  /* store column widths of profile list */
  GWEN_DB_DeleteVar(xdlg->dbParams, "profile_list_columns");
  for (i=0; i<2; i++) {
    int j;

    j=GWEN_Dialog_GetIntProperty(dlg, "wiz_profile_list", GWEN_DialogProperty_ColumnWidth, i, -1);
    if (j<PROFILE_LIST_MINCOLWIDTH)
      j=PROFILE_LIST_MINCOLWIDTH;
    GWEN_DB_SetIntValue(xdlg->dbParams,
			GWEN_DB_FLAGS_DEFAULT,
			"profile_list_columns",
			j);
  }


  /* lock configuration */
  rv=AB_Banking_LockSharedConfig(xdlg->banking,
				 GWEN_Dialog_GetId(dlg),
				 0);
  if (rv==0) {
    /* save configuration */
    rv=AB_Banking_SaveSharedConfig(xdlg->banking,
				   GWEN_Dialog_GetId(dlg),
				   xdlg->dbParams,
				   0);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    }

    /* unlock configuration */
    rv=AB_Banking_UnlockSharedConfig(xdlg->banking,
				     GWEN_Dialog_GetId(dlg),
				     0);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    }
  }
}



void AB_ImporterDialog_UpdateImporterList(GWEN_DIALOG *dlg) {
  GWEN_PLUGIN_DESCRIPTION_LIST2 *il;
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  GWEN_Dialog_SetIntProperty(dlg, "wiz_importer_list", GWEN_DialogProperty_ClearChoices, 0, 0, 0);
  il=AB_Banking_GetImExporterDescrs(xdlg->banking);
  if (il) {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *ilit;
  
    ilit=GWEN_PluginDescription_List2_First(il);
    if (ilit) {
      GWEN_PLUGIN_DESCRIPTION *pd;
      GWEN_BUFFER *tbuf;
  
      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      pd=GWEN_PluginDescription_List2Iterator_Data(ilit);
      while(pd) {
	const char *s;
  
	s=GWEN_PluginDescription_GetName(pd);
	if (s && *s) {
	  GWEN_Buffer_AppendString(tbuf, s);
	  GWEN_Buffer_AppendString(tbuf, "\t");
	  s=GWEN_PluginDescription_GetShortDescr(pd);
	  if (s && *s)
	    GWEN_Buffer_AppendString(tbuf, s);
	  GWEN_Dialog_SetCharProperty(dlg,
				      "wiz_importer_list",
				      GWEN_DialogProperty_AddChoice,
				      0,
				      GWEN_Buffer_GetStart(tbuf),
				      0);
	  GWEN_Buffer_Reset(tbuf);
	}
	pd=GWEN_PluginDescription_List2Iterator_Next(ilit);
      }
  
      GWEN_Buffer_free(tbuf);
      GWEN_PluginDescription_List2Iterator_free(ilit);
    }
    GWEN_PluginDescription_List2_free(il);
  }
}



int AB_ImporterDialog_DetermineSelectedImporter(GWEN_DIALOG *dlg) {
  AB_IMPORTER_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  /* preset */
  free(xdlg->importerName);
  xdlg->importerName=NULL;

  /* get current value */
  rv=GWEN_Dialog_GetIntProperty(dlg, "wiz_importer_list", GWEN_DialogProperty_Value, 0, -1);
  DBG_ERROR(0, "Selected value: %d", rv);
  if (rv!=-1) {
    const char *s;

    s=GWEN_Dialog_GetCharProperty(dlg, "wiz_importer_list", GWEN_DialogProperty_Value, rv, NULL);
    if (s && *s) {
      const char *p;

      p=strchr(s, '\t');
      if (p) {
	int len;

	len=(int) (p-s);
	if (len) {
	  xdlg->importerName=(char*) malloc(len+1);
	  assert(xdlg->importerName);
	  memmove(xdlg->importerName, s, len);
	  xdlg->importerName[len]=0;
	}
      }
      else
	/* no tab, use the whole line */
	xdlg->importerName=strdup(s);

      if (xdlg->importerName) {
	DBG_ERROR(0, "Selected importer [%s]", xdlg->importerName);
        return 0;
      }
    }
  }

  return GWEN_ERROR_NOT_FOUND;
}



int AB_ImporterDialog_Next(GWEN_DIALOG *dlg) {
  AB_IMPORTER_DIALOG *xdlg;
  int page;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  switch(page) {

  case PAGE_BEGIN:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, PAGE_FILE, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_FILE:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, PAGE_IMPORTER, 0);
    AB_ImporterDialog_UpdateImporterList(dlg);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_IMPORTER:
    rv=AB_ImporterDialog_DetermineSelectedImporter(dlg);
    if (rv<0) {
      /* TODO: show error message */
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    }
    else {
      GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, PAGE_PROFILE, 0);
    }
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_PROFILE:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, PAGE_END, 0);
    GWEN_Dialog_SetCharProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Title, 0, I18N("Finish"), 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_END:
    return GWEN_DialogEvent_ResultAccept;

  default:
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unknown page %d", page);
    return GWEN_DialogEvent_ResultHandled;
  }
}



int AB_ImporterDialog_Previous(GWEN_DIALOG *dlg) {
  AB_IMPORTER_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  switch(page) {

  case PAGE_BEGIN:
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_FILE:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, PAGE_BEGIN, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_IMPORTER:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, PAGE_FILE, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_PROFILE:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, PAGE_IMPORTER, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_END:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, PAGE_PROFILE, 0);
    GWEN_Dialog_SetCharProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Title, 0, I18N("Next"), 0);
    return GWEN_DialogEvent_ResultHandled;

  default:
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unknown page %d", page);
    return GWEN_DialogEvent_ResultHandled;
  }
}



int AB_ImporterDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  if (strcasecmp(sender, "wiz_prev_button")==0)
    return AB_ImporterDialog_Previous(dlg);
  else if (strcasecmp(sender, "wiz_next_button")==0)
    return AB_ImporterDialog_Next(dlg);
  else if (strcasecmp(sender, "wiz_abort_button")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "wiz_help_button")==0) {
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int AB_ImporterDialog_HandleValueChanged(GWEN_DIALOG *dlg, const char *sender,
					 int intVal,
					 const char *charVal,
					 void *ptrVal) {
  return GWEN_DialogEvent_ResultNotHandled;
}



int AB_ImporterDialog_SignalHandler(GWEN_DIALOG *dlg,
				    GWEN_DIALOG_EVENTTYPE t,
				    const char *sender,
				    int intVal,
				    const char *charVal,
				    void *ptrVal) {
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    AB_ImporterDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AB_ImporterDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return AB_ImporterDialog_HandleValueChanged(dlg, sender, intVal, charVal, ptrVal);

  case GWEN_DialogEvent_TypeActivated:
    return AB_ImporterDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeGetImagePath:
  case GWEN_DialogEvent_TypeGetIconPath:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}






