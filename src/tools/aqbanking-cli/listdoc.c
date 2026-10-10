/***************************************************************************
 begin       : Fri Mar 20 2021
 copyright   : (C) 2021 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"
#include <gwenhywfar/text.h>

#include <aqbanking/types/balance.h>
#include <aqbanking/types/document.h>



#define LISTDOC_FLAGS_SHOW_ACCOUNT  0x0001
#define LISTDOC_FLAGS_SHOW_MIMETYPE 0x0002
#define LISTDOC_FLAGS_SHOW_PATH     0x0004



static void _printAccount(const AB_IMEXPORTER_ACCOUNTINFO *iea);
static void _printDoc(const AB_DOCUMENT *doc, uint32_t flags);
static GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv);



int listDoc(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv)
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
  uint32_t flags=0;

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
  flags|=(GWEN_DB_GetIntValue(db, "showAccount", 0, 0)>0)?LISTDOC_FLAGS_SHOW_ACCOUNT:0;
  flags|=(GWEN_DB_GetIntValue(db, "showMimeType", 0, 0)>0)?LISTDOC_FLAGS_SHOW_MIMETYPE:0;
  flags|=(GWEN_DB_GetIntValue(db, "showPath", 0, 0)>0)?LISTDOC_FLAGS_SHOW_PATH:0;

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

      AB_DOCUMENT_LIST *docList;

      docList=AB_ImExporterAccountInfo_GetEStatementList(iea);
      if (docList) {
        AB_DOCUMENT *doc;

        doc=AB_Document_List_First(docList);
        if (doc && (flags & LISTDOC_FLAGS_SHOW_ACCOUNT))
          _printAccount(iea);

        while (doc) {
          _printDoc(doc, flags);
          doc=AB_Document_List_Next(doc);
        }
      }
    } /* if account matches */

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



void _printAccount(const AB_IMEXPORTER_ACCOUNTINFO *iea)
{
  const char *sBank;
  const char *sAccount;

  sAccount=AB_ImExporterAccountInfo_GetIban(iea);
  sBank=AB_ImExporterAccountInfo_GetIban(iea);

  if (!(sAccount && *sAccount)) {
    sAccount=AB_ImExporterAccountInfo_GetAccountNumber(iea);
    sBank=AB_ImExporterAccountInfo_GetBankCode(iea);
  }

  fprintf(stdout, "Account %s/%s\n",
          sBank?sBank:"<no bank id>",
          sAccount?sAccount:"<no account id>");
}



void _printDoc(const AB_DOCUMENT *doc, uint32_t flags)
{
  const char *sId;
  const char *sMimeType;
  const char *sPath;

  sId=AB_Document_GetId(doc);
  sMimeType=AB_Document_GetMimeType(doc);
  sPath=AB_Document_GetFilePath(doc);

  fprintf(stdout, "%s", sId?sId:"<no id>");

  if (flags & LISTDOC_FLAGS_SHOW_MIMETYPE)
    fprintf(stdout, "\t%s", sMimeType?sMimeType:"<no mimetype>");
  if (flags & LISTDOC_FLAGS_SHOW_PATH)
    fprintf(stdout, "\t%s", sPath?sPath:"<no path>");
  fprintf(stdout, "\n");
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
      0, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "showAccount",                /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "",                           /* short option */
      "showAccount",                /* long option */
      "Show account",               /* short description */
      "Show account"                /* long description */
    },
    {
      0, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "showMimeType",               /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "showMimeType",               /* long option */
      "Show mimetype",              /* short description */
      "Show mimetype"               /* long description */
    },
    {
      0, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "showPath",                   /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "",                           /* short option */
      "showPath",                   /* long option */
      "Show path of the file containing document's data", /* short description */
      "Show path of the file containing document's data"  /* long description */
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





