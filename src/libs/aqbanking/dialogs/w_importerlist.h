/***************************************************************************
 begin       : Tue Feb 10 2010
 copyright   : (C) 2022 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQBANKING_W_IMPORTERLIST_H
#define AQBANKING_W_IMPORTERLIST_H


#include <aqbanking/banking.h>

#include <gwenhywfar/dialog.h>


/** @name Helper Functions for Lists of Im-/Exporters
 *
 */
/*@{*/

/**
 * Init widget by setting header text for the supported columns and set selection mode to GWEN_Dialog_SelectionMode_Single.
 *
 * @param dlg dialog of which the widget is part
 * @param widgetName name of the widget in the dialog to work on
 */
void AB_ImporterListWidget_Init(GWEN_DIALOG *dlg, const char *widgetName);

/**
 * Update the list of importer shown in this widget.
 *
 * This function retrieves the list of all import modules and adds entries for every one of them.
 * Keeps the currently selected profile.
 *
 * @param dlg dialog of which the widget is part
 * @param importerName name of the widget in the dialog to work on
 * @param banking pointer to used AqBanking object
 */
void AB_ImporterListWidget_UpdateList(GWEN_DIALOG *dlg, const char *widgetName, AB_BANKING *banking);

/**
 * Select the given importer.
 *
 * @param dlg dialog of which the widget is part
 * @param widgetName name of the widget in the dialog to work on
 * @param importerName name of the profile to select
 */
void AB_ImporterListWidget_SelectImporter(GWEN_DIALOG *dlg, const char *widgetName, const char *importerName);

/**
 * Get the name of the currently selected profile.
 *
 * @return new string containing the name of the selected profile (needs to be free'd) or NULL if none selected
 * @param dlg dialog of which the widget is part
 * @param widgetName name of the widget in the dialog to work on
 */
char *AB_ImporterListWidget_GetSelectedImporter(GWEN_DIALOG *dlg, const char *widgetName);

/*@}*/




#endif
