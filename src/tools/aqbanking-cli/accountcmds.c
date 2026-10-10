/***************************************************************************
 begin       : Wed Aug 10 2022
 copyright   : (C) 2022 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"
#include <gwenhywfar/text.h>

#include <aqbanking/types/account_spec.h>




static GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv);
static void _dumpAccountSpecList(const AB_ACCOUNT_SPEC_LIST *al);
static void _dumpAccountSpec(const AB_ACCOUNT_SPEC *as);
static void _dumpLimitList(const AB_TRANSACTION_LIMITS_LIST *limitList);



int showAccountCommands(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  AB_ACCOUNT_SPEC_LIST *al=NULL;
  int rv;

  /* parse command line arguments */
  db=_readCommandLine(dbArgs, argc, argv);
  if (db==NULL) {
    /* error in command line */
    return 1;
  }

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  al=getSelectedAccounts(ab, db);
  if (al==NULL) {
    DBG_ERROR(0, "No matching accounts");
    AB_Banking_Fini(ab);
    return 2;
  }

  _dumpAccountSpecList(al);

  rv=AB_Banking_Fini(ab);
  if (rv) {
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
      "t",                           /* short option */
      "accounttype",                       /* long option */
      "Specify the type of your account",      /* short description */
      "Specify the type of your account"       /* long description */
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
  rv=AB_Cmd_Handle_Args(argc, argv, args, db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    return NULL;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    return NULL;
  }

  return db;
}



void _dumpAccountSpecList(const AB_ACCOUNT_SPEC_LIST *al)
{
  AB_ACCOUNT_SPEC *as;

  as=AB_AccountSpec_List_First(al);
  if (as) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    while (as) {
      _dumpAccountSpec(as);
      _dumpLimitList(AB_AccountSpec_GetTransactionLimitsList(as));
      fprintf(stdout, "\n");
      as=AB_AccountSpec_List_Next(as);
    } /* while (as) */

    GWEN_Buffer_free(dbuf);
  }
}



void _dumpAccountSpec(const AB_ACCOUNT_SPEC *as)
{
  const char *sIban;
  const char *sBic;
  const char *sAccountName;
  const char *sAccountNumber;
  const char *sBankCode;
  const char *sOwnerName;
  const char *sBackendName;

  sIban=AB_AccountSpec_GetIban(as);
  sAccountName=AB_AccountSpec_GetAccountName(as);
  sAccountNumber=AB_AccountSpec_GetAccountNumber(as);
  sBic=AB_AccountSpec_GetBic(as);
  sBankCode=AB_AccountSpec_GetBankCode(as);
  sOwnerName=AB_AccountSpec_GetOwnerName(as);
  sBackendName=AB_AccountSpec_GetBackendName(as);

  fprintf(stdout, "Account %lu: ", (unsigned long int) AB_AccountSpec_GetUniqueId(as));
  if (sIban && *sIban)
    fprintf(stdout,
            "IBAN: %s, BIC: %s, Account Name: %s, Owner Name: %s (%s)\n",
            sIban, sBic?sBic:"<none>", sAccountName?sAccountName:"<none>", sOwnerName?sOwnerName:"<none>", sBackendName);
  else
    fprintf(stdout,
            "Account Number: %s, Bank Code: %s, Account Name: %s, Owner Name: %s (%s)\n",
            sAccountNumber?sAccountNumber:"<none>",
            sBankCode?sBankCode:"<none>",
            sAccountName?sAccountName:"<none>",
            sOwnerName?sOwnerName:"<none>",
            sBackendName);
}



void _dumpLimitList(const AB_TRANSACTION_LIMITS_LIST *limitList)
{
  if (limitList) {
    const AB_TRANSACTION_LIMITS *lim;

    lim=AB_TransactionLimits_List_First(limitList);
    if (lim) {
      while (lim) {
        fprintf(stdout, "  %s\n", AB_Transaction_Command_toString(AB_TransactionLimits_GetCommand(lim)));
        lim=AB_TransactionLimits_List_Next(lim);
      }
      return;
    }
  }
  fprintf(stdout, "  <none>\n");
}






