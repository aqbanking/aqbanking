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
  uint32_t flags;

  GWEN_LIBLOADER *libLoader;
  AB_IMEXPORTER_IMPORT_FN importFn;
  AB_IMEXPORTER_EXPORT_FN exportFn;
  AB_IMEXPORTER_CHECKFILE_FN checkFileFn;
  AB_IMEXPORTER_GET_EDITPROFILE_DIALOG_FN getEditProfileDialogFn;
};


struct AB_IMEXPORTER_CONTEXT {
  AB_IMEXPORTER_ACCOUNTINFO_LIST *accountInfoList;
  AB_IMEXPORTER_ACCOUNTINFO *nextAccountInfo;

  AB_SECURITY_LIST *securityList;
  AB_SECURITY *nextSecurity;

  AB_MESSAGE_LIST *messageList;
  AB_MESSAGE *nextMessage;

  GWEN_BUFFER *logs;
};


struct AB_IMEXPORTER_ACCOUNTINFO {
  GWEN_LIST_ELEMENT(AB_IMEXPORTER_ACCOUNTINFO);
  /*AB_ACCOUNT *account;*/
  char *bankCode;
  char *bankName;
  char *accountNumber;
  char *accountName;
  char *iban;
  char *bic;
  char *owner;
  char *currency;
  char *description;
  AB_ACCOUNT_TYPE accountType;
  uint32_t accountId;

  AB_TRANSACTION_LIST *transactions;
  AB_TRANSACTION *nextTransaction;

  AB_ACCOUNT_STATUS_LIST *accStatusList;
  AB_ACCOUNT_STATUS *nextAccountStatus;

  AB_TRANSACTION_LIST *standingOrders;
  AB_TRANSACTION *nextStandingOrder;

  AB_TRANSACTION_LIST *datedTransfers;
  AB_TRANSACTION *nextDatedTransfer;

  AB_TRANSACTION_LIST *notedTransactions;
  AB_TRANSACTION *nextNotedTransaction;

  AB_TRANSACTION_LIST *transfers;
  AB_TRANSACTION *nextTransfer;

};


static int AH_ImExporter__Transform_Var(GWEN_DB_NODE *db, int level);
static int AH_ImExporter__Transform_Group(GWEN_DB_NODE *db, int level);

static AB_IMEXPORTER_ACCOUNTINFO *AB_ImExporterContext__GetAccountInfoForTransaction(AB_IMEXPORTER_CONTEXT *iec,
                                                                                     const AB_TRANSACTION *t);



typedef struct AB_PLUGIN_IMEXPORTER AB_PLUGIN_IMEXPORTER;
struct AB_PLUGIN_IMEXPORTER {
  AB_PLUGIN_IMEXPORTER_FACTORY_FN pluginFactoryFn;
};

static void GWENHYWFAR_CB AB_Plugin_ImExporter_FreeData(void *bp, void *p);


#endif /* AQBANKING_IMEXPORTER_P_H */




