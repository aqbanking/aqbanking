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

#include <gwenhywfar/text.h>



int sendKeys(AB_PROVIDER *pro,
             GWEN_DB_NODE *dbArgs,
             int argc,
             char **argv)
{
  GWEN_DB_NODE *db;
  uint32_t uid;
  AB_USER *u=NULL;
  int rv;
  int sendIni=0;
  int sendHia=0;
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
      0, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "hia",                        /* name */
      0,                            /* minnum */
      0,                            /* maxnum */
      0,                            /* short option */
      "hia",                        /* long option */
      "Send HIA request",           /* short description */
      "Send HIA request"            /* long description */
    },
    {
      0, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "ini",                        /* name */
      0,                            /* minnum */
      0,                            /* maxnum */
      0,                            /* short option */
      "ini",                        /* long option */
      "Send INI request",           /* short description */
      "Send INI request"            /* long description */
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

  /* doit */
  uid=(uint32_t) GWEN_DB_GetIntValue(db, "userId", 0, 0);
  if (uid==0) {
    fprintf(stderr, "ERROR: Invalid or missing unique user id\n");
    return 1;
  }

  sendIni=GWEN_DB_GetIntValue(db, "ini", 0, 0);
  sendHia=GWEN_DB_GetIntValue(db, "hia", 0, 0);
  if (sendIni==0 && sendHia==0) {
    sendIni=1;
    sendHia=1;
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
    if (sendIni) {
      if (!(EBC_User_GetFlags(u) & EBC_USER_FLAGS_INI)) {
	rv=EBC_Provider_Send_INI(pro, u, 1);
	if (rv) {
	  DBG_ERROR(0, "Error sending INI request (%d)", rv);
	  GWEN_Gui_ProgressEnd(guiid);
	  return 4;
	}
	else {
	  fprintf(stderr, "INI request sent.\n");
	}
      }
    }

    if (sendHia) {
      if (!(EBC_User_GetFlags(u) & EBC_USER_FLAGS_HIA)) {
	rv=EBC_Provider_Send_HIA(pro, u, 1);
	if (rv) {
	  DBG_ERROR(0, "Error sending HIA request (%d)", rv);
	  GWEN_Gui_ProgressEnd(guiid);
	  return 4;
	}
	else {
	  fprintf(stderr, "HIA request sent.\n");
	}
      }
    }
    GWEN_Gui_ProgressEnd(guiid);
  }

  fprintf(stderr, "INI/HIA request ok.\n");

  return 0;
}




