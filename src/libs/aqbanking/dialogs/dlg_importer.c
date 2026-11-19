/***************************************************************************
 begin       : Tue Feb 10 2010
 copyright   : (C) 2022 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "dlg_importer_p.h"
#include "w_importerlist.h"
#include "w_profilelist.h"

#include "aqbanking/i18n_l.h"

#include <aqbanking/banking_be.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

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




static GWENHYWFAR_CB void _dlgApi_FreeData(void *bp, void *p);
static int _determineSelectedImporter(GWEN_DIALOG *dlg);
static int _determineSelectedProfile(GWEN_DIALOG *dlg);

static void _updateImporterList(GWEN_DIALOG *dlg);
static void _updateProfileList(GWEN_DIALOG *dlg);

static int _editProfile(GWEN_DIALOG *dlg);
static int _newProfile(GWEN_DIALOG *dlg);

static GWENHYWFAR_CB int _dlgApi_SignalHandler(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);





GWEN_DIALOG *AB_ImporterDialog_new(AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *ctx, const char *finishedMessage)
{
  GWEN_DIALOG *dlg;
  AB_IMPORTER_DIALOG *xdlg;

  dlg=GWEN_Dialog_CreateAndLoadWithPath("ab_importwizard", AB_PM_LIBNAME, AB_PM_DATADIR,
                                        "aqbanking/dialogs/dlg_importer.dlg");
  if (dlg==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not create dialog \"ab_importwizard\".");
    return NULL;
  }

  GWEN_NEW_OBJECT(AB_IMPORTER_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg, xdlg, _dlgApi_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, _dlgApi_SignalHandler);

  xdlg->banking=ab;
  xdlg->context=ctx;
  xdlg->finishedMessage=finishedMessage;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB _dlgApi_FreeData(void *bp, void *p)
{
  AB_IMPORTER_DIALOG *xdlg;

  xdlg=(AB_IMPORTER_DIALOG *) p;
  free(xdlg->fileName);
  free(xdlg->importerName);
  free(xdlg->profileName);
  GWEN_FREE_OBJECT(xdlg);
}



const char *AB_ImporterDialog_GetFileName(const GWEN_DIALOG *dlg)
{
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->fileName;
}



void AB_ImporterDialog_SetFileName(GWEN_DIALOG *dlg, const char *s)
{
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->fileName);
  if (s)
    xdlg->fileName=strdup(s);
  else
    xdlg->fileName=NULL;
}



const char *AB_ImporterDialog_GetImporterName(const GWEN_DIALOG *dlg)
{
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->importerName;
}



void AB_ImporterDialog_SetImporterName(GWEN_DIALOG *dlg, const char *s)
{
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->importerName);
  if (s)
    xdlg->importerName=strdup(s);
  else
    xdlg->importerName=NULL;
}



const char *AB_ImporterDialog_GetProfileName(const GWEN_DIALOG *dlg)
{
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->profileName;
}



void AB_ImporterDialog_SetProfileName(GWEN_DIALOG *dlg, const char *s)
{
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->profileName);
  if (s)
    xdlg->profileName=strdup(s);
  else
    xdlg->profileName=NULL;
}



void AB_ImporterDialog_Init(GWEN_DIALOG *dlg)
{
  AB_IMPORTER_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  DBG_INFO(AQBANKING_LOGDOMAIN, "Init");

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
                              "",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N("File Import Wizard"),
                              0);

  if (xdlg->fileName)
    GWEN_Dialog_SetCharProperty(dlg, "wiz_file_edit", GWEN_DialogProperty_Value, 0, xdlg->fileName, 0);

  /* select first page */
  GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, 0, 0);

  /* setup intro page */
  GWEN_Dialog_SetCharProperty(dlg,
                              "wiz_begin_label",
                              GWEN_DialogProperty_Title,
                              0,
                              I18N(
                                "<html>"
                                "<p>This dialog assists you in importing files."
                                "The following steps are:</p>"
                                "<ul>"
                                "<li>select file to import</li>"
                                "<li>select importer module</li>"
                                "<li>select importer profile</li>"
                                "</ul>"
                                "</html>"
                                "This dialog assists you in importing files.\n"
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
  /*AB_ProfileListWidget_Init(dlg, "wiz_profile_list");*/
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

  /* setup extro page */
  if (xdlg->finishedMessage && *(xdlg->finishedMessage))
    GWEN_Dialog_SetCharProperty(dlg,
                                "wiz_end_label",
                                GWEN_DialogProperty_Title,
                                0,
                                xdlg->finishedMessage,
                                0);
  else
    GWEN_Dialog_SetCharProperty(dlg,
                                "wiz_end_label",
                                GWEN_DialogProperty_Title,
                                0,
                                I18N("The file has been successfully imported."),
                                0);

  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);

  GWEN_Dialog_ListReadColumnSettings(dlg, "wiz_importer_list", "importer_list_", 2, IMPORTER_LIST_MINCOLWIDTH, dbPrefs);

  GWEN_Dialog_ListReadColumnSettings(dlg, "wiz_profile_list", "profile_list_", 2, PROFILE_LIST_MINCOLWIDTH, dbPrefs);


  /* disable next and previous buttons */
  GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
  GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
}



void AB_ImporterDialog_Fini(GWEN_DIALOG *dlg)
{
  AB_IMPORTER_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  DBG_INFO(AQBANKING_LOGDOMAIN, "Fini");

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  /* store dialog width */
  i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, -1);
  GWEN_DB_SetIntValue(dbPrefs, GWEN_DB_FLAGS_OVERWRITE_VARS, "dialog_width", i);

  /* store dialog height */
  i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, -1);
  GWEN_DB_SetIntValue(dbPrefs, GWEN_DB_FLAGS_OVERWRITE_VARS, "dialog_height", i);

  /* store list column widths and sort settings (max two columns) */
  GWEN_Dialog_ListWriteColumnSettings(dlg, "wiz_importer_list", "importer_list_", 2, IMPORTER_LIST_MINCOLWIDTH, dbPrefs);

  GWEN_Dialog_ListWriteColumnSettings(dlg, "wiz_profile_list", "profile_list_", 2, PROFILE_LIST_MINCOLWIDTH, dbPrefs);
}



void _updateImporterList(GWEN_DIALOG *dlg)
{
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  AB_ImporterListWidget_UpdateList(dlg, "wiz_importer_list", xdlg->banking);

  if (xdlg->importerName)
    AB_ImporterListWidget_SelectImporter(dlg, "wiz_importer_list", xdlg->importerName);
}



int _determineSelectedImporter(GWEN_DIALOG *dlg)
{
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  /* preset */
  free(xdlg->importerName);
  xdlg->importerName=NULL;

  xdlg->importerName=AB_ImporterListWidget_GetSelectedImporter(dlg, "wiz_importer_list");
  if (xdlg->importerName==NULL)
    return GWEN_ERROR_NOT_FOUND;

  return 0;
}



void _updateProfileList(GWEN_DIALOG *dlg)
{
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  AB_ProfileListWidget_UpdateList(dlg, "wiz_profile_list", xdlg->banking, xdlg->importerName);
  if (xdlg->profileName)
    AB_ProfileListWidget_SelectProfile(dlg, "wiz_profile_list", xdlg->profileName);
}



int _determineSelectedProfile(GWEN_DIALOG *dlg)
{
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  /* preset */
  free(xdlg->profileName);
  xdlg->profileName=NULL;

  xdlg->profileName=AB_ProfileListWidget_GetSelectedProfile(dlg, "wiz_profile_list");
  if (xdlg->profileName==NULL)
    return GWEN_ERROR_NOT_FOUND;

  return 0;
}



int AB_ImporterDialog_DetermineFilename(GWEN_DIALOG *dlg)
{
  AB_IMPORTER_DIALOG *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  free(xdlg->fileName);
  xdlg->fileName=NULL;

  s=GWEN_Dialog_GetCharProperty(dlg, "wiz_file_edit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    xdlg->fileName=strdup(s);
    return 0;
  }

  return GWEN_ERROR_NOT_FOUND;
}




int AB_ImporterDialog_EnterPage(GWEN_DIALOG *dlg, int page, int forwards)
{
  AB_IMPORTER_DIALOG *xdlg;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  switch (page) {
  case PAGE_BEGIN:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_FILE:
    GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    if (xdlg->fileName==NULL)
      AB_ImporterDialog_DetermineFilename(dlg);
    if (xdlg->fileName==NULL)
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_IMPORTER:
    if (forwards) {
      AB_ImporterDialog_DetermineFilename(dlg);
      _updateImporterList(dlg);
    }
    if (xdlg->importerName==NULL)
      _determineSelectedImporter(dlg);
    if (xdlg->importerName==NULL)
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_PROFILE:
    if (forwards) {
      _determineSelectedImporter(dlg);
      _updateProfileList(dlg);
    }
    if (xdlg->profileName==NULL)
      _determineSelectedProfile(dlg);
    if (xdlg->profileName==NULL)
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      /* we have a selected importer, enable "next" button */
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    GWEN_Dialog_SetCharProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Title, 0, I18N("Next"), 0);
    GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
    return GWEN_DialogEvent_ResultHandled;

  case PAGE_END:
    if (forwards) {
      rv=_determineSelectedProfile(dlg);
      if (rv<0) {
        /* no profile... */
        DBG_INFO(AQBANKING_LOGDOMAIN, "No profile");
      }
      else {
        rv=AB_Banking_ImportFromFileLoadProfile(xdlg->banking,
                                                xdlg->importerName,
                                                xdlg->context,
                                                xdlg->profileName,
                                                NULL,
                                                xdlg->fileName);
        if (rv<0) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Error importing file: %d", rv);
          GWEN_Gui_ShowError(I18N("Error"),
                             I18N("Error importing file (%d: %s), please see log files for details"),
                             GWEN_Error_SimpleToString(rv));
          AB_ImExporterContext_Clear(xdlg->context);
        }
        else {
          DBG_NOTICE(0, "Import ok.");
          /* no way back */
          GWEN_Dialog_SetIntProperty(dlg, "wiz_prev_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
          GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
          GWEN_Dialog_SetCharProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Title, 0, I18N("Finished"), 0);
          GWEN_Dialog_SetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, page, 0);
          return GWEN_DialogEvent_ResultHandled;
        }
      }
    }
    return GWEN_DialogEvent_ResultHandled;

  default:
    return GWEN_DialogEvent_ResultHandled;
  }

  return GWEN_DialogEvent_ResultHandled;
}



int AB_ImporterDialog_Next(GWEN_DIALOG *dlg)
{
  AB_IMPORTER_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  DBG_NOTICE(0, "Value of wiz_stack: %d", page);

  if (page<PAGE_END) {
    page++;
    return AB_ImporterDialog_EnterPage(dlg, page, 1);
  }
  else if (page==PAGE_END)
    return GWEN_DialogEvent_ResultAccept;

  return GWEN_DialogEvent_ResultHandled;
}



int AB_ImporterDialog_Previous(GWEN_DIALOG *dlg)
{
  AB_IMPORTER_DIALOG *xdlg;
  int page;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  page=GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1);
  if (page>PAGE_BEGIN) {
    page--;
    return AB_ImporterDialog_EnterPage(dlg, page, 0);
  }

  return GWEN_DialogEvent_ResultHandled;
}



int _editProfile(GWEN_DIALOG *dlg)
{
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  if (_determineSelectedProfile(dlg)==0) {
    GWEN_DB_NODE *dbProfiles;
    GWEN_DB_NODE *dbT;
    GWEN_DIALOG *edlg=NULL;
    const char *s;
    char *fileNameCopy=NULL;
    int rv;

    dbProfiles=AB_Banking_GetImExporterProfiles(xdlg->banking, xdlg->importerName);
    if (dbProfiles==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "ImExporter [%s] has no profiles", xdlg->importerName);
      return GWEN_DialogEvent_ResultHandled;
    }

    dbT=GWEN_DB_GetFirstGroup(dbProfiles);
    while (dbT) {
      const char *s;

      s=GWEN_DB_GetCharValue(dbT, "name", 0, NULL);
      if (s && *s && strcasecmp(s, xdlg->profileName)==0)
        break;
      dbT=GWEN_DB_GetNextGroup(dbT);
    }

    if (dbT==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Profile [%s] for ImExporter [%s] not found",
                xdlg->profileName,
                xdlg->importerName);
      GWEN_DB_Group_free(dbProfiles);
      return GWEN_DialogEvent_ResultHandled;
    }

    s=GWEN_DB_GetCharValue(dbT, "fileName", 0, NULL);
    if (!(s && *s)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "No filename, can't save profile");
      GWEN_DB_Group_free(dbProfiles);
      return GWEN_DialogEvent_ResultHandled;
    }
    fileNameCopy=strdup(s);

    rv=AB_Banking_GetEditImExporterProfileDialog(xdlg->banking, xdlg->importerName, dbT, xdlg->fileName, &edlg);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "ImExporter [%s] has no EditProfileDialog", xdlg->importerName);
      free(fileNameCopy);
      GWEN_DB_Group_free(dbProfiles);
      return GWEN_DialogEvent_ResultHandled;
    }

    rv=GWEN_Gui_ExecDialog(edlg, 0);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      free(fileNameCopy);
      GWEN_Dialog_free(edlg);
      GWEN_DB_Group_free(dbProfiles);
      return GWEN_DialogEvent_ResultHandled;
    }
    if (rv==1) {
      const char *proname;

      /* accepted */
      proname=GWEN_DB_GetCharValue(dbT, "name", 0, NULL);
      DBG_NOTICE(0, "Accepted, writing profile");
      rv=AB_Banking_SaveLocalImExporterProfile(xdlg->banking,
                                               xdlg->importerName,
                                               dbT,
                                               fileNameCopy);
      if (rv<0) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
        free(fileNameCopy);
        GWEN_Dialog_free(edlg);
        GWEN_DB_Group_free(dbProfiles);
        return GWEN_DialogEvent_ResultHandled;
      }

      /* reload "wiz_profile_list", select new profile */
      _updateProfileList(dlg);
      if (proname && *proname) {
        int idx;

        idx=GWEN_Dialog_ListGetItemMatchingFirstColumn(dlg, "wiz_profile_list", proname);
        if (idx>=0) {
          GWEN_Dialog_SetIntProperty(dlg, "wiz_profile_list", GWEN_DialogProperty_Value, 0, idx, 1);
          _determineSelectedProfile(dlg);
        }
      }
    }

    GWEN_Dialog_free(edlg);
    free(fileNameCopy);
    GWEN_DB_Group_free(dbProfiles);
  }
  return GWEN_DialogEvent_ResultHandled;
}



int _newProfile(GWEN_DIALOG *dlg)
{
  AB_IMPORTER_DIALOG *xdlg;
  GWEN_DB_NODE *dbProfile;
  GWEN_DIALOG *edlg=NULL;
  char *fileNameCopy=NULL;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  dbProfile=GWEN_DB_Group_new("profile");

  rv=AB_Banking_GetEditImExporterProfileDialog(xdlg->banking, xdlg->importerName, dbProfile, xdlg->fileName, &edlg);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "ImExporter [%s] has no EditProfileDialog", xdlg->importerName);
    GWEN_DB_Group_free(dbProfile);
    return GWEN_DialogEvent_ResultHandled;
  }

  rv=GWEN_Gui_ExecDialog(edlg, 0);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_Dialog_free(edlg);
    GWEN_DB_Group_free(dbProfile);
    return GWEN_DialogEvent_ResultHandled;
  }
  if (rv==1) {
    const char *proname;

    /* accepted */
    proname=GWEN_DB_GetCharValue(dbProfile, "name", 0, NULL);
    DBG_NOTICE(0, "Accepted, writing profile");
    rv=AB_Banking_SaveLocalImExporterProfile(xdlg->banking,
                                             xdlg->importerName,
                                             dbProfile,
                                             fileNameCopy);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_Dialog_free(edlg);
      GWEN_DB_Group_free(dbProfile);
      return GWEN_DialogEvent_ResultHandled;
    }

    /* reload "wiz_profile_list", select new profile */
    _updateProfileList(dlg);
    if (proname && *proname) {
      int idx;

      idx=GWEN_Dialog_ListGetItemMatchingFirstColumn(dlg, "wiz_profile_list", proname);
      if (idx>=0) {
        GWEN_Dialog_SetIntProperty(dlg, "wiz_profile_list", GWEN_DialogProperty_Value, 0, idx, 1);
        _determineSelectedProfile(dlg);
      }
    }

  }

  GWEN_Dialog_free(edlg);
  GWEN_DB_Group_free(dbProfile);
  return GWEN_DialogEvent_ResultHandled;
}



int AB_ImporterDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender)
{
  DBG_INFO(AQBANKING_LOGDOMAIN, "Activated: %s", sender);
  if (strcasecmp(sender, "wiz_prev_button")==0)
    return AB_ImporterDialog_Previous(dlg);
  else if (strcasecmp(sender, "wiz_next_button")==0)
    return AB_ImporterDialog_Next(dlg);
  else if (strcasecmp(sender, "wiz_abort_button")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "wiz_help_button")==0) {
    /* TODO: open a help dialog */
  }
  else if (strcasecmp(sender, "wiz_importer_list")==0) {
    if (_determineSelectedImporter(dlg)<0)
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
  }

  else if (strcasecmp(sender, "wiz_profile_list")==0) {
    if (_determineSelectedProfile(dlg)<0)
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
  }

  else if (strcasecmp(sender, "wiz_file_button")==0) {
    int rv;
    const char *s;
    GWEN_BUFFER *pathBuffer;

    pathBuffer=GWEN_Buffer_new(0, 256, 0, 1);
    s=GWEN_Dialog_GetCharProperty(dlg, "wiz_file_edit", GWEN_DialogProperty_Value, 0, NULL);
    if (s && *s)
      GWEN_Buffer_AppendString(pathBuffer, s);
    rv=GWEN_Gui_GetFileName(I18N("Select File to Import"),
                            GWEN_Gui_FileNameType_OpenFileName,
                            0,
                            I18N("All Files (*)\tCSV Files (*csv;*.CSV)\t*.sta"),
                            pathBuffer,
                            GWEN_Dialog_GetGuiId(dlg));
    if (rv==0) {
      GWEN_Dialog_SetCharProperty(dlg,
                                  "wiz_file_edit",
                                  GWEN_DialogProperty_Value,
                                  0,
                                  GWEN_Buffer_GetStart(pathBuffer),
                                  0);
      rv=AB_ImporterDialog_DetermineFilename(dlg);
      if (rv<0)
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    else {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    }
    GWEN_Buffer_free(pathBuffer);
    return GWEN_DialogEvent_ResultNotHandled;
  }

  else if (strcasecmp(sender, "wiz_profile_edit_button")==0) {
    return _editProfile(dlg);
  }
  else if (strcasecmp(sender, "wiz_profile_new_button")==0) {
    return _newProfile(dlg);
  }
  else if (strcasecmp(sender, "wiz_profile_del_button")==0) {
    /* TODO: get appropriate profile editor */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int AB_ImporterDialog_HandleValueChanged(GWEN_DIALOG *dlg, const char *sender)
{
  if (strcasecmp(sender, "wiz_file_edit")==0) {
    int rv;

    rv=AB_ImporterDialog_DetermineFilename(dlg);
    if (GWEN_Dialog_GetIntProperty(dlg, "wiz_stack", GWEN_DialogProperty_Value, 0, -1)==PAGE_FILE) {
      if (rv<0)
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 0, 0);
      else
        GWEN_Dialog_SetIntProperty(dlg, "wiz_next_button", GWEN_DialogProperty_Enabled, 0, 1, 0);
    }
    return GWEN_DialogEvent_ResultHandled;
  }
  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB _dlgApi_SignalHandler(GWEN_DIALOG *dlg,
                                        GWEN_DIALOG_EVENTTYPE t,
                                        const char *sender)
{
  AB_IMPORTER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_IMPORTER_DIALOG, dlg);
  assert(xdlg);

  switch (t) {
  case GWEN_DialogEvent_TypeInit:
    AB_ImporterDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;

  case GWEN_DialogEvent_TypeFini:
    AB_ImporterDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;

  case GWEN_DialogEvent_TypeValueChanged:
    return AB_ImporterDialog_HandleValueChanged(dlg, sender);

  case GWEN_DialogEvent_TypeActivated:
    return AB_ImporterDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
  default:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}






