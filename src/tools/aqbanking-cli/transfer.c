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

#include <aqbanking/account.h>
#include <aqbanking/jobsingletransfer.h>
#include <aqbanking/jobcreatedatedtransfer.h>
#include <aqbanking/jobcreatesto.h>
#include <aqbanking/jobdeletesto.h>

#include <gwenhywfar/text.h>

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
  const char *bankId;
  const char *accountId;
  const char *subAccountId;
  AB_JOB_TYPE jobType;
  AB_IMEXPORTER_CONTEXT *ctx=0;
  AB_ACCOUNT_LIST2 *al;
  AB_ACCOUNT *a;
  AB_TRANSACTION *t;
  AB_JOB_LIST2 *jobList;
  AB_JOB *j;
  int rvExec;
  const char *rBankId;
  const char *rAccountId;
  int forceCheck;
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
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,                
    GWEN_ArgsType_Char,
    "executionDate", 
    0,  
    1,  
    0, 
    "executionDate",
    "set execution date of transfer",
    "set execution date of transfer"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,                
    GWEN_ArgsType_Char,
    "firstExecutionDate", 
    0,  
    1,  
    0, 
    "firstExecutionDate",
    "set date of first execution (standing orders)",
    "set date of first execution (standing orders)"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,                
    GWEN_ArgsType_Char,
    "lastExecutionDate", 
    0,  
    1,  
    0, 
    "lastExecutionDate",
    "set date of last execution (standing orders)",
    "set date of last execution (standing orders)"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,                
    GWEN_ArgsType_Int,
    "executionDay", 
    0,  
    1,  
    0, 
    "executionDay",
    "set day of execution (standing orders)",
    "set day of execution (standing orders)"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,                
    GWEN_ArgsType_Int,
    "executionCycle", 
    0,  
    1,  
    0, 
    "executionCycle",
    "set execution cycle (standing orders)",
    "set execution cycle (standing orders)"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,                
    GWEN_ArgsType_Char,
    "executionPeriod", 
    0,  
    1,  
    0, 
    "executionPeriod",
    "set execution period (standing orders)",
    "set execution period (standing orders)"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,
    GWEN_ArgsType_Char,
    "fiId",                       /* HKDAL */
    0,
    1,
    0,
    "fiId",
    "set the fiId (standing orders)",
    "set the fiId (standing orders) - Auftragsidentifikation fuer HKDAL"
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
  forceCheck=GWEN_DB_GetIntValue(db, "forceCheck", 0, 0);
  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);

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
  a=AB_Account_List2_GetFront(al);
  AB_Account_List2_free(al);

  /* create transaction from arguments */
  t=mkTransfer(a, db, &jobType);
  if (t==NULL) {
    DBG_ERROR(0, "Could not create transaction from arguments");
    return 2;
  }

  if (AB_Transaction_GetTextKey(t)==0)
    AB_Transaction_SetTextKey(t, 51);

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
    AB_Transaction_free(t);
    return 3;

  case AB_BankInfoCheckResult_UnknownBank:
    DBG_ERROR(0, "Remote bank code is unknown (%s/%s)",
	      rBankId, rAccountId);
    if (forceCheck) {
      AB_Transaction_free(t);
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
    AB_Transaction_free(t);
    return 4;
  }

  if (jobType==AB_Job_TypeTransfer)
    j=AB_JobSingleTransfer_new(a);
  else if (jobType==AB_Job_TypeCreateDatedTransfer)
    j=AB_JobCreateDatedTransfer_new(a);
  else if (jobType==AB_Job_TypeCreateStandingOrder)
    j=AB_JobCreateStandingOrder_new(a);
  else if (jobType==AB_Job_TypeDeleteStandingOrder)
    j=AB_JobDeleteStandingOrder_new(a);
  else {
    DBG_ERROR(0, "Unknown job type");
    AB_Transaction_free(t);
    return 6;
  }

  rv=AB_Job_CheckAvailability(j);
  if (rv<0) {
    DBG_ERROR(0, "Job not supported.");
    AB_Job_free(j);
    AB_Transaction_free(t);
    return 3;
  }

  rv=AB_Job_SetTransaction(j, t);
  AB_Transaction_free(t);
  if (rv<0) {
    DBG_ERROR(0, "Unable to add transaction");
    AB_Job_free(j);
    return 3;
  }

  /* populate job list */
  jobList=AB_Job_List2_new();
  AB_Job_List2_PushBack(jobList, j);

  /* execute job */
  rvExec=0;
  ctx=AB_ImExporterContext_new();
  rv=AB_Banking_ExecuteJobs(ab, jobList, ctx);
  if (rv) {
    fprintf(stderr, "Error on executeQueue (%d)\n", rv);
    rvExec=3;
  }
  AB_Job_List2_FreeAll(jobList);

  /* write result */
  rv=writeContext(ctxFile, ctx);
  AB_ImExporterContext_free(ctx);
  if (rv<0) {
    DBG_ERROR(0, "Error writing context file (%d)", rv);
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






