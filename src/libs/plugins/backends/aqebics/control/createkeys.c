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


#include "globals.h"
#include "aqebics/client/user.h"
#include "aqebics/client/provider.h"

#include <gwenhywfar/text.h>



int createKeys(AB_PROVIDER *pro,
               GWEN_DB_NODE *dbArgs,
               int argc,
               char **argv)
{
  GWEN_DB_NODE *db;
  uint32_t uid;
  AB_USER *u=NULL;
  int rv;
  int cryptAndAuthKeySize;
  int signKeySize;
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
      GWEN_ARGS_FLAGS_HAS_ARGUMENT,
      GWEN_ArgsType_Int,
      "cryptAndAuthKeySize",
      0,
      1,
      "s",
      "keysize",
      "Specify the keysize in bytes",
      "Specify the keysize in bytes"
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT,
      GWEN_ArgsType_Int,
      "signKeySize",
      0,
      1,
      "S",
      "signkeysize",
      "Specify the keysize in bytes",
      "Specify the keysize in bytes"
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

  uid=(uint32_t) GWEN_DB_GetIntValue(db, "userId", 0, 0);
  if (uid==0) {
    fprintf(stderr, "ERROR: Invalid or missing unique user id\n");
    return 1;
  }

  cryptAndAuthKeySize=GWEN_DB_GetIntValue(db, "cryptAndAuthKeySize", 0, 256);
  signKeySize=GWEN_DB_GetIntValue(db, "signKeySize", 0, 256);

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
                                 I18N("Creating keys"),
                                 I18N("Now the keys are created."),
                                 GWEN_GUI_PROGRESS_NONE,
                                 0);

    rv=EBC_Provider_CreateKeys(pro, u, cryptAndAuthKeySize, signKeySize, 0);
    GWEN_Gui_ProgressEnd(guiid);
    if (rv) {
      DBG_ERROR(0, "Error creating keys (%d)", rv);
      return 3;
    }
  }

  return 0;
}





