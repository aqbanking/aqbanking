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

#include <gwenhywfar/logger.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/cgui.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/directory.h>
#include <aqbanking/banking.h>
#include <aqbanking/gui/abgui.h>

#include "globals.h"



static int doControl(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv);



int main(int argc, char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  AB_BANKING *ab;
  GWEN_GUI *gui;
  int nonInteractive=0;
  int acceptValidCerts=0;
  const char *pinFile;
  const char *cfgDir;
  const char *s;
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "cfgdir",                     /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "D",                          /* short option */
      "cfgdir",                     /* long option */
      I18S("Specify the configuration folder"),
      I18S("Specify the configuration folder")
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
      0,                            /* flags */
      GWEN_ArgsType_Int,            /* type */
      "acceptValidCerts",           /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "A",                          /* short option */
      "acceptvalidcerts",           /* long option */
      "Automatically accept all valid TLS certificate",
      "Automatically accept all valid TLS certificate"
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
      GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "help",                       /* name */
      0,                            /* minnum */
      0,                            /* maxnum */
      "h",                          /* short option */
      "help",
      I18S("Show this help screen. For help on commands, "
           "run aqbanking-cli <COMMAND> --help."),
      I18S("Show this help screen. For help on commands, run aqbanking-cli <COMMAND> --help.")
    }
  };

  rv=GWEN_Init();
  if (rv) {
    fprintf(stderr, "ERROR: Unable to init Gwen.\n");
    exit(2);
  }

  GWEN_Logger_Open(0, "aqofxconnect-tool", 0,
                   GWEN_LoggerType_Console,
                   GWEN_LoggerFacility_User);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Warning);

  rv=GWEN_I18N_BindTextDomain_Dir(PACKAGE, LOCALEDIR);
  if (rv) {
    DBG_ERROR(0, "Could not bind textdomain (%d)", rv);
  }
  else {
    rv=GWEN_I18N_BindTextDomain_Codeset(PACKAGE, "UTF-8");
    if (rv) {
      DBG_ERROR(0, "Could not set codeset (%d)", rv);
    }
  }

  db=GWEN_DB_Group_new("arguments");
  rv=GWEN_Args_Check(argc, argv, 1,
                     GWEN_ARGS_MODE_ALLOW_FREEPARAM |
                     GWEN_ARGS_MODE_STOP_AT_FREEPARAM,
                     args,
                     db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    fprintf(stderr, "ERROR: Could not parse arguments main\n");
    GWEN_DB_Group_free(db);
    return 1;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    GWEN_Buffer_AppendString(ubuf, I18N("This is version "));
    GWEN_Buffer_AppendString(ubuf, AQBANKING_VERSION_STRING "\n");
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
      GWEN_DB_Group_free(db);
      return 1;
    }

    GWEN_Buffer_AppendString(ubuf, "\n");

    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    argc=0; /* only show help */
  }
  if (rv>1) {
    argc-=rv-1;
    argv+=rv-1;
  }
  else {
    /* no command */
    argc=0;
  }

  nonInteractive=GWEN_DB_GetIntValue(db, "nonInteractive", 0, 0);
  acceptValidCerts=GWEN_DB_GetIntValue(db, "acceptValidCerts", 0, 0);
  cfgDir=GWEN_DB_GetCharValue(db, "cfgdir", 0, 0);

  gui=GWEN_Gui_CGui_new();
  s=GWEN_DB_GetCharValue(db, "charset", 0, NULL);
  if (s && *s)
    GWEN_Gui_SetCharSet(gui, s);

  if (nonInteractive)
    GWEN_Gui_AddFlags(gui, GWEN_GUI_FLAGS_NONINTERACTIVE);
  else
    GWEN_Gui_SubFlags(gui, GWEN_GUI_FLAGS_NONINTERACTIVE);

  if (acceptValidCerts)
    GWEN_Gui_AddFlags(gui, GWEN_GUI_FLAGS_ACCEPTVALIDCERTS);
  else
    GWEN_Gui_SubFlags(gui, GWEN_GUI_FLAGS_ACCEPTVALIDCERTS);

  pinFile=GWEN_DB_GetCharValue(db, "pinFile", 0, NULL);
  if (pinFile) {
    GWEN_DB_NODE *dbPins;

    dbPins=GWEN_DB_Group_new("pins");
    if (GWEN_DB_ReadFile(dbPins, pinFile,
                         GWEN_DB_FLAGS_DEFAULT |
                         GWEN_PATH_FLAGS_CREATE_GROUP)) {
      fprintf(stderr, "Error reading pinfile \"%s\"\n", pinFile);
      GWEN_DB_Group_free(dbPins);
      GWEN_DB_Group_free(db);
      return 2;
    }
    GWEN_Gui_SetPasswordDb(gui, dbPins, 1);
  }

  GWEN_Gui_SetGui(gui);

  ab=AB_Banking_new("aqbanking-cli", cfgDir, 0);
  AB_Gui_Extend(gui, ab);

  rv=doControl(ab, db, argc, argv);

  GWEN_DB_Group_free(db);
  return rv;
}



int doControl(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  int rv;

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }


  rv=AB_Banking_ProviderControl(ab, "aqofxconnect", argc, argv);
  if (rv!=0) {
    DBG_ERROR(0, "Error calling control function (%d)", rv);
    AB_Banking_Fini(ab);
    return 4;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}



