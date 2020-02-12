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



int listAccs(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  AB_ACCOUNT_SPEC_LIST *al=NULL;
  AB_ACCOUNT_SPEC *as;
  int rv;
  const char *tmplString;
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
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "template",                    /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "T",                          /* short option */
      "template",                       /* long option */
      "Specify the template for the account list output",      /* short description */
      "Specify the template for the account list output"       /* long description */
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
    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }


  al=getSelectedAccounts(ab, db);
  if (al==NULL) {
    DBG_INFO(0, "No matching accounts");
    AB_Banking_Fini(ab);
    return 2;
  }

  tmplString=GWEN_DB_GetCharValue(db, "template", 0,
                                  "Account\t$(bankcode)\t$(accountnumber)\t$(bic)\t$(iban)\t$(uniqueId)\t$(typeAsString)");

  as=AB_AccountSpec_List_First(al);
  if (as) {
    GWEN_BUFFER *dbuf;

    fprintf(stdout, "       \tBankcode\tAccountnumber\tBic\tIban\tUniqueId\tType\n");

    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    while (as) {
      GWEN_DB_NODE *dbAccountSpec;

      dbAccountSpec=GWEN_DB_Group_new("accountSpec");
      AB_AccountSpec_toDb(as, dbAccountSpec);
      GWEN_DB_SetCharValue(dbAccountSpec, GWEN_DB_FLAGS_OVERWRITE_VARS, "typeAsString",
                           AB_AccountType_toChar(AB_AccountSpec_GetType(as)));

      GWEN_DB_ReplaceVars(dbAccountSpec, tmplString, dbuf);
      fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(dbuf));
      GWEN_Buffer_Reset(dbuf);

      GWEN_DB_Group_free(dbAccountSpec);

      as=AB_AccountSpec_List_Next(as);
    } /* while (as) */

    GWEN_Buffer_free(dbuf);
  }
  AB_AccountSpec_List_free(al);


  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}






