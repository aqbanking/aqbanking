/***************************************************************************
    begin       : Sat Jan 13 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AB_CSV_EDITPROFILE_L_H
#define AB_CSV_EDITPROFILE_L_H


#include <aqbanking/imexporter_be.h>

#include <gwenhywfar/dialog.h>


GWEN_DIALOG *AB_CSV_EditProfileDialog_new(AB_IMEXPORTER *ie,
					  GWEN_DB_NODE *params,
					  const char *testFileName);




#endif

