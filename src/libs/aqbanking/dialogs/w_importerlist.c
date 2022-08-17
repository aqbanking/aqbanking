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

#include "w_importerlist.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>




void AB_ImporterListWidget_Init(GWEN_DIALOG *dlg, const char *widgetName)
{
  GWEN_Dialog_SetCharProperty(dlg, widgetName, GWEN_DialogProperty_Title, 0, I18N("Name\tDescription"), 0);
  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_SelectionMode, 0, GWEN_Dialog_SelectionMode_Single, 0);
}



void AB_ImporterListWidget_UpdateList(GWEN_DIALOG *dlg, const char *widgetName, AB_BANKING *banking)
{
  GWEN_PLUGIN_DESCRIPTION_LIST2 *il;
  char *selectedImporter;

  selectedImporter=AB_ImporterListWidget_GetSelectedImporter(dlg, widgetName);

  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_ClearValues, 0, 0, 0);
  il=AB_Banking_GetImExporterDescrs(banking);
  if (il) {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *ilit;

    ilit=GWEN_PluginDescription_List2_First(il);
    if (ilit) {
      GWEN_PLUGIN_DESCRIPTION *pd;
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      pd=GWEN_PluginDescription_List2Iterator_Data(ilit);
      while (pd) {
        const char *s;

        s=GWEN_PluginDescription_GetName(pd);
        if (s && *s) {
          GWEN_Buffer_AppendString(tbuf, s);
          s=GWEN_PluginDescription_GetShortDescr(pd);
          if (s && *s) {
            GWEN_Buffer_AppendString(tbuf, "\t");
            GWEN_Buffer_AppendString(tbuf, s);
          }
          GWEN_Dialog_SetCharProperty(dlg, widgetName, GWEN_DialogProperty_AddValue, 0, GWEN_Buffer_GetStart(tbuf), 0);
          GWEN_Buffer_Reset(tbuf);
        }
        pd=GWEN_PluginDescription_List2Iterator_Next(ilit);
      }

      GWEN_Buffer_free(tbuf);
      GWEN_PluginDescription_List2Iterator_free(ilit);
    }
    GWEN_PluginDescription_List2_free(il);

    GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Sort, 0, 0, 0);
    if (selectedImporter) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Re-selecting importer \"%s\"", selectedImporter);
      AB_ImporterListWidget_SelectImporter(dlg, widgetName, selectedImporter);
    }
  }

  if (selectedImporter)
    free(selectedImporter);
}



void AB_ImporterListWidget_SelectImporter(GWEN_DIALOG *dlg, const char *widgetName, const char *importerName)
{
  int idx;

  idx=GWEN_Dialog_ListGetItemMatchingFirstColumn(dlg, widgetName, importerName);
  if (idx>=0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Selecting importer \"%s\" (%d)", importerName, idx);
    GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, idx, 0);
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Importer \"%s\" not in list", importerName);
  }
}



char *AB_ImporterListWidget_GetSelectedImporter(GWEN_DIALOG *dlg, const char *widgetName)
{
  int rv;

  /* get current value */
  rv=GWEN_Dialog_GetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, -1);
  DBG_INFO(AQBANKING_LOGDOMAIN, "Selected value: %d", rv);
  if (rv!=-1) {
    char *s;

    s=GWEN_Dialog_ListGetFirstColumnData(dlg, widgetName, rv);
    if (s && *s) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Selected importer [%s]", s);
      return s;
    }
  }

  return NULL;
}




