/***************************************************************************
 begin       : Fri Sep 18 2009
 copyright   : (C) 2009 by Martin Preuss
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


int listUsers(AB_BANKING *ab,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv) {
  GWEN_DB_NODE *db;
  int rv;
  int xml=0;
  AB_USER_LIST2 *ul;

  const GWEN_ARGS args[]={
  {
    0,                             /* flags */
    GWEN_ArgsType_Int,             /* type */
    "xml",                        /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "xml",                /* long option */
    "Export as xml",  /* short description */
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

  xml=GWEN_DB_VariableExists(db, "xml");

  if( xml ) {
    fprintf( stdout, "<?xml version=\"1.0\"?>\n" );
    fprintf( stdout, "<users>\n" );
  }

  ul=AB_Banking_FindUsers(ab, EBC_PROVIDER_NAME, "*", "*", "*", "*");
  if (ul) {
    AB_USER_LIST2_ITERATOR *uit;

    uit=AB_User_List2_First(ul);
    if (uit) {
      AB_USER *u;
      int i=0;

      u=AB_User_List2Iterator_Data(uit);
      assert(u);
      while(u) {
        if( !xml ) {
          fprintf(stdout, "User %d: Bank: %s/%s User Id: %s Customer Id: %s\n",
                  i++,
                  AB_User_GetCountry(u),
                  AB_User_GetBankCode(u),
                  AB_User_GetUserId(u),
                  AB_User_GetCustomerId(u));
        }
        else {
          const char *name = AB_User_GetUserName(u);
          fprintf( stdout, "  <user>\n" );
          fprintf( stdout, "    <userUniqueId>%d</userUniqueId>\n", AB_User_GetUniqueId(u) );
          if( !name )
            fprintf( stdout, "    <UserName></UserName>\n" );
          else
            fprintf( stdout, "    <UserName><![CDATA[%s]]></UserName>\n", name );
          fprintf( stdout, "    <UserId>%s</UserId>\n", AB_User_GetUserId(u) );
          fprintf( stdout, "    <CustomerId>%s</CustomerId>\n", AB_User_GetCustomerId(u) );
          fprintf( stdout, "    <BankCode>%s</BankCode>\n", AB_User_GetBankCode(u) );
          fprintf( stdout, "    <Country>%s</Country>\n", AB_User_GetCountry(u) );
          fprintf( stdout, "    <LastSessionId>%d</LastSessionId>\n", AB_User_GetLastSessionId(u) );
          fprintf( stdout, "  </user>\n\n" );
        }
        u=AB_User_List2Iterator_Next(uit);
      }
      AB_User_List2Iterator_free(uit);
    }
    AB_User_List2_free(ul);
  }
  else {
    fprintf(stderr, "No users found.\n");
  }


  if( xml ) {
    fprintf( stdout, "</users>\n" );
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




