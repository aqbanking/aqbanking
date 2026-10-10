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



int AH_Control_ListAccounts(AB_PROVIDER *pro,
                            GWEN_DB_NODE *dbArgs,
                            int argc,
                            char **argv)
{
  GWEN_DB_NODE *db;
  int rv, verbose;
  AB_ACCOUNT_LIST *al;
  AB_ACCOUNT *a;
  int i=0;
  const GWEN_ARGS args[]= {
    {
      0,                  /* flags */
      GWEN_ArgsType_Int,  /* type */
      "verbose",          /* name */
      0,                  /* minnum */
      1,                  /* maxnum */
      "v",                /* short option */
      "verbose",          /* long option */
      "Show list in verbose form (with more columns)",  /* short description */
      0
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
    return 1;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    return 0;
  }

  verbose = GWEN_DB_VariableExists(db, "verbose");

  al=AB_Account_List_new();
  rv=AB_Provider_ReadAccounts(pro, al);
  if (rv<0) {
    if (rv==GWEN_ERROR_NOT_FOUND) {
      DBG_ERROR(0, "No accounts found.");
    }
    else {
      DBG_ERROR_ERR(0, rv);
    }
    AB_Account_List_free(al);
    return 3;
  }

  a=AB_Account_List_First(al);
  while (a) {
    fprintf(stdout, "Account %d: Bank: %s Account Number: %s",
            i++,
            AB_Account_GetBankCode(a),
            AB_Account_GetAccountNumber(a));
    if (verbose) {
      const char *subAccountId = AB_Account_GetSubAccountId(a);

      fprintf(stdout, "  SubAccountId: %s  Account Type: %s LocalUniqueId: %d",
              subAccountId ? subAccountId : "(none)",
              AB_AccountType_toChar(AB_Account_GetAccountType(a)),
              AB_Account_GetUniqueId(a));
    }
    fprintf(stdout, "\n");
    a=AB_Account_List_Next(a);
  }
  AB_Account_List_free(al);

  return 0;
}




