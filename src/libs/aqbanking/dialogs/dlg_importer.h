/***************************************************************************
 begin       : Tue Feb 10 2010
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQBANKING_DLG_IMPORTER_H
#define AQBANKING_DLG_IMPORTER_H


#include <aqbanking/banking.h>
#include <aqbanking/backendsupport/imexporter.h>

#include <gwenhywfar/dialog.h>



/** @defgroup G_AB_DIALOGS_IMPORTER Generic File Import Dialog
 * @ingroup G_AB_DIALOGS
 *
 */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif



/**
 * Creates a file import assistent.
 *
 * @return pointer to the created dialog.
 *
 * @param banking pointer to the AqBanking object

 * @param ctx pointer to the import context to receive the content of the
 * imported file
 *
 * @param finishedMessage message to show on the last page of the assistent
 *   (i.e. the page which is shown after a successfull import into the given
 *    import context).
 */
GWEN_DIALOG *AB_ImporterDialog_new(AB_BANKING *ab,
                                   AB_IMEXPORTER_CONTEXT *ctx,
                                   const char *finishedMessage);


const char *AB_ImporterDialog_GetFileName(const GWEN_DIALOG *dlg);
void AB_ImporterDialog_SetFileName(GWEN_DIALOG *dlg, const char *s);


const char *AB_ImporterDialog_GetImporterName(const GWEN_DIALOG *dlg);
void AB_ImporterDialog_SetImporterName(GWEN_DIALOG *dlg, const char *s);

const char *AB_ImporterDialog_GetProfileName(const GWEN_DIALOG *dlg);
void AB_ImporterDialog_SetProfileName(GWEN_DIALOG *dlg, const char *s);



#ifdef __cplusplus
}
#endif


/*@}*/


#endif

