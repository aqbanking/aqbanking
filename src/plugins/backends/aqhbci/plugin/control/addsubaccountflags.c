/***************************************************************************
 begin       : Tue Sep 20 2008
 copyright   : (C) 2008 by Patrick Prasse
 copyright   : (C) 2018 by Martin Preuss
 email       : patrick-oss@prasse.info

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "globals_l.h"

#include <gwenhywfar/text.h>

#include <aqhbci/user.h>
#include <aqhbci/account.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int addsubAccountFlags(AB_PROVIDER *pro,
                       GWEN_DB_NODE *dbArgs,
                       int argc,
                       char **argv, int is_add ) {
  GWEN_DB_NODE *db;
  AB_ACCOUNT *a=NULL;
  int rv;
  uint32_t aid;
  GWEN_DB_NODE *vn;
  uint32_t flags;
  uint32_t bf;
  uint32_t c=0;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Int,           /* type */
    "accountId",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "a",                          /* short option */
    "account",                   /* long option */
    "Specify the unique id of the account",    /* short description */
    "Specify the unique id of the account"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "flags",                      /* name */
    1,                            /* minnum */
    99,                            /* maxnum */
    "f",                          /* short option */
    "flags",                   /* long option */
    "Specify the user flags",    /* short description */
    "Specify the user flags"     /* long description */
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

  /* check aid */
  aid=(uint32_t) GWEN_DB_GetIntValue(db, "accountId", 0, 0);
  if (aid==0) {
    fprintf(stderr, "ERROR: Invalid or missing unique account id\n");
    return 1;
  }

  /* parse and check flags */
  flags=AH_Account_Flags_fromDb(db, "flags");
  for (bf=flags; bf; bf>>=1) {
    if (bf&1)
      c++;
  }
  vn=GWEN_DB_FindFirstVar(db, "flags");
  if (GWEN_DB_Values_Count(vn)!=c) {
    fprintf(stderr, "ERROR: Specified flag(s) unknown\n");
    return 4;
  }

  /* get account (lock, don't unlock, so we can later call AH_Provider_EndExclUseAccount */
  rv=AB_Provider_GetAccount(pro, aid, 1, 0, &a);
  if (rv<0) {
    fprintf(stderr, "ERROR: Account with id %lu not found\n", (unsigned long int) aid);
    return 2;
  }

  /* modify account */
  if (is_add) {
    fprintf(stderr, "Adding flags: %08x\n", flags);
    AH_Account_AddFlags(a, flags);
  }
  else {
    fprintf(stderr, "Removing flags: %08x\n", flags);
    AH_Account_SubFlags(a, flags);
  }

  /* unlock account */
  rv=AB_Provider_EndExclUseAccount(pro, a, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Could not unlock account (%d)\n", rv);
    AB_Account_free(a);
    return 4;
  }
  AB_Account_free(a);

  return 0;
}




