/***************************************************************************
 begin       : Tue Mar 25 2014
 copyright   : (C) 2014 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"

#include <aqbanking/jobsepadebitnote.h>

#include <gwenhywfar/text.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>


typedef enum
{
    AQBANKING_TOOL_SEPA_TRANSFERS
    , AQBANKING_TOOL_SEPA_DEBITNOTES
} AQBANKING_TOOL_MULTISEPA_TYPE;

static
int sepaMultiJobs(AB_BANKING *ab,
                   GWEN_DB_NODE *dbArgs,
                   int argc,
                   char **argv,
                   AQBANKING_TOOL_MULTISEPA_TYPE multisepa_type) {
  GWEN_DB_NODE *db;
  int rv;
  const char *ctxFile;
  const char *inFile;
  const char *importerName;
  const char *profileName;
  const char *profileFile;
  const char *bankId;
  const char *accountId;
  const char *subAccountId;
  int fillGaps, use_flash_debitnote;
  AB_IMEXPORTER_CONTEXT *ctx=0;
  AB_IMEXPORTER_ACCOUNTINFO *iea;
  AB_ACCOUNT *forcedAccount=NULL;
  AB_JOB_LIST2 *jobList;
  int rvExec, reallyExecute = 1, transactionLine = 0;
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
    "Specify the import profile to use",   /* short description */
    "Specify the import profile to use"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "profileFile",                /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "profile-file",               /* long option */
    "Specify the file to load the import profile from (WATCH OUT: Feature might be broken)",/* short description */
    "Specify the file to load the import profile from" /* long description */
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
        0, /* flags */
        GWEN_ArgsType_Int,           /* type */
        "useCOR1",                /* name */
        0,                            /* minnum */
        1,                            /* maxnum */
        0,                          /* short option */
        "use-COR1",                   /* long option */
        "If given, use COR1 variant of debit notes (faster), otherwise CORE (slower)",    /* short description */
        "If given, use COR1 variant of debit notes (faster), otherwise CORE (slower)"     /* long description */
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
  importerName=GWEN_DB_GetCharValue(db, "importerName", 0, "csv");
  profileName=GWEN_DB_GetCharValue(db, "profileName", 0,
                                   (multisepa_type == AQBANKING_TOOL_SEPA_TRANSFERS)
                                   ? "default"
                                   : "sepadebitnotes");
  profileFile=GWEN_DB_GetCharValue(db, "profileFile", 0, NULL);
  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);
  fillGaps=GWEN_DB_GetIntValue(db, "fillGaps", 0, 0);
  inFile=GWEN_DB_GetCharValue(db, "inFile", 0, 0);
  use_flash_debitnote = GWEN_DB_GetIntValue(db, "useCOR1", 0, 0);

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

  /* find local account to set later if requested */
  if (bankId || accountId) {
    AB_ACCOUNT_LIST2 *al;

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
      return 2;
    }
    forcedAccount=AB_Account_List2_GetFront(al);
    AB_Account_List2_free(al);
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

  /* fill gaps */
  if (fillGaps)
    AB_Banking_FillGapsInImExporterContext(ab, ctx);

  /* populate job list */
  jobList=AB_Job_List2_new();
  iea=AB_ImExporterContext_GetFirstAccountInfo(ctx);
  while(iea) {
    AB_ACCOUNT *a;
    AB_TRANSACTION *t;

    if (forcedAccount)
      a=forcedAccount;
    else {
      a=AB_Banking_GetAccountByCodeAndNumber(ab,
					     AB_ImExporterAccountInfo_GetBankCode(iea),
					     AB_ImExporterAccountInfo_GetAccountNumber(iea));
      if (!a) {
    DBG_ERROR(0, "Local account %s/%s not found, aborting",
		  AB_ImExporterAccountInfo_GetBankCode(iea),
		  AB_ImExporterAccountInfo_GetAccountNumber(iea));
	AB_Job_List2_FreeAll(jobList);
	AB_ImExporterContext_free(ctx);
	return 3;
      }
    }

    t=AB_ImExporterAccountInfo_GetFirstTransaction(iea);
    while(t) {
      AB_JOB *j;
      const char *rIBAN;
      const char *lIBAN;
      const char *lBIC;
      transactionLine++;

      if (forcedAccount) {
	AB_Transaction_SetLocalIban(t, AB_Account_GetIBAN(forcedAccount));
	AB_Transaction_SetLocalBic(t, AB_Account_GetBIC(forcedAccount));
      }

      rIBAN=AB_Transaction_GetRemoteIban(t);
      lIBAN=AB_Transaction_GetLocalIban(t);
      lBIC=AB_Transaction_GetLocalBic(t);

      /* preset local BIC and IBAN from account, if not set */
      if (!lBIC || !(*lBIC))
	lBIC=AB_Account_GetBIC(a);

      if (!lIBAN || !(*lIBAN))
	lIBAN=AB_Account_GetIBAN(a);

      /* check remote account */
      if (!rIBAN || !(*rIBAN)) {
          DBG_ERROR(0, "Missing remote IBAN, in line %d", transactionLine);
          reallyExecute = 0;
      }
      rv=AB_Banking_CheckIban(rIBAN);
      if (rv != 0) {
          DBG_ERROR(0, "Invalid remote IBAN (%s), in line %d", rIBAN, transactionLine);
          reallyExecute = 0;
      }

      /* check local account */
      if (!lBIC || !(*lBIC)) {
          DBG_ERROR(0, "Missing local BIC, in line %d", transactionLine);
          reallyExecute = 0;
      }
      if (!lIBAN || !(*lIBAN)) {
          DBG_ERROR(0, "Missing local IBAN, in line %d", transactionLine);
          reallyExecute = 0;
      }
      rv=AB_Banking_CheckIban(lIBAN);
      if (rv != 0) {
          DBG_ERROR(0, "Invalid local IBAN (%s), in line %d", lIBAN, transactionLine);
          reallyExecute = 0;
      }

      /* Create job */
      j = (multisepa_type == AQBANKING_TOOL_SEPA_TRANSFERS)
              // The command was sepatransfers, so we create JobSepaTransfer
              ? AB_JobSepaTransfer_new(a)
              // The command was sepadebitnotes, so we create some debit note
              : (use_flash_debitnote
                 // Did we have --use-COR1? Use this extra job type
                 ? AB_JobSepaFlashDebitNote_new(a)
                 // No COR1, just standard CORE debit note
                 : AB_JobSepaDebitNote_new(a));
      rv=AB_Job_CheckAvailability(j);
      if (rv<0) {
          DBG_ERROR(0, "Job not supported, in line %d.", transactionLine);
          reallyExecute = 0;
      }
      rv=AB_Job_SetTransaction(j, t);
      if (rv<0) {
          DBG_ERROR(0, "Unable to add transaction for account %s/%s, line %d, aborting",
                    AB_ImExporterAccountInfo_GetBankCode(iea),
                    AB_ImExporterAccountInfo_GetAccountNumber(iea),
                    transactionLine);
          reallyExecute = 0;
      }
      AB_Job_List2_PushBack(jobList, j);
      t=AB_ImExporterAccountInfo_GetNextTransaction(iea);
    } /* while t */

    iea=AB_ImExporterContext_GetNextAccountInfo(ctx);
  } /* while */
  AB_ImExporterContext_free(ctx);

  if (reallyExecute != 1)
  {
      AB_Job_List2_FreeAll(jobList);
      return 3;
  }

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
  AB_ImExporterContext_free(ctx);
  if (rv<0) {
    AB_Banking_OnlineFini(ab);
    AB_Banking_Fini(ab);
    return 4;
  }

  /* that's it */
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






