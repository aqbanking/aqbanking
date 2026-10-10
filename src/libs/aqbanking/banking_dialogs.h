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

#include <aqbanking/error.h>

#include <gwenhywfar/dialog.h>


#ifdef __cplusplus
extern "C" {
#endif

/** @name Import Dialogs
 *
 * Dialogs from this group can be used to import any file type AqBanking supports.
 * Those dialogs are easy to use:
 *
 * @code{.c}
    AB_IMEXPORTER_CONTEXT *importAnyFileIntoContext(AB_BANKING *ab) {
      GWEN_DIALOG *dlg;
      AB_IMEXPORTER_CONTEXT *ctx;
      int rv;

      ctx=AB_ImExporterContext_new();
      dlg=AB_Banking_CreateImporterDialog(ab, ctx, "This is a simple finish message.");
      if (dbg=NULL) {
        fprintf(stderr, "Could not create dialog\n");
        AB_ImExporterContext_free(ctx);
        return NULL;
      }
      GWEN_Dialog_SetWidgetText(dlg, "", "My Selected Window Title");
      rv=GWEN_Gui_ExecDialog(dlg, 0);
      if (rv==0) {
        fprintf(stderr, "Rejected.\n");
        GWEN_Dialog_free(dlg);
        AB_ImExporterContext_free(ctx);
        return NULL;
      }

      GWEN_Dialog_free(dlg);
      return ctx;
   }

 * @endcode
 */
/*@{*/


/**
 * Creates a file import assistent dialog.
 *
 * @return pointer to the created dialog.
 * @param banking pointer to the AqBanking object
 * @param ctx pointer to the import context to receive the content of the
 *   imported file (not taken over, the caller remains owner of the object)
 * @param finishedMessage message to show on the last page of the assistent
 *   (i.e. the page which is shown after a successful import into the given
 *    import context).
 */
AQBANKING_API
GWEN_DIALOG *AB_Banking_CreateImporterDialog(AB_BANKING *ab,
                                             AB_IMEXPORTER_CONTEXT *ctx,
                                             const char *finishedMessage);


/**
 * Same as @ref AB_Banking_CreateImporterDialog but presets (i.e. preset importer and profile).
 *
 * The following vars from the DB are used:
 * - filename: name of the file to import (will probably changed by the user in the gui)
 * - importer: name of the importer to use
 * - profile: name of the import profile to use
 *
 * @return pointer to created dialog
 * @param banking pointer to the AqBanking object
 * @param ctx pointer to the import context to receive the content of the
 *   imported file
 * @param finishedMessage message to show on the last page of the assistent
 *   (i.e. the page which is shown after a successful import into the given
 *    import context).
 * @param dbPrefs GWEN_DB_NODE where presets are stored (see above)
 */
AQBANKING_API
GWEN_DIALOG *AB_Banking_CreateImporterDialogWithPresets(AB_BANKING *ab,
                                                        AB_IMEXPORTER_CONTEXT *ctx,
                                                        const char *finishedMessage,
                                                        GWEN_DB_NODE *dbPrefs);

/**
 * Gather presets from an importer dialog (see @ref AB_Banking_CreateImporterDialogWithPresets).
 *
 * The dialog must have been finished for this function to return any meaningfull values.
 * Values empty in the gui (e.g. no selected profile or dialog aborted) will not be changed in @i dbPrefs.
 */
AQBANKING_API
void AB_Banking_ImporterDialogGatherPresets(GWEN_DIALOG *dlg, GWEN_DB_NODE *dbPrefs);


/*@}*/



AQBANKING_API
GWEN_DIALOG *AB_Banking_CreateSetupDialog(AB_BANKING *ab);


#ifdef __cplusplus
}
#endif


#endif

