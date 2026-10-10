/***************************************************************************
 begin       : Thu Jan 16 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "listusers.h"

#include "aqofxconnect/user.h"
#include "aqofxconnect/provider.h"

#include "aqbanking/i18n_l.h"
#include "cli/helper.h"

#include <gwenhywfar/args.h>



static GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv);





int AO_Control_ListUsers(AB_PROVIDER *pro, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  int xml=0;
  AB_USER_LIST *ul;
  AB_USER *u;
  int i=0;

  /* parse command line */
  db=_readCommandLine(dbArgs, argc, argv);
  if (db==NULL) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Could not parse arguments\n");
    return 1;
  }

  xml=GWEN_DB_VariableExists(db, "xml");
  if (xml) {
    fprintf(stdout, "<?xml version=\"1.0\"?>\n");
    fprintf(stdout, "<users>\n");
  }

  ul=AB_User_List_new();
  rv=AB_Provider_ReadUsers(pro, ul);
  if (rv<0 && rv!=GWEN_ERROR_NOT_FOUND) {
    DBG_ERROR_ERR(AQOFXCONNECT_LOGDOMAIN, rv);
    AB_User_List_free(ul);
    return 3;
  }

  u=AB_User_List_First(ul);
  while (u) {
    if (!xml) {
      fprintf(stdout, "User %d: Bank: %s/%s User Id: %s Unique Id: %lu\n",
              i++,
              AB_User_GetCountry(u),
              AB_User_GetBankCode(u),
              AB_User_GetUserId(u),
              (unsigned long int) AB_User_GetUniqueId(u));
    }
    else {
      const char *name = AB_User_GetUserName(u);
      fprintf(stdout, "  <user>\n");
      fprintf(stdout, "    <userUniqueId>%lu</userUniqueId>\n", (unsigned long int) AB_User_GetUniqueId(u));
      if (!name)
        fprintf(stdout, "    <UserName></UserName>\n");
      else
        fprintf(stdout, "    <UserName><![CDATA[%s]]></UserName>\n", name);
      fprintf(stdout, "    <UserId>%s</UserId>\n", AB_User_GetUserId(u));
      fprintf(stdout, "    <BankCode>%s</BankCode>\n", AB_User_GetBankCode(u));
      fprintf(stdout, "    <Country>%s</Country>\n", AB_User_GetCountry(u));
      fprintf(stdout, "    <LastSessionId>%d</LastSessionId>\n", AB_User_GetLastSessionId(u));
      fprintf(stdout, "  </user>\n\n");
    }
    u=AB_User_List_Next(u);
  }
  AB_User_List_free(ul);


  if (xml) {
    fprintf(stdout, "</users>\n");
  }

  return 0;
}




GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  const GWEN_ARGS args[]= {
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
  rv=AB_Cmd_Handle_Args(argc, argv, args, db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    return NULL;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    return NULL;
  }

  return db;
}




