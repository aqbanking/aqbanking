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


#include "globals_l.h"

#include <aqhbci/account.h>

#include <gwenhywfar/text.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int AH_Control_AddAccount(AB_PROVIDER *pro,
                          GWEN_DB_NODE *dbArgs,
                          int argc,
                          char **argv)
{
  GWEN_DB_NODE *db;
  AB_USER *u=0;
  int rv;
  uint32_t userId;
  const char *bankId;
  const char *accountName;
  const char *accountId;
  const char *ownerName;
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "bankId",                     /* name */
      1,                            /* minnum */
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
      "N"                           /* short option */
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
    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
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

  rv=AB_Provider_HasUser(pro, userId);
  if (rv<0) {
    fprintf(stderr, "ERROR: User with id %lu not found\n", (unsigned long int) userId);
    return 2;
  }

  rv=AB_Provider_GetUser(pro, userId, 1, 1, &u);
  if (rv<0) {
    fprintf(stderr, "ERROR: User with id %lu not found\n", (unsigned long int) userId);
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
    rv=AB_Banking_GetBankInfoByTemplate(AB_Provider_GetBanking(pro), "de", tbi, bl);
    if (rv) {
      fprintf(stderr, "Error looking for bank info: %d\n", rv);
      AB_User_free(u);
      return 3;
    }

    bit=AB_BankInfo_List2_First(bl);
    if (bit) {
      bi=AB_BankInfo_List2Iterator_Data(bit);
      assert(bi);
      AB_BankInfo_List2Iterator_free(bit);
    }
    else {
      bi=NULL;
      fprintf(stderr, "Could not find bank with id %s\n", bankId);
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

    /* add account to system */
    rv=AB_Provider_AddAccount(pro, account, 1); /* lock corresponding user */
    if (rv<0) {
      DBG_ERROR(0, "Error adding account (%d)", rv);
      AB_Account_free(account);
      AB_User_free(u);
      return 3;
    }

    AB_Account_free(account);

    AB_User_free(u);
  }

  return 0;
}




