/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "account_p.h"


#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT(AB_ACCOUNT, AO_ACCOUNT)


void AO_Account_Extend(AB_ACCOUNT *a, AB_PROVIDER *pro,
                       AB_PROVIDER_EXTEND_MODE em) {
}



int AO_Account_GetMaxPurposeLines(const AB_ACCOUNT *a){
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);

  return GWEN_DB_GetIntValue(db, "maxPurposeLines", 0, 1);
}



void AO_Account_SetMaxPurposeLines(AB_ACCOUNT *a, int i){
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "maxPurposeLines", i);
}



int AO_Account_GetDebitAllowed(const AB_ACCOUNT *a){
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);

  return GWEN_DB_GetIntValue(db, "debitAllowed", 0, 1);
}



void AO_Account_SetDebitAllowed(AB_ACCOUNT *a, int i){
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "debitAllowed", i);
}




