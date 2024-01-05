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

#include "w_profilelist.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>




void AB_ProfileListWidget_Init(GWEN_DIALOG *dlg, const char *widgetName)
{
  GWEN_Dialog_SetCharProperty(dlg, widgetName, GWEN_DialogProperty_Title, 0, I18N("Name\tDescription"), 0);
  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_SelectionMode, 0, GWEN_Dialog_SelectionMode_Single, 0);
}



void AB_ProfileListWidget_UpdateList(GWEN_DIALOG *dlg, const char *widgetName, AB_BANKING *banking,
                                     const char *importerName)
{
  char *selectedProfile;

  selectedProfile=AB_ProfileListWidget_GetSelectedProfile(dlg, widgetName);

  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_ClearValues, 0, 0, 0);
  if (importerName && *importerName) {
    GWEN_DB_NODE *db;

    db=AB_Banking_GetImExporterProfiles(banking, importerName);
    if (db) {
      GWEN_DB_NODE *dbProfile;
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      dbProfile=GWEN_DB_GetFirstGroup(db);
      while (dbProfile) {
        const char *s;

        s=GWEN_DB_GetCharValue(dbProfile, "name", 0, NULL);
        if (s && *s) {
          GWEN_Buffer_AppendString(tbuf, s);
          s=GWEN_DB_GetCharValue(dbProfile, "shortDescr", 0, NULL);
          if (s && *s) {
            GWEN_Buffer_AppendString(tbuf, "\t");
            GWEN_Buffer_AppendString(tbuf, s);
          }
          GWEN_Dialog_SetCharProperty(dlg, widgetName, GWEN_DialogProperty_AddValue, 0, GWEN_Buffer_GetStart(tbuf), 0);
          GWEN_Buffer_Reset(tbuf);
        }

        dbProfile=GWEN_DB_GetNextGroup(dbProfile);
      }
      GWEN_Buffer_free(tbuf);
      GWEN_DB_Group_free(db);
    }
    GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Sort, 0, 0, 0);
    if (selectedProfile) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Re-selecting profile \"%s\"", selectedProfile);
      AB_ProfileListWidget_SelectProfile(dlg, widgetName, selectedProfile);
    }
  }
  if (selectedProfile)
    free(selectedProfile);
}



void AB_ProfileListWidget_SelectProfile(GWEN_DIALOG *dlg, const char *widgetName, const char *profileName)
{
  int idx;

  idx=GWEN_Dialog_ListGetItemMatchingFirstColumn(dlg, widgetName, profileName);
  if (idx>=0)
    GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, idx, 0);
}



char *AB_ProfileListWidget_GetSelectedProfile(GWEN_DIALOG *dlg, const char *widgetName)
{
  int rv;

  /* get current value */
  rv=GWEN_Dialog_GetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, -1);
  DBG_INFO(AQBANKING_LOGDOMAIN, "Selected value: %d", rv);
  if (rv!=-1) {
    char *s;

    s=GWEN_Dialog_ListGetFirstColumnData(dlg, widgetName, rv);
    if (s && *s) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Selected profile [%s]", s);
      return s;
    }
  }

  return NULL;
}




