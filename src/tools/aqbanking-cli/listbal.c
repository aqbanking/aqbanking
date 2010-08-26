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

#include <aqbanking/accstatus.h>

#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


static
AB_ACCOUNT_STATUS *_getLastAccountStatus(AB_IMEXPORTER_ACCOUNTINFO *iea) {
  AB_ACCOUNT_STATUS *lastAst=0;
  const GWEN_TIME *lastTi=0;
  AB_ACCOUNT_STATUS *ast=0;

  ast=AB_ImExporterAccountInfo_GetFirstAccountStatus(iea);
  while(ast) {
    const GWEN_TIME *ti;

    if (lastAst && lastTi && (ti=AB_AccountStatus_GetTime(ast))) {
      if (GWEN_Time_Diff(ti, lastTi)>0) {
	lastAst=ast;
        lastTi=ti;
      }
    }
    else {
      lastAst=ast;
      lastTi=AB_AccountStatus_GetTime(ast);
    }
    ast=AB_ImExporterAccountInfo_GetNextAccountStatus(iea);
  }

  return lastAst;
}



static
void _dumpBal(const AB_BALANCE *bal,
	      const GWEN_TIME *ti,
	      FILE *fd) {
  if (bal) {
    const GWEN_TIME *bti;
    const AB_VALUE *val;
  
    bti=AB_Balance_GetTime(bal);
    if (bti==0)
      bti=ti;
    if (bti) {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 24, 0, 1);
      GWEN_Time_toString(bti, "DD.MM.YYYY\thh:mm", tbuf);
      fprintf(fd, "%s\t", GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
    }
    else {
      fprintf(fd, "\t\t");
    }
  
    val=AB_Balance_GetValue(bal);
    if (val) {
      AB_VALUE *vNew;
      GWEN_BUFFER *vbuf;
      const char *cur;

      vNew=AB_Value_dup(val);
      AB_Value_SetCurrency(vNew, NULL);
      vbuf=GWEN_Buffer_new(0, 32, 0, 1);
      AB_Value_toHumanReadableString(vNew, vbuf, 2);
      fprintf(fd, "%s\t", GWEN_Buffer_GetStart(vbuf));
      GWEN_Buffer_free(vbuf);
      AB_Value_free(vNew);

      cur=AB_Value_GetCurrency(val);
      if (cur)
	fprintf(fd, "%s\t", cur);
      else
	fprintf(fd, "\t");
    }
    else {
      fprintf(fd, "\t\t");
    }
  }
  else {
    fprintf(fd, "\t\t\t\t");
  }
}



static
int listBal(AB_BANKING *ab,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv) {
  GWEN_DB_NODE *db;
  int rv;
  const char *ctxFile;
  const char *outFile;
  AB_IMEXPORTER_CONTEXT *ctx=0;
  AB_IMEXPORTER_ACCOUNTINFO *iea=0;
  const char *bankId;
  const char *accountId;
  const char *bankName;
  const char *accountName;
  FILE *f;
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
  bankName=GWEN_DB_GetCharValue(db, "bankName", 0, 0);
  accountId=GWEN_DB_GetCharValue(db, "accountId", 0, 0);
  accountName=GWEN_DB_GetCharValue(db, "accountName", 0, 0);

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);
  rv=readContext(ctxFile, &ctx, 1);
  if (rv<0) {
    DBG_ERROR(0, "Error reading context (%d)", rv);
    return 4;
  }

  /* open output stream */
  outFile=GWEN_DB_GetCharValue(db, "outFile", 0, 0);
  if (outFile==0)
    f=stdout;
  else
    f=fopen(outFile, "w+");
  if (f==0) {
    DBG_ERROR(0, "Error selecting output file: %s",
	      strerror(errno));
    return 4;
  }

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
      if (!s || !*s)
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
      if (!s || !*s)
        s=AB_ImExporterAccountInfo_GetAccountName(iea);
      if (!s || !*s || -1==GWEN_Text_ComparePattern(s, accountName, 0))
        matches=0;
    }

    if (matches) {
      AB_ACCOUNT_STATUS *ast;

      ast=_getLastAccountStatus(iea);
      if (ast) {
	const GWEN_TIME *ti;
        const char *s;

	fprintf(f, "Account\t");
	s=AB_ImExporterAccountInfo_GetBankCode(iea);
	if (!s)
	  s="";
	fprintf(f, "%s\t", s);
	s=AB_ImExporterAccountInfo_GetAccountNumber(iea);
	if (!s)
	  s="";
	fprintf(f, "%s\t", s);
	s=AB_ImExporterAccountInfo_GetBankName(iea);
	if (!s)
	  s="";
	fprintf(f, "%s\t", s);
	s=AB_ImExporterAccountInfo_GetAccountName(iea);
	if (!s)
	  s="";
	fprintf(f, "%s\t", s);

	ti=AB_AccountStatus_GetTime(ast);
	_dumpBal(AB_AccountStatus_GetBookedBalance(ast), ti, f);
	_dumpBal(AB_AccountStatus_GetNotedBalance(ast), ti, f);

        fprintf(f, "\n");
      }
    } /* if matches */
    iea=AB_ImExporterContext_GetNextAccountInfo(ctx);
  } /* while */

  if (outFile) {
    if (fclose(f)) {
      DBG_ERROR(0, "Error closing output file: %s",
		strerror(errno));
      return 4;
    }
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}






