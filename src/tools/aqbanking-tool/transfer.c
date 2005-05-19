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
#include <aqbanking/jobsingletransfer.h>



int transfer(AB_BANKING *ab,
             GWEN_DB_NODE *dbArgs,
             int argc,
             char **argv) {
  AB_ACCOUNT_LIST2 *al;
  GWEN_DB_NODE *db;
  int rv;
  int jobOk=0;
  AB_ACCOUNT *matchingAcc=0;
  int forceCheck=0;
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
    "Specify the text key (51 for normal transfer)",  /* short description */
    "Specify the text key (51 for normal transfer)"   /* long description */
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

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
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
            DBG_ERROR(0, "Ambiguous local account specification.");
            return 2;
          }
          matchingAcc=a;
        }

        a=AB_Account_List2Iterator_Next(ait);
      }
      AB_Account_List2Iterator_free(ait);
      if (matchingAcc==0) {
        DBG_ERROR(0, "No matching local account.");
        return 2;
      }

      /* Create transaction */
      t=mkTransfer(matchingAcc, db);
      if (!t) {
        DBG_ERROR(0, "Could not create transfer from arguments");
        return 1;
      }
      if (AB_Transaction_GetTextKey(t)<1)
        AB_Transaction_SetTextKey(t, 51);
      j=AB_JobSingleTransfer_new(matchingAcc);
      rv=AB_Job_CheckAvailability(j);
      if (rv) {
        DBG_ERROR(0, "Jobs is not available with this account");
        return 3;
      }
      res=AB_Banking_CheckAccount(ab,
                                  AB_Transaction_GetRemoteCountry(t),
                                  0,
                                  AB_Transaction_GetRemoteBankCode(t),
                                  AB_Transaction_GetRemoteAccountNumber(t));
      switch(res) {
      case AB_BankInfoCheckResult_NotOk:
        DBG_ERROR(0,
                  "Invalid combination of bank code and account number "
                  "for remote account");
        return 1;

      case AB_BankInfoCheckResult_UnknownBank:
        if (forceCheck) {
          DBG_ERROR(0, "Remote bank code is unknown");
          return 1;
        }
        break;
      case AB_BankInfoCheckResult_UnknownResult:
        if (forceCheck) {
          DBG_ERROR(0,
                    "Indifferent result for remote account check");
          return 1;
        }
        break;
      case AB_BankInfoCheckResult_Ok:
        break;
      default:
        DBG_ERROR(0, "Unknown check result %d", res);
        return 3;
      }

      rv=AB_JobSingleTransfer_SetTransaction(j, t);
      if (rv) {
        DBG_ERROR(0, "Could not store transaction to job (%d)", rv);
        return 3;
      }
      rv=AB_Banking_EnqueueJob(ab, j);
      if (rv) {
        DBG_ERROR(0, "Could not enqueue job (%d)", rv);
        return 3;
      }
      else {
        DBG_INFO(0, "Job successfully enqueued");
        jobOk=1;
      }
    }
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  if (!jobOk) {
    DBG_ERROR(0, "Could not create transfer job");
    return 3;
  }

  return 0;
}






