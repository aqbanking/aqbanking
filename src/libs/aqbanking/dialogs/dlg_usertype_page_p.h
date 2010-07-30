/***************************************************************************
 begin       : Wed Apr 14 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AB_DLG_USERTYPE_PAGE_P_H
#define AB_DLG_USERTYPE_PAGE_P_H


#include "dlg_usertype_page_be.h"



typedef struct AB_USERTYPE_PAGE_DIALOG AB_USERTYPE_PAGE_DIALOG;
struct AB_USERTYPE_PAGE_DIALOG {
  AB_BANKING *banking;
  int selectedType;
};


static void GWENHYWFAR_CB AB_UserTypePageDialog_FreeData(void *bp, void *p);





#endif

