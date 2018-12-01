/***************************************************************************
    begin       : Sat Dec 01 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/* included from provider.c */




GWEN_DIALOG *APY_Provider_GetNewUserDialog(AB_PROVIDER *pro, int i) {
  APY_PROVIDER *xp;
  GWEN_DIALOG *dlg;

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(xp);

  dlg=APY_NewUserDialog_new(AB_Provider_GetBanking(pro));
  if (dlg==NULL) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (no dialog)");
    return NULL;
  }

  return dlg;
}



GWEN_DIALOG *APY_Provider_GetEditUserDialog(AB_PROVIDER *pro, AB_USER *u) {
  APY_PROVIDER *xp;
  GWEN_DIALOG *dlg;

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(xp);

  dlg=APY_EditUserDialog_new(AB_Provider_GetBanking(pro), u, 1);
  if (dlg==NULL) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (no dialog)");
    return NULL;
  }

  return dlg;
}

