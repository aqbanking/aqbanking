/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2005-2014 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"

#include <aqbanking/jobgettransactions.h>
#include <aqbanking/jobgetbalance.h>
#include <aqbanking/jobgetstandingorders.h>
#include <aqbanking/jobsepagetstandingorders.h>
#include <aqbanking/jobgetdatedtransfers.h>

#include <gwenhywfar/text.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>



static
int request(AB_BANKING *ab,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv) {
  AB_ACCOUNT_LIST2 *al;
  GWEN_DB_NODE *db;
  const char *s;
  int rv;
  const char *ctxFile;
  int requests=0;
  int reqTrans=0;
  int reqBalance=0;
  int reqSto=0;
  int reqSepaSto=0;
  int reqDT=0;
  GWEN_TIME *fromTime=0;
  GWEN_TIME *toTime=0;
  AB_JOB_LIST2 *jobList;
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
    0,                            /* flags */
    GWEN_ArgsType_Int,             /* type */
    "reqTrans",                   /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "transactions",               /* long option */
    "Request transactions",       /* short description */
    "Request transactions"        /* long description */
  },
  {
    0,                            /* flags */
    GWEN_ArgsType_Int,             /* type */
    "reqBalance",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "balance",                    /* long option */
    "Request balance",            /* short description */
    "Request balance"             /* long description */
  },
  {
    0,                            /* flags */
    GWEN_ArgsType_Int,             /* type */
    "reqSto",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "sto",                        /* long option */
    "Request standing orders",    /* short description */
    "Request standing orders"     /* long description */
  },
  {
    0,                            /* flags */
    GWEN_ArgsType_Int,             /* type */
    "reqSepaSto",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "sepaSto",                    /* long option */
    "Request SEPA standing orders",    /* short description */
    "Request SEPA standing orders"     /* long description */
  },
  {
    0,                            /* flags */
    GWEN_ArgsType_Int,             /* type */
    "reqDT",                      /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "dated",                      /* long option */
    "Request dated transfers",    /* short description */
    "Request dated transfers"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "fromDate",                   /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "fromdate",                   /* long option */
    "Specify the first date for which transactions are wanted (YYYYMMDD)", /* short */
    "Specify the first date for which transactions are wanted (YYYYMMDD)" /* long */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "toDate",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "todate",                     /* long option */
    "Specify the first date for which transactions are wanted (YYYYMMDD)", /* short */
    "Specify the first date for which transactions are wanted (YYYYMMDD)" /* long */
  },
  {
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsType_Int,            /* type */
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

  reqTrans=GWEN_DB_GetIntValue(db, "reqTrans", 0, 0);
  reqBalance=GWEN_DB_GetIntValue(db, "reqBalance", 0, 0);
  reqSto=GWEN_DB_GetIntValue(db, "reqSto", 0, 0);
  reqSepaSto=GWEN_DB_GetIntValue(db, "reqSepaSto", 0, 0);
  reqDT=GWEN_DB_GetIntValue(db, "reqDT", 0, 0);
  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);
  s=GWEN_DB_GetCharValue(db, "fromDate", 0, 0);
  if (s && *s) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(dbuf, s);
    GWEN_Buffer_AppendString(dbuf, "-12:00");
    fromTime=GWEN_Time_fromUtcString(GWEN_Buffer_GetStart(dbuf),
                                     "YYYYMMDD-hh:mm");
    GWEN_Buffer_free(dbuf);
    if (fromTime==0) {
      DBG_ERROR(0, "Invalid fromdate value \"%s\"", s);
      return 1;
    }
  }
  s=GWEN_DB_GetCharValue(db, "toDate", 0, 0);
  if (s && *s) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(dbuf, s);
    GWEN_Buffer_AppendString(dbuf, "-12:00");
    toTime=GWEN_Time_fromUtcString(GWEN_Buffer_GetStart(dbuf),
                                   "YYYYMMDD-hh:mm");
    GWEN_Buffer_free(dbuf);
    if (toTime==0) {
      DBG_ERROR(0, "Invalid todate value \"%s\"", s);
      GWEN_Time_free(fromTime);
      return 1;
    }
  }

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    GWEN_Time_free(toTime);
    GWEN_Time_free(fromTime);
    return 2;
  }

  rv=AB_Banking_OnlineInit(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    GWEN_Time_free(toTime);
    GWEN_Time_free(fromTime);
    return 2;
  }

  jobList=AB_Job_List2_new();
  al=AB_Banking_GetAccounts(ab);
  if (al) {
    AB_ACCOUNT_LIST2_ITERATOR *ait;

    ait=AB_Account_List2_First(al);
    if (ait) {
      AB_ACCOUNT *a;
      const char *bankId;
      const char *accountId;
      const char *subAccountId;
      const char *bankName;
      const char *accountName;
      const char *s;

      bankId=GWEN_DB_GetCharValue(db, "bankId", 0, 0);
      bankName=GWEN_DB_GetCharValue(db, "bankName", 0, 0);
      accountId=GWEN_DB_GetCharValue(db, "accountId", 0, 0);
      subAccountId=GWEN_DB_GetCharValue(db, "subAccountId", 0, 0);
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

        if (matches && subAccountId) {
          s=AB_Account_GetSubAccountId(a);
          if (!s || !*s || -1==GWEN_Text_ComparePattern(s, subAccountId, 0))
            matches=0;
        }

        if (matches && accountName) {
          s=AB_Account_GetAccountName(a);
          if (!s || !*s || -1==GWEN_Text_ComparePattern(s, accountName, 0))
            matches=0;
        }

        if (matches) {
          if (reqTrans) {
            AB_JOB *j;

	    j=AB_JobGetTransactions_new(a);
	    rv=AB_Job_CheckAvailability(j);
	    if (rv<0) {
	      DBG_ERROR(0,
			"Error requesting transactions for %s/%s: %d",
			AB_Account_GetBankCode(a),
			AB_Account_GetAccountNumber(a),
			rv);
	      AB_Account_List2Iterator_free(ait);
	      AB_Account_List2_free(al);
	      AB_Job_List2_FreeAll(jobList);
	      GWEN_Time_free(toTime);
	      GWEN_Time_free(fromTime);
	      return 3;
	    }
            if (fromTime)
	      AB_JobGetTransactions_SetFromTime(j, fromTime);
            if (toTime)
	      AB_JobGetTransactions_SetToTime(j, toTime);

            AB_Job_List2_PushBack(jobList, j);
	    requests++;
	  }
	  if (reqBalance) {
            AB_JOB *j;

	    j=AB_JobGetBalance_new(a);
	    rv=AB_Job_CheckAvailability(j);
	    if (rv<0) {
	      DBG_ERROR(0,
			"Error requesting balance for %s/%s: %d",
			AB_Account_GetBankCode(a),
			AB_Account_GetAccountNumber(a),
			rv);
	      AB_Account_List2Iterator_free(ait);
	      AB_Account_List2_free(al);
	      AB_Job_List2_FreeAll(jobList);
	      GWEN_Time_free(toTime);
	      GWEN_Time_free(fromTime);
	      return 3;
	    }
            AB_Job_List2_PushBack(jobList, j);
            requests++;
          }
          if (reqSto) {
            AB_JOB *j;

	    j=AB_JobGetStandingOrders_new(a);
	    rv=AB_Job_CheckAvailability(j);
	    if (rv<0) {
	      DBG_ERROR(0,
			"Error requesting standing order for %s/%s: %d",
			AB_Account_GetBankCode(a),
			AB_Account_GetAccountNumber(a),
			rv);
	      AB_Account_List2Iterator_free(ait);
	      AB_Account_List2_free(al);
	      AB_Job_List2_FreeAll(jobList);
	      GWEN_Time_free(toTime);
	      GWEN_Time_free(fromTime);
	      return 3;
	    }
            AB_Job_List2_PushBack(jobList, j);
	    requests++;
	  }
          if (reqSepaSto) {
            AB_JOB *j;

	    j=AB_JobSepaGetStandingOrders_new(a);
	    rv=AB_Job_CheckAvailability(j);
	    if (rv<0) {
	      DBG_ERROR(0,
			"Error requesting SEPA standing order for %s/%s: %d",
			AB_Account_GetBankCode(a),
			AB_Account_GetAccountNumber(a),
			rv);
	      AB_Account_List2Iterator_free(ait);
	      AB_Account_List2_free(al);
	      AB_Job_List2_FreeAll(jobList);
	      GWEN_Time_free(toTime);
	      GWEN_Time_free(fromTime);
	      return 3;
	    }
            AB_Job_List2_PushBack(jobList, j);
	    requests++;
	  }
	  if (reqDT) {
            AB_JOB *j;

	    j=AB_JobGetDatedTransfers_new(a);
	    rv=AB_Job_CheckAvailability(j);
	    if (rv<0) {
	      DBG_ERROR(0,
			"Error requesting dated transfers for %s/%s: %d",
			AB_Account_GetBankCode(a),
			AB_Account_GetAccountNumber(a),
			rv);
	      AB_Account_List2Iterator_free(ait);
	      AB_Account_List2_free(al);
	      AB_Job_List2_FreeAll(jobList);
	      GWEN_Time_free(toTime);
	      GWEN_Time_free(fromTime);
	      return 3;
	    }
            AB_Job_List2_PushBack(jobList, j);
	    requests++;
	  }
	}

        a=AB_Account_List2Iterator_Next(ait);
      }
      AB_Account_List2Iterator_free(ait);
    }
  }
  AB_Account_List2_free(al);
  GWEN_Time_free(toTime);
  GWEN_Time_free(fromTime);

  if (requests) {
    AB_IMEXPORTER_CONTEXT *ctx;

    DBG_INFO(0, "%d requests created", requests);
    ctx=AB_ImExporterContext_new();
    rv=AB_Banking_ExecuteJobs(ab, jobList, ctx);
    AB_Job_List2_FreeAll(jobList);
    if (rv) {
      fprintf(stderr, "Error on executeQueue (%d)\n", rv);
      AB_ImExporterContext_free(ctx);
      return 3;
    }

    rv=writeContext(ctxFile, ctx);
    AB_ImExporterContext_free(ctx);
    if (rv<0) {
      DBG_ERROR(0, "Error writing context file (%d)", rv);
      AB_Banking_OnlineFini(ab);
      AB_Banking_Fini(ab);
      return 4;
    }
  }
  else {
    DBG_ERROR(0, "No requests created");
    AB_Job_List2_FreeAll(jobList);
    AB_Banking_OnlineFini(ab);
    AB_Banking_Fini(ab);
    return 4;
  }

  rv=AB_Banking_OnlineFini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    AB_Banking_Fini(ab);
    return 5;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}






