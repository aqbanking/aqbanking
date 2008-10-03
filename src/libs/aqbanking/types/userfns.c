/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "userfns.h"
#include "user_p.h"
#include <aqbanking/banking_be.h>



AB_USER *AB_User_new(AB_BANKING *ab) {
  AB_USER *u;
  assert(ab);

  u=AB_User__new();
  AB_User_SetBanking(u, ab);

  return u;
}



AB_USER *AB_User_fromDb(AB_BANKING *ab, GWEN_DB_NODE *db) {
  AB_USER *u;
  assert(ab);

  u=AB_User_new(ab);
  AB_User_SetBanking(u, ab);
  AB_User_ReadDb(u, db);
  return u;
}



AB_PROVIDER *AB_User_GetProvider(const AB_USER *u) {
  const char *s;
  AB_PROVIDER *pro;

  s=AB_User_GetBackendName(u);
  assert(s);

  pro=AB_Banking_GetProvider(u->banking, s);
  assert(pro);

  return pro;
}



int AB_User__dbToDb(GWEN_DB_NODE *n, GWEN_DB_NODE *where) {
  return GWEN_DB_AddGroupChildren(where, n);
}





