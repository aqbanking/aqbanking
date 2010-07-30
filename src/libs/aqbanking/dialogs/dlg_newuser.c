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


#include "dlg_newuser_p.h"


GWEN_INHERIT(GWEN_DIALOG, AB_NEWUSER_DIALOG)



GWEN_DIALOG *AB_NewUserDialog_new(AB_BANKING *ab, const char *dname) {
  GWEN_DIALOG *dlg;
  AB_NEWUSER_DIALOG *xdlg;

  dlg=GWEN_Dialog_new(dname);
  GWEN_NEW_OBJECT(AB_NEWUSER_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AB_NEWUSER_DIALOG, dlg, xdlg,
		       AB_NewUserDialog_FreeData);

  xdlg->banking=ab;
  return dlg;
}



void GWENHYWFAR_CB AB_NewUserDialog_FreeData(void *bp, void *p) {
  AB_NEWUSER_DIALOG *xdlg;

  xdlg=(AB_NEWUSER_DIALOG*) p;
  GWEN_FREE_OBJECT(xdlg);
}



AB_USER *AB_NewUserDialog_GetUser(const GWEN_DIALOG *dlg) {
  AB_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->user;
}



void AB_NewUserDialog_SetUser(GWEN_DIALOG *dlg, AB_USER *u) {
  AB_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  xdlg->user=u;
}



AB_BANKING *AB_NewUserDialog_GetBanking(const GWEN_DIALOG *dlg) {
  AB_NEWUSER_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_NEWUSER_DIALOG, dlg);
  assert(xdlg);

  return xdlg->banking;
}







