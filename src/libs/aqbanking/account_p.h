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


#ifndef AQBANKING_ACCOUNT_P_H
#define AQBANKING_ACCOUNT_P_H


#include "account_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc2.h>
#include <gwenhywfar/db.h>


struct AB_ACCOUNT {
  GWEN_INHERIT_ELEMENT(AB_ACCOUNT)
  GWEN_LIST_ELEMENT(AB_ACCOUNT)
  GWEN_TYPE_UINT32 usage;
  GWEN_TYPE_UINT32 uniqueId;
  AB_ACCOUNT_TYPE accountType;

  AB_BANKING *banking;
  AB_PROVIDER *provider;
  char *providerName;
  GWEN_DB_NODE *data;
  int availability;
};

AB_ACCOUNT *AB_Account__freeAll_cb(AB_ACCOUNT *a, void *userData);



#endif /* AQBANKING_ACCOUNT_P_H */
