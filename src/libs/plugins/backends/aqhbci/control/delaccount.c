/***************************************************************************
 begin       : Thu Nov 06 2008
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
#include <aqhbci/user.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/url.h>
#include <gwenhywfar/ct.h>
#include <gwenhywfar/ctplugin.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>



int AH_Control_DelAccount(AB_PROVIDER *pro,
                          GWEN_DB_NODE *dbArgs,
                          int argc,
                          char **argv)
{
  GWEN_DB_NODE *db;
  AB_ACCOUNT *a=NULL;
  int rv;
  uint32_t aid;
  uint32_t pretend=0;
  const GWEN_ARGS args[]= {
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
      0, /* flags */
      GWEN_ArgsType_Int,           /* type */
      "pretend",                 /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "p",                          /* short option */
      "pretend",                   /* long option */
      "Only print account, don't delete",    /* short description */
      "Only print account, don't delete"     /* long description */
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

  /* check aid */
  aid=(uint32_t) GWEN_DB_GetIntValue(db, "accountId", 0, 0);
  if (aid==0) {
    fprintf(stderr, "ERROR: Invalid or missing unique account id\n");
    return 1;
  }

  pretend=GWEN_DB_GetIntValue(db, "pretend", 0, 0);

  rv=AB_Provider_GetAccount(pro, aid, 1, 1, &a);
  if (rv<0) {
    fprintf(stderr, "ERROR: Account with id %lu not found\n", (unsigned long int) aid);
    return 2;
  }

  if (pretend) {
    fprintf(stdout, "Account 0:\tUniqueId: %d\t\tAccount Number: %s\tBank: %s/%s\n",
            AB_Account_GetUniqueId(a),
            AB_Account_GetAccountNumber(a),
            AB_Account_GetCountry(a),
            AB_Account_GetBankCode(a)
           );
  }
  else {
    /* delete account */
    rv=AB_Provider_DeleteAccount(pro, aid);
    if (rv<0) {
      fprintf(stderr, "ERROR: Could not delete account (%d)\n", rv);
      AB_Account_free(a);
      return 4;
    }
  }
  AB_Account_free(a);

  return 0;
}


