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

#include <aqbanking/transaction.h>

#include <gwenhywfar/text.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>



static int addSepaDebitNote(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv, int use_flash_debitnote) {
  GWEN_DB_NODE *db;
  AB_ACCOUNT_SPEC_LIST *al=NULL;
  AB_ACCOUNT_SPEC *as;
  int rv;
  const char *ctxFile;
  AB_IMEXPORTER_CONTEXT *ctx=0;
  AB_TRANSACTION *t;
  AB_TRANSACTION_LIMITS *lim;
  const char *rIBAN;
  const char *lIBAN;
  const char *lBIC;
  const GWEN_ARGS args[]={
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
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "creditorSchemeId",                 /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "creditorSchemeId",           /* long option */
    "Specify the creditor scheme id (\"Glaeubiger-ID\")",    /* short description */
    "Specify the creditor scheme id (\"Glaeubiger-ID\")"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "mandateId",                  /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "mandateId",                  /* long option */
    "Specify the mandate id",    /* short description */
    "Specify the mandate id"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "mandateDate",                /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "mandateDate",                  /* long option */
    "Specify the date when the mandate was issued",    /* short description */
    "Specify the date when the mandate was issued"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "sequenceType",                /* name */
    0,                             /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "sequenceType",                  /* long option */
    "Specify the sequence type (once, first, following)",    /* short description */
    "Specify the sequence type (once, first, following)"     /* long description */
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


  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  /* find account */
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

  if (AB_AccountSpec_List_GetCount(al)>1) {
    DBG_ERROR(0, "Ambiguous account specification");
    AB_AccountSpec_List_free(al);
    AB_Banking_Fini(ab);
    return 2;
  }

  as=AB_AccountSpec_List_First(al);
  assert(as);
  AB_AccountSpec_List_Del(as);
  AB_AccountSpec_List_free(al);
  al=NULL;

    /* create transaction from arguments */
  t=mkSepaDebitNote(db, use_flash_debitnote?AB_Transaction_CommandSepaFlashDebitNote:AB_Transaction_CommandSepaDebitNote);
  if (t==NULL) {
    DBG_ERROR(0, "Could not create SEPA transaction from arguments");
    AB_Transaction_free(t);
    AB_AccountSpec_free(as);
    AB_Banking_Fini(ab);
    return 2;
  }

  /* set local account info */
  AB_Banking_FillTransactionFromAccountSpec(t, as);

  /* some checks */
  rIBAN=AB_Transaction_GetRemoteIban(t);
  lIBAN=AB_Transaction_GetLocalIban(t);
  lBIC=AB_Transaction_GetLocalBic(t);

  if (!rIBAN || !(*rIBAN)) {
    DBG_ERROR(0, "Missing remote IBAN");
    AB_Transaction_free(t);
    AB_AccountSpec_free(as);
    AB_Banking_Fini(ab);
    return 1;
  }
  rv=AB_Banking_CheckIban(rIBAN);
  if (rv != 0) {
    DBG_ERROR(0, "Invalid remote IBAN (%s)", rIBAN);
    AB_Transaction_free(t);
    AB_AccountSpec_free(as);
    AB_Banking_Fini(ab);
    return 3;
  }

  if (!lBIC || !(*lBIC)) {
    DBG_ERROR(0, "Missing local BIC");
    AB_Transaction_free(t);
    AB_AccountSpec_free(as);
    AB_Banking_Fini(ab);
    return 1;
  }
  if (!lIBAN || !(*lIBAN)) {
    DBG_ERROR(0, "Missing local IBAN");
    AB_Transaction_free(t);
    AB_AccountSpec_free(as);
    AB_Banking_Fini(ab);
    return 1;
  }
  rv=AB_Banking_CheckIban(lIBAN);
  if (rv != 0) {
    DBG_ERROR(0, "Invalid local IBAN (%s)", rIBAN);
    AB_Transaction_free(t);
    AB_AccountSpec_free(as);
    AB_Banking_Fini(ab);
    return 3;
  }

  AB_Transaction_SetUniqueAccountId(t, AB_AccountSpec_GetUniqueId(as));

  /* check limits */
  lim=AB_AccountSpec_GetTransactionLimitsForCommand(as, AB_Transaction_GetCommand(t));
  if (lim==NULL) {
    fprintf(stderr, "ERROR: Job not supported with this account.\n");
    AB_Transaction_free(t);
    AB_AccountSpec_free(as);
    AB_Banking_Fini(ab);
    return 3;
  }

  if (AB_Banking_CheckTransactionAgainstLimits_Purpose(t, lim)) {
    fprintf(stderr, "ERROR: Purpose violates job limits.\n");
    AB_Transaction_free(t);
    AB_AccountSpec_free(as);
    AB_Banking_Fini(ab);
    return 3;
  }

  if (AB_Banking_CheckTransactionAgainstLimits_Names(t, lim)) {
    fprintf(stderr, "ERROR: Names violate job limits.\n");
    AB_Transaction_free(t);
    AB_AccountSpec_free(as);
    AB_Banking_Fini(ab);
    return 3;
  }

  if (AB_Banking_CheckTransactionAgainstLimits_Sequence(t, lim)) {
    fprintf(stderr, "ERROR: Sequence violate job limits.\n");
    AB_Transaction_free(t);
    AB_AccountSpec_free(as);
    AB_Banking_Fini(ab);
    return 3;
  }

  if (AB_Banking_CheckTransactionAgainstLimits_Date(t, lim)) {
    fprintf(stderr, "ERROR: Date violate job limits.\n");
    AB_Transaction_free(t);
    AB_AccountSpec_free(as);
    AB_Banking_Fini(ab);
    return 3;
  }

  if (AB_Banking_CheckTransactionForSepaConformity(t, 0)) {
    fprintf(stderr, "ERROR: Transaction fails SEPA conformity check.\n");
    AB_Transaction_free(t);
    AB_AccountSpec_free(as);
    AB_Banking_Fini(ab);
    return 3;
  }

  /* load ctx file */
  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);
  rv=readContext(ctxFile, &ctx, 0);
  if (rv<0) {
    DBG_ERROR(0, "Error reading context (%d)", rv);
    AB_Transaction_free(t);
    AB_AccountSpec_free(as);
    AB_Banking_Fini(ab);
    return 4;
  }

  /* add transfer to */
  AB_ImExporterContext_AddTransaction(ctx, t);

  AB_AccountSpec_free(as);

  /* write result back */
  rv=writeContext(ctxFile, ctx);
  AB_ImExporterContext_free(ctx);
  if (rv<0) {
    DBG_ERROR(0, "Error writing context file (%d)", rv);
    AB_Transaction_free(t);
    AB_Banking_Fini(ab);
    return 4;
  }


  /* write result */
  rv=writeContext(ctxFile, ctx);
  AB_ImExporterContext_free(ctx);
  if (rv<0) {
    DBG_ERROR(0, "Error writing context file (%d)", rv);
    AB_Banking_Fini(ab);
    return 4;
  }

  /* that's it */
  rv=AB_Banking_Fini(ab);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;












#if 0
  GWEN_DB_NODE *db;
  int rv;
  const char *ctxFile;
  const char *bankId;
  const char *accountId;
  const char *subAccountId;
  AB_IMEXPORTER_CONTEXT *ctx=NULL;
  AB_ACCOUNT_LIST2 *al;
  AB_ACCOUNT *a;
  AB_TRANSACTION *t;
  const char *rIBAN;
  const char *rBIC;
  const char *lIBAN;
  const char *lBIC;
  const GWEN_ARGS args[]={
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
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,  /* flags */
    GWEN_ArgsType_Char,             /* type */
    "remoteBIC",                /* name */
    1,                             /* minnum */
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
    "creditorSchemeId",                 /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "creditorSchemeId",           /* long option */
    "Specify the creditor scheme id (\"Glaeubiger-ID\")",    /* short description */
    "Specify the creditor scheme id (\"Glaeubiger-ID\")"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "mandateId",                  /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "mandateId",                  /* long option */
    "Specify the mandate id",    /* short description */
    "Specify the mandate id"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "mandateDate",                /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "mandateDate",                  /* long option */
    "Specify the date of the mandate",    /* short description */
    "Specify the date of the mandate"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "sequenceType",                /* name */
    0,                             /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "sequenceType",                  /* long option */
    "Specify the sequence type (once, first, following)",    /* short description */
    "Specify the sequence type (once, first, following)"     /* long description */
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
    "executionDate",              /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "execdate",                   /* long option */
    "Specify the execution date (YYYYMMDD)", /* short */
    "Specify the execution date (YYYYMMDD)" /* long */
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

  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, 0);
  accountId=GWEN_DB_GetCharValue(db, "accountId", 0, 0);
  subAccountId=GWEN_DB_GetCharValue(db, "subAccountId", 0, 0);

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  /* get account */
  al=AB_Banking_FindAccounts(ab, "*", "*", bankId, accountId, subAccountId);
  if (al==NULL || AB_Account_List2_GetSize(al)==0) {
    DBG_ERROR(0, "Account not found");
    AB_Account_List2_free(al);
    return 2;
  }
  else if (AB_Account_List2_GetSize(al)>1) {
    DBG_ERROR(0, "Ambiguous account specification");
    AB_Account_List2_free(al);
    AB_Banking_OnlineFini(ab);
    AB_Banking_Fini(ab);
    return 2;
  }
  a=AB_Account_List2_GetFront(al);
  AB_Account_List2_free(al);

  /* create transaction from arguments */
  t=mkSepaDebitNote(db);
  if (t==NULL) {
    DBG_ERROR(0, "Could not create SEPA transaction from arguments");
    AB_Banking_OnlineFini(ab);
    AB_Banking_Fini(ab);
    return 2;
  }
  AB_Transaction_SetType(t, AB_Transaction_TypeDebitNote);

  rIBAN=AB_Transaction_GetRemoteIban(t);
  rBIC=AB_Transaction_GetRemoteBic(t);
  lIBAN=AB_Transaction_GetLocalIban(t);
  lBIC=AB_Transaction_GetLocalBic(t);

  if (!rBIC || !(*rBIC)) {
    DBG_ERROR(0, "Missing remote BIC");
    AB_Transaction_free(t);
    AB_Banking_OnlineFini(ab);
    AB_Banking_Fini(ab);
    return 1;
  }
  if (!rIBAN || !(*rIBAN)) {
    DBG_ERROR(0, "Missing remote IBAN");
    AB_Transaction_free(t);
    AB_Banking_OnlineFini(ab);
    AB_Banking_Fini(ab);
    return 1;
  }
  rv=AB_Banking_CheckIban(rIBAN);
  if (rv != 0) {
    DBG_ERROR(0, "Invalid remote IBAN (%s)", rIBAN);
    AB_Transaction_free(t);
    AB_Banking_OnlineFini(ab);
    AB_Banking_Fini(ab);
    return 3;
  }


  if (!lBIC || !(*lBIC)) {
    DBG_ERROR(0, "Missing local BIC");
    AB_Transaction_free(t);
    AB_Banking_OnlineFini(ab);
    AB_Banking_Fini(ab);
    return 1;
  }
  if (!lIBAN || !(*lIBAN)) {
    DBG_ERROR(0, "Missing local IBAN");
    AB_Transaction_free(t);
    AB_Banking_OnlineFini(ab);
    AB_Banking_Fini(ab);
    return 1;
  }
  rv=AB_Banking_CheckIban(lIBAN);
  if (rv != 0) {
    DBG_ERROR(0, "Invalid local IBAN (%s)", rIBAN);
    AB_Transaction_free(t);
    AB_Banking_OnlineFini(ab);
    AB_Banking_Fini(ab);
    return 3;
  }

  if (!lBIC || !(*lBIC)) {
    DBG_ERROR(0, "Missing local BIC");
    AB_Transaction_free(t);
    AB_Banking_OnlineFini(ab);
    AB_Banking_Fini(ab);
    return 1;
  }

  /* load ctx file */
  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);
  rv=readContext(ctxFile, &ctx, 0);
  if (rv<0) {
    DBG_ERROR(0, "Error reading context (%d)", rv);
    AB_Transaction_free(t);
    AB_Banking_OnlineFini(ab);
    AB_Banking_Fini(ab);
    return 4;
  }

  /* add transfer to */
  AB_ImExporterContext_AddTransaction(ctx, t);

  /* write result back */
  rv=writeContext(ctxFile, ctx);
  AB_ImExporterContext_free(ctx);
  if (rv<0) {
    DBG_ERROR(0, "Error writing context file (%d)", rv);
    AB_Banking_OnlineFini(ab);
    AB_Banking_Fini(ab);
    return 4;
  }

  /* that's it */
  rv=AB_Banking_OnlineFini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    AB_Banking_Fini(ab);
    return 5;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

#endif
  return 0;
}






