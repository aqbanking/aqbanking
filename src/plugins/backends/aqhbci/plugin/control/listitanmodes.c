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

#include <gwenhywfar/text.h>

#include <aqhbci/user.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int listItanModes(AB_PROVIDER *pro,
		  GWEN_DB_NODE *dbArgs,
		  int argc,
		  char **argv) {
  GWEN_DB_NODE *db;
  uint32_t uid;
  AB_USER *u=NULL;
  int rv;
  const GWEN_ARGS args[]={
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
    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }

  /* doit */
  uid=(uint32_t) GWEN_DB_GetIntValue(db, "userId", 0, 0);
  if (uid==0) {
    fprintf(stderr, "ERROR: Invalid or missing unique user id\n");
    return 1;
  }

  rv=AB_Provider_GetUser(pro, uid, 1, 1, &u);
  if (rv<0) {
    fprintf(stderr, "ERROR: User with id %lu not found\n", (unsigned long int) uid);
    return 2;
  }
  else {
    const AH_TAN_METHOD_LIST *tl;

    tl=AH_User_GetTanMethodDescriptions(u);
    if (tl) {
      const AH_TAN_METHOD *tm;

      tm=AH_TanMethod_List_First(tl);
      fprintf(stdout, "TAN Methods\n");
      while(tm) {
	const char *mid;
        const char *mname;
        int combinedVersion;

        combinedVersion=AH_TanMethod_GetFunction(tm)+(AH_TanMethod_GetGvVersion(tm)*1000);
        fprintf(stdout,
                "- %4d (F%3d/V%1d/P%1d)",
                combinedVersion,
                AH_TanMethod_GetFunction(tm),
                AH_TanMethod_GetGvVersion(tm),
                AH_TanMethod_GetProcess(tm));
        mid=AH_TanMethod_GetMethodId(tm);
	mname=AH_TanMethod_GetMethodName(tm);
	if (mid && mname) {
	  fprintf(stdout, ": %s (%s)", mid, mname);
	}
	else if (mid && !mname) {
	  fprintf(stdout, ": %s", mid);
	}
	else if (!mid && mname) {
	  fprintf(stdout, ": %s", mname);
	}

	if (AH_User_HasTanMethod(u, AH_TanMethod_GetFunction(tm))) {
          if (AH_User_GetSelectedTanMethod(u)==combinedVersion)
	    fprintf(stdout, " [available and selected]");
          else
	    fprintf(stdout, " [available]");
	}
	else
	  fprintf(stdout, " [not available]");
	fprintf(stdout, "\n");

	tm=AH_TanMethod_List_Next(tm);
      }
    }

  }
  AB_User_free(u);

  return 0;
}




