/***************************************************************************
 begin       : Wed Apr 14 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AB_DLG_NEWUSER_P_H
#define AB_DLG_NEWUSER_P_H


#include "dlg_newuser_be.h"



typedef struct AB_NEWUSER_DIALOG AB_NEWUSER_DIALOG;
struct AB_NEWUSER_DIALOG {
  AB_BANKING *banking;
  AB_USER *user;
};


static void GWENHYWFAR_CB AB_NewUserDialog_FreeData(void *bp, void *p);





#endif

