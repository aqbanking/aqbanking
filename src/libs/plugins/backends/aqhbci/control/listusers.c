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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int AH_Control_ListUsers(AB_PROVIDER *pro,
                         GWEN_DB_NODE *dbArgs,
                         int argc,
                         char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  int xml=0;
  AB_USER_LIST *ul;
  AB_USER *u;
  int i=0;

  const GWEN_ARGS args[]= {
    {
      0,                            /* flags */
      GWEN_ArgsType_Int,             /* type */
      "xml",                 /* name */
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

  xml=GWEN_DB_VariableExists(db, "xml");

  if (xml) {
    fprintf(stdout, "<?xml version=\"1.0\"?>\n");
    fprintf(stdout, "<users>\n");
  }

  ul=AB_User_List_new();
  rv=AB_Provider_ReadUsers(pro, ul);
  if (rv<0) {
    DBG_ERROR_ERR(0, rv);
    AB_User_List_free(ul);
    return 3;
  }

  u=AB_User_List_First(ul);
  while (u) {
    if (!xml) {
      fprintf(stdout, "User %d: Bank: %s/%s User Id: %s Customer Id: %s Unique Id: %lu\n",
              i++,
              AB_User_GetCountry(u),
              AB_User_GetBankCode(u),
              AB_User_GetUserId(u),
              AB_User_GetCustomerId(u),
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
      fprintf(stdout, "    <CustomerId>%s</CustomerId>\n", AB_User_GetCustomerId(u));
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




