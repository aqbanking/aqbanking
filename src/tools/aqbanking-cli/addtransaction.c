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




int addTransaction(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  AB_ACCOUNT_SPEC *as;
  int rv;
  const char *ctxFile;
  AB_TRANSACTION *t;
  int noCheck;

  /* parse command line arguments */
  db=_readCommandLine(dbArgs, argc, argv);
  if (db==NULL) {
    /* error in command line */
    return 1;
  }

  /* read arguments */
  noCheck=GWEN_DB_GetIntValue(db, "noCheck", 0, 0);
  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);

  /* init AqBanking */
  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  /* get account to work with */
  as=getSingleSelectedAccount(ab, db);
  if (as==NULL) {
    AB_Banking_Fini(ab);
    return 2;
  }

  /* create transaction from arguments */
  t=mkSepaTransfer(db, AB_Transaction_CommandSepaTransfer);
  if (t==NULL) {
    DBG_ERROR(0, "Could not create SEPA transaction from arguments");
    AB_Transaction_free(t);
    AB_AccountSpec_free(as);
    AB_Banking_Fini(ab);
    return 2;
  }
  AB_Transaction_SetType(t, AB_Transaction_TypeTransfer);
  AB_Transaction_SetUniqueAccountId(t, AB_AccountSpec_GetUniqueId(as));

  /* set local account info from selected AB_ACCOUNT_SPEC */
  AB_Banking_FillTransactionFromAccountSpec(t, as);

  /* some checks */
  rv=checkTransactionIbans(t);
  if (rv!=0) {
    AB_Transaction_free(t);
    AB_AccountSpec_free(as);
    AB_Banking_Fini(ab);
    return rv;
  }

  /* probably check against transaction limits */
  if (!noCheck) {
    rv=checkTransactionLimits(t,
                              AB_AccountSpec_GetTransactionLimitsForCommand(as, AB_Transaction_GetCommand(t)),
                              AQBANKING_TOOL_LIMITFLAGS_PURPOSE  |
                              AQBANKING_TOOL_LIMITFLAGS_NAMES    |
                              AQBANKING_TOOL_LIMITFLAGS_DATE     |
                              AQBANKING_TOOL_LIMITFLAGS_SEPA);
    if (rv!=0) {
      AB_Transaction_free(t);
      AB_AccountSpec_free(as);
      AB_Banking_Fini(ab);
      return rv;
    }
  }
  AB_AccountSpec_free(as);

  /* add to context file */
  rv=addTransactionToContextFile(t, ctxFile);
  if (rv!=0) {
    DBG_ERROR(0, "Error adding to context (%d)", rv);
    AB_Transaction_free(t);
    AB_Banking_Fini(ab);
    return 4;
  }
  AB_Transaction_free(t);

  /* that's it */
  rv=AB_Banking_Fini(ab);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

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
      "overwrite the bank code",      /* short description */
      "overwrite the bank code"       /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "accountId",                  /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "a",                          /* short option */
      "account",                    /* long option */
      "overwrite the account number",     /* short description */
      "overwrite the account number"      /* long description */
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
      GWEN_ArgsType_Char,           /* type */
      "accountType",                /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "at",                         /* short option */
      "accounttype",                /* long option */
      "Specify the account type",    /* short description */
      "Specify the account type"     /* long description */
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
      GWEN_ARGS_FLAGS_HAS_ARGUMENT,  /* flags */
      GWEN_ArgsType_Char,             /* type */
      "remoteBIC",                /* name */
      0,                             /* minnum */
      1,                             /* maxnum */
      0,                             /* short option */
      "rbic",                       /* long option */
      "Specify the remote SWIFT BIC",/* short description */
      "Specify the remote SWIFT BIC" /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "remoteIBAN",                  /* name */
      1,                            /* minnum */
      1,                            /* maxnum */
      0,                            /* short option */
      "riban",                      /* long option */
      "Specify the remote IBAN",    /* short description */
      "Specify the remote IBAN"     /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "value",                      /* name */
      1,                            /* minnum */
      1,                            /* maxnum */
      "v",                          /* short option */
      "value",                      /* long option */
      "Specify the transfer amount",     /* short description */
      "Specify the transfer amount"      /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "name",                 /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                            /* short option */
      "name",                      /* long option */
      "Specify your name",    /* short description */
      "Specify your name"     /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "remoteName",                 /* name */
      1,                            /* minnum */
      2,                            /* maxnum */
      0,                            /* short option */
      "rname",                      /* long option */
      "Specify the remote name",    /* short description */
      "Specify the remote name"     /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "purpose",                    /* name */
      1,                            /* minnum */
      6,                            /* maxnum */
      "p",                          /* short option */
      "purpose",                    /* long option */
      "Specify the purpose",        /* short description */
      "Specify the purpose"         /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "endToEndReference",          /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "E",                          /* short option */
      "endtoendid",                 /* long option */
      "Specify the SEPA End-to-end-reference",        /* short description */
      "Specify the SEPA End-to-end-reference"         /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "executionDate",              /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                            /* short option */
      "execdate",                   /* long option */
      "Specify the execution date (YYYYMMDD)", /* short */
      "Specify the execution date (YYYYMMDD)" /* long */
    },
    {
      0,                  /* flags */
      GWEN_ArgsType_Int,  /* type */
      "noCheck",          /* name */
      0,                  /* minnum */
      1,                  /* maxnum */
      NULL,                /* short option */
      "noCheck",          /* long option */
      "Dont check transaction limits",  /* short description */
      "Dont check transaction limits"
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





