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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int AH_Control_GetAccSepa(AB_PROVIDER *pro,
                          GWEN_DB_NODE *dbArgs,
                          int argc,
                          char **argv) {
  GWEN_DB_NODE *db;
  AB_ACCOUNT *a=NULL;
  int rv;
  uint32_t aid;
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
    0,                            /* minnum */
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

  /* get account */
  rv=AB_Provider_GetAccount(pro, aid, 1, 1, &a);
  if (rv<0) {
    fprintf(stderr, "ERROR: Account with id %lu not found\n", (unsigned long int) aid);
    return 2;
  }
  else {
    AB_IMEXPORTER_CONTEXT *ctx;

    ctx=AB_ImExporterContext_new();
    rv=AH_Provider_GetAccountSepaInfo(pro, a, ctx, 1, 0, 1);
    if (rv<0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not execute outbox.\n");
      AB_Account_free(a);
      return 4;
    }
  }
  AB_Account_free(a);

  return 0;
}




