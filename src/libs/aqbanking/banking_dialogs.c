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



GWEN_DIALOG *AB_Banking_CreateSetupDialog(AB_BANKING *ab)
{
  return AB_SetupDialog_new(ab);
}




