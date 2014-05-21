/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
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
#include <aqbanking/banking.h>
#include <aqbanking/abgui.h>

#include "globals.h"



static void showVersions() {
  int vmajor, vminor, vpatchLevel, vbuild;

  fprintf(stdout, "Versions:\n");
  GWEN_Version(&vmajor,
	       &vminor,
	       &vpatchLevel,
	       &vbuild);
  fprintf(stdout, " Gwenhywfar   : %d.%d.%d.%d\n",
	  vmajor, vminor, vpatchLevel, vbuild);

  AB_Banking_GetVersion(&vmajor,
			&vminor,
			&vpatchLevel,
			&vbuild);
  fprintf(stdout, " AqBanking    : %d.%d.%d.%d\n",
	  vmajor, vminor, vpatchLevel, vbuild);

}



int main(int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *s;
  const char *cmd;
  const char *pinFile;
  int nonInteractive=0;
  int acceptValidCerts=0;
  int rv;
  AB_BANKING *ab;
  GWEN_GUI *gui;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
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
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsType_Int,            /* type */
    "help",                       /* name */
    0,                            /* minnum */
    0,                            /* maxnum */
    "h",                          /* short option */
    "help",                       /* long option */
    "Show this help screen. For help on commands, run aqhbci-tool <COMMAND> --help.",      /* short description */
    "Show this help screen. For help on commands, run aqhbci-tool <COMMAND> --help."       /* long description */
  }
  };

  rv=GWEN_Init();
  if (rv) {
    fprintf(stderr, "ERROR: Unable to init Gwen.\n");
    exit(2);
  }

  GWEN_Logger_Open("aqhbci-tool", "aqhbci-tool", 0,
		   GWEN_LoggerType_Console,
                   GWEN_LoggerFacility_User);
  GWEN_Logger_SetLevel("aqhbci-tool", GWEN_LoggerLevel_Warning);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Warning);

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
                             I18N("  mkpinlist:\n"
                                  "    This command creates an empty PIN "
                                  "file\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  adduser:\n"
                                  "    Adds a user "
                                  "(-> setup HBCI for a bank)\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  deluser:\n"
                                  "    Deletes a user.\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  getkeys:\n"
                                  "    Requests the server's key\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  getcert:\n"
                                  "    Requests the server's SSL certificate\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  createkeys:\n"
                                  "    Create user keys.\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  sendkeys:\n"
                                  "    Send the user keys to the bank.\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  getaccounts:\n"
                                  "    Requests account list for a "
                                  "user\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  getaccsepa:\n"
                                  "    Requests SEPA account list for a "
                                  "user\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  getsysid:\n"
                                  "    Requests a system id for the given "
                                  "user\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  getitanmodes:\n"
                                  "    Requests supported iTAN modes for the given "
                                  "user\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  listusers:\n"
                                  "    List the users\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  listaccounts:\n"
                                  "    List the accounts\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  changepin:\n"
                                  "    Change the PIN of a key file\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  iniletter:\n"
                                  "    Print the INI letter for a given "
                                  "user\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  addaccount:\n"
                                  "    Manually add account \n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  delaccount:\n"
                                  "    Deletes account \n\n"));

    GWEN_Buffer_AppendString(ubuf,
                             I18N("  setMaxTransfers:\n"
                                  "    Set the maximum number of transfers/debit notes per job \n\n"));

    GWEN_Buffer_AppendString(ubuf,
                             I18N("  setsepaprofile:\n"
                                  "    Set the SEPA profile for transfers/debit notes\n\n"));

    GWEN_Buffer_AppendString(ubuf,
                             I18N("  setTanMediumId:\n"
                                  "    Set the medium id for some PIN/TAN methods (like mTAN) \n\n"));

    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    GWEN_DB_Group_free(db);
    return 0;
  }
  if (rv) {
    argc-=rv-1;
    argv+=rv-1;
  }

  /*GWEN_Logger_SetLevel(AQHBCI_LOGDOMAIN, GWEN_LoggerLevelInfo); */
  cmd=GWEN_DB_GetCharValue(db, "params", 0, 0);
  if (!cmd) {
    fprintf(stderr, "ERROR: Command needed.\n");
    GWEN_DB_Group_free(db);
    return 1;
  }

  gui=GWEN_Gui_CGui_new();
  s=GWEN_DB_GetCharValue(db, "charset", 0, NULL);
  if (s && *s)
    GWEN_Gui_SetCharSet(gui, s);
  nonInteractive=GWEN_DB_GetIntValue(db, "nonInteractive", 0, 0);
  acceptValidCerts=GWEN_DB_GetIntValue(db, "acceptValidCerts", 0, 0);
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

  ab=AB_Banking_new("aqhbci-tool", GWEN_DB_GetCharValue(db, "cfgfile", 0, 0),
		    0);
  AB_Gui_Extend(gui, ab);

  if (strcasecmp(cmd, "mkpinlist")==0) {
    rv=mkPinList(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "adduser")==0) {
    rv=addUser(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "deluser")==0) {
    rv=delUser(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "getaccounts")==0) {
    rv=getAccounts(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "addaccount")==0) {
    rv=addAccount(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "delaccount")==0) {
    rv=delAccount(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "getsysid")==0) {
    rv=getSysId(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "getcert")==0) {
    rv=getCert(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "getkeys")==0) {
    rv=getKeys(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "createkeys")==0) {
    rv=createKeys(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "sendkeys")==0) {
    rv=sendKeys(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "listusers")==0) {
    rv=listUsers(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "listaccounts")==0) {
    rv=listAccounts(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "iniletter")==0) {
    rv=iniLetter(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "getitanmodes")==0) {
    rv=getItanModes(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "listitanmodes")==0) {
    rv=listItanModes(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "setitanmode")==0) {
    rv=setItanMode(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "changepin")==0) {
    rv=changePin(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "sethbciversion")==0) {
    rv=setHbciVersion(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "adduserflags")==0) {
    rv=addUserFlags(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "subuserflags")==0) {
    rv=subUserFlags(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "addaccountflags")==0) {
    rv=addsubAccountFlags(ab, db, argc, argv, 1);
  }
  else if (strcasecmp(cmd, "subaccountflags")==0) {
    rv=addsubAccountFlags(ab, db, argc, argv, 0);
  }
  else if (strcasecmp(cmd, "setmaxtransfers")==0) {
    rv=setMaxTransfers(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "setTanMediumId")==0) {
    rv=setTanMediumId(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "logfile")==0) {
    rv=logFile(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "getaccsepa")==0) {
    rv=getAccSepa(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "setsepaprofile")==0) {
    rv=setSepaProfile(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "versions")==0) {
    showVersions();
    rv=0;
  }
  else if (strcasecmp(cmd, "test1")==0) {
    rv=test1(ab, db, argc, argv);
  }

  else {
    fprintf(stderr, "ERROR: Unknown command \"%s\".\n", cmd);
    rv=1;
  }

  GWEN_DB_Group_free(db);
  return rv;
}



