/***************************************************************************
    begin       : Mon May 10 2010
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "globals_l.h"
#include <aqpaypal/user.h>

#include <aqbanking/banking_be.h>
#include <aqbanking/backendsupport/provider_be.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/url.h>
#include <gwenhywfar/args.h>
#include <gwenhywfar/debug.h>




int APY_Control_AddUser(AB_PROVIDER *pro,
                        GWEN_DB_NODE *dbArgs,
                        int argc,
                        char **argv)
{
  GWEN_DB_NODE *db;
  AB_USER *user;
  int rv;
  const char *userId;
  const char *apiUserId;
  const char *apiPassword;
  const char *apiSignature;
  const char *userName;
  const char *server;
  const GWEN_ARGS args[]= {
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
      "apiUserId",                  /* name */
      1,                            /* minnum */
      1,                            /* maxnum */
      "U",                          /* short option */
      "apiuserid",                  /* long option */
      "Specify the API user id",    /* short description */
      "Specify the API user id"     /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "serverAddr",                 /* name */
      0,                            /* minnum */
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

  userId=GWEN_DB_GetCharValue(db, "userId", 0, NULL);
  apiUserId=GWEN_DB_GetCharValue(db, "apiUserId", 0, NULL);
  apiPassword=GWEN_DB_GetCharValue(db, "password", 0, NULL);
  apiSignature=GWEN_DB_GetCharValue(db, "signature", 0, NULL);
  userName=GWEN_DB_GetCharValue(db, "userName", 0, NULL);
  server=GWEN_DB_GetCharValue(db, "serverAddr", 0, "https://api-3t.paypal.com/nvp");

  user=AB_Provider_CreateUserObject(pro);
  assert(user);

  AB_User_SetCountry(user, "de");
  AB_User_SetBankCode(user, "PAYPAL");
  AB_User_SetUserId(user, userId);
  AB_User_SetCustomerId(user, userId);
  AB_User_SetUserName(user, userName);
  APY_User_SetServerUrl(user, server);

  /* add user */
  rv=AB_Provider_AddUser(pro, user);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error on AB_Provider_AddUser (%d)\n", rv);
    AB_User_free(user);
    return 3;
  }

  rv=AB_Provider_BeginExclUseUser(pro, user);
  if (rv<0) {
    fprintf(stderr, "ERROR: Could not lock user (%d)\n", rv);
    AB_User_free(user);
    return 3;
  }

  rv=APY_User_SetApiSecrets(user, apiPassword, apiSignature, apiUserId);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error on APY_User_SetApiSecrets (%d)\n", rv);
    AB_Provider_EndExclUseUser(pro, user, 1);
    AB_User_free(user);
    return 3;
  }

  rv=AB_Provider_EndExclUseUser(pro, user, 0);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    AB_Provider_EndExclUseUser(pro, user, 1);
    AB_User_free(user);
    return rv;
  }

  AB_User_free(user);

  return 0;
}



