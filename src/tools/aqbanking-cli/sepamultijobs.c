/***************************************************************************
 begin       : Tue Mar 25 2014
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"

#include <aqbanking/types/transaction.h>

#include <gwenhywfar/text.h>



static GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv);

static int _createJobsFromContext(AB_IMEXPORTER_CONTEXT *ctx,
                                  const AB_ACCOUNT_SPEC_LIST *accountSpecList,
                                  AB_ACCOUNT_SPEC *forcedAccount,
                                  AB_TRANSACTION_COMMAND cmd,
                                  AB_TRANSACTION_LIST2 *jobList);





int sepaMultiJobs(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv,
                  AQBANKING_TOOL_MULTISEPA_TYPE multisepa_type)
{
  GWEN_DB_NODE *db;
  int rv;
  const char *ctxFile;
  const char *inFile;
  const char *importerName;
  const char *profileName;
  const char *profileFile;
  int use_flash_debitnote;
  AB_IMEXPORTER_CONTEXT *ctx=0;
  AB_ACCOUNT_SPEC *forcedAccount=NULL;
  AB_TRANSACTION_LIST2 *jobList;
  AB_ACCOUNT_SPEC_LIST *accountSpecList=NULL;
  AB_TRANSACTION_COMMAND cmd;
  int dryRun=0;
  int rvExec;

  /* parse command line arguments */
  db=_readCommandLine(dbArgs, argc, argv);
  if (db==NULL) {
    /* error in command line */
    return 1;
  }

  importerName=GWEN_DB_GetCharValue(db, "importerName", 0, "csv");
  profileName=GWEN_DB_GetCharValue(db, "profileName", 0,
                                   (multisepa_type == AQBANKING_TOOL_SEPA_TRANSFERS)
                                   ? "default"
                                   : "sepadebitnotes");
  profileFile=GWEN_DB_GetCharValue(db, "profileFile", 0, NULL);
  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);
  inFile=GWEN_DB_GetCharValue(db, "inFile", 0, 0);
  use_flash_debitnote=GWEN_DB_GetIntValue(db, "useCOR1", 0, 0);
  dryRun=GWEN_DB_GetIntValue(db, "dryRun", 0, 0);

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  rv=AB_Banking_GetAccountSpecList(ab, &accountSpecList);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    AB_Banking_Fini(ab);
    return 4;
  }

  /* find local account to set later if requested */
  if (GWEN_DB_VariableExists(db, "uniqueAccountId") ||
      GWEN_DB_VariableExists(db, "bankId") ||
      GWEN_DB_VariableExists(db, "accountId") ||
      GWEN_DB_VariableExists(db, "subAccountId") ||
      GWEN_DB_VariableExists(db, "iban")) {
    forcedAccount=pickAccountSpecForArgs(accountSpecList, db);
    if (forcedAccount==NULL) {
      DBG_ERROR(0, "Invalid account specification.");
      AB_Banking_Fini(ab);
      return 4;
    }
  }

  /* import new context */
  ctx=AB_ImExporterContext_new();
  rv=AB_Banking_ImportFromFileLoadProfile(ab, importerName, ctx,
                                          profileName, profileFile,
                                          inFile);
  if (rv<0) {
    DBG_ERROR(0, "Error reading file: %d", rv);
    AB_ImExporterContext_free(ctx);
    AB_Banking_Fini(ab);
    return 4;
  }

  /* create jobs from imported transactions */
  cmd=(multisepa_type == AQBANKING_TOOL_SEPA_TRANSFERS)
      // The command was sepatransfers, so we create JobSepaTransfer
      ? AB_Transaction_CommandSepaTransfer
      // The command was sepadebitnotes, so we create some debit note
      : (use_flash_debitnote
         // Did we have --use-COR1? Use this extra job type
         ? AB_Transaction_CommandSepaFlashDebitNote
         // No COR1, just standard CORE debit note
         : AB_Transaction_CommandSepaDebitNote);

  /* populate job list */
  jobList=AB_Transaction_List2_new();
  rv=_createJobsFromContext(ctx, accountSpecList, forcedAccount, cmd, jobList);
  AB_ImExporterContext_free(ctx);
  if (rv<0) {
    DBG_INFO(0, "Error (%d)", rv);
    writeJobsAsContextFile(jobList, ctxFile);
    AB_Transaction_List2_freeAll(jobList);
    AB_Banking_Fini(ab);
    return 3;
  }

  /* execute jobs */
  rvExec=0;
  if (dryRun) {
    DBG_NOTICE(0, "Dry-run requested, not sending jobs");
    writeJobsAsContextFile(jobList, ctxFile);
  }
  else {
    rv=execBankingJobs(ab, jobList, ctxFile);
    if (rv) {
      DBG_ERROR(0, "Error on executeQueue (%d)", rv);
      rvExec=3;
    }
  }
  AB_Transaction_List2_freeAll(jobList);

  /* that's it */
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



GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  const GWEN_ARGS args[]= {
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
      "accountType",                 /* name */
      0,                             /* minnum */
      1,                             /* maxnum */
      "t",                           /* short option */
      "accounttype",                       /* long option */
      "Specify the type of your account",      /* short description */
      "Specify the type of your account"       /* long description */
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
      0, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "dryRun",                     /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                           /* short option */
      "dryRun",                    /* long option */
      "If given jobs will not be really executed, just written to the context file",    /* short description */
      "If given jobs will not be really executed, just written to the context file"     /* long description */
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



int _createJobsFromContext(AB_IMEXPORTER_CONTEXT *ctx,
                           const AB_ACCOUNT_SPEC_LIST *accountSpecList,
                           AB_ACCOUNT_SPEC *forcedAccount,
                           AB_TRANSACTION_COMMAND cmd,
                           AB_TRANSACTION_LIST2 *jobList)
{
  AB_IMEXPORTER_ACCOUNTINFO *iea;
  int reallyExecute=1;
  int transactionLine=0;

  iea=AB_ImExporterContext_GetFirstAccountInfo(ctx);
  while (iea) {
    const AB_TRANSACTION *t;

    t=AB_ImExporterAccountInfo_GetFirstTransaction(iea, 0, 0);
    while (t) {
      AB_ACCOUNT_SPEC *as;
      AB_TRANSACTION *job=NULL;
      const char *rIBAN;
      const char *lIBAN;
      const char *lBIC;
      const AB_TRANSACTION_LIMITS *lim;
      int rv;

      job=AB_Transaction_dup(t);

      if (forcedAccount)
        as=forcedAccount;
      else
        as=pickAccountSpecForTransaction(accountSpecList, t);
      if (as==NULL) {
        DBG_ERROR(0, "Could not determine account for job in line %d", transactionLine);
        reallyExecute=0;
        AB_Transaction_SetStatus(job, AB_Transaction_StatusError);
      }

      /* fill missing fields in transaction from account spec */
      AB_Banking_FillTransactionFromAccountSpec(job, as);

      rIBAN=AB_Transaction_GetRemoteIban(job);
      lIBAN=AB_Transaction_GetLocalIban(job);
      lBIC=AB_Transaction_GetLocalBic(job);

      /* check remote account */
      if (!rIBAN || !(*rIBAN)) {
        DBG_ERROR(0, "Missing remote IBAN, in line %d", transactionLine);
        reallyExecute=0;
      }
      rv=AB_Banking_CheckIban(rIBAN);
      if (rv != 0) {
        DBG_ERROR(0, "Invalid remote IBAN (%s), in line %d", rIBAN, transactionLine);
        reallyExecute=0;
        AB_Transaction_SetStatus(job, AB_Transaction_StatusError);
      }

      /* check local account */
      if (!lBIC || !(*lBIC)) {
        DBG_WARN(0, "Missing local BIC, in line %d (ignoring, not needed anymore anyway)", transactionLine);
      }
      if (!lIBAN || !(*lIBAN)) {
        DBG_ERROR(0, "Missing local IBAN, in line %d", transactionLine);
        reallyExecute=0;
      }
      rv=AB_Banking_CheckIban(lIBAN);
      if (rv != 0) {
        DBG_ERROR(0, "Invalid local IBAN (%s), in line %d", lIBAN, transactionLine);
        reallyExecute=0;
        AB_Transaction_SetStatus(job, AB_Transaction_StatusError);
      }

      AB_Transaction_SetType(job,
                             (cmd==AB_Transaction_CommandSepaTransfer)
                             ? AB_Transaction_TypeTransfer
                             : AB_Transaction_TypeDebitNote
                            );

      lim=AB_AccountSpec_GetTransactionLimitsForCommand(as, cmd);
      if (lim==NULL) {
        DBG_ERROR(0, "Job %s not supported, in line %d.", AB_Transaction_Command_toString(cmd), transactionLine);
        reallyExecute=0;
        AB_Transaction_SetStatus(job, AB_Transaction_StatusError);
      }
      else {
        rv=checkTransactionLimits(job, lim,
                                  AQBANKING_TOOL_LIMITFLAGS_PURPOSE |
                                  AQBANKING_TOOL_LIMITFLAGS_NAMES |
                                  AQBANKING_TOOL_LIMITFLAGS_SEPA);
        if (rv<0) {
          DBG_ERROR(0, "Job %s violates limits, in line %d.", AB_Transaction_Command_toString(cmd), transactionLine);
          reallyExecute=0;
          AB_Transaction_SetStatus(job, AB_Transaction_StatusError);
        }
      }
      AB_Transaction_SetCommand(job, cmd);

      AB_Transaction_List2_PushBack(jobList, job);
      transactionLine++;
      t=AB_Transaction_List_Next(t);
    } /* while t */

    iea=AB_ImExporterAccountInfo_List_Next(iea);
  } /* while */

  if (reallyExecute==0)
    return GWEN_ERROR_GENERIC;
  return 0;
}




