/***************************************************************************
    begin       : Tue May 11 2010
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



int setSecrets(AB_BANKING *ab,
	       GWEN_DB_NODE *dbArgs,
	       int argc,
	       char **argv) {
  GWEN_DB_NODE *db;
  AB_PROVIDER *pro;
  AB_USER_LIST2 *ul;
  AB_USER *u=NULL;
  int rv;
  const char *userId;
  const char *apiPassword;
  const char *apiSignature;
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

  ul=AB_Banking_FindUsers(ab, APY_PROVIDER_NAME, "*",
			  "PAYPAL", userId, "*");
  if (ul) {
    if (AB_User_List2_GetSize(ul)!=1) {
      DBG_ERROR(0, "Ambiguous customer specification");
      return 3;
    }
    else {
      AB_USER_LIST2_ITERATOR *uit;

      uit=AB_User_List2_First(ul);
      assert(uit);
      u=AB_User_List2Iterator_Data(uit);
      AB_User_List2Iterator_free(uit);
    }
    AB_User_List2_free(ul);
  }
  if (!u) {
    DBG_ERROR(0, "No matching customer");
    return 3;
  }
  else {
    /* lock user */
    rv=AB_Banking_BeginExclUseUser(ab, u);
    if (rv<0) {
      fprintf(stderr,
	      "ERROR: Could not lock user, maybe it is used in another application? (%d)\n",
	      rv);
      AB_Banking_OnlineFini(ab);
      AB_Banking_Fini(ab);
      return 4;
    }

    /* modifications */
    rv=APY_User_SetApiSecrets(u, apiPassword, apiSignature);
    if (rv<0) {
      fprintf(stderr, "ERROR: Error on APY_User_SetApiSecrets (%d)\n", rv);
      AB_Banking_EndExclUseUser(ab, u, 1);
      AB_Banking_OnlineFini(ab);
      AB_Banking_Fini(ab);
      return 3;
    }

    /* unlock user */
    rv=AB_Banking_EndExclUseUser(ab, u, 0);
    if (rv<0) {
      fprintf(stderr,
	      "ERROR: Could not unlock user (%d)\n",
	      rv);
      AB_Banking_OnlineFini(ab);
      AB_Banking_Fini(ab);
      return 4;
    }
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



