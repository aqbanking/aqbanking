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


#include "getaccounts.h"

#include "aqofxconnect/user.h"
#include "aqofxconnect/provider.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/args.h>



static GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv);





int AO_Control_GetAccounts(AB_PROVIDER *pro, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  uint32_t uid;
  AB_USER *u=NULL;
  int rv;

  /* parse command line */
  db=_readCommandLine(dbArgs, argc, argv);
  if (db==NULL) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Could not parse arguments\n");
    return 1;
  }

  uid=(uint32_t) GWEN_DB_GetIntValue(db, "userId", 0, 0);
  if (uid==0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Invalid or missing unique user id\n");
    return 1;
  }

  rv=AB_Provider_HasUser(pro, uid);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "User with id %lu not found\n", (unsigned long int) uid);
    return 2;
  }
  rv=AB_Provider_GetUser(pro, uid, 1, 1, &u);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "User with id %lu not found\n", (unsigned long int) uid);
    return 2;
  }
  else {
    rv=AO_Provider_RequestAccounts(pro, u, 0);
    if (rv) {
      DBG_ERROR_ERR(0, rv);
      AB_User_free(u);
      return 3;
    }
  }
  AB_User_free(u);

  return 0;
}




GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "userId",                     /* name */
      1,                            /* minnum */
      1,                            /* maxnum */
      "u",                          /* short option */
      "user",                       /* long option */
      "Specify the unique user id", /* short description */
      "Specify the unique user id"  /* long description */
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
  rv=AB_Cmd_Handle_Args(argc, argv, args, db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    return NULL;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    return NULL;
  }

  return db;
}




