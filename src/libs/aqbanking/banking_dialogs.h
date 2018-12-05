/***************************************************************************
 begin       : Wed Dec 05 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQBANKING_BANKING_DIALOGS_H
#define AQBANKING_BANKING_DIALOGS_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a file import assistent dialog.
 *
 * @return pointer to the created dialog.
 * @param banking pointer to the AqBanking object
 * @param ctx pointer to the import context to receive the content of the
 *   imported file
 * @param finishedMessage message to show on the last page of the assistent
 *   (i.e. the page which is shown after a successfull import into the given
 *    import context).
 */
AQBANKING_API
GWEN_DIALOG *AB_Banking_CreateImporterDialog(AB_BANKING *ab,
                                             AB_IMEXPORTER_CONTEXT *ctx,
                                             const char *finishedMessage);


AQBANKING_API
GWEN_DIALOG *AB_Banking_CreateSetupDialog(AB_BANKING *ab);


#ifdef __cplusplus
}
#endif


#endif

