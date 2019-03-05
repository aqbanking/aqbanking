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

#include <aqbanking/types/balance.h>

#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


static GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv);



int listBal(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv)
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
  const char *tmplString;
  const char *s;
  AB_BALANCE_TYPE bt=AB_Balance_TypeBooked;

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
                                  "$(dateAsString)\t"
                                  "$(valueAsString)\t"
                                  "$(iban)");

  /* determine balance type */
  s=GWEN_DB_GetCharValue(db, "balanceType", 0, "noted");
  if (s && *s) {
    AB_BALANCE_TYPE tempBalanceType;

    tempBalanceType=AB_Balance_Type_fromString(s);
    if (tempBalanceType==AB_Balance_TypeUnknown) {
      DBG_ERROR(0, "Invalid balance type given (%s)", s);
      return 1;
    }
    bt=tempBalanceType;
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
      AB_BALANCE *bal;
      GWEN_DB_NODE *dbAccount;
      const char *s;

      dbAccount=GWEN_DB_Group_new("dbAccount");

      s=AB_ImExporterAccountInfo_GetBankCode(iea);
      if (s && *s)
        GWEN_DB_SetCharValue(dbAccount, GWEN_DB_FLAGS_OVERWRITE_VARS, "bankCode", s);

      s=AB_ImExporterAccountInfo_GetAccountNumber(iea);
      if (s && *s)
        GWEN_DB_SetCharValue(dbAccount, GWEN_DB_FLAGS_OVERWRITE_VARS, "accountNumber", s);

      s=AB_ImExporterAccountInfo_GetBic(iea);
      if (s && *s)
        GWEN_DB_SetCharValue(dbAccount, GWEN_DB_FLAGS_OVERWRITE_VARS, "bic", s);

      s=AB_ImExporterAccountInfo_GetIban(iea);
      if (s && *s)
        GWEN_DB_SetCharValue(dbAccount, GWEN_DB_FLAGS_OVERWRITE_VARS, "iban", s);

      bal=AB_Balance_List_GetLatestByType(AB_ImExporterAccountInfo_GetBalanceList(iea), bt);
      if (bal) {
        GWEN_DB_NODE *dbElement;
        const AB_VALUE *v;
        const GWEN_DATE *dt;
        GWEN_BUFFER *dbuf;

        dbElement=GWEN_DB_Group_dup(dbAccount);
        AB_Balance_toDb(bal, dbElement);

        /* translate value */
        dbuf=GWEN_Buffer_new(0, 256, 0, 1);
        v=AB_Balance_GetValue(bal);
        if (v) {
          AB_Value_toHumanReadableString(v, dbuf, 2);
          GWEN_DB_SetCharValue(dbElement, GWEN_DB_FLAGS_OVERWRITE_VARS, "valueAsString", GWEN_Buffer_GetStart(dbuf));
          GWEN_Buffer_Reset(dbuf);
        }

        /* translate date */
        dt=AB_Balance_GetDate(bal);
        if (dt) {
          rv=GWEN_Date_toStringWithTemplate(dt, I18N("DD.MM.YYYY"), dbuf);
          if (rv>=0) {
            GWEN_DB_SetCharValue(dbElement, GWEN_DB_FLAGS_OVERWRITE_VARS, "dateAsString", GWEN_Buffer_GetStart(dbuf));
          }
          GWEN_Buffer_Reset(dbuf);
        }

        GWEN_DB_ReplaceVars(dbElement, tmplString, dbuf);
        fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(dbuf));
        GWEN_Buffer_free(dbuf);
        GWEN_DB_Group_free(dbElement);
      } /* if bal */

      GWEN_DB_Group_free(dbAccount);
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
      GWEN_ArgsType_Char,           /* type */
      "balanceType",                /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "bt",                          /* short option */
      "balanceType",                   /* long option */
      "Specify the balance type",    /* short description */
      "Specify the balance type (e.g. noted, booked, temporary)"     /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "template",                    /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "T",                          /* short option */
      "template",                       /* long option */
      "Specify the template for the balance list output",      /* short description */
      "Specify the template for the balance list output"       /* long description */
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





