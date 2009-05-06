/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: import.c 764 2006-01-13 14:00:00Z cstim $
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"

#include <aqbanking/account.h>
#include <aqbanking/jobsingletransfer.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/io_file.h>
#include <gwenhywfar/iomanager.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>



static
int transfer(AB_BANKING *ab,
	     GWEN_DB_NODE *dbArgs,
	     int argc,
	     char **argv) {
  GWEN_DB_NODE *db;
  int rv;
  const char *ctxFile;
  const char *country;
  const char *bankId;
  const char *accountId;
  GWEN_DB_NODE *dbCtx;
  AB_IMEXPORTER_CONTEXT *ctx=0;
  AB_ACCOUNT_LIST2 *al;
  AB_ACCOUNT *a;
  AB_TRANSACTION *t;
  AB_JOB_LIST2 *jobList;
  AB_JOB *j;
  int fd;
  int rvExec;
  const char *rCountry;
  const char *rBankId;
  const char *rAccountId;
  int forceCheck;
#ifdef WITH_AQFINANCE
  int useDb;
#endif
  AB_BANKINFO_CHECKRESULT res;
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
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,  /* flags */
    GWEN_ArgsType_Char,             /* type */
    "remoteCountry",               /* name */
    0,                             /* minnum */
    1,                             /* maxnum */
    0,                             /* short option */
    "rcountry",                    /* long option */
    "Specify the remote country code",/* short description */
    "Specify the remote country code" /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,  /* flags */
    GWEN_ArgsType_Char,             /* type */
    "remoteBankId",                /* name */
    1,                             /* minnum */
    1,                             /* maxnum */
    0,                             /* short option */
    "rbank",                       /* long option */
    "Specify the remote bank code",/* short description */
    "Specify the remote bank code" /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "remoteAccountId",                  /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "raccount",                    /* long option */
    "Specify the remote account number",     /* short description */
    "Specify the remote account number"      /* long description */
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
    GWEN_ArgsType_Int,             /* type */
    "textkey",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "t",                          /* short option */
    "textkey",                    /* long option */
    "Specify the text key (51 for normal transfer)",  /* short description */
    "Specify the text key (51 for normal transfer)"   /* long description */
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
#ifdef WITH_AQFINANCE
  {
    0,                
    GWEN_ArgsType_Int,
    "usedb",
    0,  
    1,  
    0, 
    "usedb",
    "store transfer in internal database",
    "store transfer in internal database"
  },
#endif
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

  country=GWEN_DB_GetCharValue(db, "country", 0, "de");
  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, 0);
  accountId=GWEN_DB_GetCharValue(db, "accountId", 0, 0);
  forceCheck=GWEN_DB_GetIntValue(db, "forceCheck", 0, 0);
  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);
#ifdef WITH_AQFINANCE
  useDb=GWEN_DB_GetIntValue(db, "usedb", 0, 0);
  if (useDb && ctxFile) {
    fprintf(stderr, "Option \"-c\" doesn't work with \"--usedb\"\n");
    return 1;
  }
#endif

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  rv=AB_Banking_OnlineInit(ab, 0);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  initDbCerts(ab);

  /* get account */
  al=AB_Banking_FindAccounts(ab, "*", "*", bankId, accountId);
  if (AB_Account_List2_GetSize(al)==0) {
    DBG_ERROR(0, "Account not found");
    return 2;
  }
  else if (AB_Account_List2_GetSize(al)>1) {
    DBG_ERROR(0, "Ambiguous account specification");
    return 2;
  }
  a=AB_Account_List2_GetFront(al);
  //AB_Account_List2_free(al);

  /* populate job list */
  jobList=AB_Job_List2_new();

  /* create transaction from arguments */
  t=mkTransfer(a, db);
  if (t==NULL) {
    DBG_ERROR(0, "Could not create transaction from arguments");
    return 2;
  }

  if (AB_Transaction_GetTextKey(t)==0)
    AB_Transaction_SetTextKey(t, 51);

  rCountry=AB_Transaction_GetRemoteCountry(t);
  if (rCountry==NULL)
    rCountry=country;
  rBankId=AB_Transaction_GetRemoteBankCode(t);
  rAccountId=AB_Transaction_GetRemoteAccountNumber(t);
  res=AB_Banking_CheckAccount(ab,
			      rCountry,
			      0,
			      rBankId,
			      rAccountId);
  switch(res) {
  case AB_BankInfoCheckResult_NotOk:
    DBG_ERROR(0,
	      "Invalid combination of bank code and account number "
	      "for remote account (%s/%s)",
	      rBankId, rAccountId);
    return 3;

  case AB_BankInfoCheckResult_UnknownBank:
    DBG_ERROR(0, "Remote bank code is unknown (%s/%s)",
	      rBankId, rAccountId);
    if (forceCheck) {
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
    return 4;
  }

  j=AB_JobSingleTransfer_new(a);
  rv=AB_Job_CheckAvailability(j, 0);
  if (rv<0) {
    DBG_ERROR(0, "Job not supported.");
    AB_ImExporterContext_free(ctx);
    return 3;
  }

#ifdef WITH_AQFINANCE
  if (useDb) {
    rv=AFM_add_transfer(ab, t, AE_Statement_StatusSending);
    if (rv<0) {
      DBG_ERROR(0, "Unable to store transfer to DB (%d)", rv);
      AB_ImExporterContext_free(ctx);
      return 3;
    }
  }
#endif
  rv=AB_JobSingleTransfer_SetTransaction(j, t);
  if (rv<0) {
    DBG_ERROR(0, "Unable to add transaction");
    AB_ImExporterContext_free(ctx);
    return 3;
  }
  AB_Job_List2_PushBack(jobList, j);

  /* execute job */
  rvExec=0;
  ctx=AB_ImExporterContext_new();
  rv=AB_Banking_ExecuteJobs(ab, jobList, ctx, 0);
  if (rv) {
    fprintf(stderr, "Error on executeQueue (%d)\n", rv);
    rvExec=3;
  }

#ifdef WITH_AQFINANCE
  if (useDb) {
    rv=AFM_update_transfers(ab, ctx);
    if (rv<0) {
      DBG_ERROR(0, "Unable to update transfers in DB (%d)", rv);
      AB_ImExporterContext_free(ctx);
      return 4;
    }
  }
  else {
#endif
    /* write context */
    dbCtx=GWEN_DB_Group_new("context");
    if (AB_ImExporterContext_toDb(ctx, dbCtx)) {
      DBG_ERROR(0, "Error writing context to db");
      return 4;
    }
    if (ctxFile==0)
      fd=fileno(stdout);
    else
      fd=open(ctxFile, O_RDWR|O_CREAT|O_TRUNC,
	      S_IRUSR | S_IWUSR
#ifdef S_IRGRP
	      | S_IRGRP
#endif
#ifdef S_IWGRP
	      | S_IWGRP
#endif
	     );
    if (fd<0) {
      DBG_ERROR(0, "Error selecting output file: %s",
		strerror(errno));
      return 4;
    }
    else {
      GWEN_DB_NODE *dbCtx;
      int rv;

      dbCtx=GWEN_DB_Group_new("context");
      rv=AB_ImExporterContext_toDb(ctx, dbCtx);
      if (rv<0) {
	DBG_ERROR(0, "Error writing context to db (%d)", rv);
	GWEN_DB_Group_free(dbCtx);
	finiDbCerts(ab);
	AB_Banking_OnlineFini(ab, 0);
	AB_Banking_Fini(ab);
	return 4;
      }

      rv=GWEN_DB_WriteToFd(dbCtx, fd,
			   GWEN_DB_FLAGS_DEFAULT,
			   0,
			   2000);
      if (rv<0) {
	DBG_ERROR(0, "Error writing context (%d)", rv);
	GWEN_DB_Group_free(dbCtx);
	if (ctxFile)
	  close(fd);
	finiDbCerts(ab);
	AB_Banking_OnlineFini(ab, 0);
	AB_Banking_Fini(ab);
	return 4;
      }

      if (ctxFile) {
	if (close(fd)) {
	  DBG_ERROR(0, "Error writing context (%d)", rv);
	  GWEN_DB_Group_free(dbCtx);
	  finiDbCerts(ab);
	  AB_Banking_OnlineFini(ab, 0);
	  AB_Banking_Fini(ab);
	  return 4;
	}
      }

      GWEN_DB_Group_free(dbCtx);
    }
#ifdef WITH_AQFINANCE
  }
#endif


  /* that's it */
  finiDbCerts(ab);

  rv=AB_Banking_OnlineFini(ab, 0);
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






