/***************************************************************************
 begin       : Sat Dec 28 2013
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




int sepaRecurTransfer(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  AB_ACCOUNT_SPEC *as;
  int rv;
  int rvExec=0;
  const char *ctxFile;
  AB_TRANSACTION *t;
  int stoCommand=AB_Transaction_CommandUnknown;
  int noCheck;

  /* parse command line arguments */
  db=_readCommandLine(dbArgs, argc, argv);
  if (db==NULL) {
    /* error in command line */
    return 1;
  }

  /* read arguments */
  if (GWEN_DB_GetIntValue(db, "createSto", 0, 0))
    stoCommand=AB_Transaction_CommandSepaCreateStandingOrder;
  if (GWEN_DB_GetIntValue(db, "modifySto", 0, 0)) {
    if (stoCommand!=AB_Transaction_CommandUnknown) {
      DBG_ERROR(0, "Contradicting command line arguments");
      return 1;
    }
    stoCommand=AB_Transaction_CommandSepaModifyStandingOrder;
  }
  if (GWEN_DB_GetIntValue(db, "deleteSto", 0, 0)) {
    if (stoCommand!=AB_Transaction_CommandUnknown) {
      DBG_ERROR(0, "Contradicting command line arguments");
      return 1;
    }
    stoCommand=AB_Transaction_CommandSepaDeleteStandingOrder;
  }
  if (stoCommand==AB_Transaction_CommandUnknown) {
    DBG_ERROR(0, "Missing Option: '--create', '--delete' or '--modify'");
    return 1;
  }
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
  t=mkSepaTransfer(db, stoCommand);
  if (t==NULL) {
    DBG_ERROR(0, "Could not create SEPA standing order from arguments");
    AB_Transaction_free(t);
    AB_AccountSpec_free(as);
    AB_Banking_Fini(ab);
    return 2;
  }
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

  /* execute job */
  rv=execSingleBankingJob(ab, t, ctxFile);
  if (rv) {
    fprintf(stderr, "Error on executeQueue (%d)\n", rv);
    rvExec=rv;
  }

  /* cleanup */
  AB_Transaction_free(t);

  /* that's it */
  rv=AB_Banking_Fini(ab);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    if (rvExec==0)
      rvExec=5;
  }

  return rvExec;
}



/* parse command line */
GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  const GWEN_ARGS args[]= {
    {
      0,                            /* flags */
      GWEN_ArgsType_Int,            /* type */
      "createSto",                  /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,
      "create",
      "Create standing orders",
      "Create standing orders"
    },
    {
      0,                            /* flags */
      GWEN_ArgsType_Int,            /* type */
      "modifySto",                  /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,
      "modify",
      "Modify standing orders",
      "Modify standing orders"
    },
    {
      0,                            /* flags */
      GWEN_ArgsType_Int,            /* type */
      "deleteSto",                  /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "delete",                     /* long option */
      "Delete standing orders",     /* short */
      "Delete standing orders"      /* long */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
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
      "uniqueAccountId",            /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "aid",                        /* long option */
      "Specify the unique account id",      /* short description */
      "Specify the unique account id"       /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "backendName",                /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "backend",                    /* long option */
      "Specify the name of the backend for your account",      /* short description */
      "Specify the name of the backend for your account"       /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "country",                    /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "country",                    /* long option */
      "Specify the country for your account (e.g. \"de\")",      /* short description */
      "Specify the country for your account (e.g. \"de\")"       /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "bankId",                     /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "b",                          /* short option */
      "bank",                       /* long option */
      "overwrite the bank code",    /* short description */
      "overwrite the bank code"     /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
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
      "subAccountId",               /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "aa",                         /* short option */
      "subaccount",                 /* long option */
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
      "Specify the account type",   /* short description */
      "Specify the account type"    /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "iban",                       /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "A",                          /* short option */
      "iban",                       /* long option */
      "Specify the iban of your account",      /* short description */
      "Specify the iban of your account"       /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "remoteBIC",                  /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "rbic",                       /* long option */
      "Specify the remote SWIFT BIC",/* short description */
      "Specify the remote SWIFT BIC" /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "remoteIBAN",                 /* name */
      1,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "riban",                      /* long option */
      "Specify the remote IBAN",    /* short description */
      "Specify the remote IBAN"     /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
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
      GWEN_ArgsType_Char,           /* type */
      "name",                       /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "name",                       /* long option */
      "Specify your name",          /* short description */
      "Specify your name"           /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "remoteName",                 /* name */
      1,                            /* minnum */
      2,                            /* maxnum */
      NULL,                         /* short option */
      "rname",                      /* long option */
      "Specify the remote name",    /* short description */
      "Specify the remote name"     /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
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
      "firstExecutionDate",         /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "firstExecutionDate",         /* long option */
      "Set date of first execution (YYYYMMDD)", /* short description */
      "Set date of first execution (YYYYMMDD)" /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "nextExecutionDate",          /* HKCDL and HKCDN only */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "nextExecutionDate",          /* long option */
      "Set date of next execution (YYYYMMDD)", /* short description */
      "Set this date given from the command request --sepaSto (delete and modify sto only)" /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "lastExecutionDate",          /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "lastExecutionDate",          /* long option */
      "Set date of last execution (YYYYMMDD)", /* short description */
      "Set date of last execution (YYYYMMDD)" /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "executionDay",               /* name */
      1,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "executionDay",               /* long option */
      "Set day of execution",       /* short description */
      "Set day of execution"        /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "executionCycle",             /* name */
      1,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "executionCycle",             /* long option */
      "Set execution cycle",        /* short description */
      "Set execution cycle"         /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "executionPeriod",            /* name */
      1,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "executionPeriod",            /* long option */
      "Set execution period (monthly / weekly)", /* short description */
      "Set execution period (monthly / weekly)" /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "fiId",                       /* HKCDL */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "fiId",                       /* long option */
      "Set the fiId (standing orders)", /* short description */
      "Set the fiId (standing orders) - Auftragsidentifikation for HKCDL or HKCDN" /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "executionDate",              /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "execdate",                   /* long option */
      "Specify the execution date (YYYYMMDD)", /* short */
      "Specify the execution date (YYYYMMDD)" /* long */
    },
    {
      0,                            /* flags */
      GWEN_ArgsType_Int,            /* type */
      "noCheck",                    /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "noCheck",                    /* long option */
      "Dont check transaction limits",  /* short description */
      "Dont check transaction limits"   /* long description */
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
  rv=AB_Cmd_Handle_Args(argc, argv, args, db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    return NULL;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    return NULL;
  }

  return db;
}
