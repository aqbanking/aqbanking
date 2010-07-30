/***************************************************************************
 begin       : Fri Jul 30 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "dlg_usertype_page_p.h"


GWEN_INHERIT(GWEN_DIALOG, AB_USERTYPE_PAGE_DIALOG)



GWEN_DIALOG *AB_UserTypePageDialog_new(AB_BANKING *ab, const char *dname) {
  GWEN_DIALOG *dlg;
  AB_USERTYPE_PAGE_DIALOG *xdlg;

  dlg=GWEN_Dialog_new(dname);
  GWEN_NEW_OBJECT(AB_USERTYPE_PAGE_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AB_USERTYPE_PAGE_DIALOG, dlg, xdlg,
		       AB_UserTypePageDialog_FreeData);

  xdlg->banking=ab;
  return dlg;
}



void GWENHYWFAR_CB AB_UserTypePageDialog_FreeData(void *bp, void *p) {
  AB_USERTYPE_PAGE_DIALOG *xdlg;

  xdlg=(AB_USERTYPE_PAGE_DIALOG*) p;
  GWEN_FREE_OBJECT(xdlg);
}



AB_BANKING *AB_UserTypePageDialog_GetBanking(const GWEN_DIALOG *dlg) {
  AB_USERTYPE_PAGE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_USERTYPE_PAGE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->banking;
}



int AB_UserTypePageDialog_GetSelectedType(const GWEN_DIALOG *dlg) {
  AB_USERTYPE_PAGE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_USERTYPE_PAGE_DIALOG, dlg);
  assert(xdlg);

  return xdlg->selectedType;
}



void AB_UserTypePageDialog_SetSelectedType(GWEN_DIALOG *dlg, int t) {
  AB_USERTYPE_PAGE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_USERTYPE_PAGE_DIALOG, dlg);
  assert(xdlg);

  xdlg->selectedType=t;
}




