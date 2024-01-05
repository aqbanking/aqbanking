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



static GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv);




int listTrans(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  const char *ctxFile;
  AB_IMEXPORTER_CONTEXT *ctx=0;
  AB_IMEXPORTER_ACCOUNTINFO *iea=0;
  uint32_t aid;
  const char *bankId;
  const char *accountId;
  const char *subAccountId;
  const char *iban;
  int transactionType=0;
  int transactionCommand=0;
  const char *tmplString;
  const char *s;

  /* parse command line arguments */
  db=_readCommandLine(dbArgs, argc, argv);
  if (db==NULL) {
    /* error in command line */
    return 1;
  }

  /* read command line arguments */
  aid=(uint32_t)GWEN_DB_GetIntValue(db, "uniqueAccountId", 0, 0);
  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, 0);
  accountId=GWEN_DB_GetCharValue(db, "accountId", 0, 0);
  subAccountId=GWEN_DB_GetCharValue(db, "subAccountId", 0, 0);
  iban=GWEN_DB_GetCharValue(db, "iban", 0, 0);
  tmplString=GWEN_DB_GetCharValue(db, "template", 0,
                                  "$(dateOrValutaDateAsString)\t"
                                  "$(valueAsString)\t"
                                  "$(localBankcode)\t"
                                  "$(localAccountNumber)\t"
                                  "$(localIban)\t"
                                  "$(remoteName)\t"
                                  "$(remoteIban)\t"
                                  "$(purposeInOneLine)");

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
      AB_TRANSACTION_LIST *tl;

      tl=AB_ImExporterAccountInfo_GetTransactionList(iea);
      if (tl) {
        const AB_TRANSACTION *t;
        GWEN_BUFFER *dbuf;

        dbuf=GWEN_Buffer_new(0, 256, 0, 1);

        t=AB_Transaction_List_FindFirstByType(tl, transactionType, transactionCommand);
        while (t) {
          rv=addTransactionToBufferByTemplate(t, tmplString, dbuf);
          if (rv<0) {
          }

          fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(dbuf));
          GWEN_Buffer_Reset(dbuf);

          t=AB_Transaction_List_FindNextByType(t, transactionType, transactionCommand);
        }
        GWEN_Buffer_free(dbuf);
      }
    }

    iea=AB_ImExporterAccountInfo_List_Next(iea);
  } /* while */
  AB_ImExporterContext_free(ctx);

  /* deinit */
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
      "template",                    /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "T",                          /* short option */
      "template",                       /* long option */
      "Specify the template for the transaction list output",      /* short description */
      "Specify the template for the transaction list output"       /* long description */
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
    GWEN_Buffer_AppendString(ubuf, "\n");
    GWEN_Buffer_AppendString(ubuf, "The template string given to \"-T\" may contain variables to specify the output.\n");
    GWEN_Buffer_AppendString(ubuf, "Default is (all in one line): \n");
    GWEN_Buffer_AppendString(ubuf, "  $(dateOrValutaDateAsString)\\t$(valueAsString)\\t$(localBankcode)\\t\n");
    GWEN_Buffer_AppendString(ubuf,
                             "   $(localAccountNumber)\\t$(localIban)\\t$(remoteName)\\t$(remoteIban)\\t$(purposeInOneLine)\n");
    GWEN_Buffer_AppendString(ubuf, "\n");
    GWEN_Buffer_AppendString(ubuf, "Often used variables are:\n");
    GWEN_Buffer_AppendString(ubuf, " $(status)              : Transaction status\n");
    GWEN_Buffer_AppendString(ubuf, " $(idForApplication)    : Transaction id assigned by the application\n");
    GWEN_Buffer_AppendString(ubuf, " $(fiId)                : Transaction id assigned by the bank\n");

    GWEN_Buffer_AppendString(ubuf, " $(localIban)           : Local IBAN\n");
    GWEN_Buffer_AppendString(ubuf, " $(localBic)            : Local BIC\n");
    GWEN_Buffer_AppendString(ubuf, " $(localBankcode)       : Local bank code (Bankleitzahl)\n");
    GWEN_Buffer_AppendString(ubuf, " $(localAccountnumber)  : Local account number\n");

    GWEN_Buffer_AppendString(ubuf, " $(remoteIban)          : Remote IBAN\n");
    GWEN_Buffer_AppendString(ubuf, " $(remoteBic)           : Remote account number\n");
    GWEN_Buffer_AppendString(ubuf, " $(remoteBankcode)      : Remote bank code (Bankleitzahl)\n");
    GWEN_Buffer_AppendString(ubuf, " $(remoteAccountnumber) : Remote account number\n");

    GWEN_Buffer_AppendString(ubuf, " $(dateAsString)        : Date of the transaction in format ");
    GWEN_Buffer_AppendString(ubuf, I18N("DD.MM.YYYY"));
    GWEN_Buffer_AppendString(ubuf, " \n");
    GWEN_Buffer_AppendString(ubuf, " $(valutaDateAsString)  : Valuta Date of the transaction in format ");
    GWEN_Buffer_AppendString(ubuf, I18N("DD.MM.YYYY"));
    GWEN_Buffer_AppendString(ubuf, " \n");
    GWEN_Buffer_AppendString(ubuf,
                             " $(dateOrValutaDateAsString) : Date (or if missing: valuta date) of the transaction in format ");
    GWEN_Buffer_AppendString(ubuf, I18N("DD.MM.YYYY"));
    GWEN_Buffer_AppendString(ubuf, " \n");
    GWEN_Buffer_AppendString(ubuf, " $(valueAsString)       : Amount of the transaction\n");

    GWEN_Buffer_AppendString(ubuf, " $(purpose)             : Memo/purpose (\"Verwendungszweck\")\n");
    GWEN_Buffer_AppendString(ubuf, " $(purposeLine[n])      : Memo/purpose line n (\"Verwendungszweckzeile\")\n");
    GWEN_Buffer_AppendString(ubuf, " $(purposeInOneLine)    : Full purpose in one line (\"Verwendungszweckzeile\")\n");

    GWEN_Buffer_AppendString(ubuf, " $(customerReference)   : Customer reference\n");
    GWEN_Buffer_AppendString(ubuf, " $(bankReference)       : Bank reference\n");
    GWEN_Buffer_AppendString(ubuf, " $(endToEndReference)   : SEPA end to end reference\n");
    GWEN_Buffer_AppendString(ubuf,
                             " $(ultimateCreditor)    : Ultimate creditor e.g. when payment handled by a service provider\n");
    GWEN_Buffer_AppendString(ubuf,
                             " $(ultimateDebtor)      : Ultimate debitor e.g. when payment handled by a service provider\n");
    GWEN_Buffer_AppendString(ubuf, "\n");

    GWEN_Buffer_AppendString(ubuf, "Variables used with debit notes:\n");

    GWEN_Buffer_AppendString(ubuf, " $(creditorSchemeId)    : Creditor Schema Id\n");
    GWEN_Buffer_AppendString(ubuf, " $(originatorId)        : Originator Id\n");
    GWEN_Buffer_AppendString(ubuf, " $(mandateId)           : Mandate Id\n");
    GWEN_Buffer_AppendString(ubuf, " $(sequence)            : (once, first, following, final)\n");
    GWEN_Buffer_AppendString(ubuf, "\n");

    GWEN_Buffer_AppendString(ubuf, "Variables seldomly used:\n");
    GWEN_Buffer_AppendString(ubuf, " $(type)                : Transaction type\n");
    GWEN_Buffer_AppendString(ubuf, " $(subType)             : Transaction subtype\n");
    GWEN_Buffer_AppendString(ubuf, " $(command)             : Command (for banking jobs, normally \"none\"\n");
    GWEN_Buffer_AppendString(ubuf,
                             " $(uniqueAccountId)     : Unique id of the account to which this Transaction belongs\n");
    GWEN_Buffer_AppendString(ubuf, " $(uniqueId)            : Unique id of the transaction (mostly unused)\n");
    GWEN_Buffer_AppendString(ubuf,
                             " $(refUniqueId)         : Unique id of another transaction this one refers to (mostly unused)\n");
    GWEN_Buffer_AppendString(ubuf, " $(transactionText)     : Transaction text (e.g. \"Gehalt\")\n");
    GWEN_Buffer_AppendString(ubuf, " $(transactionKey)      : Some other value assigned by the bank\n");
    GWEN_Buffer_AppendString(ubuf, "\n");

    GWEN_Buffer_AppendString(ubuf, "\n");
    GWEN_Buffer_AppendString(ubuf, "The following transaction types can be used with \"-tt\":\n");
    GWEN_Buffer_AppendString(ubuf, " statement              : standard bank statements\n");
    GWEN_Buffer_AppendString(ubuf,
                             " notedStatement         : statements not yet booked but planned to be booked by the bank\n");
    GWEN_Buffer_AppendString(ubuf, " transfer               : transfer (pay to someone else)\n");
    GWEN_Buffer_AppendString(ubuf, " debitNote              : debit note (request someone to pay to you)\n");
    GWEN_Buffer_AppendString(ubuf, " standingOrder          : standing order (periodical payment to someone else)\n");
    GWEN_Buffer_AppendString(ubuf, " internalTransfer       : transfer between two of your accounts at one bank\n");


    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return NULL;
  }

  return db;
}



