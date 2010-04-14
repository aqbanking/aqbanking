/***************************************************************************
 begin       : Wed Apr 14 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "dlg_newaccount_p.h"


GWEN_INHERIT(GWEN_DIALOG, AB_NEWACCOUNT_DIALOG)



GWEN_DIALOG *AB_NewAccountDialog_new(AB_BANKING *ab, const char *dname) {
  GWEN_DIALOG *dlg;
  AB_NEWACCOUNT_DIALOG *xdlg;

  dlg=GWEN_Dialog_new(dname);
  GWEN_NEW_OBJECT(AB_NEWACCOUNT_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AB_NEWACCOUNT_DIALOG, dlg, xdlg,
		       AB_NewAccountDialog_FreeData);

  return dlg;
}



void GWENHYWFAR_CB AB_NewAccountDialog_FreeData(void *bp, void *p) {
  AB_NEWACCOUNT_DIALOG *xdlg;

  xdlg=(AB_NEWACCOUNT_DIALOG*) p;
  GWEN_FREE_OBJECT(xdlg);
}



AB_ACCOUNT *AB_NewAccountDialog_GetAccount(const GWEN_DIALOG *dlg) {
  AB_NEWACCOUNT_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_NEWACCOUNT_DIALOG, dlg);
  assert(dlg);

  return xdlg->account;
}



void AB_NewAccountDialog_SetAccount(GWEN_DIALOG *dlg, AB_ACCOUNT *a) {
  AB_NEWACCOUNT_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_NEWACCOUNT_DIALOG, dlg);
  assert(dlg);

  xdlg->account=a;
}






