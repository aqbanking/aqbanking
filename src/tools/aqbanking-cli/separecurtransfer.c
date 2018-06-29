/***************************************************************************
 begin       : Sat Dec 28 2013
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"

#include <aqbanking/account.h>
#include <aqbanking/job.h>
#include <aqbanking/jobsepacreatesto.h>
#include <aqbanking/jobsepamodifysto.h>
#include <aqbanking/jobsepadeletesto.h>

#include <gwenhywfar/text.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>



static
int sepaRecurTransfer(AB_BANKING *ab,
                      GWEN_DB_NODE *dbArgs,
                      int argc,
                      char **argv) {
  GWEN_DB_NODE *db;
  int rv;
  const char *ctxFile;
  const char *bankId;
  const char *accountId;
  const char *subAccountId;
  AB_IMEXPORTER_CONTEXT *ctx=0;
  AB_ACCOUNT_LIST2 *al;
  AB_ACCOUNT *a;
  AB_TRANSACTION *t;
  AB_JOB_LIST2 *jobList;
  AB_JOB *j;
  int rvExec;
  int createSto=0;
  int modifySto=0;
  int deleteSto=0;
  AB_JOB_TYPE jobType;
  const char *rIBAN;
  const char *lIBAN;
  const char *s;
  const GWEN_ARGS args[]={
  {
    0,
    GWEN_ArgsType_Int,
    "createSto",
    0,
    1,
    0,
    "create",
    "Create standing orders",
    "Create standing orders"
  },
  {
    0,
    GWEN_ArgsType_Int,
    "modifySto",
    0,
    1,
    0,
    "modify",
    "Modify standing orders",
    "Modify standing orders"
  },
  {
    0,
    GWEN_ArgsType_Int,
    "deleteSto",
    0,
    1,
    0,
    "delete",
    "Delete standing orders",
    "Delete standing orders"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,   /* flags */
    GWEN_ArgsType_Char,             /* type */
    "ctxFile",                      /* name */
    0,                              /* minnum */
    1,                              /* maxnum */
    "c",                            /* short option */
    "ctxfile",                      /* long option */
    "Specify the file to store the context in",    /* short description */
    "Specify the file to store the context in"    /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,   /* flags */
    GWEN_ArgsType_Char,             /* type */
    "bankId",                       /* name */
    0,                              /* minnum */
    1,                              /* maxnum */
    "b",                            /* short option */
    "bank",                         /* long option */
    "overwrite the bank code",      /* short description */
    "overwrite the bank code"       /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,   /* flags */
    GWEN_ArgsType_Char,             /* type */
    "accountId",                    /* name */
    0,                              /* minnum */
    1,                              /* maxnum */
    "a",                            /* short option */
    "account",                      /* long option */
    "overwrite the account number", /* short description */
    "overwrite the account number"  /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,   /* flags */
    GWEN_ArgsType_Char,             /* type */
    "name",                         /* name */
    0,                              /* minnum */
    1,                              /* maxnum */
    0,                              /* short option */
    "name",                         /* long option */
    "Specify your name",            /* short description */
    "Specify your name"             /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,   /* flags */
    GWEN_ArgsType_Char,             /* type */
    "subAccountId",                 /* name */
    0,                              /* minnum */
    1,                              /* maxnum */
    "aa",                           /* short option */
    "subaccount",                   /* long option */
    "Specify the sub account id (Unterkontomerkmal)",    /* short description */
    "Specify the sub account id (Unterkontomerkmal)"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,   /* flags */
    GWEN_ArgsType_Char,             /* type */
    "remoteBIC",                    /* name */
    0,                              /* minnum */
    1,                              /* maxnum */
    0,                              /* short option */
    "rbic",                         /* long option */
    "Specify the remote SWIFT BIC", /* short description */
    "Specify the remote SWIFT BIC"  /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,   /* flags */
    GWEN_ArgsType_Char,             /* type */
    "remoteIBAN",                   /* name */
    1,                              /* minnum */
    1,                              /* maxnum */
    0,                              /* short option */
    "riban",                        /* long option */
    "Specify the remote IBAN",      /* short description */
    "Specify the remote IBAN"       /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,   /* flags */
    GWEN_ArgsType_Char,             /* type */
    "value",                        /* name */
    1,                              /* minnum */
    1,                              /* maxnum */
    "v",                            /* short option */
    "value",                        /* long option */
    "Specify the transfer amount",  /* short description */
    "Specify the transfer amount"   /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,   /* flags */
    GWEN_ArgsType_Char,             /* type */
    "remoteName",                   /* name */
    1,                              /* minnum */
    2,                              /* maxnum */
    0,                              /* short option */
    "rname",                        /* long option */
    "Specify the remote name",      /* short description */
    "Specify the remote name"       /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,   /* flags */
    GWEN_ArgsType_Char,             /* type */
    "purpose",                      /* name */
    1,                              /* minnum */
    6,                              /* maxnum */
    "p",                            /* short option */
    "purpose",                      /* long option */
    "Specify the purpose",          /* short description */
    "Specify the purpose"           /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,   /* flags */
    GWEN_ArgsType_Char,             /* type */
    "endToEndReference",            /* name */
    0,                              /* minnum */
    1,                              /* maxnum */
    "E",                            /* short option */
    "endtoendid",                   /* long option */
    "Specify the SEPA End-to-end-reference",        /* short description */
    "Specify the SEPA End-to-end-reference"         /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,
    GWEN_ArgsType_Char,
    "firstExecutionDate",
    0,
    1,
    0,
    "firstExecutionDate",
    "set date of first execution",
    "set date of first execution"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,
    GWEN_ArgsType_Char,
    "nextExecutionDate",            /* HKCDL and HKCDN only */
    0,
    1,
    0,
    "nextExecutionDate",
    "set date of next execution",
    "set this date given from the command request --sepaSto   (delete and modify sto only)"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,
    GWEN_ArgsType_Char,
    "lastExecutionDate",
    0,
    1,
    0,
    "lastExecutionDate",
    "set date of last execution",
    "set date of last execution"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,                
    GWEN_ArgsType_Int,
    "executionDay", 
    1,
    1,  
    0, 
    "executionDay",
    "set day of execution",
    "set day of execution"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,                
    GWEN_ArgsType_Int,
    "executionCycle", 
    1,
    1,  
    0, 
    "executionCycle",
    "set execution cycle",
    "set execution cycle"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,                
    GWEN_ArgsType_Char,
    "executionPeriod", 
    1,
    1,  
    0, 
    "executionPeriod",
    "set execution period",
    "set execution period"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,
    GWEN_ArgsType_Char,
    "fiId",                         /* HKCDL */
    0,
    1,
    0,
    "fiId",
    "set the fiId (standing orders)",
    "set the fiId (standing orders) - Auftragsidentifikation for HKCDL or HKCDN"
  },
  {
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsType_Int,              /* type */
    "help",                         /* name */
    0,                              /* minnum */
    0,                              /* maxnum */
    "h",                            /* short option */
    "help",                         /* long option */
    "Show this help screen",        /* short description */
    "Show this help screen"         /* long description */
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

  createSto=GWEN_DB_GetIntValue(db, "createSto", 0, 0);
  modifySto=GWEN_DB_GetIntValue(db, "modifySto", 0, 0);
  deleteSto=GWEN_DB_GetIntValue(db, "deleteSto", 0, 0);
  s=GWEN_DB_GetCharValue(db, "fiId", 0, 0);

  if (createSto)
    jobType=AB_Job_TypeSepaCreateStandingOrder;
  else if (modifySto)
    if (s && *s)
      jobType=AB_Job_TypeSepaModifyStandingOrder;
    else {
      DBG_ERROR(0, "Missing fiId");
      return 2;
    }
  else if (deleteSto) {
    jobType=AB_Job_TypeSepaDeleteStandingOrder;
  }
  else {
    DBG_ERROR(0, "Missing Option: '--create', '--delete' or '--modify'");
    return 2;
  }

  /* create transaction from arguments */
  t=mkSepaTransfer(a, db, jobType);
  if (t==NULL) {
    DBG_ERROR(0, "Could not create SEPA transaction from arguments");
    AB_Banking_OnlineFini(ab);
    AB_Banking_Fini(ab);
    return 2;
  }
  AB_Transaction_SetType(t, AB_Transaction_TypeSepaTransfer);
  AB_Transaction_SetSubType(t, AB_Transaction_SubTypeStandingOrder);

  /* determine the type of job and create it */
  if (jobType==AB_Job_TypeSepaCreateStandingOrder)
    j=AB_JobSepaCreateStandingOrder_new(a);
  else if (jobType==AB_Job_TypeSepaModifyStandingOrder)
    j=AB_JobSepaModifyStandingOrder_new(a);
  else
    j=AB_JobSepaDeleteStandingOrder_new(a);

  rv=AB_Job_CheckAvailability(j);
  if (rv<0) {
    DBG_ERROR(0, "Job not supported.");
    AB_Job_free(j);
    AB_Transaction_free(t);
    return 3;
  }

  /* check remote IBAN */
  rIBAN=AB_Transaction_GetRemoteIban(t);
  if (!rIBAN || !(*rIBAN)) {
    DBG_ERROR(0, "Missing remote IBAN");
    AB_Job_free(j);
    AB_Transaction_free(t);
    return 1;
  }
  rv=AB_Banking_CheckIban(rIBAN);
  if (rv != 0) {
    DBG_ERROR(0, "Invalid remote IBAN (%s)", rIBAN);
    AB_Job_free(j);
    AB_Transaction_free(t);
    return 3;
  }

  /* check local IBAN */
  lIBAN=AB_Transaction_GetLocalIban(t);
  rv=AB_Banking_CheckIban(lIBAN);
  if (rv != 0) {
    DBG_ERROR(0, "Invalid local IBAN (%s)", rIBAN);
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

