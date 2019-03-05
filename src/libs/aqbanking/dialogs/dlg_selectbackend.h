/***************************************************************************
 begin       : Wed Apr 14 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQBANKING_DLG_SELECTBACKEND_H
#define AQBANKING_DLG_SELECTBACKEND_H


#include <aqbanking/banking.h>
#include <aqbanking/backendsupport/provider.h>

#include <gwenhywfar/dialog.h>
#include <gwenhywfar/db.h>


/** @defgroup G_AB_DIALOGS_SELECT_BACKEND Select a Backend
 * @ingroup G_AB_DIALOGS
 *
 */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif


/**
 * This function creates and executes a dialog which lets the user choose an online banking
 * backend from the list of installed backends.
 * This is a convenience function.
 * @param ab pointer to the AB_BANKING object used by the application
 * @param initial name of the initially selected backend
 * @param text text to be shown as introduction declaring what the backend is needed for (e.g.
 *   "Please select the backend to create a new user for").
 * @return pointer to the selected backend (or NULL on error)
 */
AB_PROVIDER *AB_SelectBackend(AB_BANKING *ab, const char *initial, const char *text);



GWEN_DIALOG *AB_SelectBackendDialog_new(AB_BANKING *ab, const char *text);

const char *AB_SelectBackendDialog_GetSelectedProvider(const GWEN_DIALOG *dlg);
void AB_SelectBackendDialog_SetSelectedProvider(GWEN_DIALOG *dlg, const char *s);



#ifdef __cplusplus
}
#endif



/*@}*/


#endif

