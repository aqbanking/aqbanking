/***************************************************************************
    begin       : Mon May 10 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "globals.h"
#include <aqpaypal/user.h>

#include <aqbanking/banking_be.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/url.h>
#include <gwenhywfar/args.h>
#include <gwenhywfar/debug.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>



int addUser(AB_BANKING *ab,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv) {
  GWEN_DB_NODE *db;
  AB_PROVIDER *pro;
  AB_USER *user;
  int rv;
  const char *userId;
  const char *apiPassword;
  const char *apiSignature;
  const char *userName;
  const char *server;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "userId",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "u",                          /* short option */
    "user",                       /* long option */
    "Specify the user id (Benutzerkennung)",        /* short description */
    "Specify the user id (Benutzerkennung)"         /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "password",                   /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "P",                          /* short option */
    "password",                   /* long option */
    "Specify the API password",   /* short description */
    "Specify the API password"    /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "signature",                  /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "S",                          /* short option */
    "signature",                  /* long option */
    "Specify the API signature",  /* short description */
    "Specify the API signature"   /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "serverAddr",                 /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "s",                          /* short option */
    "server",                     /* long option */
    "Specify the server URL",     /* short description */
    "Specify the server URL"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "userName",                 /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "N",                          /* short option */
    "username",                     /* long option */
    "Specify the realname of the user",     /* short description */
    "Specify the realname of the user"      /* long description */
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
    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
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

  pro=AB_Banking_GetProvider(ab, APY_PROVIDER_NAME);
  assert(pro);

  userId=GWEN_DB_GetCharValue(db, "userId", 0, NULL);
  apiPassword=GWEN_DB_GetCharValue(db, "password", 0, NULL);
  apiSignature=GWEN_DB_GetCharValue(db, "signature", 0, NULL);
  userName=GWEN_DB_GetCharValue(db, "userName", 0, NULL);
  server=GWEN_DB_GetCharValue(db, "serverAddr", 0, NULL);

  user=AB_Banking_FindUser(ab, APY_PROVIDER_NAME,
			   "*",
			   "PAYPAL", userId, "*");
  if (user) {
    DBG_ERROR(0, "User %s already exists", userId);
    return 3;
  }

  user=AB_Banking_CreateUser(ab, APY_PROVIDER_NAME);
  assert(user);

  AB_User_SetCountry(user, "de");
  AB_User_SetBankCode(user, "PAYPAL");
  AB_User_SetUserId(user, userId);
  AB_User_SetCustomerId(user, userId);
  AB_User_SetUserName(user, userName);
  APY_User_SetServerUrl(user, server);

  /* add user */
  rv=AB_Banking_AddUser(ab, user);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error on AB_Banking_AddUser (%d)\n", rv);
    AB_Banking_OnlineFini(ab);
    AB_Banking_Fini(ab);
    return 3;
  }

  rv=AB_Banking_BeginExclUseUser(ab, user);
  if (rv<0) {
    fprintf(stderr, "ERROR: Could not lock user (%d)\n", rv);
    return 3;
  }

  rv=APY_User_SetApiSecrets(user, apiPassword, apiSignature);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error on APY_User_SetApiSecrets (%d)\n", rv);
    AB_Banking_EndExclUseUser(ab, user, 1);
    AB_Banking_Fini(ab);
    return 3;
  }

  rv=AB_Banking_EndExclUseUser(ab, user, 0);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    AB_Banking_EndExclUseUser(ab, user, 1);
    return rv;
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



