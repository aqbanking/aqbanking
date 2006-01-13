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


#define AH_IMEXPORTER_TRANSFORM_MAXLEVEL 16

#include "imexporter_l.h"
#include "account_l.h"
#include "transaction_l.h"
#include "accstatus_l.h"

#include <gwenhywfar/misc.h>


GWEN_LIST_FUNCTION_LIB_DEFS(AB_IMEXPORTER_ACCOUNTINFO,
                            AB_ImExporterAccountInfo,
                            AQBANKING_API)


struct AB_IMEXPORTER {
  GWEN_LIST_ELEMENT(AB_IMEXPORTER);
  GWEN_INHERIT_ELEMENT(AB_IMEXPORTER);

  AB_BANKING *banking;
  char *name;

  GWEN_LIBLOADER *libLoader;
  AB_IMEXPORTER_IMPORT_FN importFn;
  AB_IMEXPORTER_EXPORT_FN exportFn;
  AB_IMEXPORTER_CHECKFILE_FN checkFileFn;
};


struct AB_IMEXPORTER_CONTEXT {
  AB_IMEXPORTER_ACCOUNTINFO_LIST *accountInfoList;
  AB_IMEXPORTER_ACCOUNTINFO *nextAccountInfo;

};


struct AB_IMEXPORTER_ACCOUNTINFO {
  GWEN_LIST_ELEMENT(AB_IMEXPORTER_ACCOUNTINFO);
  /*AB_ACCOUNT *account;*/
  char *bankCode;
  char *bankName;
  char *accountNumber;
  char *accountName;
  char *owner;
  char *description;
  AB_ACCOUNT_TYPE accountType;
  AB_TRANSACTION_LIST *transactions;
  AB_TRANSACTION *nextTransaction;
  AB_ACCOUNT_STATUS_LIST *accStatusList;
  AB_ACCOUNT_STATUS *nextAccountStatus;

  AB_TRANSACTION_LIST *standingOrders;
  AB_TRANSACTION *nextStandingOrder;

  AB_TRANSACTION_LIST *datedTransfers;
  AB_TRANSACTION *nextDatedTransfer;

  AB_TRANSACTION_LIST *transfers;
  AB_TRANSACTION *nextTransfer;

};


static int AH_ImExporter__Transform_Var(GWEN_DB_NODE *db, int level);
static int AH_ImExporter__Transform_Group(GWEN_DB_NODE *db, int level);




#endif /* AQBANKING_IMEXPORTER_P_H */




