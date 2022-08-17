/***************************************************************************
 begin       : Tue Feb 10 2010
 copyright   : (C) 2022 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQBANKING_W_PROFILELIST_H
#define AQBANKING_W_PROFILELIST_H


#include <aqbanking/banking.h>

#include <gwenhywfar/dialog.h>


/** @name Helper Functions for Lists of Im-/Exporter Profiles
 *
 */
/*@{*/

/**
 * Init widget by setting header text for the supported columns and set selection mode to GWEN_Dialog_SelectionMode_Single.
 *
 * @param dlg dialog of which the widget is part
 * @param widgetName name of the widget in the dialog to work on
 */
void AB_ProfileListWidget_Init(GWEN_DIALOG *dlg, const char *widgetName);

/**
 * Update the list of profiles shown in this widget.
 *
 * This function reads all profiles for the given importer and adds entries for every one of them which has a valid name.
 * Keeps the currently selected profile.
 *
 * @param dlg dialog of which the widget is part
 * @param widgetName name of the widget in the dialog to work on
 * @param banking pointer to used AqBanking object
 * @param importerName name of the AqBanking import module for which profiles are to be loaded
 */
void AB_ProfileListWidget_UpdateList(GWEN_DIALOG *dlg, const char *widgetName, AB_BANKING *banking, const char *importerName);

/**
 * Select the given profile.
 *
 * @param dlg dialog of which the widget is part
 * @param widgetName name of the widget in the dialog to work on
 * @param profileName name of the profile to select
 */
void AB_ProfileListWidget_SelectProfile(GWEN_DIALOG *dlg, const char *widgetName, const char *profileName);

/**
 * Get the name of the currently selected profile.
 *
 * @return new string containing the name of the selected profile (needs to be free'd) or NULL if none selected
 * @param dlg dialog of which the widget is part
 * @param widgetName name of the widget in the dialog to work on
 */
char *AB_ProfileListWidget_GetSelectedProfile(GWEN_DIALOG *dlg, const char *widgetName);

/*@}*/




#endif
