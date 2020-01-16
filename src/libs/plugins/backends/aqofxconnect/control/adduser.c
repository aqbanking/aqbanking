/***************************************************************************
 begin       : Thu Jan 16 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "adduser.h"
#include "aqofxconnect/user.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/args.h>



GWEN_DB_NODE *readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv);





int AO_Control_AddUser(AB_PROVIDER *pro, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  AB_USER *user;
  int rv;
  const char *userId;
  const char *userName;
  const char *server;
  const char *bankId;
  const char *brokerId;
  const char *org;
  const char *fid;
  const char *appId;
  const char *appVer;
  const char *headerVer;
  const char *clientUid;

  /* parse command line */
  db=readCommandLine(dbArgs, argc, argv);
  if (db==NULL) {
    fprintf(stderr, "ERROR: Could not parse arguments\n");
    return 1;
  }

  userId=GWEN_DB_GetCharValue(db, "userId", 0, NULL);
  userName=GWEN_DB_GetCharValue(db, "userName", 0, NULL);
  server=GWEN_DB_GetCharValue(db, "serverAddr", 0, NULL);

  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, NULL);
  brokerId=GWEN_DB_GetCharValue(db, "brokerId", 0, NULL);
  org=GWEN_DB_GetCharValue(db, "org", 0, NULL);
  fid=GWEN_DB_GetCharValue(db, "fid", 0, NULL);
  appId=GWEN_DB_GetCharValue(db, "appId", 0, NULL);
  appVer=GWEN_DB_GetCharValue(db, "appVer", 0, NULL);
  headerVer=GWEN_DB_GetCharValue(db, "headerVer", 0, NULL);
  clientUid=GWEN_DB_GetCharValue(db, "clientUid", 0, NULL);

  user=AB_Provider_CreateUserObject(pro);
  assert(user);

  AB_User_SetCountry(user, "us");
  AB_User_SetBankCode(user, bankId);
  AB_User_SetUserId(user, userId);
  AB_User_SetUserName(user, userName);
  AO_User_SetServerAddr(user, server);

  if (brokerId && *brokerId)
    AO_User_SetBrokerId(user, brokerId);

  if (org && *org)
    AO_User_SetOrg(user, org);

  if (fid && *fid)
    AO_User_SetFid(user, fid);

  if (appId && *appId && appVer && *appVer) {
    AO_User_SetAppId(user, appId);
    AO_User_SetAppVer(user, appVer);
  }

  if (headerVer && *headerVer)
    AO_User_SetHeaderVer(user, headerVer);

  if (clientUid && *clientUid)
    AO_User_SetClientUid(user, clientUid);


  /* add user */
  rv=AB_Provider_AddUser(pro, user);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Error on AB_Provider_AddUser (%d)\n", rv);
    AB_User_free(user);
    return 3;
  }

  rv=AB_Provider_BeginExclUseUser(pro, user);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Could not lock user (%d)\n", rv);
    AB_User_free(user);
    return 3;
  }

  rv=AB_Provider_EndExclUseUser(pro, user, 0);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Could not unlock user (%d)", rv);
    AB_Provider_EndExclUseUser(pro, user, 1);
    AB_User_free(user);
    return rv;
  }

  AB_User_free(user);

  return 0;
}




GWEN_DB_NODE *readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "userName",                   /* name */
      1,                            /* minnum */
      1,                            /* maxnum */
      "N",                          /* short option */
      "username",                   /* long option */
      "Specify the user name", /* short description */
      "Specify the user name (not the userid!)"  /* long description */
    },
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
      GWEN_ArgsType_Char,           /* type */
      "userId",                     /* name */
      1,                            /* minnum */
      1,                            /* maxnum */
      "u",                          /* short option */
      "user",                       /* long option */
      "Specify the user id (Benutzerkennung)",        /* short description */
      "Specify the user id (Benutzerkennung)"         /* long description */
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
      "brokerId",                   /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "B",                          /* short option */
      "brokerid",                   /* long option */
      "Specify the broker id",      /* short description */
      "Specify the broker id"       /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "org",                        /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "O",                          /* short option */
      "org",                        /* long option */
      "Specify the ORG id",         /* short description */
      "Specify the ORG id"          /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "fid",                        /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "F",                          /* short option */
      "fid",                        /* long option */
      "Specify the FID id",         /* short description */
      "Specify the FID id"          /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "appId",                      /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "appid",                      /* long option */
      "Specify the APP id",         /* short description */
      "Specify the APP id"          /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "appVer",                     /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "appver",                     /* long option */
      "Specify the APP version",    /* short description */
      "Specify the APP version"     /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "clientUid",                  /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "clientuid",                  /* long option */
      "Specify a client unique id", /* short description */
      "Specify a client unique id"  /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "headerVer",                  /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "headerver",                  /* long option */
      "Specify the header version", /* short description */
      "Specify the header version"  /* long description */
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
    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return NULL;
  }

  return db;
}




