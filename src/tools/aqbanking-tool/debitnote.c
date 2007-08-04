/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
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
#include <gwenhywfar/text.h>
#include <aqbanking/jobsingledebitnote.h>



int debitNote(AB_BANKING *ab,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv) {
  AB_ACCOUNT_LIST2 *al;
  GWEN_DB_NODE *db;
  int rv;
  int jobOk=0;
  AB_ACCOUNT *matchingAcc=0;
  int forceCheck=0;
  int doExec=0;

  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
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
    GWEN_ArgsTypeChar,            /* type */
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
    GWEN_ArgsTypeChar,            /* type */
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
    GWEN_ArgsTypeChar,            /* type */
    "accountName",                /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "n",                          /* short option */
    "accountname",                    /* long option */
    "Specify the account name",     /* short description */
    "Specify the account name"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,  /* flags */
    GWEN_ArgsTypeChar,             /* type */
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
    GWEN_ArgsTypeChar,             /* type */
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
    GWEN_ArgsTypeChar,            /* type */
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
    GWEN_ArgsTypeChar,            /* type */
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
    GWEN_ArgsTypeInt,             /* type */
    "textkey",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "t",                          /* short option */
    "textkey",                    /* long option */
    "Specify the text key (05 for normal debit note)",  /* short description */
    "Specify the text key (05 for normal debit note)"   /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
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
    GWEN_ArgsTypeChar,            /* type */
    "purpose",                    /* name */
    1,                            /* minnum */
    6,                            /* maxnum */
    "p",                          /* short option */
    "purpose",                    /* long option */
    "Specify the purpose",        /* short description */
    "Specify the purpose"         /* long description */
  },
  {
    0,                            /* flags */
    GWEN_ArgsTypeInt,             /* type */
    "forceCheck",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "force-check",                /* long option */
    "check of remote account must succeed",  /* short description */
    0
  },
  {
    0, /* flags */
    GWEN_ArgsTypeInt,            /* type */
    "exec",                      /* name */
    0,                           /* minnum */
    1,                           /* maxnum */
    "x",                         /* short option */
    "exec",                      /* long option */
    /* short description */
    "Immediately execute the queue after enqueuing transfer",
    /* long description */
    "Immediately execute the queue after enqueuing transfer"
  },

  {
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsTypeInt,             /* type */
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
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutTypeTXT)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return 1;
    }
    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }

  forceCheck=GWEN_DB_VariableExists(db, "forceCheck");
  doExec=GWEN_DB_VariableExists(db, "exec");

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(AQT_LOGDOMAIN, "Error on init (%d)", rv);
    return 2;
  }

  al=AB_Banking_GetAccounts(ab);
  if (al) {
    AB_ACCOUNT_LIST2_ITERATOR *ait;

    ait=AB_Account_List2_First(al);
    if (ait) {
      AB_ACCOUNT *a;
      AB_TRANSACTION *t;
      AB_JOB *j;
      AB_BANKINFO_CHECKRESULT res;
      const char *bankId;
      const char *accountId;
      const char *bankName;
      const char *accountName;
      const char *s;

      bankId=GWEN_DB_GetCharValue(db, "bankId", 0, 0);
      bankName=GWEN_DB_GetCharValue(db, "bankName", 0, 0);
      accountId=GWEN_DB_GetCharValue(db, "accountId", 0, 0);
      accountName=GWEN_DB_GetCharValue(db, "accountName", 0, 0);

      a=AB_Account_List2Iterator_Data(ait);
      assert(a);
      while(a) {
        int matches=1;

        if (matches && bankId) {
          s=AB_Account_GetBankCode(a);
          if (!s || !*s || -1==GWEN_Text_ComparePattern(s, bankId, 0))
            matches=0;
        }

        if (matches && bankName) {
          s=AB_Account_GetBankName(a);
          if (!s || !*s || -1==GWEN_Text_ComparePattern(s, bankName, 0))
            matches=0;
        }

        if (matches && accountId) {
          s=AB_Account_GetAccountNumber(a);
          if (!s || !*s || -1==GWEN_Text_ComparePattern(s, accountId, 0))
            matches=0;
        }
        if (matches && accountName) {
          s=AB_Account_GetAccountName(a);
          if (!s || !*s || -1==GWEN_Text_ComparePattern(s, accountName, 0))
            matches=0;
        }

        if (matches) {
          if (matchingAcc) {
            DBG_ERROR(AQT_LOGDOMAIN, "Ambiguous local account specification.");
            return 2;
          }
          matchingAcc=a;
        }

        a=AB_Account_List2Iterator_Next(ait);
      }
      AB_Account_List2Iterator_free(ait);
      if (matchingAcc==0) {
        DBG_ERROR(AQT_LOGDOMAIN, "No matching local account.");
        return 2;
      }

      /* Create transaction */
      t=mkTransfer(matchingAcc, db);
      if (!t) {
        DBG_ERROR(AQT_LOGDOMAIN, "Could not create transfer from arguments");
        return 1;
      }
      if (AB_Transaction_GetTextKey(t)<1)
        AB_Transaction_SetTextKey(t, 5);
      j=AB_JobSingleDebitNote_new(matchingAcc);
      rv=AB_Job_CheckAvailability(j);
      if (rv) {
        DBG_ERROR(AQT_LOGDOMAIN, "Jobs is not available with this account");
        return 3;
      }
      res=accountcheck(ab,
		       AB_Transaction_GetRemoteCountry(t),
		       AB_Transaction_GetRemoteBankCode(t),
		       AB_Transaction_GetRemoteAccountNumber(t),
		       forceCheck);
      if (res == AB_BankInfoCheckResult_NotOk
	  || (forceCheck && (res == AB_BankInfoCheckResult_UnknownBank
			     || res == AB_BankInfoCheckResult_UnknownResult)))
	return 1;

      rv=AB_JobSingleDebitNote_SetTransaction(j, t);
      if (rv) {
        DBG_ERROR(AQT_LOGDOMAIN, "Could not store transaction to job (%d)", rv);
        return 3;
      }
      rv=AB_Banking_EnqueueJob(ab, j);
      if (rv) {
        DBG_ERROR(AQT_LOGDOMAIN, "Could not enqueue job (%d)", rv);
        return 3;
      }
      else {
        DBG_INFO(AQT_LOGDOMAIN, "Job successfully enqueued");

        if (doExec) {
          AB_JOB_LIST2 *jl;
          AB_IMEXPORTER_CONTEXT *ctx;

          ctx=AB_ImExporterContext_new();
          jl=AB_Job_List2_new();
          AB_Job_List2_PushBack(jl, j);
          rv=AB_Banking_ExecuteJobListWithCtx(ab, jl, ctx);
          AB_Job_List2_free(jl);
          AB_ImExporterContext_free(ctx);
          if (rv) {
            DBG_ERROR(AQT_LOGDOMAIN, "Error executing queue: %d", rv);
            jobOk=-1;
          }
          else {
            switch(AB_Job_GetStatus(j)) {
            case AB_Job_StatusPending:
              jobOk=2;
              break;
            case AB_Job_StatusFinished:
              jobOk=1;
              break;
            case AB_Job_StatusError:
            default:
              jobOk=-2;
            }
          }
        }
        else
          jobOk=1;
      }
    }
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  switch(jobOk) {
  case 0:
    return 3;
  case 1:
    return 0;
  case 2:
    return 99;
  case -1:
    return 3;
  case -2:
  default:
    return 4;
  }
}



AB_BANKINFO_CHECKRESULT accountcheck(AB_BANKING *ab, const char *country,
				     const char *bankcode, const char *accountid,
				     int forceCheck)
{
  AB_BANKINFO_CHECKRESULT res=
    AB_Banking_CheckAccount(ab,
			    country,
			    0,
			    bankcode, accountid);
  switch(res) {
  case AB_BankInfoCheckResult_NotOk:
    DBG_ERROR(AQT_LOGDOMAIN,
	      "Invalid remote bank code and account number: Bank code %s, account %s",
	      bankcode, accountid);
    break;

  case AB_BankInfoCheckResult_UnknownBank:
    if (forceCheck) {
      DBG_ERROR(AQT_LOGDOMAIN, "Remote bank code %s is unknown at account %s",
		bankcode, accountid);
    }
    break;
  case AB_BankInfoCheckResult_UnknownResult:
    if (forceCheck) {
      DBG_ERROR(AQT_LOGDOMAIN,
		"Indifferent result for remote account check: Bank code %s, account %s",
		bankcode, accountid);
    }
    break;
  case AB_BankInfoCheckResult_Ok:
    break;
  default:
    DBG_ERROR(AQT_LOGDOMAIN, "Unknown check result %d at Bank code %s, account %s",
	      res, bankcode, accountid);
  }
  return res;
}

