/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"

#include <aqbanking/transaction.h>

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
  GWEN_DB_NODE *db;
  AB_ACCOUNT_SPEC_LIST *al=NULL;
  AB_ACCOUNT_SPEC *as;
  const char *s;
  int rv;
  const char *ctxFile;
  int requests=0;
  int reqTrans=0;
  int reqBalance=0;
  int reqSto=0;
  int reqSepaSto=0;
  int reqEStatements=0;
  int ignoreUnsupported=0;
  GWEN_DATE *fromDate=0;
  GWEN_DATE *toDate=0;
  AB_TRANSACTION_LIST *jobList;
  const GWEN_ARGS args[]={
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
    "backendName",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    NULL,                          /* short option */
    "backend",                       /* long option */
    "Specify the name of the backend for your account",      /* short description */
    "Specify the name of the backend for your account"       /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "country",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    NULL,                          /* short option */
    "country",                       /* long option */
    "Specify the country for your account (e.g. \"de\")",      /* short description */
    "Specify the country for your account (e.g. \"de\")"       /* long description */
  },
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
    "iban",                        /* name */
    0,                             /* minnum */
    1,                             /* maxnum */
    "A",                           /* short option */
    "iban",                       /* long option */
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
    "reqEStatements",             /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "estatements",                   /* long option */
    "Request electronic statements", /* short description */
    "Request electronic statements"  /* long description */
  },
  {
    0,                
    GWEN_ArgsType_Int,
    "ignoreUnsupported",
    0,  
    1,  
    0, 
    "ignoreUnsupported",
    "let AqBanking ignore unsupported requests for accounts",
    "let AqBanking ignore unsupported requests for accounts",
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
    "Specify the last date for which transactions are wanted (YYYYMMDD)", /* short */
    "Specify the last date for which transactions are wanted (YYYYMMDD)" /* long */
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

  ignoreUnsupported=GWEN_DB_GetIntValue(db, "ignoreUnsupported", 0, 0);
  reqTrans=GWEN_DB_GetIntValue(db, "reqTrans", 0, 0);
  reqBalance=GWEN_DB_GetIntValue(db, "reqBalance", 0, 0);
  reqSto=GWEN_DB_GetIntValue(db, "reqSto", 0, 0);
  reqSepaSto=GWEN_DB_GetIntValue(db, "reqSepaSto", 0, 0);
  reqEStatements=GWEN_DB_GetIntValue(db, "reqEStatements", 0, 0);
  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);
  s=GWEN_DB_GetCharValue(db, "fromDate", 0, 0);
  if (s && *s) {
    fromDate=GWEN_Date_fromStringWithTemplate(s, "YYYYMMDD");
    if (fromDate==NULL) {
      DBG_ERROR(0, "Invalid fromdate value \"%s\"", s);
      return 1;
    }
  }
  s=GWEN_DB_GetCharValue(db, "toDate", 0, 0);
  if (s && *s) {
    toDate=GWEN_Date_fromStringWithTemplate(s, "YYYYMMDD");
    if (toDate==NULL) {
      DBG_ERROR(0, "Invalid todate value \"%s\"", s);
      GWEN_Date_free(fromDate);
      return 1;
    }
  }

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    GWEN_Date_free(toDate);
    GWEN_Date_free(fromDate);
    return 2;
  }

  rv=getSelectedAccounts(ab, db, &al);
  if (rv<0) {
    if (rv==GWEN_ERROR_NOT_FOUND) {
      DBG_ERROR(0, "No matching accounts");
    }
    else {
      DBG_ERROR(0, "Error getting selected accounts (%d)", rv);
    }
    GWEN_Date_free(toDate);
    GWEN_Date_free(fromDate);
    AB_Banking_Fini(ab);
    return 2;
  }

  jobList=AB_Transaction_List_new();

  as=AB_AccountSpec_List_First(al);
  while(as) {
    uint32_t aid;

    aid=AB_AccountSpec_GetUniqueId(as);

    if (reqTrans) {
      AB_TRANSACTION *j;
  
      j=AB_Transaction_new();
      AB_Transaction_SetUniqueAccountId(j, aid);
      AB_Transaction_SetCommand(j, AB_Transaction_CommandGetTransactions);
      if (1) { /* TODO: check availability */
        if (fromDate)
          AB_Transaction_SetFirstDate(j, fromDate);
        if (toDate)
          AB_Transaction_SetLastDate(j, toDate);
        AB_Transaction_List_Add(j, jobList);
        requests++;
      }
      else {
        if (ignoreUnsupported) {
          DBG_ERROR(0, "Ignoring transfer request for %lu: Not supported.", (unsigned long int) aid);
          AB_Transaction_free(j);
        }
        else {
          DBG_ERROR(0, "Error requesting transfers for account %lu: Not supported.", (unsigned long int) aid);
          AB_Transaction_free(j);
          AB_Transaction_List_free(jobList);
          AB_AccountSpec_List_free(al);
          GWEN_Date_free(toDate);
          GWEN_Date_free(fromDate);
          AB_Banking_Fini(ab);
          return 3;
        }
      }
    }

    if (reqBalance) {
      AB_TRANSACTION *j;
  
      j=AB_Transaction_new();
      AB_Transaction_SetUniqueAccountId(j, aid);
      AB_Transaction_SetCommand(j, AB_Transaction_CommandGetBalance);
      if (1) { /* TODO: check availability */
        AB_Transaction_List_Add(j, jobList);
        requests++;
      }
      else {
        if (ignoreUnsupported) {
          DBG_ERROR(0, "Ignoring balance request for %lu: Not supported.", (unsigned long int) aid);
          AB_Transaction_free(j);
        }
        else {
          DBG_ERROR(0, "Error requesting balance for account %lu: Not supported.", (unsigned long int) aid);
          AB_Transaction_free(j);
          AB_Transaction_List_free(jobList);
          AB_AccountSpec_List_free(al);
          GWEN_Date_free(toDate);
          GWEN_Date_free(fromDate);
          AB_Banking_Fini(ab);
          return 3;
        }
      }
    }

    if (reqSto) {
      AB_TRANSACTION *j;
  
      j=AB_Transaction_new();
      AB_Transaction_SetUniqueAccountId(j, aid);
      AB_Transaction_SetCommand(j, AB_Transaction_CommandGetStandingOrders);
      if (1) { /* TODO: check availability */
        AB_Transaction_List_Add(j, jobList);
        requests++;
      }
      else {
        if (ignoreUnsupported) {
          DBG_ERROR(0, "Ignoring standing order request for %lu: Not supported.", (unsigned long int) aid);
          AB_Transaction_free(j);
        }
        else {
          DBG_ERROR(0, "Error requesting standing orders for account %lu: Not supported.", (unsigned long int) aid);
          AB_Transaction_free(j);
          AB_Transaction_List_free(jobList);
          AB_AccountSpec_List_free(al);
          GWEN_Date_free(toDate);
          GWEN_Date_free(fromDate);
          AB_Banking_Fini(ab);
          return 3;
        }
      }
    }

    if (reqSepaSto) {
      AB_TRANSACTION *j;
  
      j=AB_Transaction_new();
      AB_Transaction_SetUniqueAccountId(j, aid);
      AB_Transaction_SetCommand(j, AB_Transaction_CommandSepaGetStandingOrders);
      if (1) { /* TODO: check availability */
        AB_Transaction_List_Add(j, jobList);
        requests++;
      }
      else {
        if (ignoreUnsupported) {
          DBG_ERROR(0, "Ignoring SEPA standing order request for %lu: Not supported.", (unsigned long int) aid);
          AB_Transaction_free(j);
        }
        else {
          DBG_ERROR(0, "Error requesting SEPA standing orders for account %lu: Not supported.", (unsigned long int) aid);
          AB_Transaction_free(j);
          AB_Transaction_List_free(jobList);
          AB_AccountSpec_List_free(al);
          GWEN_Date_free(toDate);
          GWEN_Date_free(fromDate);
          AB_Banking_Fini(ab);
          return 3;
        }
      }
    }

    if (reqEStatements) {
      AB_TRANSACTION *j;
  
      j=AB_Transaction_new();
      AB_Transaction_SetUniqueAccountId(j, aid);
      AB_Transaction_SetCommand(j, AB_Transaction_CommandGetEStatements);
      if (1) { /* TODO: check availability */
        AB_Transaction_List_Add(j, jobList);
        requests++;
      }
      else {
        if (ignoreUnsupported) {
          DBG_ERROR(0, "Ignoring electronic statement request for %lu: Not supported.", (unsigned long int) aid);
          AB_Transaction_free(j);
        }
        else {
          DBG_ERROR(0, "Error requesting electronic statements for account %lu: Not supported.", (unsigned long int) aid);
          AB_Transaction_free(j);
          AB_Transaction_List_free(jobList);
          AB_AccountSpec_List_free(al);
          GWEN_Date_free(toDate);
          GWEN_Date_free(fromDate);
          AB_Banking_Fini(ab);
          return 3;
        }
      }
    }

    as=AB_AccountSpec_List_Next(as);
  } /* while (as) */

  AB_AccountSpec_List_free(al);
  GWEN_Date_free(toDate);
  GWEN_Date_free(fromDate);

  if (requests) {
    AB_IMEXPORTER_CONTEXT *ctx;

    DBG_INFO(0, "%d requests created", requests);
    ctx=AB_ImExporterContext_new();
    rv=AB_Banking_SendCommands(ab, jobList, ctx);
    AB_Transaction_List_free(jobList);
    if (rv) {
      fprintf(stderr, "Error on sendCommands (%d)\n", rv);
      AB_ImExporterContext_free(ctx);
      return 3;
    }

    rv=writeContext(ctxFile, ctx);
    AB_ImExporterContext_free(ctx);
    if (rv<0) {
      DBG_ERROR(0, "Error writing context file (%d)", rv);
      AB_Banking_Fini(ab);
      return 4;
    }
  }
  else {
    DBG_ERROR(0, "No requests created");
    AB_Transaction_List_free(jobList);
    AB_Banking_Fini(ab);
    return 4;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}






