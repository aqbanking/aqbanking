/***************************************************************************
    begin       : Tue May 11 2010
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




int APY_Control_SetSecrets(AB_PROVIDER *pro,
                           GWEN_DB_NODE *dbArgs,
                           int argc,
                           char **argv)
{
  GWEN_DB_NODE *db;
  AB_USER *u=NULL;
  int rv;
  uint32_t userId;
  const char *apiUserId;
  const char *apiPassword;
  const char *apiSignature;
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "userId",                     /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "u",                          /* short option */
      "user",                       /* long option */
      "Specify the unique user id",    /* short description */
      "Specify the unique user id"     /* long description */
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

  userId=GWEN_DB_GetIntValue(db, "userId", 0, 0);
  apiUserId=GWEN_DB_GetCharValue(db, "apiUserId", 0, NULL);
  apiPassword=GWEN_DB_GetCharValue(db, "password", 0, NULL);
  apiSignature=GWEN_DB_GetCharValue(db, "signature", 0, NULL);

  rv=AB_Provider_GetUser(pro, userId, 1, 1, &u);
  if (rv<0) {
    fprintf(stderr, "ERROR: User with id %lu not found\n", (unsigned long int) userId);
    return 2;
  }

  if (!u) {
    DBG_ERROR(0, "No matching customer");
    return 3;
  }
  else {
    /* lock user */
    rv=AB_Provider_BeginExclUseUser(pro, u);
    if (rv<0) {
      fprintf(stderr,
              "ERROR: Could not lock user, maybe it is used in another application? (%d)\n",
              rv);
      return 4;
    }

    /* modifications */
    rv=APY_User_SetApiSecrets(u, apiPassword, apiSignature, apiUserId);
    if (rv<0) {
      fprintf(stderr, "ERROR: Error on APY_User_SetApiSecrets (%d)\n", rv);
      AB_Provider_EndExclUseUser(pro, u, 1);
      return 3;
    }

    /* unlock user */
    rv=AB_Provider_EndExclUseUser(pro, u, 0);
    if (rv<0) {
      fprintf(stderr,
              "ERROR: Could not unlock user (%d)\n",
              rv);
      AB_Provider_EndExclUseUser(pro, u, 1); /* abandon */
      return 4;
    }
  }

  return 0;
}



