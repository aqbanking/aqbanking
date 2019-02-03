/***************************************************************************
 begin       : Mon Oct 13 2008
 copyright   : (C) 2018 by Martin Preuss
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


int setEbicsVersion(AB_PROVIDER *pro,
                    GWEN_DB_NODE *dbArgs,
                    int argc,
                    char **argv)
{
  GWEN_DB_NODE *db;
  uint32_t uid;
  AB_USER *u=NULL;
  int rv;
  const char *ebicsVersion;
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "ebicsVersion",                /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "E",                          /* short option */
      "ebicsversion",               /* long option */
      "Specify the EBICS version to use (e.g. H002)",     /* short description */
      "Specify the EBICS version to use (e.g. H002)"      /* long description */
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

  ebicsVersion=GWEN_DB_GetCharValue(db, "ebicsVersion", 0, "H003");

  /* doit */
  uid=(uint32_t) GWEN_DB_GetIntValue(db, "userId", 0, 0);
  if (uid==0) {
    fprintf(stderr, "ERROR: Invalid or missing unique user id\n");
    return 1;
  }

  rv=AB_Provider_GetUser(pro, uid, 1, 0, &u); /* don't unlock to allow for AH_Provider_EndExclUseUser */
  if (rv<0) {
    fprintf(stderr, "ERROR: User with id %lu not found\n", (unsigned long int) uid);
    return 2;
  }
  else {
    if (ebicsVersion) {
      if (strcasecmp(ebicsVersion, "H002")==0) {
        EBC_User_SetProtoVersion(u, "H002");
        EBC_User_SetSignVersion(u, "A004");
        EBC_User_SetAuthVersion(u, "X001");
        EBC_User_SetCryptVersion(u, "E001");
      }
      else if (strcasecmp(ebicsVersion, "H003")==0) {
        EBC_User_SetProtoVersion(u, "H003");
        EBC_User_SetSignVersion(u, "A005");
        EBC_User_SetAuthVersion(u, "X002");
        EBC_User_SetCryptVersion(u, "E002");
      }
      else {
        fprintf(stderr, "%s",
                I18N("Invalid protocol version.\n"
                     "Possible versions are H002 and H003.\n"));
        return 3;
      }
    }

    rv=AB_Provider_EndExclUseUser(pro, u, 0);
    if (rv<0) {
      DBG_ERROR(0, "Could not unlock customer");
      AB_Provider_EndExclUseUser(pro, u, 1); /* abandon */
      return 3;
    }
  }

  fprintf(stderr, "EBICS version set to %s.\n", ebicsVersion);

  return 0;
}




