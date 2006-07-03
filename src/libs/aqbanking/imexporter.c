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

#include "imexporter_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>


GWEN_INHERIT_FUNCTIONS(AB_IMEXPORTER)
GWEN_LIST_FUNCTIONS(AB_IMEXPORTER, AB_ImExporter)
GWEN_LIST_FUNCTIONS(AB_IMEXPORTER_ACCOUNTINFO, AB_ImExporterAccountInfo)



AB_IMEXPORTER *AB_ImExporter_new(AB_BANKING *ab, const char *name){
  AB_IMEXPORTER *ie;

  assert(ab);
  assert(name);
  GWEN_NEW_OBJECT(AB_IMEXPORTER, ie);
  GWEN_LIST_INIT(AB_IMEXPORTER, ie);
  GWEN_INHERIT_INIT(AB_IMEXPORTER, ie);

  ie->banking=ab;
  ie->name=strdup(name);

  return ie;
}


void AB_ImExporter_free(AB_IMEXPORTER *ie){
  if (ie) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Destroying AB_IMEXPORTER");
    GWEN_INHERIT_FINI(AB_IMEXPORTER, ie);
    if (ie->libLoader) {
      GWEN_LibLoader_CloseLibrary(ie->libLoader);
      GWEN_LibLoader_free(ie->libLoader);
    }
    free(ie->name);
    GWEN_LIST_FINI(AB_IMEXPORTER, ie);
    GWEN_FREE_OBJECT(ie);
  }
}



int AB_ImExporter_Import(AB_IMEXPORTER *ie,
                         AB_IMEXPORTER_CONTEXT *ctx,
                         GWEN_BUFFEREDIO *bio,
                         GWEN_DB_NODE *params){
  assert(ie);
  assert(ctx);
  assert(bio);
  assert(params);

  if (ie->importFn)
    return ie->importFn(ie, ctx, bio, params);
  else
    return AB_ERROR_NOT_SUPPORTED;
}



int AB_ImExporter_Export(AB_IMEXPORTER *ie,
                         AB_IMEXPORTER_CONTEXT *ctx,
                         GWEN_BUFFEREDIO *bio,
                         GWEN_DB_NODE *params){
  assert(ie);
  assert(ctx);
  assert(bio);
  assert(params);

  if (ie->exportFn)
    return ie->exportFn(ie, ctx, bio, params);
  else
    return AB_ERROR_NOT_SUPPORTED;
}



int AB_ImExporter_CheckFile(AB_IMEXPORTER *ie,
                            const char *fname){
  assert(ie);
  assert(fname);

  if (ie->checkFileFn)
    return ie->checkFileFn(ie, fname);
  else
    return AB_ERROR_NOT_SUPPORTED;
}



int AB_ImExporter_ImportFile(AB_IMEXPORTER *ie,
                             AB_IMEXPORTER_CONTEXT *ctx,
                             const char *fname,
                             GWEN_DB_NODE *dbProfile){
  int fd;
  GWEN_BUFFEREDIO *bio;
  int rv;

  assert(ie);
  assert(ctx);
  assert(fname);
  assert(dbProfile);

  fd=open(fname, O_RDONLY);
  if (fd==-1) {
    /* error */
    DBG_ERROR(AQBANKING_LOGDOMAIN, "open(%s): %s", fname, strerror(errno));
    return AB_ERROR_NOT_FOUND;
  }

  bio=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);
  rv=AB_ImExporter_Import(ie, ctx, bio, dbProfile);
  GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);

  return rv;
}



void AB_ImExporter_SetImportFn(AB_IMEXPORTER *ie,
                               AB_IMEXPORTER_IMPORT_FN f){
  assert(ie);
  ie->importFn=f;
}



void AB_ImExporter_SetExportFn(AB_IMEXPORTER *ie,
                               AB_IMEXPORTER_EXPORT_FN f){
  assert(ie);
  ie->exportFn=f;
}



void AB_ImExporter_SetCheckFileFn(AB_IMEXPORTER *ie,
                                  AB_IMEXPORTER_CHECKFILE_FN f){
  assert(ie);
  ie->checkFileFn=f;
}



AB_BANKING *AB_ImExporter_GetBanking(const AB_IMEXPORTER *ie){
  assert(ie);
  return ie->banking;
}



const char *AB_ImExporter_GetName(const AB_IMEXPORTER *ie){
  assert(ie);
  return ie->name;
}



void AB_ImExporter_SetLibLoader(AB_IMEXPORTER *ie, GWEN_LIBLOADER *ll) {
  assert(ie);
  ie->libLoader=ll;
}



AB_IMEXPORTER_ACCOUNTINFO*
AB_ImExporterContext_GetFirstAccountInfo(AB_IMEXPORTER_CONTEXT *iec){
  AB_IMEXPORTER_ACCOUNTINFO *ai;

  assert(iec);
  ai=AB_ImExporterAccountInfo_List_First(iec->accountInfoList);
  if (ai) {
    iec->nextAccountInfo=AB_ImExporterAccountInfo_List_Next(ai);
    return ai;
  }
  iec->nextAccountInfo=0;
  return 0;
}



AB_IMEXPORTER_ACCOUNTINFO*
AB_ImExporterContext_GetNextAccountInfo(AB_IMEXPORTER_CONTEXT *iec){
  AB_IMEXPORTER_ACCOUNTINFO *ai;

  assert(iec);
  ai=iec->nextAccountInfo;
  if (ai) {
    iec->nextAccountInfo=AB_ImExporterAccountInfo_List_Next(ai);
    return ai;
  }
  iec->nextAccountInfo=0;
  return 0;
}

AB_IMEXPORTER_ACCOUNTINFO *
AB_ImExporterContext_AccountInfoForEach(AB_IMEXPORTER_CONTEXT *iec,
					AB_IMEXPORTER_ACCOUNTINFO_LIST2_FOREACH func,
					void* user_data) {
  /* If the accountInfoList were a LIST2, then we would write: */
  /* return AB_ImExporterAccountInfo_List2_ForEach(iec->accountInfoList, func, user_data); */
  
  AB_IMEXPORTER_ACCOUNTINFO *it;
  AB_IMEXPORTER_ACCOUNTINFO *retval;
  assert(iec);

  it = AB_ImExporterAccountInfo_List_First(iec->accountInfoList);
  while (it) {
    retval = func(it, user_data);
    if (retval) {
      return retval;
    }
    it = AB_ImExporterAccountInfo_List_Next(it);
  }
  return 0;

}






AB_IMEXPORTER_ACCOUNTINFO *AB_ImExporterAccountInfo_new() {
  AB_IMEXPORTER_ACCOUNTINFO *iea;

  GWEN_NEW_OBJECT(AB_IMEXPORTER_ACCOUNTINFO, iea);
  GWEN_LIST_INIT(AB_IMEXPORTER_ACCOUNTINFO, iea);
  iea->transactions=AB_Transaction_List_new();
  iea->standingOrders=AB_Transaction_List_new();
  iea->accStatusList=AB_AccountStatus_List_new();
  iea->transfers=AB_Transaction_List_new();
  iea->datedTransfers=AB_Transaction_List_new();
  iea->notedTransactions=AB_Transaction_List_new();
  return iea;
}



void AB_ImExporterAccountInfo_free(AB_IMEXPORTER_ACCOUNTINFO *iea){
  if (iea) {
    free(iea->bankCode);
    free(iea->bankName);
    free(iea->accountNumber);
    free(iea->accountName);
    free(iea->iban);
    free(iea->owner);
    free(iea->currency);
    free(iea->description);
    AB_Transaction_List_free(iea->notedTransactions);
    AB_Transaction_List_free(iea->datedTransfers);
    AB_Transaction_List_free(iea->transfers);
    AB_Transaction_List_free(iea->standingOrders);
    AB_Transaction_List_free(iea->transactions);
    AB_AccountStatus_List_free(iea->accStatusList);
    GWEN_LIST_FINI(AB_IMEXPORTER_ACCOUNTINFO, iea);
    GWEN_FREE_OBJECT(iea);
  }
}



AB_IMEXPORTER_ACCOUNTINFO*
AB_ImExporterAccountInfo_dup(const AB_IMEXPORTER_ACCOUNTINFO *oi) {
  AB_IMEXPORTER_ACCOUNTINFO *iea;

  GWEN_NEW_OBJECT(AB_IMEXPORTER_ACCOUNTINFO, iea);
  GWEN_LIST_INIT(AB_IMEXPORTER_ACCOUNTINFO, iea);

#define COPY_CHAR(NAME) \
  if (oi->NAME) \
    iea->NAME=strdup(oi->NAME);
  COPY_CHAR(bankCode);
  COPY_CHAR(bankName);
  COPY_CHAR(accountNumber);
  COPY_CHAR(accountName);
  COPY_CHAR(iban);
  COPY_CHAR(owner);
  COPY_CHAR(currency);
  COPY_CHAR(description);
  iea->accountType=oi->accountType;
#undef COPY_CHAR

  iea->accStatusList=AB_AccountStatus_List_dup(oi->accStatusList);
  iea->transactions=AB_Transaction_List_dup(oi->transactions);
  iea->standingOrders=AB_Transaction_List_dup(oi->standingOrders);
  iea->transfers=AB_Transaction_List_dup(oi->transfers);
  iea->datedTransfers=AB_Transaction_List_dup(oi->datedTransfers);
  iea->notedTransactions=AB_Transaction_List_dup(oi->notedTransactions);
  return iea;
}



int AB_ImExporterAccountInfo_toDb(const AB_IMEXPORTER_ACCOUNTINFO *iea,
				  GWEN_DB_NODE *db){
  assert(iea);

#define STORE_CHAR(NAME) \
  if (iea->NAME) \
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, \
                         __STRING(NAME), iea->NAME)
#define STORE_INT(NAME) \
  if (iea->NAME) \
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, \
                        __STRING(NAME), iea->NAME)
  STORE_CHAR(bankCode);
  STORE_CHAR(bankName);
  STORE_CHAR(accountNumber);
  STORE_CHAR(accountName);
  STORE_CHAR(iban);
  STORE_CHAR(owner);
  STORE_CHAR(currency);
  STORE_CHAR(description);
  STORE_INT(accountType);
#undef STORE_CHAR
#undef STORE_INT

  if (iea->accStatusList) {
    AB_ACCOUNT_STATUS *ast;

    ast=AB_AccountStatus_List_First(iea->accStatusList);
    if (ast) {
      GWEN_DB_NODE *dbG;

      dbG=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
			   "statusList");
      assert(dbG);

      while(ast) {
	GWEN_DB_NODE *dbT;

	dbT=GWEN_DB_GetGroup(dbG, GWEN_PATH_FLAGS_CREATE_GROUP,
			     "status");
	assert(dbT);
	if (AB_AccountStatus_toDb(ast, dbT))
	  return -1;

	ast=AB_AccountStatus_List_Next(ast);
      }
    }
  }

  if (iea->transactions) {
    AB_TRANSACTION *t;

    t=AB_Transaction_List_First(iea->transactions);
    if (t) {
      GWEN_DB_NODE *dbG;

      dbG=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
			   "transactionList");
      assert(dbG);

      while(t) {
	GWEN_DB_NODE *dbT;

	dbT=GWEN_DB_GetGroup(dbG, GWEN_PATH_FLAGS_CREATE_GROUP,
			     "transaction");
	assert(dbT);
	if (AB_Transaction_toDb(t, dbT))
	  return -1;
	t=AB_Transaction_List_Next(t);
      }
    }
  }

  if (iea->standingOrders) {
    AB_TRANSACTION *t;

    t=AB_Transaction_List_First(iea->standingOrders);
    if (t) {
      GWEN_DB_NODE *dbG;

      dbG=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
			   "standingOrderList");
      assert(dbG);

      while(t) {
	GWEN_DB_NODE *dbT;

	dbT=GWEN_DB_GetGroup(dbG, GWEN_PATH_FLAGS_CREATE_GROUP,
			     "standingOrder");
	assert(dbT);
	if (AB_Transaction_toDb(t, dbT))
	  return -1;
	t=AB_Transaction_List_Next(t);
      }
    }
  }

  if (iea->transfers) {
    AB_TRANSACTION *t;

    t=AB_Transaction_List_First(iea->transfers);
    if (t) {
      GWEN_DB_NODE *dbG;

      dbG=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
			   "transferList");
      assert(dbG);

      while(t) {
	GWEN_DB_NODE *dbT;

	dbT=GWEN_DB_GetGroup(dbG, GWEN_PATH_FLAGS_CREATE_GROUP,
			     "transfer");
	assert(dbT);
	if (AB_Transaction_toDb(t, dbT))
	  return -1;
	t=AB_Transaction_List_Next(t);
      }
    }
  }

  if (iea->datedTransfers) {
    AB_TRANSACTION *t;

    t=AB_Transaction_List_First(iea->datedTransfers);
    if (t) {
      GWEN_DB_NODE *dbG;

      dbG=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
			   "datedTransferList");
      assert(dbG);

      while(t) {
	GWEN_DB_NODE *dbT;

	dbT=GWEN_DB_GetGroup(dbG, GWEN_PATH_FLAGS_CREATE_GROUP,
			     "datedTransfer");
	assert(dbT);
	if (AB_Transaction_toDb(t, dbT))
	  return -1;
	t=AB_Transaction_List_Next(t);
      }
    }
  }

  if (iea->notedTransactions) {
    AB_TRANSACTION *t;

    t=AB_Transaction_List_First(iea->notedTransactions);
    if (t) {
      GWEN_DB_NODE *dbG;

      dbG=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
			   "notedTransactionList");
      assert(dbG);

      while(t) {
	GWEN_DB_NODE *dbT;

	dbT=GWEN_DB_GetGroup(dbG, GWEN_PATH_FLAGS_CREATE_GROUP,
			     "notedTransaction");
	assert(dbT);
	if (AB_Transaction_toDb(t, dbT))
	  return -1;
	t=AB_Transaction_List_Next(t);
      }
    }
  }

  return 0;
}


AB_IMEXPORTER_ACCOUNTINFO*
AB_ImExporterAccountInfo_fromDb(GWEN_DB_NODE *db){
  AB_IMEXPORTER_ACCOUNTINFO *iea;
  const char *s;
  GWEN_DB_NODE *dbT;

  iea=AB_ImExporterAccountInfo_new();

#define RESTORE_CHAR(NAME) \
  s=GWEN_DB_GetCharValue(db, __STRING(NAME), 0, 0);\
  if (s)\
    iea->NAME=strdup(s);
#define RESTORE_INT(NAME, DEFAULT) \
  iea->NAME=GWEN_DB_GetIntValue(db, __STRING(NAME), 0, DEFAULT);
  RESTORE_CHAR(bankCode);
  RESTORE_CHAR(bankName);
  RESTORE_CHAR(accountNumber);
  RESTORE_CHAR(iban);
  RESTORE_CHAR(owner);
  RESTORE_CHAR(currency);
  RESTORE_CHAR(description);
  RESTORE_INT(accountType, AB_AccountType_Bank);
#undef RESTORE_CHAR
#undef RESTORE_INT

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		       "statusList");
  if (dbT) {
    dbT=GWEN_DB_FindFirstGroup(dbT, "status");
    while(dbT) {
      AB_ACCOUNT_STATUS *ast;

      ast=AB_AccountStatus_fromDb(dbT);
      assert(ast);
      AB_AccountStatus_List_Add(ast, iea->accStatusList);
      dbT=GWEN_DB_FindNextGroup(dbT, "status");
    }
  }

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		       "transactionList");
  if (dbT) {
    dbT=GWEN_DB_FindFirstGroup(dbT, "transaction");
    while(dbT) {
      AB_TRANSACTION *t;

      t=AB_Transaction_fromDb(dbT);
      assert(t);
      AB_Transaction_List_Add(t, iea->transactions);
      dbT=GWEN_DB_FindNextGroup(dbT, "transaction");
    }
  }

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		       "standingOrderList");
  if (dbT) {
    dbT=GWEN_DB_FindFirstGroup(dbT, "standingOrder");
    while(dbT) {
      AB_TRANSACTION *t;

      t=AB_Transaction_fromDb(dbT);
      assert(t);
      AB_Transaction_List_Add(t, iea->standingOrders);
      dbT=GWEN_DB_FindNextGroup(dbT, "standingOrder");
    }
  }

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		       "transferList");
  if (dbT) {
    dbT=GWEN_DB_FindFirstGroup(dbT, "transfer");
    while(dbT) {
      AB_TRANSACTION *t;

      t=AB_Transaction_fromDb(dbT);
      assert(t);
      AB_Transaction_List_Add(t, iea->transfers);
      dbT=GWEN_DB_FindNextGroup(dbT, "transfer");
    }
  }

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		       "datedTransferList");
  if (dbT) {
    dbT=GWEN_DB_FindFirstGroup(dbT, "datedTransfer");
    while(dbT) {
      AB_TRANSACTION *t;

      t=AB_Transaction_fromDb(dbT);
      assert(t);
      AB_Transaction_List_Add(t, iea->datedTransfers);
      dbT=GWEN_DB_FindNextGroup(dbT, "datedTransfer");
    }
  }

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		       "notedTransactionList");
  if (dbT) {
    dbT=GWEN_DB_FindFirstGroup(dbT, "notedTransaction");
    while(dbT) {
      AB_TRANSACTION *t;

      t=AB_Transaction_fromDb(dbT);
      assert(t);
      AB_Transaction_List_Add(t, iea->notedTransactions);
      dbT=GWEN_DB_FindNextGroup(dbT, "notedTransaction");
    }
  }

  return iea;
}






void AB_ImExporterAccountInfo_AddTransaction(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                             AB_TRANSACTION *t){
  assert(iea);
  assert(t);

  AB_Transaction_List_Add(t, iea->transactions);
}



const AB_TRANSACTION*
AB_ImExporterAccountInfo_GetFirstTransaction(AB_IMEXPORTER_ACCOUNTINFO *iea){
  AB_TRANSACTION *t;

  assert(iea);
  t=AB_Transaction_List_First(iea->transactions);
  if (t) {
    iea->nextTransaction=AB_Transaction_List_Next(t);
    return t;
  }
  iea->nextTransaction=0;
  return 0;
}



const AB_TRANSACTION*
AB_ImExporterAccountInfo_GetNextTransaction(AB_IMEXPORTER_ACCOUNTINFO *iea){
  AB_TRANSACTION *t;

  assert(iea);
  t=iea->nextTransaction;
  if (t) {
    iea->nextTransaction=AB_Transaction_List_Next(t);
    return t;
  }
  iea->nextTransaction=0;
  return 0;
}

const AB_TRANSACTION *
AB_ImExporterAccountInfo_TransactionsForEach(AB_IMEXPORTER_ACCOUNTINFO *iea,
					     AB_TRANSACTION_CONSTLIST2_FOREACH func,
					     void* user_data) {
  /* In theory, if the transaction list were a LIST2, then we
     would simply write: */
  /* return AB_Transaction_List2_ForEach(iea->transactions, func, user_data); */
  /* well, probably not, because the "const" wouldn't
     work. Sorry. */
  
  const AB_TRANSACTION *it;
  const AB_TRANSACTION *retval;
  assert(iea);

  it = AB_Transaction_List_First(iea->transactions);
  while (it) {
    retval = func(it, user_data);
    if (retval) {
      return retval;
    }
    it = AB_Transaction_List_Next(it);
  }
  return 0;
}


void AB_ImExporterAccountInfo_AddStandingOrder(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                               AB_TRANSACTION *t){
  assert(iea);
  assert(t);

  AB_Transaction_List_Add(t, iea->standingOrders);
}



const AB_TRANSACTION*
AB_ImExporterAccountInfo_GetFirstStandingOrder(AB_IMEXPORTER_ACCOUNTINFO *iea){
  AB_TRANSACTION *t;

  assert(iea);
  t=AB_Transaction_List_First(iea->standingOrders);
  if (t) {
    iea->nextStandingOrder=AB_Transaction_List_Next(t);
    return t;
  }
  iea->nextStandingOrder=0;
  return 0;
}



const AB_TRANSACTION*
AB_ImExporterAccountInfo_GetNextStandingOrder(AB_IMEXPORTER_ACCOUNTINFO *iea){
  AB_TRANSACTION *t;

  assert(iea);
  t=iea->nextStandingOrder;
  if (t) {
    iea->nextStandingOrder=AB_Transaction_List_Next(t);
    return t;
  }
  iea->nextStandingOrder=0;
  return 0;
}



void AB_ImExporterAccountInfo_AddDatedTransfer(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                               AB_TRANSACTION *t){
  assert(iea);
  assert(t);

  AB_Transaction_List_Add(t, iea->datedTransfers);
}



const AB_TRANSACTION*
AB_ImExporterAccountInfo_GetFirstDatedTransfer(AB_IMEXPORTER_ACCOUNTINFO *iea){
  AB_TRANSACTION *t;

  assert(iea);
  t=AB_Transaction_List_First(iea->datedTransfers);
  if (t) {
    iea->nextDatedTransfer=AB_Transaction_List_Next(t);
    return t;
  }
  iea->nextDatedTransfer=0;
  return 0;
}



const AB_TRANSACTION*
AB_ImExporterAccountInfo_GetNextDatedTransfer(AB_IMEXPORTER_ACCOUNTINFO *iea){
  AB_TRANSACTION *t;

  assert(iea);
  t=iea->nextDatedTransfer;
  if (t) {
    iea->nextDatedTransfer=AB_Transaction_List_Next(t);
    return t;
  }
  iea->nextDatedTransfer=0;
  return 0;
}



void AB_ImExporterAccountInfo_AddNotedTransaction(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                               AB_TRANSACTION *t){
  assert(iea);
  assert(t);

  AB_Transaction_List_Add(t, iea->notedTransactions);
}



const AB_TRANSACTION*
AB_ImExporterAccountInfo_GetFirstNotedTransaction(AB_IMEXPORTER_ACCOUNTINFO *iea){
  AB_TRANSACTION *t;

  assert(iea);
  t=AB_Transaction_List_First(iea->notedTransactions);
  if (t) {
    iea->nextNotedTransaction=AB_Transaction_List_Next(t);
    return t;
  }
  iea->nextNotedTransaction=0;
  return 0;
}



const AB_TRANSACTION*
AB_ImExporterAccountInfo_GetNextNotedTransaction(AB_IMEXPORTER_ACCOUNTINFO *iea){
  AB_TRANSACTION *t;

  assert(iea);
  t=iea->nextNotedTransaction;
  if (t) {
    iea->nextNotedTransaction=AB_Transaction_List_Next(t);
    return t;
  }
  iea->nextNotedTransaction=0;
  return 0;
}



void AB_ImExporterAccountInfo_AddTransfer(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                             AB_TRANSACTION *t){
  assert(iea);
  assert(t);

  AB_Transaction_List_Add(t, iea->transfers);
}



const AB_TRANSACTION*
AB_ImExporterAccountInfo_GetFirstTransfer(AB_IMEXPORTER_ACCOUNTINFO *iea){
  AB_TRANSACTION *t;

  assert(iea);
  t=AB_Transaction_List_First(iea->transfers);
  if (t) {
    iea->nextTransfer=AB_Transaction_List_Next(t);
    return t;
  }
  iea->nextTransfer=0;
  return 0;
}



const AB_TRANSACTION*
AB_ImExporterAccountInfo_GetNextTransfer(AB_IMEXPORTER_ACCOUNTINFO *iea){
  AB_TRANSACTION *t;

  assert(iea);
  t=iea->nextTransfer;
  if (t) {
    iea->nextTransfer=AB_Transaction_List_Next(t);
    return t;
  }
  iea->nextTransfer=0;
  return 0;
}



const char*
AB_ImExporterAccountInfo_GetBankCode(const AB_IMEXPORTER_ACCOUNTINFO *iea){
  assert(iea);
  return iea->bankCode;
}



void AB_ImExporterAccountInfo_SetBankCode(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                          const char *s){
  assert(iea);
  free(iea->bankCode);
  if (s) iea->bankCode=strdup(s);
  else iea->bankCode=0;
}



const char*
AB_ImExporterAccountInfo_GetBankName(const AB_IMEXPORTER_ACCOUNTINFO *iea){
  assert(iea);
  return iea->bankName;
}



void AB_ImExporterAccountInfo_SetBankName(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                          const char *s){
  assert(iea);
  free(iea->bankName);
  if (s) iea->bankName=strdup(s);
  else iea->bankName=0;
}



const char*
AB_ImExporterAccountInfo_GetAccountNumber(const AB_IMEXPORTER_ACCOUNTINFO *iea){
  assert(iea);
  return iea->accountNumber;
}



void AB_ImExporterAccountInfo_SetAccountNumber(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                               const char *s){
  assert(iea);
  free(iea->accountNumber);
  if (s) iea->accountNumber=strdup(s);
  else iea->accountNumber=0;
}



const char*
AB_ImExporterAccountInfo_GetIban(const AB_IMEXPORTER_ACCOUNTINFO *iea) {
  assert(iea);
  return iea->iban;
}



void AB_ImExporterAccountInfo_SetIban(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                      const char *s) {
  assert(iea);
  free(iea->iban);
  if (s) iea->iban=strdup(s);
  else iea->iban=0;
}



const char*
AB_ImExporterAccountInfo_GetCurrency(const AB_IMEXPORTER_ACCOUNTINFO *iea) {
  assert(iea);
  return iea->currency;
}



void AB_ImExporterAccountInfo_SetCurrency(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                          const char *s) {
  assert(iea);
  free(iea->currency);
  if (s) iea->currency=strdup(s);
  else iea->currency=0;
}



const char*
AB_ImExporterAccountInfo_GetAccountName(const AB_IMEXPORTER_ACCOUNTINFO *iea){
  assert(iea);
  return iea->accountName;
}



void AB_ImExporterAccountInfo_SetAccountName(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                             const char *s){
  assert(iea);
  free(iea->accountName);
  if (s) iea->accountName=strdup(s);
  else iea->accountName=0;
}



const char*
AB_ImExporterAccountInfo_GetOwner(const AB_IMEXPORTER_ACCOUNTINFO *iea){
  assert(iea);
  return iea->owner;
}



void AB_ImExporterAccountInfo_SetOwner(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                       const char *s){
  assert(iea);
  free(iea->owner);
  if (s) iea->owner=strdup(s);
  else iea->owner=0;
}


AB_ACCOUNT_TYPE
AB_ImExporterAccountInfo_GetType(const AB_IMEXPORTER_ACCOUNTINFO *iea){
  assert(iea);
  return iea->accountType;
}



void AB_ImExporterAccountInfo_SetType(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                      AB_ACCOUNT_TYPE t){
  assert(iea);
  iea->accountType=t;
}



const char*
AB_ImExporterAccountInfo_GetDescription(const AB_IMEXPORTER_ACCOUNTINFO *iea){
  assert(iea);
  return iea->description;
}



void AB_ImExporterAccountInfo_SetDescription(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                             const char *s){
  assert(iea);
  free(iea->description);
  if (s) iea->description=strdup(s);
  else iea->description=0;
}



void AB_ImExporterAccountInfo_AddAccountStatus(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                             AB_ACCOUNT_STATUS *t){
  assert(iea);
  assert(t);
  AB_AccountStatus_List_Add(t, iea->accStatusList);
}



AB_ACCOUNT_STATUS*
AB_ImExporterAccountInfo_GetFirstAccountStatus(AB_IMEXPORTER_ACCOUNTINFO *iea){
  AB_ACCOUNT_STATUS *t;

  assert(iea);
  t=AB_AccountStatus_List_First(iea->accStatusList);
  if (t) {
    iea->nextAccountStatus=AB_AccountStatus_List_Next(t);
    return t;
  }
  iea->nextAccountStatus=0;
  return 0;
}



AB_ACCOUNT_STATUS*
AB_ImExporterAccountInfo_GetNextAccountStatus(AB_IMEXPORTER_ACCOUNTINFO *iea){
  AB_ACCOUNT_STATUS *t;

  assert(iea);
  t=iea->nextAccountStatus;
  if (t) {
    iea->nextAccountStatus=AB_AccountStatus_List_Next(t);
    AB_AccountStatus_List_Del(t);
    return t;
  }
  iea->nextAccountStatus=0;
  return 0;
}












AB_IMEXPORTER_CONTEXT *AB_ImExporterContext_new(){
  AB_IMEXPORTER_CONTEXT *iec;

  GWEN_NEW_OBJECT(AB_IMEXPORTER_CONTEXT, iec);
  iec->accountInfoList=AB_ImExporterAccountInfo_List_new();

  return iec;
}



void AB_ImExporterContext_free(AB_IMEXPORTER_CONTEXT *iec){
  if (iec) {
    AB_ImExporterAccountInfo_List_free(iec->accountInfoList);
    GWEN_FREE_OBJECT(iec);
  }
}



int AB_ImExporterContext_toDb(const AB_IMEXPORTER_CONTEXT *iec,
			       GWEN_DB_NODE *db){
  AB_IMEXPORTER_ACCOUNTINFO *iea;

  iea=AB_ImExporterAccountInfo_List_First(iec->accountInfoList);
  if (iea) {
    GWEN_DB_NODE *dbG;

    dbG=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
			 "accountInfoList");
    assert(dbG);

    while(iea) {
      GWEN_DB_NODE *dbT;

      dbT=GWEN_DB_GetGroup(dbG, GWEN_PATH_FLAGS_CREATE_GROUP,
			   "accountInfo");
      assert(dbT);

      if (AB_ImExporterAccountInfo_toDb(iea, dbT))
	return -1;
      iea=AB_ImExporterAccountInfo_List_Next(iea);
    }
  }

  return 0;
}



int AB_ImExporterContext_ReadDb(AB_IMEXPORTER_CONTEXT *iec,
                                GWEN_DB_NODE *db) {
  GWEN_DB_NODE *dbT;

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		       "accountInfoList");
  if (dbT) {
    dbT=GWEN_DB_FindFirstGroup(dbT, "accountInfo");
    while(dbT) {
      AB_IMEXPORTER_ACCOUNTINFO *iea;

      iea=AB_ImExporterAccountInfo_fromDb(dbT);
      assert(iea);
      AB_ImExporterAccountInfo_List_Add(iea, iec->accountInfoList);
      dbT=GWEN_DB_FindNextGroup(dbT, "accountInfo");
    }
  }

  return 0;
}



AB_IMEXPORTER_CONTEXT *AB_ImExporterContext_fromDb(GWEN_DB_NODE *db) {
  AB_IMEXPORTER_CONTEXT *iec;

  iec=AB_ImExporterContext_new();
  AB_ImExporterContext_ReadDb(iec, db);
  return iec;
}



AB_IMEXPORTER_ACCOUNTINFO*
AB_ImExporterContext_GetFirstAccount(AB_IMEXPORTER_CONTEXT *iec){
  AB_IMEXPORTER_ACCOUNTINFO *iea;

  assert(iec);
  iea=AB_ImExporterAccountInfo_List_First(iec->accountInfoList);
  if (iea) {
    iec->nextAccountInfo=AB_ImExporterAccountInfo_List_Next(iea);
    return iea;
  }
  iec->nextAccountInfo=0;
  return 0;
}



AB_IMEXPORTER_ACCOUNTINFO*
AB_ImExporterContext_GetNextAccount(AB_IMEXPORTER_CONTEXT *iec){
  AB_IMEXPORTER_ACCOUNTINFO *iea;

  assert(iec);
  iea=iec->nextAccountInfo;
  if (iea) {
    iec->nextAccountInfo=AB_ImExporterAccountInfo_List_Next(iea);
    AB_ImExporterAccountInfo_List_Del(iea);
    return iea;
  }
  iec->nextAccountInfo=0;
  return 0;
}



void AB_ImExporterContext_AddAccountInfo(AB_IMEXPORTER_CONTEXT *iec,
                                         AB_IMEXPORTER_ACCOUNTINFO *iea){
  assert(iec);
  assert(iea);
  AB_ImExporterAccountInfo_List_Add(iea, iec->accountInfoList);
}



AB_IMEXPORTER_ACCOUNTINFO*
AB_ImExporterContext_FindAccountInfo(AB_IMEXPORTER_CONTEXT *iec,
				     const char *bankCode,
                                     const char *accountNumber){
  AB_IMEXPORTER_ACCOUNTINFO *iea;

  if (!bankCode)
    bankCode="";
  if (!accountNumber)
    accountNumber="";

  assert(iec);
  iea=AB_ImExporterAccountInfo_List_First(iec->accountInfoList);
  while(iea) {
    const char *sBankCode;
    const char *sAccountNumber;

    sBankCode=AB_ImExporterAccountInfo_GetBankCode(iea);
    if (sBankCode==0)
      sBankCode="";
    sAccountNumber=AB_ImExporterAccountInfo_GetAccountNumber(iea);
    if (sAccountNumber==0)
      sAccountNumber="";
    if (strcasecmp(sBankCode,
		   bankCode)==0 &&
	strcasecmp(sAccountNumber,
		   accountNumber)==0) {
      return iea;
    }
    iea=AB_ImExporterAccountInfo_List_Next(iea);
  }
  return 0;
}



AB_IMEXPORTER_ACCOUNTINFO*
AB_ImExporterContext_GetAccountInfo(AB_IMEXPORTER_CONTEXT *iec,
                                    const char *bankCode,
                                    const char *accountNumber){
  AB_IMEXPORTER_ACCOUNTINFO *iea;

  if (!bankCode)
    bankCode="";
  if (!accountNumber)
    accountNumber="";

  assert(iec);
  iea=AB_ImExporterContext_FindAccountInfo(iec, bankCode, accountNumber);
  if (!iea) {
    /* not found, append it */
    iea=AB_ImExporterAccountInfo_new();
    AB_ImExporterAccountInfo_SetBankCode(iea, bankCode);
    AB_ImExporterAccountInfo_SetAccountNumber(iea, accountNumber);
    AB_ImExporterAccountInfo_List_Add(iea, iec->accountInfoList);
  }
  return iea;
}



void AB_ImExporterContext_AddTransaction(AB_IMEXPORTER_CONTEXT *iec,
                                         AB_TRANSACTION *t){
  AB_IMEXPORTER_ACCOUNTINFO *iea;

  iea=AB_ImExporterContext_GetAccountInfo
    (iec,
     AB_Transaction_GetLocalBankCode(t),
     AB_Transaction_GetLocalAccountNumber(t)
    );
  assert(iea);
  AB_ImExporterAccountInfo_AddTransaction(iea, t);
}




void AB_ImExporter_Utf8ToDta(const char *p,
                             int size,
                             GWEN_BUFFER *buf) {
  while(*p) {
    unsigned int c;

    if (!size)
      break;

    c=(unsigned char)(*(p++));
    if (c==0xc3) {
      if (size!=-1)
        size--;
      if (!size) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Incomplete UTF-8 sequence");
        break;
      }
      c=(unsigned char)(*(p++));
      switch(c) {
      case 0x84:
      case 0xa4: c=0x5b; break;
      case 0x96:
      case 0xb6: c=0x5c; break;
      case 0x9c:
      case 0xbc: c=0x5d; break;
      case 0x9f: c=0x7e; break;
      default:   c=' '; break;
      } /* switch */
    }
    else {
      c=toupper(c);
      if (!(isdigit(c) ||
	    (c>='A' && c<='Z') ||
	    (c>='a' && c<='z') ||
	    (strchr(" .,&-+*%/$", c))))
        c=' ';
    }
    GWEN_Buffer_AppendByte(buf, c);
    if (size!=-1)
      size--;
  } /* while */
}



void AB_ImExporter_DtaToUtf8(const char *p,
                             int size,
                             GWEN_BUFFER *buf) {
  while(*p) {
    unsigned int c;

    if (!size)
      break;

    c=(unsigned char)(*(p++));
    switch(c) {
    case 0x5b: /* AE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x84);
      break;

    case 0x5c: /* OE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x96);
      break;

    case 0x5d: /* UE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x9c);
      break;

    case 0x7e: /* sharp s */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x9f);
      break;

    default:
      GWEN_Buffer_AppendByte(buf, c);
    }
    if (size!=-1)
      size--;
  } /* while */
}



GWEN_TIME *AB_ImExporter_DateFromString(const char *p, const char *tmpl,
					int inUtc) {
  GWEN_TIME *ti;

  if (strchr(tmpl, 'h')==0) {
    GWEN_BUFFER *dbuf;
    GWEN_BUFFER *tbuf;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(dbuf, p);
    GWEN_Buffer_AppendString(dbuf, "-12:00");

    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(tbuf, tmpl);
    GWEN_Buffer_AppendString(tbuf, "-hh:mm");

    ti=GWEN_Time_fromUtcString(GWEN_Buffer_GetStart(dbuf),
			       GWEN_Buffer_GetStart(tbuf));
    assert(ti);
    GWEN_Buffer_free(tbuf);
    GWEN_Buffer_free(dbuf);
  }
  else {
    if (inUtc)
      ti=GWEN_Time_fromUtcString(p, tmpl);
    else
      ti=GWEN_Time_fromString(p, tmpl);
  }
  return ti;
}


void AB_ImExporter_Iso8859_1ToUtf8(const char *p,
                                   int size,
                                   GWEN_BUFFER *buf) {
  while(*p) {
    unsigned int c;

    if (!size)
      break;

    c=(unsigned char)(*(p++));
    switch(c) {
    case 0xc4: /* AE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x84);
      break;

    case 0xe4: /* ae */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xa4);
      break;

    case 0xd6: /* OE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x96);
      break;

    case 0xf6: /* oe */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xb6);
      break;

    case 0xdc: /* UE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x9c);
      break;

    case 0xfc: /* ue */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xbc);
      break;

    case 0xdf: /* sz */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x9f);
      break;

    case 0xa7: /* section sign */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x67);
      break;

      /* english chars */
    case 0xa3: /* pound swign */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x63);
      break;

      /* french chars */
    case 0xc7: /* C cedille */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x87);
      break;

    case 0xe0: /* a accent grave */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xa0);
      break;

    case 0xe1: /* a accent aigu */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xa1);
      break;

    case 0xe2: /* a accent circumflex */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xa2);
      break;

    case 0xe7: /* c cedille */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xa7);
      break;

    case 0xe8: /* e accent grave */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xa8);
      break;

    case 0xe9: /* e accent aigu */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xa9);
      break;

    case 0xea: /* e accent circumflex */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xaa);
      break;

    case 0xec: /* i accent grave (never heard of this) */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xac);
      break;

    case 0xed: /* i accent aigu (never heard of this, either) */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xad);
      break;

    case 0xee: /* i accent circumflex (never heard of this, either) */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xae);
      break;

    case 0xf2: /* o accent grave */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xb2);
      break;

    case 0xf3: /* o accent aigu */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xb3);
      break;

    case 0xf4: /* o accent circumflex */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xb4);
      break;

    case 0xf9: /* u accent grave */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xb9);
      break;

    case 0xfa: /* u accent aigu */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xba);
      break;

    case 0xfb: /* u accent circumflex */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xbb);
      break;

    default:
      GWEN_Buffer_AppendByte(buf, c);
    }
    if (size!=-1)
      size--;
  } /* while */
}



int AH_ImExporter__Transform_Var(GWEN_DB_NODE *db, int level) {
  GWEN_DB_NODE *dbC;

  dbC=GWEN_DB_GetFirstValue(db);
  while(dbC) {
    if (GWEN_DB_GetValueType(dbC)==GWEN_DB_VALUETYPE_CHAR) {
      const char *s;
      unsigned int l;

      s=GWEN_DB_GetCharValueFromNode(dbC);
      assert(s);
      l=strlen(s);
      if (l) {
        GWEN_BUFFER *vbuf;

        vbuf=GWEN_Buffer_new(0, 1+(l*15/10), 0, 1);
        AB_ImExporter_Iso8859_1ToUtf8(s, l, vbuf);
        GWEN_DB_SetCharValueInNode(dbC, GWEN_Buffer_GetStart(vbuf));
        GWEN_Buffer_free(vbuf);
      }
    }
    dbC=GWEN_DB_GetNextValue(dbC);
  }

  return 0;
}



int AH_ImExporter__Transform_Group(GWEN_DB_NODE *db, int level) {
  GWEN_DB_NODE *dbC;
  int rv;

  if (level>AH_IMEXPORTER_TRANSFORM_MAXLEVEL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "DB too deep (%d)", level);
    return -1;
  }

  dbC=GWEN_DB_GetFirstGroup(db);
  while(dbC) {
    rv=AH_ImExporter__Transform_Group(dbC, level+1);
    if (rv)
      return rv;
    dbC=GWEN_DB_GetNextGroup(dbC);
  }

  dbC=GWEN_DB_GetFirstVar(db);
  while(dbC) {
    rv=AH_ImExporter__Transform_Var(dbC, level+1);
    if (rv)
      return rv;
    dbC=GWEN_DB_GetNextVar(dbC);
  }

  return 0;
}



int AH_ImExporter_DbFromIso8859_1ToUtf8(GWEN_DB_NODE *db) {
  return AH_ImExporter__Transform_Group(db, 0);
}



int AB_ImExporter_DbFromIso8859_1ToUtf8(GWEN_DB_NODE *db) {
  return AH_ImExporter__Transform_Group(db, 0);
}



void AB_ImExporterContext_AddContext(AB_IMEXPORTER_CONTEXT *iec,
                                     AB_IMEXPORTER_CONTEXT *toAdd) {
  AB_IMEXPORTER_ACCOUNTINFO *iea;

  assert(iec);
  iea=AB_ImExporterAccountInfo_List_First(toAdd->accountInfoList);
  while(iea) {
    AB_IMEXPORTER_ACCOUNTINFO *nextIea;

    nextIea=AB_ImExporterAccountInfo_List_Next(iea);
    AB_ImExporterAccountInfo_List_Del(iea);
    AB_ImExporterAccountInfo_List_Add(iea, iec->accountInfoList);
    iea=nextIea;
  }
  AB_ImExporterContext_free(toAdd);
}





