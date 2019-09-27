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

  verbose = GWEN_DB_VariableExists(db, "verbose");

  al=AB_Account_List_new();
  rv=AB_Provider_ReadAccounts(pro, al);
  if (rv<0) {
    DBG_ERROR_ERR(0, rv);
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




