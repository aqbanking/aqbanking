/***************************************************************************
    begin       : Sat Jan 13 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AB_CSV_EDITPROFILE_P_H
#define AB_CSV_EDITPROFILE_P_H


#include "csv_editprofile_l.h"



struct AB_CSV_EDIT_PROFILE_DIALOG {
  AB_BANKING *banking;
  AB_IMEXPORTER *imExporter;
  const char *testFileName;
  GWEN_DB_NODE *dbProfile;

  GWEN_BUFFER *dataBuffer;
  GWEN_STRINGLIST *columns;

};
typedef struct AB_CSV_EDIT_PROFILE_DIALOG AB_CSV_EDIT_PROFILE_DIALOG;


static GWENHYWFAR_CB void AB_CSV_EditProfileDialog_FreeData(void *bp, void *p);


static GWENHYWFAR_CB
int AB_CSV_EditProfileDialog_SignalHandler(GWEN_DIALOG *dlg,
					   GWEN_DIALOG_EVENTTYPE t,
					   const char *sender);


#endif


