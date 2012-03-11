/***************************************************************************
 begin       : Tue Feb 10 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQBANKING_DLG_IMPORTER_P_H
#define AQBANKING_DLG_IMPORTER_P_H

#include "dlg_importer.h"


#include <gwenhywfar/stringlist.h>


typedef struct AB_IMPORTER_DIALOG AB_IMPORTER_DIALOG;
struct AB_IMPORTER_DIALOG {
  AB_BANKING *banking;
  AB_IMEXPORTER_CONTEXT *context;
  const char *finishedMessage;

  char *fileName;
  char *importerName;
  char *profileName;
};


static GWENHYWFAR_CB void AB_ImporterDialog_FreeData(void *bp, void *p);

#if 0
static const char *AB_ImporterDialog_GetFileName(const GWEN_DIALOG *dlg);
static void AB_ImporterDialog_SetFileName(GWEN_DIALOG *dlg, const char *s);

static const char *AB_ImporterDialog_GetImporterName(const GWEN_DIALOG *dlg);
static void AB_ImporterDialog_SetImporterName(GWEN_DIALOG *dlg, const char *s);

static const char *AB_ImporterDialog_GetProfileName(const GWEN_DIALOG *dlg);
static void AB_ImporterDialog_SetProfileName(GWEN_DIALOG *dlg, const char *s);
#endif

static int AB_ImporterDialog_FindIndexOfProfile(GWEN_DIALOG *dlg, const char *proname);

static int AB_ImporterDialog_EditProfile(GWEN_DIALOG *dlg);
static int AB_ImporterDialog_NewProfile(GWEN_DIALOG *dlg);


static GWENHYWFAR_CB int AB_ImporterDialog_SignalHandler(GWEN_DIALOG *dlg,
							 GWEN_DIALOG_EVENTTYPE t,
							 const char *sender);


#endif

