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



/* ------------------------------------------------------------------------------------------------
 * defs
 * ------------------------------------------------------------------------------------------------
 */

#define A_ARG GWEN_ARGS_FLAGS_HAS_ARGUMENT
#define A_END (GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST)
#define A_CHAR GWEN_ArgsType_Char
#define A_INT GWEN_ArgsType_Int



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static AB_USER *_getUserFromUidOrAid(AB_PROVIDER *pro, GWEN_DB_NODE *db);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

int AH_Control_GetAccSepa(AB_PROVIDER *pro, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  AB_USER *u=NULL;
  int rv;
  const GWEN_ARGS args[]= {
    /* flags type    name         min max s    long       short_descr, long_descr */
    { A_ARG, A_INT,  "userId",    0,  1,  "u", "user",    "Specify unique user id", NULL},
    { A_ARG, A_INT,  "accountId", 0,  1,  "a", "account", "Specify unique id of account", NULL},
    { A_END, A_INT,  "help",      0,  0,  "h", "help",    "Show this help screen", NULL}
  };

  db=GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_DEFAULT, "local");
  rv=AB_Cmd_Handle_Args(argc, argv, args, db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    return 1;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    return 0;
  }

  u=_getUserFromUidOrAid(pro, db);
  if (u==NULL) {
    return 1;
  }
  else {
    AB_IMEXPORTER_CONTEXT *ctx;

    ctx=AB_ImExporterContext_new();
    rv=AH_Provider_GetAccountSepaInfo(pro, u, ctx, 1, 0, 1);
    if (rv<0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not execute outbox.\n");
      AB_User_free(u);
      return 4;
    }
  }
  AB_User_free(u);

  return 0;
}




AB_USER *_getUserFromUidOrAid(AB_PROVIDER *pro, GWEN_DB_NODE *db)
{
  uint32_t uid;
  AB_USER *u=NULL;
  int rv;

  uid=(uint32_t) GWEN_DB_GetIntValue(db, "userId", 0, 0);
  if (uid==0) {
    uint32_t aid;
    AB_ACCOUNT *a;

    aid=(uint32_t) GWEN_DB_GetIntValue(db, "accountId", 0, 0);
    if (aid==0) {
      fprintf(stderr, "ERROR: Neither unique user id nor unique account id given\n");
      return NULL;
    }
  
    /* get account */
    rv=AB_Provider_HasAccount(pro, aid);
    if (rv<0) {
      fprintf(stderr, "ERROR: Account with id %lu not found\n", (unsigned long int) aid);
      return NULL;
    }
    rv=AB_Provider_GetAccount(pro, aid, 1, 1, &a);
    if (rv<0) {
      fprintf(stderr, "ERROR: Account with id %lu not found\n", (unsigned long int) aid);
      return NULL;
    }

    uid=AB_Account_GetUserId(a);
    if (uid==0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No user for this account");
      AB_Account_free(a);
      return NULL;
    }
    AB_Account_free(a);
  }

  rv=AB_Provider_HasUser(pro, uid);
  if (rv<0) {
    fprintf(stderr, "ERROR: User with id %lu not found\n", (unsigned long int) uid);
    return NULL;
  }
  rv=AB_Provider_GetUser(pro, uid, 1, 1, &u);
  if (rv<0) {
    fprintf(stderr, "ERROR: User with id %lu not found\n", (unsigned long int) uid);
    return NULL;
  }

  return u;
}




