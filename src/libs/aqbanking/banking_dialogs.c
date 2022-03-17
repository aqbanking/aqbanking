/***************************************************************************
 begin       : Wed Dec 05 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/* This file is included by banking.c */


#include "aqbanking/dialogs/dlg_importer.h"
#include "aqbanking/dialogs/dlg_setup.h"




GWEN_DIALOG *AB_Banking_CreateImporterDialog(AB_BANKING *ab,
                                             AB_IMEXPORTER_CONTEXT *ctx,
                                             const char *finishedMessage)
{
  return AB_ImporterDialog_new(ab, ctx, finishedMessage);
}



GWEN_DIALOG *AB_Banking_CreateImporterDialogWithPresets(AB_BANKING *ab,
                                                        AB_IMEXPORTER_CONTEXT *ctx,
                                                        const char *finishedMessage,
                                                        GWEN_DB_NODE *dbPrefs)
{
  GWEN_DIALOG *dlg;
  const char *s;

  dlg=AB_ImporterDialog_new(ab, ctx, finishedMessage);
  if (dlg==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Could not create import dialog");
    return NULL;
  }


  s=GWEN_DB_GetCharValue(dbPrefs, "filename", 0, NULL);
  if (s && *s)
    AB_ImporterDialog_SetFileName(dlg, s);

  s=GWEN_DB_GetCharValue(dbPrefs, "importer", 0, NULL);
  if (s && *s)
    AB_ImporterDialog_SetImporterName(dlg, s);

  s=GWEN_DB_GetCharValue(dbPrefs, "profile", 0, NULL);
  if (s && *s)
    AB_ImporterDialog_SetProfileName(dlg, s);

  return dlg;
}



void AB_Banking_ImporterDialogGatherPresets(GWEN_DIALOG *dlg, GWEN_DB_NODE *dbPrefs)
{
  const char *s;

  s=AB_ImporterDialog_GetFileName(dlg);
  if (s && *s)
    GWEN_DB_SetCharValue(dbPrefs, GWEN_DB_FLAGS_OVERWRITE_VARS, "filename", s);

  s=AB_ImporterDialog_GetImporterName(dlg);
  if (s && *s)
    GWEN_DB_SetCharValue(dbPrefs, GWEN_DB_FLAGS_OVERWRITE_VARS, "importer", s);

  s=AB_ImporterDialog_GetProfileName(dlg);
  if (s && *s)
    GWEN_DB_SetCharValue(dbPrefs, GWEN_DB_FLAGS_OVERWRITE_VARS, "profile", s);
}



GWEN_DIALOG *AB_Banking_CreateSetupDialog(AB_BANKING *ab)
{
  return AB_SetupDialog_new(ab);
}




