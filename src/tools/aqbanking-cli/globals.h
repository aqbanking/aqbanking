/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQB_TOOL_GLOBALS_H
#define AQB_TOOL_GLOBALS_H


#include <aqbanking/banking.h>
#include <aqbanking/types/account_spec.h>

#include <gwenhywfar/args.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/i18n.h>
#include <gwenhywfar/cgui.h>



#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)
#define I18S(msg) msg


typedef enum {
  AQBANKING_TOOL_SEPA_TRANSFERS,
  AQBANKING_TOOL_SEPA_DEBITNOTES
} AQBANKING_TOOL_MULTISEPA_TYPE;



#define AQBANKING_TOOL_LIMITFLAGS_PURPOSE    0x0001
#define AQBANKING_TOOL_LIMITFLAGS_NAMES      0x0002
#define AQBANKING_TOOL_LIMITFLAGS_SEQUENCE   0x0004
#define AQBANKING_TOOL_LIMITFLAGS_DATE       0x0008
#define AQBANKING_TOOL_LIMITFLAGS_SEPA       0x0010



#define AQBANKING_TOOL_REQUEST_BALANCE       0x0001
#define AQBANKING_TOOL_REQUEST_STATEMENTS    0x0002
#define AQBANKING_TOOL_REQUEST_SEPASTO       0x0004
#define AQBANKING_TOOL_REQUEST_ESTATEMENTS   0x0008
#define AQBANKING_TOOL_REQUEST_DEPOT         0x0010

#define AQBANKING_TOOL_REQUEST_ACKNOWLEDGE   0x4000
#define AQBANKING_TOOL_REQUEST_IGNORE_UNSUP  0x8000




/* ========================================================================================================================
 *                                                util.c
 * ========================================================================================================================
 */


int readContext(const char *ctxFile, AB_IMEXPORTER_CONTEXT **pCtx, int mustExist);
int writeContext(const char *ctxFile, const AB_IMEXPORTER_CONTEXT *ctx);

AB_TRANSACTION *mkSepaTransfer(GWEN_DB_NODE *db, int cmd);

AB_TRANSACTION *mkSepaDebitNote(GWEN_DB_NODE *db, int cmd);


/**
 * Get selected AqBanking account sepcs matching the user given parameters in command line db.
 */
AB_ACCOUNT_SPEC_LIST *getSelectedAccounts(AB_BANKING *ab, GWEN_DB_NODE *db);

/**
 * Return a single account spec matching the user given parameters in command line db.
 * If multiple candidates match NULL will be returned
 */
AB_ACCOUNT_SPEC *getSingleSelectedAccount(AB_BANKING *ab, GWEN_DB_NODE *db);


AB_ACCOUNT_SPEC *pickAccountSpecForArgs(const AB_ACCOUNT_SPEC_LIST *accountSpecList, GWEN_DB_NODE *db);
AB_ACCOUNT_SPEC *pickAccountSpecForTransaction(const AB_ACCOUNT_SPEC_LIST *as, const AB_TRANSACTION *t);



int checkTransactionIbans(const AB_TRANSACTION *t);
int checkTransactionLimits(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim, uint32_t flags);


int addTransactionToContextFile(const AB_TRANSACTION *t, const char *ctxFile);

int writeJobsAsContextFile(AB_TRANSACTION_LIST2 *tList, const char *ctxFile);


int execBankingJobs(AB_BANKING *ab, AB_TRANSACTION_LIST2 *tList, const char *ctxFile);
int execSingleBankingJob(AB_BANKING *ab, AB_TRANSACTION *t, const char *ctxFile);

AB_TRANSACTION *createAndCheckRequest(AB_BANKING *ab, AB_ACCOUNT_SPEC *as, AB_TRANSACTION_COMMAND cmd);

int createAndAddRequest(AB_BANKING *ab,
                        AB_TRANSACTION_LIST2 *tList,
                        AB_ACCOUNT_SPEC *as,
                        AB_TRANSACTION_COMMAND cmd,
                        const GWEN_DATE *fromDate,
                        const GWEN_DATE *toDate,
                        int ignoreUnsupported,
                        AB_TRANSACTION_ACK ackMethod,
                        uint32_t number);

/**
 * Create requests (getBalance, getStatements etc.) according to the given flags.
 *
 * @return 0 if ok, !=0 on error
 */
int createAndAddRequests(AB_BANKING *ab,
                         AB_TRANSACTION_LIST2 *tList,
                         AB_ACCOUNT_SPEC *as,
                         const GWEN_DATE *fromDate,
                         const GWEN_DATE *toDate,
                         uint32_t requestFlags,
                         uint32_t number);

int addTransactionToBufferByTemplate(const AB_TRANSACTION *t, const char *tmplString, GWEN_BUFFER *dbuf);



/* ========================================================================================================================
 *                                                Commands
 * ========================================================================================================================
 */

int addSepaDebitNote(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv, int use_flash_debitnote);
int addTransaction(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv);
int chkAcc(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv);
int chkIban(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv);
int control(AB_BANKING *ab, const char *ctrlBackend, GWEN_DB_NODE *dbArgs, int argc, char **argv);
int fillGaps(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv);
int import(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv);
int exportCtx(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv);
int listAccs(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv);
int listBal(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv);
int listTrans(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv);
int listDoc(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv);
int getDoc(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv);
int listProfiles(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv);
int request(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv);
int sepaDebitNote(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv, int use_flash_debitnote);
int sepaMultiJobs(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv,
                  AQBANKING_TOOL_MULTISEPA_TYPE multisepa_type);
int sepaRecurTransfer(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv);
int sepaTransfer(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv);
int updateConf(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv);
int versions(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv);















#endif




