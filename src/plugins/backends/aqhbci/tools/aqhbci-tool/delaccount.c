/***************************************************************************
 begin       : Thu Nov 06 2008
 copyright   : (C) 2008 by Patrick Prasse
 email       : patrick-oss@prasse.info

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "globals.h"
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



int delAccount(AB_BANKING *ab,
	       GWEN_DB_NODE *dbArgs,
	       int argc,
	       char **argv) {
  GWEN_DB_NODE *db;
  AB_PROVIDER *pro;
  int rv;
  const char *bankId;
  const char *userId;
  const char *customerId;
  const char *account;
  const char *subAccountId;
  uint32_t delAll = 0;
  uint32_t pretend = 0;
  uint32_t uniqueId = 0;
  uint32_t userUniqueId = 0;
  AB_ACCOUNT_LIST2 *al;
  AB_ACCOUNT_LIST2 *matches;
  AB_ACCOUNT_LIST2_ITERATOR *ait;
  int match_count = 0;
  int error_count = 0;

  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "account",                   /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "a",                          /* short option */
    "account",                   /* long option */
    "Specify the account number", /* short description */
    "Specify the account number"  /* long description */
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
    "accountUniqueId",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "i",                          /* short option */
    "unique",                   /* long option */
    "Specify the account unique id",    /* short description */
    "Specify the account unique id"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "userUniqueId",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                          /* short option */
    "user-unique",                   /* long option */
    "Specify the user unique id",    /* short description */
    "Specify the user unique id"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "userId",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "u",                          /* short option */
    "user",                       /* long option */
    "Specify the user id (Benutzerkennung)",        /* short description */
    "Specify the user id (Benutzerkennung)"         /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "customerId",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "c",                          /* short option */
    "customer",                   /* long option */
    "Specify the customer id (Kundennummer)",    /* short description */
    "Specify the customer id (Kundennummer)"     /* long description */
  },
  {
    0, /* flags */
    GWEN_ArgsType_Int,           /* type */
    "all",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "A",                          /* short option */
    "all",                   /* long option */
    "Delete all matching accounts, do not abort if more than one account matches",    /* short description */
    "Delete all matching accounts, do not abort if more than one account matches"     /* long description */
  },
  {
    0, /* flags */
    GWEN_ArgsType_Int,           /* type */
    "pretend",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "p",                          /* short option */
    "pretend",                   /* long option */
    "Only print matching accounts, don't delete",    /* short description */
    "Only print matching accounts, don't delete"     /* long description */
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

  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, 0);
  userId=GWEN_DB_GetCharValue(db, "userId", 0, 0);
  customerId=GWEN_DB_GetCharValue(db, "customerId", 0, 0);
  account=GWEN_DB_GetCharValue(db, "account", 0, 0);
  subAccountId=GWEN_DB_GetCharValue(db, "subAccountId", 0, 0);
  delAll=GWEN_DB_GetIntValue(db, "all", 0, 0);
  pretend=GWEN_DB_GetIntValue(db, "pretend", 0, 0);
  uniqueId=GWEN_DB_GetIntValue(db, "accountUniqueId", 0, 0);
  userUniqueId=GWEN_DB_GetIntValue(db, "userUniqueId", 0, 0);

  matches = AB_Account_List2_new();

  al=AB_Banking_FindAccounts(ab, AH_PROVIDER_NAME, "*", "*", "*", "*");
  if (al) {
    ait=AB_Account_List2_First(al);
    if (ait) {
      const char *s;
      AB_ACCOUNT *a;
      int i=0;

      a=AB_Account_List2Iterator_Data(ait);
      assert(a);
      while(a) {
        int match = 1;

        if (match && bankId) {
          s=AB_Account_GetBankCode(a);
          if (!s || !*s || -1==GWEN_Text_ComparePattern(s, bankId, 0))
            match=0;
        }

        if (match && account) {
          s=AB_Account_GetAccountNumber(a);
          if (!s || !*s || -1==GWEN_Text_ComparePattern(s, account, 0))
            match=0;
        }

        if (match && subAccountId) {
          s=AB_Account_GetSubAccountId(a);
          if (!s || !*s || -1==GWEN_Text_ComparePattern(s, subAccountId, 0))
            match=0;
        }

        if (match && uniqueId) {
          uint32_t id = AB_Account_GetUniqueId(a);
          if ( uniqueId != id )
            match=0;
        }

	if (match && (userId || customerId || userUniqueId)) {
	  AB_USER *u;
	  u = AB_Account_GetFirstUser( a );
          if( !u )
            match = 0;

          if (match && userUniqueId) {
	    uint32_t id = AB_User_GetUniqueId(u);
	    if ( userUniqueId != id )
	      match=0;
	  }
	  if (match && userId) {
	    s = AB_User_GetUserId(u);
	    if (!s || !*s || -1==GWEN_Text_ComparePattern(s, userId, 0))
	      match=0;
	  }
	  if (match && customerId) {
	    s=AB_User_GetCustomerId(u);
	    if (!s || !*s || -1==GWEN_Text_ComparePattern(s, customerId, 0))
	      match=0;
	  }
	}


        if( match ) {
	  match_count++;
	  fprintf(stdout, "Account %d:\tUniqueId: %d\t\tAccount Number: %s\tBank: %s/%s\n",
		  i++,
		  AB_Account_GetUniqueId(a),
		  AB_Account_GetAccountNumber(a),
		  AB_Account_GetCountry(a),
		  AB_Account_GetBankCode(a)
		 );
	  AB_Account_List2_PushBack( matches, a );
	}

        a=AB_Account_List2Iterator_Next(ait);
      }
      AB_Account_List2Iterator_free(ait);
    }
    AB_Account_List2_free(al);
  }


  if( !match_count ) {
    fprintf( stderr, "ERROR: No matching accounts\n" );
    return 3;
  }

  if( match_count > 1 && !delAll ) {
    fprintf( stderr,
	    "ERROR: %d accounts match. Refusing to delete more than one account. "
	    "Please specify --all to delete all matching accounts.\n",
	    match_count );
    return 3;
  }


  if( !pretend ) {
    ait=AB_Account_List2_First(matches);
    if (ait) {
      AB_ACCOUNT *a;
      int i=0;

      a=AB_Account_List2Iterator_Data(ait);
      assert(a);
      while(a) {
	rv = AB_Banking_DeleteAccount( ab, a );
	if( rv ) {
	  fprintf( stderr, "ERROR: Error deleting account %d (%d).\n", i, rv );
	  error_count++;
	}
	else
	  fprintf( stdout, "Account %d deleted.\n", i );

	a=AB_Account_List2Iterator_Next(ait);
	i++;
      }
      AB_Account_List2Iterator_free(ait);
    }
    AB_Account_List2_free(matches);
  } /* !pretend */
  else {
    fprintf( stdout, "Nothing deleted.\n" );
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

  return error_count > 0 ? 3 : 0;
}


