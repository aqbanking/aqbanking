/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* local includes */
#include "globals.h"

/* Gwenhywfar includes */
#include <gwenhywfar/text.h>



/* forward declarations */
static GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv);
static int _copyTransactionsAndFillGaps(AB_IMEXPORTER_CONTEXT *inCtx,
                                        AB_ACCOUNT_SPEC_LIST *accountSpecList,
                                        AB_IMEXPORTER_CONTEXT *outCtx);




int fillGaps(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  const char *ctxFile;
  int noWriteOnError=0;
  AB_IMEXPORTER_CONTEXT *inCtx=NULL;
  AB_IMEXPORTER_CONTEXT *outCtx=NULL;
  AB_ACCOUNT_SPEC_LIST *accountSpecList=NULL;

  /* parse command line arguments */
  db=_readCommandLine(dbArgs, argc, argv);
  if (db==NULL) {
    /* error in command line */
    return 1;
  }

  /* read arguments */
  noWriteOnError=GWEN_DB_GetIntValue(db, "noWriteOnError", 0, 0);

  /* go */
  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  /* read in-context */
  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);
  rv=readContext(ctxFile, &inCtx, 0);
  if (rv<0) {
    DBG_ERROR(0, "Error reading context (%d)", rv);
    AB_Banking_Fini(ab);
    return 4;
  }

  /* read account list */
  rv=AB_Banking_GetAccountSpecList(ab, &accountSpecList);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    AB_Banking_Fini(ab);
    return 4;
  }

  /* fill gaps */
  outCtx=AB_ImExporterContext_new();
  rv=_copyTransactionsAndFillGaps(inCtx, accountSpecList, outCtx);
  if (rv<0) {
    if (noWriteOnError) {
      DBG_ERROR(0, "Some transactions could not be assigned to configured accounts, nothing written.");
      AB_ImExporterContext_free(outCtx);
      AB_Banking_Fini(ab);
      return 4;
    }
    DBG_ERROR(0, "Some transactions could not be assigned to configured accounts, those have status=error");
  }
  AB_ImExporterContext_free(inCtx);

  rv=writeContext(ctxFile, outCtx);
  if (rv<0) {
    DBG_ERROR(0, "Error writing context (%d)", rv);
    AB_ImExporterContext_free(outCtx);
    AB_Banking_Fini(ab);
    return 4;
  }
  AB_ImExporterContext_free(outCtx);

  /* that's it */
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}



GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "ctxFile",                    /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "c",                          /* short option */
      "ctxfile",                    /* long option */
      "Specify the file to store the context in",   /* short description */
      "Specify the file to store the context in"      /* long description */
    },
    {
      0, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "noWriteOnError",             /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                            /* short option */
      "noWriteOnError",                     /* long option */
      "Only write file if all transactions are okay",    /* short description */
      "Only write file if all transactions are okay"     /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
      GWEN_ArgsType_Int,             /* type */
      "help",                       /* name */
      0,                            /* minnum */
      0,                            /* maxnum */
      "h",                          /* short option */
      "help",                       /* long option */
      "Show this help screen",      /* short description */
      "Show this help screen"       /* long description */
    }
  };

  db=GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_DEFAULT, "local");
  rv=GWEN_Args_Check(argc, argv, 1,
                     0 /*GWEN_ARGS_MODE_ALLOW_FREEPARAM*/,
                     args,
                     db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    fprintf(stderr, "ERROR: Could not parse arguments\n");
    return NULL;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutType_Txt)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return NULL;
    }
    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return NULL;
  }

  return db;
}




int _copyTransactionsAndFillGaps(AB_IMEXPORTER_CONTEXT *inCtx,
                                 AB_ACCOUNT_SPEC_LIST *accountSpecList,
                                 AB_IMEXPORTER_CONTEXT *outCtx)
{
  AB_IMEXPORTER_ACCOUNTINFO *iea;
  int allOk=1;
  int transactionCount=0;

  iea=AB_ImExporterContext_GetFirstAccountInfo(inCtx);
  while (iea) {
    const AB_TRANSACTION *t;

    t=AB_ImExporterAccountInfo_GetFirstTransaction(iea, 0, 0);
    while (t) {
      AB_ACCOUNT_SPEC *as;
      AB_TRANSACTION *tCopy=NULL;

      tCopy=AB_Transaction_dup(t);

      as=pickAccountSpecForTransaction(accountSpecList, tCopy);
      if (as==NULL) {
        DBG_ERROR(0, "Could not determine account for transaction %d", transactionCount);
        allOk=0;
        AB_Transaction_SetStatus(tCopy, AB_Transaction_StatusError);
      }

      /* fill missing fields in transaction from account spec */
      AB_Banking_FillTransactionFromAccountSpec(tCopy, as);

      /* add to new context */
      AB_ImExporterContext_AddTransaction(outCtx, tCopy);

      transactionCount++;
      t=AB_Transaction_List_Next(t);
    } /* while t */

    iea=AB_ImExporterAccountInfo_List_Next(iea);
  } /* while */

  if (allOk==0)
    return GWEN_ERROR_GENERIC;
  return 0;
}



