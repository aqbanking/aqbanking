/***************************************************************************
 begin       : Thu Apr 24 2008
 copyright   : (C) 2008-2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"

#include <aqbanking/jobsingledebitnote.h>

#include <gwenhywfar/text.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>



static
int debitNotes(AB_BANKING *ab,
               GWEN_DB_NODE *dbArgs,
               int argc,
               char **argv) {
  GWEN_DB_NODE *db;
  int rv;
  const char *ctxFile;
  const char *inFile;
  const char *importerName;
  const char *profileName;
  const char *profileFile;
  const char *bankId;
  const char *accountId;
  int forceCheck;
  int fillGaps;
  AB_IMEXPORTER_CONTEXT *ctx=0;
  AB_IMEXPORTER_ACCOUNTINFO *iea;
  AB_JOB_LIST2 *jobList;
  int rvExec;
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
    "inFile",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "f",                          /* short option */
    "infile",                    /* long option */
    "Specify the file to read the data from",   /* short description */
    "Specify the file to read the data from"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "importerName",               /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "importer",                    /* long option */
    "Specify the importer to use",   /* short description */
    "Specify the importer to use"      /* long description */
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
    0,                
    GWEN_ArgsType_Int,
    "forceCheck", 
    0,  
    1,  
    0, 
    "force-check",
    "force account number check",
    "force account number check"
  },
  {
    0,                
    GWEN_ArgsType_Int,
    "fillGaps",
    0,  
    1,  
    0, 
    "fill-gaps",
    "let AqBanking fill-in missing account information if possible",
    "let AqBanking fill-in missing account information if possible",
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
  importerName=GWEN_DB_GetCharValue(db, "importerName", 0, "csv");
  profileName=GWEN_DB_GetCharValue(db, "profileName", 0, "default");
  profileFile=GWEN_DB_GetCharValue(db, "profileFile", 0, NULL);
  forceCheck=GWEN_DB_GetIntValue(db, "forceCheck", 0, 0);
  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);
  fillGaps=GWEN_DB_GetIntValue(db, "fillGaps", 0, 0);
  inFile=GWEN_DB_GetCharValue(db, "inFile", 0, 0);

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  rv=AB_Banking_OnlineInit(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  /* import new context */
  ctx=AB_ImExporterContext_new();
  rv=AB_Banking_ImportFileWithProfile(ab, importerName, ctx,
				      profileName, profileFile,
				      inFile);
  if (rv<0) {
    DBG_ERROR(0, "Error reading file: %d", rv);
    AB_ImExporterContext_free(ctx);
    return 4;
  }

  /* adjust local account id if requested */
  if (bankId || accountId) {
    iea=AB_ImExporterContext_GetFirstAccountInfo(ctx);
    while(iea) {
      if (bankId)
	AB_ImExporterAccountInfo_SetBankCode(iea, bankId);
      if (accountId)
        AB_ImExporterAccountInfo_SetAccountNumber(iea, accountId);
      iea=AB_ImExporterContext_GetNextAccountInfo(ctx);
    } /* while */
  }

  /* fill gaps */
  if (fillGaps)
    AB_Banking_FillGapsInImExporterContext(ab, ctx);

  /* populate job list */
  jobList=AB_Job_List2_new();
  iea=AB_ImExporterContext_GetFirstAccountInfo(ctx);
  while(iea) {
    AB_ACCOUNT *a;

    a=AB_Banking_GetAccountByCodeAndNumber(ab,
					   AB_ImExporterAccountInfo_GetBankCode(iea),
					   AB_ImExporterAccountInfo_GetAccountNumber(iea));
    if (!a) {
      DBG_ERROR(0, "Account %s/%s not found, aborting",
		AB_ImExporterAccountInfo_GetBankCode(iea),
		AB_ImExporterAccountInfo_GetAccountNumber(iea));
      AB_Job_List2_FreeAll(jobList);
      AB_ImExporterContext_free(ctx);
      return 3;
    }
    else {
      AB_TRANSACTION *t;

      t=AB_ImExporterAccountInfo_GetFirstTransaction(iea);
      while(t) {
	const char *rBankId;
	const char *rAccountId;
	AB_BANKINFO_CHECKRESULT res;
	AB_JOB *j;

        rBankId=AB_Transaction_GetRemoteBankCode(t);
	rAccountId=AB_Transaction_GetRemoteAccountNumber(t);
	res=AB_Banking_CheckAccount(ab,
				    "de",
				    0,
				    rBankId,
				    rAccountId);
	switch(res) {
	case AB_BankInfoCheckResult_NotOk:
	  DBG_ERROR(0,
		    "Invalid combination of bank code and account number "
		    "for remote account (%s/%s)",
		    rBankId, rAccountId);
	  AB_Job_List2_FreeAll(jobList);
	  AB_ImExporterContext_free(ctx);
	  return 3;

	case AB_BankInfoCheckResult_UnknownBank:
	  DBG_ERROR(0, "Remote bank code is unknown (%s/%s)",
		    rBankId, rAccountId);
	  if (forceCheck) {
	    AB_Job_List2_FreeAll(jobList);
	    AB_ImExporterContext_free(ctx);
	    return 4;
	  }
          break;

	case AB_BankInfoCheckResult_UnknownResult:
	  DBG_WARN(0,
		   "Indifferent result for remote account check (%s/%s)",
		   rBankId, rAccountId);
	  break;

	case AB_BankInfoCheckResult_Ok:
	  break;

	default:
	  DBG_ERROR(0, "Unknown check result %d", res);
	  AB_Job_List2_FreeAll(jobList);
	  AB_ImExporterContext_free(ctx);
	  return 4;
	}

	/* update transaction */
	if (AB_Transaction_GetTextKey(t)==0)
	  AB_Transaction_SetTextKey(t, 5);

	j=AB_JobSingleDebitNote_new(a);
	rv=AB_Job_CheckAvailability(j);
	if (rv<0) {
	  DBG_ERROR(0, "Job not supported.");
	  AB_Job_free(j);
	  AB_Job_List2_FreeAll(jobList);
	  AB_ImExporterContext_free(ctx);
	  return 3;
	}
	rv=AB_Job_SetTransaction(j, t);
	if (rv<0) {
	  DBG_ERROR(0, "Unable to add transaction for account %s/%s, aborting",
		    AB_ImExporterAccountInfo_GetBankCode(iea),
		    AB_ImExporterAccountInfo_GetAccountNumber(iea));
	  AB_Job_free(j);
	  AB_Job_List2_FreeAll(jobList);
	  AB_ImExporterContext_free(ctx);
	  return 3;
	}
	AB_Job_List2_PushBack(jobList, j);
	t=AB_ImExporterAccountInfo_GetNextTransaction(iea);
      } /* while t */
    }

    iea=AB_ImExporterContext_GetNextAccountInfo(ctx);
  } /* while */
  AB_ImExporterContext_free(ctx);

  /* execute jobs */
  rvExec=0;
  ctx=AB_ImExporterContext_new();
  rv=AB_Banking_ExecuteJobs(ab, jobList, ctx);
  if (rv) {
    fprintf(stderr, "Error on executeQueue (%d)\n", rv);
    rvExec=3;
  }
  AB_Job_List2_FreeAll(jobList);

  /* write context */
  rv=writeContext(ctxFile, ctx);
  if (rv<0) {
    AB_ImExporterContext_free(ctx);
    AB_Banking_OnlineFini(ab);
    AB_Banking_Fini(ab);
    return 4;
  }
  AB_ImExporterContext_free(ctx);

  /* that's is */
  rv=AB_Banking_OnlineFini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    AB_Banking_Fini(ab);
    if (rvExec)
      return rvExec;
    else
      return 5;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    if (rvExec)
      return rvExec;
    else
      return 5;
  }

  if (rvExec)
    return rvExec;
  else
    return 0;
}






