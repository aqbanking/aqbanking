/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


/* tool includes */
#include "globals.h"

/* aqbanking includes */
#include <aqbanking/types/transaction.h>

/* gwenhywfar includes */
#include <gwenhywfar/text.h>


/* forward declarations */
static GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv);

static int _createAndAndSendRequests(AB_BANKING *ab,
                                     AB_ACCOUNT_SPEC_LIST *asl,
                                     const GWEN_DATE *fromDate,
                                     const GWEN_DATE *toDate,
                                     uint32_t requestFlags,
                                     const char *ctxFile,
                                     uint32_t number);




int request(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  AB_ACCOUNT_SPEC_LIST *al=NULL;
  uint32_t requestFlags=0;
  uint32_t number;
  const char *s;
  int rv;
  const char *ctxFile;
  GWEN_DATE *fromDate=0;
  GWEN_DATE *toDate=0;

  /* parse command line */
  db=_readCommandLine(dbArgs, argc, argv);
  if (db==NULL) {
    fprintf(stderr, "ERROR: Could not parse arguments\n");
    return 1;
  }

  /* read arguments */
  if (GWEN_DB_GetIntValue(db, "reqTrans", 0, 0))
    requestFlags|=AQBANKING_TOOL_REQUEST_STATEMENTS;
  if (GWEN_DB_GetIntValue(db, "reqBalance", 0, 0))
    requestFlags|=AQBANKING_TOOL_REQUEST_BALANCE;
  if (GWEN_DB_GetIntValue(db, "reqSepaSto", 0, 0))
    requestFlags|=AQBANKING_TOOL_REQUEST_SEPASTO;
  if (GWEN_DB_GetIntValue(db, "reqEStatements", 0, 0))
    requestFlags|=AQBANKING_TOOL_REQUEST_ESTATEMENTS;
  if (GWEN_DB_GetIntValue(db, "reqDepot", 0, 0))
    requestFlags|=AQBANKING_TOOL_REQUEST_DEPOT;
  if (GWEN_DB_GetIntValue(db, "ignoreUnsupported", 0, 0))
    requestFlags|=AQBANKING_TOOL_REQUEST_IGNORE_UNSUP;
  if (GWEN_DB_GetIntValue(db, "acknowledge", 0, 0))
    requestFlags|=AQBANKING_TOOL_REQUEST_ACKNOWLEDGE;

  /* read command line arguments */
  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);
  s=GWEN_DB_GetCharValue(db, "fromDate", 0, 0);
  if (s && *s) {
    fromDate=GWEN_Date_fromStringWithTemplate(s, "YYYYMMDD");
    if (fromDate==NULL) {
      DBG_ERROR(0, "Invalid fromdate value \"%s\"", s);
      return 1;
    }
  }
  s=GWEN_DB_GetCharValue(db, "toDate", 0, 0);
  if (s && *s) {
    toDate=GWEN_Date_fromStringWithTemplate(s, "YYYYMMDD");
    if (toDate==NULL) {
      DBG_ERROR(0, "Invalid todate value \"%s\"", s);
      GWEN_Date_free(fromDate);
      return 1;
    }
  }

  number = GWEN_DB_GetIntValue(db, "number", 0, 0);

  /* init AqBanking */
  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    GWEN_Date_free(toDate);
    GWEN_Date_free(fromDate);
    return 2;
  }

  /* get accounts */
  al=getSelectedAccounts(ab, db);
  if (al==NULL) {
    DBG_ERROR(0, "No matching accounts");
    GWEN_Date_free(toDate);
    GWEN_Date_free(fromDate);
    AB_Banking_Fini(ab);
    return 2;
  }

  /* create requests for every account spec and send them */
  rv=_createAndAndSendRequests(ab, al, fromDate, toDate, requestFlags, ctxFile, number);
  if (rv) {
    AB_AccountSpec_List_free(al);
    GWEN_Date_free(toDate);
    GWEN_Date_free(fromDate);
    AB_Banking_Fini(ab);
    return 3;
  }

  /* cleanup */
  AB_AccountSpec_List_free(al);
  GWEN_Date_free(toDate);
  GWEN_Date_free(fromDate);

  /* deinit */
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}



int _createAndAndSendRequests(AB_BANKING *ab,
                              AB_ACCOUNT_SPEC_LIST *asl,
                              const GWEN_DATE *fromDate,
                              const GWEN_DATE *toDate,
                              uint32_t requestFlags,
                              const char *ctxFile,
                              uint32_t number)
{
  AB_ACCOUNT_SPEC *as;
  AB_TRANSACTION_LIST2 *jobList;

  /* sample jobs */
  jobList=AB_Transaction_List2_new();

  as=AB_AccountSpec_List_First(asl);
  while (as) {
    int rv;

    rv=createAndAddRequests(ab, jobList, as, fromDate, toDate, requestFlags, number);
    if (rv) {
      AB_Transaction_List2_free(jobList);
      return 3;
    }

    /* next */
    as=AB_AccountSpec_List_Next(as);
  } /* while (as) */

  /* send jobs */
  if (AB_Transaction_List2_GetSize(jobList)) {
    int rv;

    rv=execBankingJobs(ab, jobList, ctxFile);
    if (rv) {
      fprintf(stderr, "Error on sendCommands (%d)\n", rv);
      AB_Transaction_List2_free(jobList);
      return 3;
    }
  }
  else {
    DBG_ERROR(0, "No requests created");
    AB_Transaction_List2_free(jobList);
    return 4;
  }

  AB_Transaction_List2_free(jobList);
  return 0;
}



/* parse command line */
GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "uniqueAccountId",             /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "aid",                        /* long option */
      "Specify the unique account id",      /* short description */
      "Specify the unique account id"       /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "backendName",                     /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                          /* short option */
      "backend",                       /* long option */
      "Specify the name of the backend for your account",      /* short description */
      "Specify the name of the backend for your account"       /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "country",                     /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                          /* short option */
      "country",                       /* long option */
      "Specify the country for your account (e.g. \"de\")",      /* short description */
      "Specify the country for your account (e.g. \"de\")"       /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "bankId",                     /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "b",                          /* short option */
      "bank",                       /* long option */
      "Specify the bank code",      /* short description */
      "Specify the bank code"       /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "accountId",                  /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "a",                          /* short option */
      "account",                    /* long option */
      "Specify the account number",     /* short description */
      "Specify the account number"      /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "subAccountId",                /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "aa",                          /* short option */
      "subaccount",                   /* long option */
      "Specify the sub account id (Unterkontomerkmal)",    /* short description */
      "Specify the sub account id (Unterkontomerkmal)"     /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "iban",                        /* name */
      0,                             /* minnum */
      1,                             /* maxnum */
      "A",                           /* short option */
      "iban",                       /* long option */
      "Specify the iban of your account",      /* short description */
      "Specify the iban of your account"       /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "accountType",                 /* name */
      0,                             /* minnum */
      1,                             /* maxnum */
      "at",                          /* short option */
      "accounttype",                       /* long option */
      "Specify the type of your account",      /* short description */
      "Specify the type of your account"       /* long description */
    },
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
      0,                            /* flags */
      GWEN_ArgsType_Int,             /* type */
      "reqTrans",                   /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                            /* short option */
      "transactions",               /* long option */
      "Request transactions",       /* short description */
      "Request transactions"        /* long description */
    },
    {
      0,                            /* flags */
      GWEN_ArgsType_Int,             /* type */
      "reqBalance",                 /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                            /* short option */
      "balance",                    /* long option */
      "Request balance",            /* short description */
      "Request balance"             /* long description */
    },
    {
      0,                            /* flags */
      GWEN_ArgsType_Int,             /* type */
      "reqSto",                     /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                            /* short option */
      "sto",                        /* long option */
      "Request standing orders",    /* short description */
      "Request standing orders"     /* long description */
    },
    {
      0,                            /* flags */
      GWEN_ArgsType_Int,             /* type */
      "reqSepaSto",                     /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                            /* short option */
      "sepaSto",                    /* long option */
      "Request SEPA standing orders",    /* short description */
      "Request SEPA standing orders"     /* long description */
    },
    {
      0,                            /* flags */
      GWEN_ArgsType_Int,             /* type */
      "reqEStatements",             /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                            /* short option */
      "estatements",                   /* long option */
      "Request electronic statements", /* short description */
      "Request electronic statements"  /* long description */
    },
    {
      0,                            /* flags */
      GWEN_ArgsType_Int,            /* type */
      "reqDepot",                   /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                            /* short option */
      "depot",                      /* long option */
      "Request depot (security list)", /* short description */
      "Request depot (security list)"  /* long description */
    },
    {
      0,                              /* flags */
      GWEN_ArgsType_Int,              /* type */
      "acknowledge",                  /* name */
      0,                              /* minnum */
      1,                              /* maxnum */
      0,                              /* short option */
      "acknowledge",                  /* long option */
      "Acknowledge jobs",             /* short description */
      "Acknowledge each job where the bank supports."   /* long description */
    },
    {
      0,
      GWEN_ArgsType_Int,
      "ignoreUnsupported",
      0,
      1,
      0,
      "ignoreUnsupported",
      "let AqBanking ignore unsupported requests for accounts",
      "let AqBanking ignore unsupported requests for accounts",
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "fromDate",                   /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                            /* short option */
      "fromdate",                   /* long option */
      "Specify the first date for which transactions are wanted (YYYYMMDD)", /* short */
      "Specify the first date for which transactions are wanted (YYYYMMDD)" /* long */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "toDate",                     /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                            /* short option */
      "todate",                     /* long option */
      "Specify the last date for which transactions are wanted (YYYYMMDD)", /* short */
      "Specify the last date for which transactions are wanted (YYYYMMDD)" /* long */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "number",                     /* name */
      0,                            /* minnum */
      0,                            /* maxnum */
      0,                            /* short option */
      "docnumber",                  /* long option */
      "Document number",            /* short description */
      "Fetch a specific document number"  /* long description */
    },

    {
      GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
      GWEN_ArgsType_Int,            /* type */
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


