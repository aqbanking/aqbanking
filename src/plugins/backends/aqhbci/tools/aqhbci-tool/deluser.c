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



int delUser(AB_BANKING *ab,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv) {
  GWEN_DB_NODE *db;
  AB_PROVIDER *pro;
  int rv;
  const char *bankId;
  const char *userId;
  const char *customerId;
  const char *userName;
  int uninitialized = -1;
  uint32_t delAll = 0;
  uint32_t delAccounts = 0;
  uint32_t pretend = 0;
  uint32_t userUniqueId = 0;
  AB_USER_LIST2 *ul;
  AB_USER_LIST2 *matches;
  AB_USER_LIST2_ITERATOR *uit;
  int match_count = 0;
  int haveaccounts_count = 0;
  int error_count = 0;

  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "userName",                   /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "N",                          /* short option */
    "username",                   /* long option */
    "Specify the user name", /* short description */
    "Specify the user name (not the userid!)"  /* long description */
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
    GWEN_ArgsType_Int,           /* type */
    "uninitialized",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    NULL,                          /* short option */
    "uninitialized",                   /* long option */
    "Match uninitialized users (PARAM=1) or initialized users (PARAM=0)",    /* short description */
    "Match uninitialized users (PARAM=1) or initialized users (PARAM=0)"     /* long description */
  },
  {
    0, /* flags */
    GWEN_ArgsType_Int,           /* type */
    "all",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "A",                          /* short option */
    "all",                   /* long option */
    "Delete all matching users, do not abort if more than one user matches",    /* short description */
    "Delete all matching users, do not abort if more than one user matches"     /* long description */
  },
  {
    0, /* flags */
    GWEN_ArgsType_Int,           /* type */
    "withAccounts",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "a",                          /* short option */
    "with-accounts",                   /* long option */
    "Delete all accounts of user",    /* short description */
    "Delete all accounts of user"     /* long description */
  },
  {
    0, /* flags */
    GWEN_ArgsType_Int,           /* type */
    "pretend",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "p",                          /* short option */
    "pretend",                   /* long option */
    "Only print matching users, don't delete",    /* short description */
    "Only print matching users, don't delete"     /* long description */
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
  userName=GWEN_DB_GetCharValue(db, "userName", 0, 0);
  uninitialized=GWEN_DB_GetIntValue(db, "uninitialized", 0, -1);
  delAll=GWEN_DB_GetIntValue(db, "all", 0, 0);
  delAccounts=GWEN_DB_GetIntValue(db, "withAccounts", 0, 0);
  pretend=GWEN_DB_GetIntValue(db, "pretend", 0, 0);
  userUniqueId=GWEN_DB_GetIntValue(db, "userUniqueId", 0, 0);

  if( uninitialized != 0 && uninitialized != 1  && uninitialized != -1 )
  {
    fprintf( stderr, "Please specify either 0 or 1 for --uninitialized\n" );
    return 1;
  }

  matches = AB_User_List2_new();

  ul=AB_Banking_FindUsers(ab, AH_PROVIDER_NAME, "*", "*", "*", "*");
  if (ul) {

    uit=AB_User_List2_First(ul);
    if (uit) {
      const char *s;
      AB_USER *u;
      int i=0;

      u=AB_User_List2Iterator_Data(uit);
      assert(u);
      while(u) {
        int match = 1;

        if (match && bankId) {
          s=AB_User_GetBankCode(u);
          if (!s || !*s || -1==GWEN_Text_ComparePattern(s, bankId, 0))
            match=0;
        }

        if (match && userId) {
          s=AB_User_GetUserId(u);
          if (!s || !*s || -1==GWEN_Text_ComparePattern(s, userId, 0))
            match=0;
        }

        if (match && customerId) {
          s=AB_User_GetCustomerId(u);
          if (!s || !*s || -1==GWEN_Text_ComparePattern(s, customerId, 0))
            match=0;
        }

        if (match && userName) {
          s=AB_User_GetUserName(u);
          if (!s || !*s || -1==GWEN_Text_ComparePattern(s, userName, 0))
            match=0;
        }

        if (match && userUniqueId) {
          uint32_t id = AB_User_GetUniqueId(u);
          if ( userUniqueId != id )
            match=0;
        }

        if (match && uninitialized != -1) {
          s=AH_User_GetSystemId(u);
          if( (uninitialized == 1 && s != NULL) || (uninitialized == 0 && s == NULL) )
            match = 0;
        }

        if( match )
        {
          match_count++;
          fprintf(stdout, "User %d: Bank: %s/%s User Id: %s Customer Id: %s\n",
                  i++,
                  AB_User_GetCountry(u),
                  AB_User_GetBankCode(u),
                  AB_User_GetUserId(u),
                  AB_User_GetCustomerId(u));
          AB_User_List2_PushBack( matches, u );

          haveaccounts_count += (AB_Banking_FindFirstAccountOfUser(ab, u) != NULL ? 1 : 0 );
        }

        u=AB_User_List2Iterator_Next(uit);
      }
      AB_User_List2Iterator_free(uit);
    }
    AB_User_List2_free(ul);
  }


  if( !match_count )
  {
    fprintf( stderr, "ERROR: No matching users\n" );
    return 3;
  }

  if( match_count > 1 && !delAll )
  {
    fprintf( stderr, "ERROR: %d users match. Refusing to delete more than one user. Please specify --all to delete all matching users.\n", match_count );
    return 3;
  }

  if( haveaccounts_count && !delAccounts )
  {
    fprintf( stderr, "ERROR: %d users still have accounts. Refusing to delete those users. Please specify --with-accounts to delete all accounts of matching users.\n", haveaccounts_count );
    return 3;
  }

  if( !pretend )
  {
    uit=AB_User_List2_First(matches);
    if (uit) {
      AB_USER *u;
      int i=0;

      u=AB_User_List2Iterator_Data(uit);
      assert(u);
      while(u) {
        int error = 0;

        if( delAccounts )
        {
          AB_ACCOUNT *a;
          a = AB_Banking_FindFirstAccountOfUser( ab, u );
          while( a )
          {
            rv = AB_Banking_DeleteAccount( ab, a );
            if( rv )
            {
              fprintf( stderr, "ERROR: Error deleting account %d for user %d (%d), aborting this user.\n", AB_Account_GetUniqueId(a), i, rv );
              error++;
              error_count++;
            }
            else
              fprintf( stdout, "Account %d deleted.\n", AB_Account_GetUniqueId(a) );
            a = AB_Banking_FindFirstAccountOfUser( ab, u );
          }
        }

        if( !error )
        {
          rv = AB_Banking_DeleteUser( ab, u );
          if( rv )
          {
            fprintf( stderr, "ERROR: Error deleting user %d (%d).\n", i, rv );
            error++;
            error_count++;
          }
          else
            fprintf( stdout, "User %d deleted.\n", i );
        }

        u=AB_User_List2Iterator_Next(uit);
        i++;
      }
      AB_User_List2Iterator_free(uit);
    }
    AB_User_List2_free(matches);
  } // !pretend
  else
  {
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





