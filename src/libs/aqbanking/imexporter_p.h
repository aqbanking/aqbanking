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


#ifndef AQBANKING_IMEXPORTER_P_H
#define AQBANKING_IMEXPORTER_P_H


#include "imexporter_l.h"
#include "account_l.h"
#include "transaction_l.h"

#include <gwenhywfar/misc.h>


struct AB_IMEXPORTER {
  GWEN_LIST_ELEMENT(AB_IMEXPORTER);
  GWEN_INHERIT_ELEMENT(AB_IMEXPORTER);

  AB_BANKING *banking;
  char *name;

  GWEN_LIBLOADER *libLoader;
  AB_IMEXPORTER_IMPORT_FN importFn;
};


struct AB_IMEXPORTER_CONTEXT {
  AB_ACCOUNT_LIST *accounts;
  AB_ACCOUNT *nextAccount;
  AB_TRANSACTION_LIST *transactions;
  AB_TRANSACTION *nextTransaction;

};



#endif /* AQBANKING_IMEXPORTER_P_H */


