/***************************************************************************
 begin       : Thu Jun 24 2010
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "globals.h"

#include <gwenhywfar/text.h>




int sendSignKey(AB_PROVIDER *pro,
                GWEN_DB_NODE *dbArgs,
                int argc,
                char **argv)
{
  GWEN_DB_NODE *db;
  uint32_t uid;
  AB_USER *u=NULL;
  int rv;
  const char *signVersion;
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "userId",                     /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "u",                          /* short option */
      "user",                       /* long option */
      "Specify the unique user id",    /* short description */
      "Specify the unique user id"     /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "signVersion",                /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "A",                          /* short option */
      "signversion",                /* long option */
      "Specify the signature version (e.g. A005)",        /* short description */
      "Specify the signature version (e.g. A005)"         /* long description */
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

  signVersion=GWEN_DB_GetCharValue(db, "signVersion", 0, "A005");

  /* doit */
  uid=(uint32_t) GWEN_DB_GetIntValue(db, "userId", 0, 0);
  if (uid==0) {
    fprintf(stderr, "ERROR: Invalid or missing unique user id\n");
    return 1;
  }

  rv=AB_Provider_GetUser(pro, uid, 1, 1, &u);
  if (rv<0) {
    fprintf(stderr, "ERROR: User with id %lu not found\n", (unsigned long int) uid);
    return 2;
  }
  else {
    uint32_t guiid;

    guiid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS |
                                 GWEN_GUI_PROGRESS_SHOW_PROGRESS |
                                 GWEN_GUI_PROGRESS_SHOW_LOG |
                                 GWEN_GUI_PROGRESS_ALWAYS_SHOW_LOG |
                                 GWEN_GUI_PROGRESS_KEEP_OPEN |
                                 GWEN_GUI_PROGRESS_SHOW_ABORT,
                                 I18N("Executing Request"),
                                 I18N("Now the request is send to the credit institute."),
                                 GWEN_GUI_PROGRESS_NONE,
                                 0);
    rv=EBC_Provider_Send_PUB(pro, u, signVersion, 1);
    if (rv) {
      DBG_ERROR(0, "Error sending INI request (%d)", rv);
      GWEN_Gui_ProgressEnd(guiid);
      return 4;
    }
    else {
      fprintf(stderr, "PUB request sent.\n");
    }
    GWEN_Gui_ProgressEnd(guiid);
  }

  fprintf(stderr, "INI/HIA request ok.\n");

  return 0;
}




