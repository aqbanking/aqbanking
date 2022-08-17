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


#endif

