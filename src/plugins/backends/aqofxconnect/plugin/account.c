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
		       AB_PROVIDER_EXTEND_MODE em,
		       GWEN_DB_NODE *db) {
  AO_ACCOUNT *ae;
  assert(a);

  if (em==AB_ProviderExtendMode_Create ||
      em==AB_ProviderExtendMode_Extend) {

    GWEN_NEW_OBJECT(AO_ACCOUNT, ae);
    GWEN_INHERIT_SETDATA(AB_ACCOUNT, AO_ACCOUNT, a, ae,
			 AO_Account_freeData);

    if (em==AB_ProviderExtendMode_Create) {
      /* setup defaults */
      ae->maxPurposeLines=1;
      ae->debitAllowed=0;
    }
    else {
      ae->maxPurposeLines=GWEN_DB_GetIntValue(db, "maxPurposeLines", 0, 1);
      ae->debitAllowed=GWEN_DB_GetIntValue(db, "debitAllowed", 0, 1);
    }

  }
  else if (em==AB_ProviderExtendMode_Reload) {
    ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AO_ACCOUNT, a);
    assert(ae);
    ae->maxPurposeLines=GWEN_DB_GetIntValue(db, "maxPurposeLines", 0, 1);
    ae->debitAllowed=GWEN_DB_GetIntValue(db, "debitAllowed", 0, 1);
  }
  else if (em==AB_ProviderExtendMode_Save) {
    ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AO_ACCOUNT, a);
    assert(ae);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"maxPurposeLines", ae->maxPurposeLines);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "debitAllowed", ae->debitAllowed);
  }
}



void GWENHYWFAR_CB AO_Account_freeData(void *bp, void *p) {
  AO_ACCOUNT *ae;

  ae=(AO_ACCOUNT*) p;

  GWEN_FREE_OBJECT(ae);
}



int AO_Account_GetMaxPurposeLines(const AB_ACCOUNT *a){
  AO_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AO_ACCOUNT, a);
  assert(ae);

  return ae->maxPurposeLines;
}



void AO_Account_SetMaxPurposeLines(AB_ACCOUNT *a, int i){
  AO_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AO_ACCOUNT, a);
  assert(ae);

  ae->maxPurposeLines=i;
}



int AO_Account_GetDebitAllowed(const AB_ACCOUNT *a){
  AO_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AO_ACCOUNT, a);
  assert(ae);

  return ae->debitAllowed;
}



void AO_Account_SetDebitAllowed(AB_ACCOUNT *a, int i){
  AO_ACCOUNT *ae;

  assert(a);
  ae=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AO_ACCOUNT, a);
  assert(ae);

  ae->debitAllowed=i;
}




