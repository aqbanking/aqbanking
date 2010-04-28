/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: getaccounts.c 964 2006-03-17 10:35:21Z cstim $
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int addAccount(AB_BANKING *ab,
	       GWEN_DB_NODE *dbArgs,
	       int argc,
	       char **argv) {
  GWEN_DB_NODE *db;
  AB_PROVIDER *pro;
  AB_USER_LIST2 *ul;
  AB_USER *u=0;
  int rv;
  const char *bankId;
  const char *userId;
  const char *accountName;
  const char *accountId;
  const char *customerId;
  const char *ownerName;
  const GWEN_ARGS args[]={
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
    GWEN_ArgsType_Char,           /* type */
    "userId",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "u",                          /* short option */
    "user",                       /* long option */
    "Specify the user id (Benutzerkennung)",    /* short description */
    "Specify the user id (Benutzerkennung)"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "customerId",                 /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "c",                          /* short option */
    "customer",                   /* long option */
    "Specify the customer id (Kundennummer)",    /* short description */
    "Specify the customer id (Kundennummer)"     /* long description */
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
    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }

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

  pro=AB_Banking_GetProvider(ab, "aqhbci");
  assert(pro);

  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, "*");
  userId=GWEN_DB_GetCharValue(db, "userId", 0, "*");
  accountId=GWEN_DB_GetCharValue(db, "accountId", 0, "*");
  accountName=GWEN_DB_GetCharValue(db, "accountName", 0, "Account");
  customerId=GWEN_DB_GetCharValue(db, "customerId", 0, "*");
  ownerName=GWEN_DB_GetCharValue(db, "ownerName", 0, NULL);

  ul=AB_Banking_FindUsers(ab, AH_PROVIDER_NAME,
                          "de", bankId, userId, customerId);
  if (ul) {
    if (AB_User_List2_GetSize(ul)!=1) {
      DBG_ERROR(0, "Ambiguous customer specification");
      AB_Banking_Fini(ab);
      return 3;
    }
    else {
      AB_USER_LIST2_ITERATOR *cit;

      cit=AB_User_List2_First(ul);
      assert(cit);
      u=AB_User_List2Iterator_Data(cit);
      AB_User_List2Iterator_free(cit);
    }
    AB_User_List2_free(ul);
  }
  if (!u) {
    DBG_ERROR(0, "No matching user");
    AB_Banking_Fini(ab);
    return 3;
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
    AB_BankInfo_SetBankId( tbi, bankId );
    rv=AB_Banking_GetBankInfoByTemplate(ab, "de", tbi, bl);
    if (rv) {
      fprintf(stderr, "Error looking for bank info: %d\n", rv);
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


    account=AB_Banking_CreateAccount(ab, "aqhbci");
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
    AB_Account_SetUser(account, u);
    AB_Account_SetSelectedUser(account, u);


    rv=AB_Banking_AddAccount(ab, account);
    if (rv) {
      DBG_ERROR(0, "Error adding account (%d)", rv);
      AB_Banking_Fini(ab);
      return 3;
    }
  }

  rv=AB_Banking_OnlineFini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}




