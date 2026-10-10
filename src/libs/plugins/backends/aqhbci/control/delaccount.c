/***************************************************************************
 begin       : Thu Nov 06 2008
 copyright   : (C) 2008 by Patrick Prasse
 copyright   : (C) 2025 by Martin Preuss
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

int AH_Control_DelAccount(AB_PROVIDER *pro, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  AB_ACCOUNT *a=NULL;
  int rv;
  uint32_t aid;
  uint32_t pretend=0;
  const GWEN_ARGS args[]= {
    /* flags type    name         min max s    long       short_descr, long_descr */
    { A_ARG, A_INT,  "accountId", 0,  1,  "a", "account", "Specify unique id of account", NULL},
    { 0,     A_INT,  "pretend",   0,  1,  "p", "pretend", "Only print account, don't delete", NULL},
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

  /* check aid */
  aid=(uint32_t) GWEN_DB_GetIntValue(db, "accountId", 0, 0);
  if (aid==0) {
    fprintf(stderr, "ERROR: Invalid or missing unique account id\n");
    return 1;
  }

  pretend=GWEN_DB_GetIntValue(db, "pretend", 0, 0);

  rv=AB_Provider_HasAccount(pro, aid);
  if (rv<0) {
    fprintf(stderr, "ERROR: Account with id %lu not found\n", (unsigned long int) aid);
    return 2;
  }
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


