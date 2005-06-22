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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int chkTransfer(AB_BANKING *ab,
                GWEN_DB_NODE *dbArgs,
                int argc,
                char **argv) {
  AB_ACCOUNT_LIST2 *al;
  GWEN_DB_NODE *db;
  int rv;
  const char *ctxFile;
  AB_IMEXPORTER_CONTEXT *ctx=0;
  AB_ACCOUNT *matchingAcc=0;
  int fd;
  AB_TRANSACTION_STATUS st=AB_Transaction_StatusUnknown;
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
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "ctxFile",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "c",                          /* short option */
    "ctxfile",                    /* long option */
    "Specify the file to store the context in",   /* short description */
    "Specify the file to store the context in"      /* long description */
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

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  /* read context */
  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);
  if (ctxFile==0)
    fd=fileno(stdin);
  else
    fd=open(ctxFile, O_RDONLY);
  if (fd<0) {
    DBG_ERROR(0, "Error selecting output file: %s",
              strerror(errno));
    return 4;
  }
  else {
    GWEN_BUFFEREDIO *bio;
    GWEN_ERRORCODE err;
    GWEN_DB_NODE *dbCtx;

    dbCtx=GWEN_DB_Group_new("context");
    bio=GWEN_BufferedIO_File_new(fd);
    if (!ctxFile)
      GWEN_BufferedIO_SubFlags(bio, GWEN_BUFFEREDIO_FLAGS_CLOSE);
    GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);
    if (GWEN_DB_ReadFromStream(dbCtx, bio,
                               GWEN_DB_FLAGS_DEFAULT |
                               GWEN_PATH_FLAGS_CREATE_GROUP)) {
      DBG_ERROR(0, "Error reading context");
      GWEN_DB_Group_free(dbCtx);
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      return 4;
    }
    err=GWEN_BufferedIO_Close(bio);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      GWEN_DB_Group_free(dbCtx);
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      return 4;
    }
    GWEN_BufferedIO_free(bio);

    ctx=AB_ImExporterContext_fromDb(dbCtx);
    if (!ctx) {
      DBG_ERROR(0, "No context in input data");
      GWEN_DB_Group_free(dbCtx);
      return 4;
    }
    GWEN_DB_Group_free(dbCtx);
  }

  al=AB_Banking_GetAccounts(ab);
  if (al) {
    AB_ACCOUNT_LIST2_ITERATOR *ait;

    ait=AB_Account_List2_First(al);
    if (ait) {
      AB_ACCOUNT *a;
      AB_TRANSACTION *t;
      AB_JOB *j;
      AB_IMEXPORTER_ACCOUNTINFO *iea;
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
      rv=AB_JobSingleTransfer_SetTransaction(j, t);
      if (rv) {
        DBG_ERROR(0, "Could not store transaction to job (%d)", rv);
        return 3;
      }

      /* search for matching transfer in context */
      iea=AB_ImExporterContext_FindAccountInfo(ctx,
                                               AB_Account_GetBankCode(matchingAcc),
                                               AB_Account_GetAccountNumber(matchingAcc));
      if (iea) {
        const AB_TRANSACTION *ct;

        ct=AB_ImExporterAccountInfo_GetFirstTransfer(iea);
        while(ct) {
          if (AB_Transaction_Compare(ct, t)==0)
            break;
          ct=AB_ImExporterAccountInfo_GetNextTransfer(iea);
        } /* while */
        if (ct) {
          /* found a transfer */
	  st=AB_Transaction_GetStatus(ct);
        }
      }
      AB_Job_free(j);
    }
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  switch(st) {
  case AB_Transaction_StatusAccepted: return 0;
  case AB_Transaction_StatusRejected: return 3;
  case AB_Transaction_StatusPending:  return 99;
  default:                            return 4;
  }
}






