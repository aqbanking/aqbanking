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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>



int exportCtx(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  const char *ctxFile;
  const char *outFile;
  const char *exporterName;
  const char *profileName;
  const char *profileFile;
  AB_IMEXPORTER_CONTEXT *ctx=0;
  AB_IMEXPORTER_CONTEXT *nctx=0;
  AB_IMEXPORTER_ACCOUNTINFO *iea=0;
  uint32_t aid;
  const char *bankId;
  const char *accountId;
  const char *subAccountId;
  const char *iban;
  int transactionType=0;
  int transactionCommand=0;
  const char *s;
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
      GWEN_ArgsType_Char,           /* type */
      "iban",                       /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "A",                          /* short option */
      "iban",                    /* long option */
      "Specify the iban of your account",      /* short description */
      "Specify the iban of your account"       /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "transactionType",            /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "tt",                         /* short option */
      "transactiontype",            /* long option */
      "Specify the transaction type to filter",      /* short description */
      "Specify the transaction type to filter"       /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "transactionCommand",         /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "tc",                         /* short option */
      "transactioncommand",         /* long option */
      "Specify the transaction command to filter",      /* short description */
      "Specify the transaction command to filter"       /* long description */
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
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "outFile",                    /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "o",                          /* short option */
      "outfile",                    /* long option */
      "Specify the file to store the data in",   /* short description */
      "Specify the file to store the data in"      /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "exporterName",               /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                            /* short option */
      "exporter",                    /* long option */
      "Specify the exporter to use",   /* short description */
      "Specify the exporter to use"      /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "profileName",                /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                            /* short option */
      "profile",                    /* long option */
      "Specify the export profile to use",   /* short description */
      "Specify the export profile to use"      /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "profileFile",                /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                            /* short option */
      "profile-file",               /* long option */
      "Specify the file to load the export profile from",/* short description */
      "Specify the file to load the export profile from" /* long description */
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

  /* read command line arguments */
  exporterName=GWEN_DB_GetCharValue(db, "exporterName", 0, "csv");
  profileName=GWEN_DB_GetCharValue(db, "profileName", 0, "default");
  profileFile=GWEN_DB_GetCharValue(db, "profileFile", 0, NULL);
  aid=(uint32_t)GWEN_DB_GetIntValue(db, "uniqueAccountId", 0, 0);
  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, 0);
  accountId=GWEN_DB_GetCharValue(db, "accountId", 0, 0);
  subAccountId=GWEN_DB_GetCharValue(db, "subAccountId", 0, 0);
  iban=GWEN_DB_GetCharValue(db, "iban", 0, 0);
  outFile=GWEN_DB_GetCharValue(db, "outFile", 0, 0);

  s=GWEN_DB_GetCharValue(db, "transactionType", 0, NULL);
  if (s && *s) {
    transactionType=AB_Transaction_Type_fromString(s);
    if (transactionType==AB_Transaction_TypeUnknown) {
      fprintf(stderr, "ERROR: Invalid transaction type \"%s\"\n", s);
      return 1;
    }
  }

  s=GWEN_DB_GetCharValue(db, "transactionCommand", 0, NULL);
  if (s && *s) {
    transactionCommand=AB_Transaction_Command_fromString(s);
    if (transactionCommand==AB_Transaction_CommandUnknown) {
      fprintf(stderr, "ERROR: Invalid transaction command \"%s\"\n", s);
      return 1;
    }
  }

  /* init AqBanking */
  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  /* load ctx file */
  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);
  rv=readContext(ctxFile, &ctx, 1);
  if (rv<0) {
    DBG_ERROR(0, "Error reading context (%d)", rv);
    AB_ImExporterContext_free(ctx);
    return 4;
  }

  /* copy context, but only keep wanted accounts and transactions */
  nctx=AB_ImExporterContext_new();
  iea=AB_ImExporterContext_GetFirstAccountInfo(ctx);
  while (iea) {
    if (AB_ImExporterAccountInfo_Matches(iea,
                                         aid,  /* unique account id */
                                         "*",
                                         bankId,
                                         accountId,
                                         subAccountId,
                                         iban,
                                         "*", /* currency */
                                         AB_AccountType_Unknown)) {
      AB_IMEXPORTER_ACCOUNTINFO *nai;
      AB_TRANSACTION_LIST *tl;

      nai=AB_ImExporterAccountInfo_dup(iea);
      tl=AB_ImExporterAccountInfo_GetTransactionList(nai);
      if (tl)
        AB_Transaction_List_KeepByType(tl, transactionType, transactionCommand);

      AB_ImExporterContext_AddAccountInfo(nctx, nai);
    }

    iea=AB_ImExporterAccountInfo_List_Next(iea);
  } /* while */
  AB_ImExporterContext_free(ctx); /* old context */

  /* export new context */
  rv=AB_Banking_ExportToFileLoadProfile(ab, exporterName, nctx,
                                        profileName, profileFile,
                                        outFile);
  if (rv<0) {
    DBG_ERROR(0, "Error exporting (%d).", rv);
    AB_ImExporterContext_free(nctx);
    return 4;
  }
  AB_ImExporterContext_free(nctx);

  /* deinit */
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}






