/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>

#include <gwenhywfar/logger.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/cgui.h>
#include <gwenhywfar/args.h>
#include <gwenhywfar/i18n.h>

#include <aqbanking/banking.h>
#include <aqbanking/abgui.h>


#include "globals.h"


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)




int main(int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *cmd;
  const char *pinFile;
  int nonInteractive=0;
  const char *s;
  int rv;
  AB_BANKING *ab;
  GWEN_GUI *gui;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "cfgfile",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "C",                          /* short option */
    "cfgfile",                    /* long option */
    "Specify the configuration file",     /* short description */
    "Specify the configuration file"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "pinfile",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "P",                          /* short option */
    "pinfile",                    /* long option */
    "Specify the PIN file",       /* short description */
    "Specify the PIN file"        /* long description */
  },
  {
    0,                            /* flags */
    GWEN_ArgsType_Int,            /* type */
    "nonInteractive",             /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "n",                          /* short option */
    "noninteractive",             /* long option */
    "Select non-interactive mode",/* short description */
    "Select non-interactive mode.\n"        /* long description */
    "This automatically returns a confirmative answer to any non-critical\n"
    "message."
  },
  {
    0,                           
    GWEN_ArgsType_Int,           
    "verbosity",
    0,                           
    10,                          
    "v",                         
    0,                            
    "Increase the verbosity level",
    "Increase the verbosity level"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "charset",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "charset",                    /* long option */
    "Specify the output character set",       /* short description */
    "Specify the output character set"        /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsType_Int,             /* type */
    "help",                       /* name */
    0,                            /* minnum */
    0,                            /* maxnum */
    "h",                          /* short option */
    "help",                       /* long option */
    "Show this help screen",      /* short description */
    "Show this help screen"       /* long description */
  }
  };

  rv=GWEN_Init();
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to init GWEN (%d).\n", rv);
    return 2;
  }

  GWEN_Logger_Open("aqpaypal-tool", "aqpaypal-tool", 0,
		   GWEN_LoggerType_Console,
                   GWEN_LoggerFacility_User);
  GWEN_Logger_SetLevel("aqpaypal-tool", GWEN_LoggerLevel_Warning);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Warning);

  db=GWEN_DB_Group_new("arguments");
  rv=GWEN_Args_Check(argc, argv, 1,
		     GWEN_ARGS_MODE_ALLOW_FREEPARAM |
		     GWEN_ARGS_MODE_STOP_AT_FREEPARAM,
		     args,
		     db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    fprintf(stderr, "ERROR: Could not parse arguments main\n");
    return -1;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    GWEN_Buffer_AppendString(ubuf,
                             I18N("Usage: "));
    GWEN_Buffer_AppendString(ubuf, argv[0]);
    GWEN_Buffer_AppendString(ubuf,
                             I18N(" [GLOBAL OPTIONS] COMMAND "
                                  "[LOCAL OPTIONS]\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("\nGlobal Options:\n"));
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutType_Txt)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return 1;
    }
    GWEN_Buffer_AppendString(ubuf,
                             I18N("\nCommands:\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  listusers:\n"
                                  "    blurb "
                                  "file\n\n"));
   GWEN_Buffer_AppendString(ubuf,
                             I18N("  listaccounts:\n"
                                  "    blurb "
                                  "file\n\n"));
   GWEN_Buffer_AppendString(ubuf,
                             I18N("  adduser:\n"
                                  "    blurb "
                                  "file\n\n"));
   GWEN_Buffer_AppendString(ubuf,
                             I18N("  addaccount:\n"
                                  "    blurb "
                                  "file\n\n"));
   GWEN_Buffer_AppendString(ubuf,
                             I18N("  setsecret:\n"
                                  "    blurb "
                                  "file\n\n"));

    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }
  if (rv) {
    argc-=rv-1;
    argv+=rv-1;
  }

  cmd=GWEN_DB_GetCharValue(db, "params", 0, 0);
  if (!cmd) {
    fprintf(stderr, "ERROR: Command needed.\n");
    return 1;
  }

  gui=GWEN_Gui_CGui_new();
  s=GWEN_DB_GetCharValue(db, "charset", 0, "ISO-8859-15");
  GWEN_Gui_SetCharSet(gui, s);
  nonInteractive=GWEN_DB_GetIntValue(db, "nonInteractive", 0, 0);
  if (nonInteractive)
    GWEN_Gui_AddFlags(gui, GWEN_GUI_FLAGS_NONINTERACTIVE);
  else
    GWEN_Gui_SubFlags(gui, GWEN_GUI_FLAGS_NONINTERACTIVE);
  pinFile=GWEN_DB_GetCharValue(db, "pinFile", 0, NULL);
  if (pinFile) {
    GWEN_DB_NODE *dbPins;

    dbPins=GWEN_DB_Group_new("pins");
    if (GWEN_DB_ReadFile(dbPins, pinFile,
			 GWEN_DB_FLAGS_DEFAULT |
			 GWEN_PATH_FLAGS_CREATE_GROUP)) {
      fprintf(stderr, "Error reading pinfile \"%s\"\n", pinFile);
      return 2;
    }
    /* set argument "persistent" to one in non-interactive mode */
    GWEN_Gui_SetPasswordDb(gui, dbPins, nonInteractive);
  }
  GWEN_Gui_SetGui(gui);

  ab=AB_Banking_new("aqpaypal-tool",
		    GWEN_DB_GetCharValue(db, "cfgfile", 0, 0),
		    0);
  AB_Gui_Extend(gui, ab);

  if (strcasecmp(cmd, "listusers")==0) {
    rv=listUsers(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "listaccounts")==0) {
    rv=listAccounts(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "adduser")==0) {
    rv=addUser(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "addaccount")==0) {
    rv=addAccount(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "setsecrets")==0) {
    rv=setSecrets(ab, db, argc, argv);
  }
  else {
    fprintf(stderr, "ERROR: Unknown command \"%s\".\n", cmd);
    rv=1;
  }

  return rv;
}



