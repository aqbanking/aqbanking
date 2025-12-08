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
#include "aqhbci/banking/user.h"

#include <gwenhywfar/text.h>
#include <gwenhywfar/url.h>
#include <gwenhywfar/ct.h>
#include <gwenhywfar/ctplugin.h>



/* ------------------------------------------------------------------------------------------------
 * defs
 * ------------------------------------------------------------------------------------------------
 */

#define A_ARG GWEN_ARGS_FLAGS_HAS_ARGUMENT
#define A_END (GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST)
#define A_CHAR GWEN_ArgsType_Char
#define A_INT GWEN_ArgsType_Int



/* ------------------------------------------------------------------------------------------------
 * code
 * ------------------------------------------------------------------------------------------------
 */

int APY_Control_DelUser(AB_PROVIDER *pro, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  AB_USER *u=NULL;
  uint32_t uid;
  int rv;
  uint32_t pretend = 0;
  const GWEN_ARGS args[]= {
    /* flags type    name         min max s    long       short_descr, long_descr */
    { A_ARG, A_INT,  "userId",    0,  1,  "u", "user",    "Specify unique user id", NULL},
    { 0,     A_INT,  "pretend",   0,  1,  "p", "pretend", "Only print user, don't delete", NULL},
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

  uid=(uint32_t) GWEN_DB_GetIntValue(db, "userId", 0, 0);
  if (uid==0) {
    fprintf(stderr, "ERROR: Invalid or missing unique user id\n");
    return 1;
  }

  pretend=GWEN_DB_GetIntValue(db, "pretend", 0, 0);

  rv=AB_Provider_HasUser(pro, uid);
  if (rv<0) {
    fprintf(stderr, "ERROR: User with id %lu not found\n", (unsigned long int) uid);
    return 2;
  }
  rv=AB_Provider_GetUser(pro, uid, 1, 1, &u);
  if (rv<0) {
    fprintf(stderr, "ERROR: User with id %lu not found\n", (unsigned long int) uid);
    return 2;
  }

  if (pretend) {
    fprintf(stdout, "User 0: Bank: %s/%s User Id: %s Customer Id: %s Unique Id: %lu\n",
            AB_User_GetCountry(u),
            AB_User_GetBankCode(u),
            AB_User_GetUserId(u),
            AB_User_GetCustomerId(u),
            (unsigned long int) AB_User_GetUniqueId(u));
  }
  else {
    rv=AB_Provider_DeleteUser(pro, uid);
    if (rv<0) {
      fprintf(stderr, "ERROR: Could not delete user %lu (%d)\n", (unsigned long int) uid, rv);
      AB_User_free(u);
      return 2;
    }
  }
  AB_User_free(u);

  return 0;
}





