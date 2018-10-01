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

#include "globals.h"
#include <gwenhywfar/text.h>



static
int listAccs(AB_BANKING *ab,
             GWEN_DB_NODE *dbArgs,
             int argc,
             char **argv) {
  GWEN_DB_NODE *db;
  AB_ACCOUNT_SPEC_LIST *al=NULL;
  AB_ACCOUNT_SPEC *as;
  int rv;
  const GWEN_ARGS args[]={
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
  rv=GWEN_Args_Check(argc, argv, 1,
                     0 /*GWEN_ARGS_MODE_ALLOW_FREEPARAM*/,
                     args,
                     db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    fprintf(stderr, "ERROR: Could not parse arguments\n");
    return 1;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutType_Txt)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return 1;
    }
    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }


  rv=getSelectedAccounts(ab, db, &al);
  if (rv<0) {
    if (rv==GWEN_ERROR_NOT_FOUND) {
      DBG_ERROR(0, "No matching accounts");
    }
    else {
      DBG_ERROR(0, "Error getting selected accounts (%d)", rv);
    }
    AB_Banking_Fini(ab);
    return 2;
  }


  as=AB_AccountSpec_List_First(al);
  while(as) {
    uint32_t aid;
    const char *s;

    aid=AB_AccountSpec_GetUniqueId(as);

    fprintf(stdout, "Account\t");
    s=AB_AccountSpec_GetBankCode(as);
    if (!s)
      s="";
    fprintf(stdout, "%s\t", s);          /* bank code */
    s=AB_AccountSpec_GetAccountNumber(as);
    if (!s)
      s="";
    fprintf(stdout, "%s\t", s);          /* account number */
    s="";                                /* empty */
    fprintf(stdout, "%s\t", s);          /* bank name */
    s=AB_AccountSpec_GetAccountName(as);
    if (!s)
      s="";
    fprintf(stdout, "%s\t", s);          /* account name */
    s=AB_AccountSpec_GetBic(as);
    if (!s)
      s="";
    fprintf(stdout, "%s\t", s);          /* SWIFT BIC */
    s=AB_AccountSpec_GetIban(as);
    if (!s)
      s="";
    fprintf(stdout, "%s\t", s);          /* IBAN */
    fprintf(stdout, "%lu\t", (unsigned long int)aid); /* unique account id */
    fprintf(stdout, "%s\n", AB_AccountType_toChar(AB_AccountSpec_GetType(as))); /* account type */

    as=AB_AccountSpec_List_Next(as);
  } /* while (as) */

  AB_AccountSpec_List_free(al);


  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}






