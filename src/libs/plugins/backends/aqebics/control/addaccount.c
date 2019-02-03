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
#include <aqebics/user.h>
#include <aqebics/provider.h>

#include <gwenhywfar/text.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


static GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv);



int addAccount(AB_PROVIDER *pro, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  AB_USER *u=0;
  int rv;
  uint32_t userId;
  const char *bankId;
  const char *accountName;
  const char *accountId;
  const char *ownerName;
  int forceAdd;

  /* parse command line arguments */
  db=_readCommandLine(dbArgs, argc, argv);
  if (db==NULL) {
    /* error in command line */
    return 1;
  }

  userId=GWEN_DB_GetIntValue(db, "userId", 0, 0);
  if (userId<1) {
    fprintf(stderr, "ERROR: Invalid user id\n");
    return 1;
  }
  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, "*");
  accountId=GWEN_DB_GetCharValue(db, "accountId", 0, "*");
  accountName=GWEN_DB_GetCharValue(db, "accountName", 0, "Account");
  ownerName=GWEN_DB_GetCharValue(db, "ownerName", 0, NULL);
  forceAdd=GWEN_DB_GetIntValue(db, "force", 0, 0);

  rv=AB_Provider_GetUser(pro, userId, 1, 1, &u);
  if (rv<0) {
    DBG_ERROR(0, "ERROR: User with id %lu not found", (unsigned long int) userId);
    return 2;
  }
  else {
    AB_ACCOUNT *account;
    AB_BANKINFO_LIST2 *bl;
    AB_BANKINFO_LIST2_ITERATOR *bit;
    AB_BANKINFO *tbi;
    AB_BANKINFO *bi;
    int rv;

    bl=AB_BankInfo_List2_new();
    tbi=AB_BankInfo_new();
    AB_BankInfo_SetBankId(tbi, bankId);
    AB_Banking_GetBankInfoByTemplate(AB_Provider_GetBanking(pro), "de", tbi, bl);

    bit=AB_BankInfo_List2_First(bl);
    if (bit) {
      bi=AB_BankInfo_List2Iterator_Data(bit);
      assert(bi);
      AB_BankInfo_List2Iterator_free(bit);
    }
    else {
      if (!forceAdd) {
        fprintf(stderr,
                "ERROR: Could not find bank with id %s\n",
                bankId);
        return 3;
      }
      bi=NULL;
      fprintf(stderr, "Warning: Could not find bank with id %s\n", bankId);
    }
    AB_BankInfo_List2_free(bl);

    account=AB_Provider_CreateAccountObject(pro);
    assert(account);

    if (!ownerName)
      AB_Account_SetOwnerName(account, AB_User_GetUserName(u));
    else
      AB_Account_SetOwnerName(account, ownerName);
    AB_Account_SetAccountNumber(account, accountId);
    if (accountName)
      AB_Account_SetAccountName(account, accountName);
    AB_Account_SetBankCode(account, bankId);
    if (bi)
      AB_Account_SetBankName(account, AB_BankInfo_GetBankName(bi));
    AB_Account_SetUserId(account, userId);

    rv=AB_Provider_AddAccount(pro, account, 1); /* do lock corresponding user */
    if (rv) {
      DBG_ERROR(0, "Error adding account (%d)", rv);
      return 3;
    }
  }

  return 0;
}


GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
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
      GWEN_ArgsType_Int,           /* type */
      "userId",                     /* name */
      1,                            /* minnum */
      1,                            /* maxnum */
      "u",                          /* short option */
      "user",                       /* long option */
      "Specify the unique user id",    /* short description */
      "Specify the unique user id"     /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "ownerName",                  /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                            /* short option */
      "owner",                      /* long option */
      "Specify the owner name",     /* short description */
      "Specify the owner name"      /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "accountName",                /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "n",                          /* short option */
      "name",                       /* long option */
      "Specify the account name (Konto-Name)",    /* short description */
      "Specify the account name (Konto-Name)"     /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "accountId",                  /* name */
      1,                            /* minnum */
      1,                            /* maxnum */
      "a",                          /* short option */
      "account",                    /* long option */
      "Specify the account id (Kontonummer)",    /* short description */
      "Specify the account id (Kontonummer)"     /* long description */
    },
    {
      0,
      GWEN_ArgsType_Int,            /* type */
      "force",                      /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                          /* short option */
      "force",                    /* long option */
      "Force adding the account even if there is no bank info",
      "Force adding the account even if there is no bank info"
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
    return NULL;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutType_Txt)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return NULL;
    }
    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return NULL;
  }

  return db;
}




