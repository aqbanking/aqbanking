/***************************************************************************
 begin       : Tue Sep 20 2008
 copyright   : (C) 2008 by Patrick Prasse
 email       : patrick-oss@prasse.info

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "globals.h"

#include <gwenhywfar/text.h>

#include <aqhbci/user.h>
#include <aqhbci/account.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int addsubAccountFlags(AB_BANKING *ab,
		 GWEN_DB_NODE *dbArgs,
		 int argc,
		 char **argv, int is_add ) {
  GWEN_DB_NODE *db;
  AB_PROVIDER *pro;
  AB_ACCOUNT_LIST2 *al;
  AB_ACCOUNT *a=0;
  int rv;
  const char *bankId;
  const char *accountId;
  const char *subAccountId;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "bankId",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "b",                          /* short option */
    "bank",                       /* long option */
    "Specify the bank code",      /* short description */
    "Specify the bank code"       /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "accountId",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "a",                          /* short option */
    "account",                   /* long option */
    "Specify the account id (Kontonummer)",    /* short description */
    "Specify the account id (Kontonummer)"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "subAccountId",                /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "aa",                          /* short option */
    "subaccount",                   /* long option */
    "Specify the sub account id (Unterkontomerkmal)",    /* short description */
    "Specify the sub account id (Unterkontomerkmal)"     /* long description */
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

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  rv=AB_Banking_OnlineInit(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  pro=AB_Banking_GetProvider(ab, "aqhbci");
  assert(pro);

  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, "*");
  accountId=GWEN_DB_GetCharValue(db, "accountId", 0, "*");
  subAccountId=GWEN_DB_GetCharValue(db, "subAccountId", 0, "*");

  al=AB_Banking_FindAccounts(ab, AH_PROVIDER_NAME, "de",
                             bankId, accountId, subAccountId);
  if (al) {
    if (AB_Account_List2_GetSize(al)!=1) {
      DBG_ERROR(0, "Ambiguous account specification");
      AB_Account_List2_free(al);
      return 3;
    }
    else {
      AB_ACCOUNT_LIST2_ITERATOR *ait;

      ait=AB_Account_List2_First(al);
      assert(ait);
      a=AB_Account_List2Iterator_Data(ait);
      AB_Account_List2Iterator_free(ait);
    }
    AB_Account_List2_free(al);
  }
  if (!a) {
    DBG_ERROR(0, "No matching customer");
    return 3;
  }
  else {
    GWEN_DB_NODE *vn;
    uint32_t flags, bf, c=0;

    /* parse flags */
    flags=AH_Account_Flags_fromDb(db, "flags");
    for (bf=flags; bf; bf>>=1) {
      if (bf&1)
	c++;
    }
    vn=GWEN_DB_FindFirstVar(db, "flags");
    if (GWEN_DB_Values_Count(vn)!=c) {
      fprintf(stderr, "ERROR: Specified flag(s) unknown\n");
      AB_Banking_OnlineFini(ab);
      AB_Banking_Fini(ab);
      return 4;
    }
      
    /* lock account */
    rv=AB_Banking_BeginExclUseAccount(ab, a);
    if (rv<0) {
      fprintf(stderr,
	      "ERROR: Could not lock account, maybe it is used in another application? (%d)\n",
	      rv);
      AB_Banking_OnlineFini(ab);
      AB_Banking_Fini(ab);
      return 4;
    }

    /* modify account */
    if( is_add ) {
      fprintf(stderr, "Adding flags: %08x\n", flags);
      AH_Account_AddFlags(a, flags);
    }
    else {
      fprintf(stderr, "Removing flags: %08x\n", flags);
      AH_Account_SubFlags(a, flags);
    }

    /* unlock account */
    rv=AB_Banking_EndExclUseAccount(ab, a, 0);
    if (rv<0) {
      fprintf(stderr,
	      "ERROR: Could not unlock account (%d)\n",
	      rv);
      AB_Banking_OnlineFini(ab);
      AB_Banking_Fini(ab);
      return 4;
    }
  }

  rv=AB_Banking_OnlineFini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}




