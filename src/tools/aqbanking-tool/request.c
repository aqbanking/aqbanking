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
#include <gwenhywfar/bufferedio.h>
#include <gwenhywfar/bio_file.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>



int request(AB_BANKING *ab,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv) {
  AB_ACCOUNT_LIST2 *al;
  GWEN_DB_NODE *db;
  const char *s;
  int rv;
  int requests=0;
  int reqTrans=0;
  int reqBalance=0;
  int reqSto=0;
  GWEN_TIME *fromTime=0;
  GWEN_TIME *toTime=0;
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
    0,                            /* flags */
    GWEN_ArgsTypeInt,             /* type */
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
    GWEN_ArgsTypeInt,             /* type */
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
    GWEN_ArgsTypeInt,             /* type */
    "reqSto",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "sto",                        /* long option */
    "Request standing orders",    /* short description */
    "Request standing orders"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
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
    GWEN_ArgsTypeChar,            /* type */
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

  reqTrans=GWEN_DB_GetIntValue(db, "reqTrans", 0, 0);
  reqBalance=GWEN_DB_GetIntValue(db, "reqBalance", 0, 0);
  reqSto=GWEN_DB_GetIntValue(db, "reqSto", 0, 0);
  s=GWEN_DB_GetCharValue(db, "fromDate", 0, 0);
  if (s && *s) {
    fromTime=GWEN_Time_fromString(s, "YYYYMMDD");
    if (fromTime==0) {
      DBG_ERROR(AQT_LOGDOMAIN, "Invalid fromdate value \"%s\"", s);
      return 1;
    }
  }
  s=GWEN_DB_GetCharValue(db, "toDate", 0, 0);
  if (s && *s) {
    toTime=GWEN_Time_fromString(s, "YYYYMMDD");
    if (toTime==0) {
      DBG_ERROR(AQT_LOGDOMAIN, "Invalid todate value \"%s\"", s);
      return 1;
    }
  }

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
          if (reqTrans) {
            rv=AB_Banking_RequestTransactions(ab,
                                              AB_Account_GetBankCode(a),
                                              AB_Account_GetAccountNumber(a),
					      fromTime,
					      toTime);
            if (rv) {
              DBG_ERROR(AQT_LOGDOMAIN,
                        "Error requesting transactions for %s/%s: %d",
                        AB_Account_GetBankCode(a),
                        AB_Account_GetAccountNumber(a),
                        rv);
              return 3;
            }
            requests++;
          }
          if (reqBalance) {
            rv=AB_Banking_RequestBalance(ab,
                                         AB_Account_GetBankCode(a),
                                         AB_Account_GetAccountNumber(a));
            if (rv) {
              DBG_ERROR(AQT_LOGDOMAIN,
                        "Error requesting balance for %s/%s: %d",
                        AB_Account_GetBankCode(a),
                        AB_Account_GetAccountNumber(a),
                        rv);
              return 3;
            }
            requests++;
          }
          if (reqSto) {
            rv=AB_Banking_RequestStandingOrders(ab,
                                                AB_Account_GetBankCode(a),
                                                AB_Account_GetAccountNumber(a));
            if (rv) {
              DBG_ERROR(AQT_LOGDOMAIN,
                        "Error requesting standing orders for %s/%s: %d",
                        AB_Account_GetBankCode(a),
                        AB_Account_GetAccountNumber(a),
                        rv);
              return 3;
            }
            requests++;
          }
        }

        a=AB_Account_List2Iterator_Next(ait);
      }
      AB_Account_List2Iterator_free(ait);
      GWEN_Time_free(toTime);
      GWEN_Time_free(fromTime);
    }
  }

  if (requests) {
    DBG_INFO(AQT_LOGDOMAIN, "%d requests created", requests);
  }
  else {
    DBG_ERROR(AQT_LOGDOMAIN, "No requests created");
    return 4;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}






