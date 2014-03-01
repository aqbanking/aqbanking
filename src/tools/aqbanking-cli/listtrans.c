/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2005-2010 by Martin Preuss
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



static
int listTrans(AB_BANKING *ab,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv) {
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
  const char *bankId;
  const char *accountId;
  const char *bankName;
  const char *accountName;
  const GWEN_ARGS args[]={
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
    GWEN_ArgsType_Char,            /* type */
    "bankName",                   /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "N",                          /* short option */
    "bankname",                   /* long option */
    "Specify the bank name",      /* short description */
    "Specify the bank name"       /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "accountName",                /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "n",                          /* short option */
    "accountname",                    /* long option */
    "Specify the account name",     /* short description */
    "Specify the account name"      /* long description */
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

  exporterName=GWEN_DB_GetCharValue(db, "exporterName", 0, "csv");
  profileName=GWEN_DB_GetCharValue(db, "profileName", 0, "default");
  profileFile=GWEN_DB_GetCharValue(db, "profileFile", 0, NULL);
  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, 0);
  bankName=GWEN_DB_GetCharValue(db, "bankName", 0, 0);
  accountId=GWEN_DB_GetCharValue(db, "accountId", 0, 0);
  accountName=GWEN_DB_GetCharValue(db, "accountName", 0, 0);
  outFile=GWEN_DB_GetCharValue(db, "outFile", 0, 0);

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

  nctx=AB_ImExporterContext_new();
  iea=AB_ImExporterContext_GetFirstAccountInfo(ctx);
  while(iea) {
    int matches=1;
    const char *s;

    if (matches && bankId) {
      s=AB_ImExporterAccountInfo_GetBankCode(iea);
      if (!s || !*s || -1==GWEN_Text_ComparePattern(s, bankId, 0))
        matches=0;
    }

    if (matches && bankName) {
      s=AB_ImExporterAccountInfo_GetBankName(iea);
      if (!s || !*s || -1==GWEN_Text_ComparePattern(s, bankName, 0))
        matches=0;
    }

    if (matches && accountId) {
      s=AB_ImExporterAccountInfo_GetAccountNumber(iea);
      if (!s || !*s || -1==GWEN_Text_ComparePattern(s, accountId, 0))
        matches=0;
    }
    if (matches && accountName) {
      s=AB_ImExporterAccountInfo_GetAccountName(iea);
      if (!s || !*s || -1==GWEN_Text_ComparePattern(s, accountName, 0))
        matches=0;
    }

    if (matches) {
      AB_IMEXPORTER_ACCOUNTINFO *nai;

      nai=AB_ImExporterAccountInfo_dup(iea);
      AB_ImExporterContext_AddAccountInfo(nctx, nai);
    } /* if matches */
    iea=AB_ImExporterContext_GetNextAccountInfo(ctx);
  } /* while */
  AB_ImExporterContext_free(ctx);

  /* export new context */
  rv=AB_Banking_ExportToFileWithProfile(ab, exporterName, nctx,
					profileName, profileFile,
                                        outFile);
  if (rv<0) {
    DBG_ERROR(0, "Error exporting (%d).", rv);
    AB_ImExporterContext_free(nctx);
    return 4;
  }
  AB_ImExporterContext_free(nctx);

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}






