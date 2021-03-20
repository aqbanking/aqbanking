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



static void cmdAddHelpStr(GWEN_BUFFER *ubuf,
                          const char *cmdname,
                          const char *cmdhelp)
{
  // Indentation of the command: one space
  GWEN_Buffer_AppendString(ubuf, " ");
  GWEN_Buffer_AppendString(ubuf, cmdname);
  GWEN_Buffer_AppendString(ubuf, ":\n");
  // Indentation of the help: three spaces
  GWEN_Buffer_AppendString(ubuf, "   ");
  GWEN_Buffer_AppendString(ubuf, cmdhelp);
  GWEN_Buffer_AppendString(ubuf, "\n");
}


int main(int argc, char **argv)
{
  GWEN_DB_NODE *db;
  const char *cmd;
  int rv;
  AB_BANKING *ab;
  GWEN_GUI *gui;
  const char *ctrlBackend=NULL;
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
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "opticalTan",                 /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                         /* short option */
      "opticaltan",                 /* long option */
      "Tool for optical TAN challenges", /* short description */
      "Specify an external tool to display optical TAN challenges" /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT,   /* flags */
      GWEN_ArgsType_Char,             /* type */
      "control",                      /* name */
      0,                              /* minnum */
      1,                              /* maxnum */
      0,                              /* short option */
      "control",                      /* long option */
      "backend for control function", /* short description */
      "Call the CONTROL function of the given backend"          /* long description */
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

  GWEN_Logger_Open(0, "aqbanking-cli", 0,
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
    GWEN_Buffer_AppendString(ubuf,
                             I18N("\nCommands:\n"));
    cmdAddHelpStr(ubuf, "listaccs",
                  I18N("Prints the list of accounts"));

    cmdAddHelpStr(ubuf, "listbal",
                  I18N("Export balances from a context file."));

    cmdAddHelpStr(ubuf, "listtrans",
                  I18N("List transactions or transfers from a context file."));

    cmdAddHelpStr(ubuf, "listtransfers",
                  I18N("Export transactions from a context file which match certain status."));

    cmdAddHelpStr(ubuf, "request",
                  I18N("Requests transactions, balances, standing orders etc."));

    cmdAddHelpStr(ubuf, "chkiban",
                  I18N("Check an IBAN"));

    cmdAddHelpStr(ubuf, "import",
                  I18N("Import a file into an import context file"));

    cmdAddHelpStr(ubuf, "export",
                  I18N("Export an import context file into a file (previously \"listtrans\")"));

    cmdAddHelpStr(ubuf, "sepatransfer",
                  I18N("Issue a single SEPA transfer (data from command line)"));

    cmdAddHelpStr(ubuf, "sepatransfers",
                  I18N("Issue a number of SEPA transfers (data from a file)"));

    cmdAddHelpStr(ubuf, "sepadebitnote",
                  I18N("Issue a single SEPA debit note (data from command line)"));

    cmdAddHelpStr(ubuf, "sepaflashdebitnote",
                  I18N("Issue a single flash SEPA debit note COR1 (data from command line)"));

    cmdAddHelpStr(ubuf, "sepadebitnotes",
                  I18N("Issue a number of SEPA debit notes (data from a file)"));

    cmdAddHelpStr(ubuf, "addtrans",
                  I18N("Add a transfer to an existing import context file"));

    cmdAddHelpStr(ubuf, "addsepadebitnote",
                  I18N("Add a SEPA debit note to an existing import context file"));

    cmdAddHelpStr(ubuf, "addflashsepadebitnote",
                  I18N("Add a flash SEPA debit note to an existing import context file"));

    cmdAddHelpStr(ubuf, "sepasto",
                  I18N("Manage SEPA standing orders"));

    cmdAddHelpStr(ubuf, "fillgaps",
                  I18N("Fill gaps in an import context file from configuration settings"));

    cmdAddHelpStr(ubuf, "listprofiles",
                  I18N("Print existing profiles"));

    cmdAddHelpStr(ubuf, "versions",
                  I18N("Print the program and library versions"));

    GWEN_Buffer_AppendString(ubuf, "\n");

    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    GWEN_DB_Group_free(db);
    return 0;
  }
  if (rv) {
    argc-=rv-1;
    argv+=rv-1;
  }

  nonInteractive=GWEN_DB_GetIntValue(db, "nonInteractive", 0, 0);
  acceptValidCerts=GWEN_DB_GetIntValue(db, "acceptValidCerts", 0, 0);
  cfgDir=GWEN_DB_GetCharValue(db, "cfgdir", 0, 0);
  ctrlBackend=GWEN_DB_GetCharValue(db, "control", 0, 0);

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

  AB_Banking_RuntimeConfig_SetCharValue(ab, "fintsRegistrationKey", "32F8A67FE34B57AB8D7E4FE70");
  AB_Banking_RuntimeConfig_SetCharValue(ab, "fintsApplicationVersionString", AQBANKING_FINTS_VERSION_STRING);

  AB_Gui_Extend(gui, ab);

  s = GWEN_DB_GetCharValue(db, "opticalTan", 0, NULL);
  if ((NULL != s) && ('\0' != s [0]) && (0 != AB_Gui_SetCliCallbackForOpticalTan(gui, s))) {
    fprintf(stderr, "Error registering \"%s\".\n", s);
    GWEN_DB_Group_free(db);
    return 2;
  }

  if (ctrlBackend && *ctrlBackend) {
    rv=control(ab, ctrlBackend, db, argc, argv);
  }
  else {
    cmd=GWEN_DB_GetCharValue(db, "params", 0, 0);
    if (!cmd) {
      fprintf(stderr, "ERROR: Command needed.\n");
      GWEN_DB_Group_free(db);
      return 1;
    }

    if (strcasecmp(cmd, "listaccs")==0 ||
        strcasecmp(cmd, "listaccounts")==0) {
      rv=listAccs(ab, db, argc, argv);
    }
    else if (strcasecmp(cmd, "listbal")==0) {
      rv=listBal(ab, db, argc, argv);
    }
    else if (strcasecmp(cmd, "listtrans")==0) {
      rv=listTrans(ab, db, argc, argv);
    }
    else if (strcasecmp(cmd, "listtransfers")==0) {
      fprintf(stderr,
              "ERROR: Please use the commands \"listtrans\" or \"export\" and specify the transaction type via \"-tt TYPE\"\n");
      GWEN_DB_Group_free(db);
      return 1;
    }
    else if (strcasecmp(cmd, "listdoc")==0) {
      rv=listDoc(ab, db, argc, argv);
    }
    else if (strcasecmp(cmd, "getdoc")==0) {
      rv=getDoc(ab, db, argc, argv);
    }
    else if (strcasecmp(cmd, "request")==0) {
      rv=request(ab, db, argc, argv);
    }
    else if (strcasecmp(cmd, "chkiban")==0) {
      rv=chkIban(ab, db, argc, argv);
    }
    else if (strcasecmp(cmd, "import")==0) {
      rv=import(ab, db, argc, argv);
    }
    else if (strcasecmp(cmd, "export")==0) {
      rv=exportCtx(ab, db, argc, argv);
    }
    else if (strcasecmp(cmd, "sepatransfer")==0) {
      rv=sepaTransfer(ab, db, argc, argv);
    }
    else if (strcasecmp(cmd, "sepatransfers")==0) {
      rv=sepaMultiJobs(ab, db, argc, argv, AQBANKING_TOOL_SEPA_TRANSFERS);
    }
    else if (strcasecmp(cmd, "sepadebitnote")==0) {
      rv=sepaDebitNote(ab, db, argc, argv, 0);
    }
    else if (strcasecmp(cmd, "sepaFlashDebitNote")==0) {
      rv=sepaDebitNote(ab, db, argc, argv, 1);
    }
    else if (strcasecmp(cmd, "sepadebitnotes")==0) {
      rv=sepaMultiJobs(ab, db, argc, argv, AQBANKING_TOOL_SEPA_DEBITNOTES);
    }
    else if (strcasecmp(cmd, "addtrans")==0) {
      rv=addTransaction(ab, db, argc, argv);
    }
    else if (strcasecmp(cmd, "addsepadebitnote")==0) {
      rv=addSepaDebitNote(ab, db, argc, argv, 0);
    }
    else if (strcasecmp(cmd, "addFlashSepadebitnote")==0) {
      rv=addSepaDebitNote(ab, db, argc, argv, 1);
    }
    else if (strcasecmp(cmd, "sepasto")==0) {
      rv=sepaRecurTransfer(ab, db, argc, argv);
    }
    else if (strcasecmp(cmd, "fillgaps")==0) {
      rv=fillGaps(ab, db, argc, argv);
    }
    else if (strcasecmp(cmd, "listprofiles")==0) {
      rv=listProfiles(ab, db, argc, argv);
    }
    else if (strcasecmp(cmd, "versions")==0) {
      rv=versions(ab, db, argc, argv);
    }
    else {
      fprintf(stderr, "ERROR: Unknown command \"%s\".\n", cmd);
      rv=1;
    }
  }

  GWEN_DB_Group_free(db);
  return rv;
}



