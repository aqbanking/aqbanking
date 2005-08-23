/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "globals.h"

#include <gwenhywfar/text.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int listAccounts(AB_BANKING *ab,
                 GWEN_DB_NODE *dbArgs,
                 int argc,
                 char **argv) {
  GWEN_DB_NODE *db;
  AB_PROVIDER *pro;
  AH_HBCI *hbci;
  int rv;
  AH_ACCOUNT_LIST2 *al;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsTypeInt,             /* type */
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
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutTypeTXT)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return 1;
    }
    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  pro=AB_Banking_GetProvider(ab, "aqhbci");
  assert(pro);
  hbci=AH_Provider_GetHbci(pro);
  assert(hbci);

  al=AH_HBCI_GetAccounts(hbci, 0, "*", "*");
  if (al) {
    AH_ACCOUNT_LIST2_ITERATOR *ait;

    ait=AH_Account_List2_First(al);
    if (ait) {
      AH_ACCOUNT *a;
      int i=0;

      a=AH_Account_List2Iterator_Data(ait);
      assert(a);
      while(a) {
        AH_BANK *b;

        b=AH_Account_GetBank(a);
        assert(b);
        fprintf(stdout, "Account %d: Bank: %s Account Number: %s\n",
                i++, AH_Bank_GetBankId(b),
                AH_Account_GetAccountId(a));
        a=AH_Account_List2Iterator_Next(ait);
      }
      AH_Account_List2Iterator_free(ait);
    }
    AH_Account_List2_free(al);
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}




