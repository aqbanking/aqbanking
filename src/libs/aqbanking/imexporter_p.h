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


GWEN_LIST_FUNCTION_DEFS(AB_IMEXPORTER_DAY, AB_ImExporterDay)

struct AB_IMEXPORTER_DAY {
  GWEN_LIST_ELEMENT(AB_IMEXPORTER_DAY);
  GWEN_TIME *date;
  AB_TRANSACTION_LIST *transactions;
  AB_TRANSACTION *nextTransaction;
};
AB_IMEXPORTER_DAY *AB_ImExporterDay_new(const GWEN_TIME *ti);
void AB_ImExporterDay_free(AB_IMEXPORTER_DAY *ied);
/**
 * Takes over ownership of the given transaction.
 */
void AB_ImExporterDay_AddTransaction(AB_IMEXPORTER_DAY *ied,
                                     AB_TRANSACTION *t);


struct AB_IMEXPORTER_CONTEXT {
  AB_IMEXPORTER_DAY_LIST *days;
  AB_IMEXPORTER_DAY *nextDay;
  AB_ACCOUNT_LIST *accounts;
  AB_ACCOUNT *nextAccount;
};
/**
 * This function keeps the ownership of the object returned (if any).
 */
AB_IMEXPORTER_DAY *AB_ImExporterContext_GetDay(AB_IMEXPORTER_CONTEXT *iec,
                                               const GWEN_TIME *ti,
                                               int crea);



#endif /* AQBANKING_IMEXPORTER_P_H */


