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

#include <gwenhywfar/logger.h>
#include <gwenhywfar/debug.h>
#include <aqbanking/banking.h>


#include <cbanking/cbanking.h>
#include "globals.h"




int main(int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *cmd;
  int rv;
  AB_BANKING *ab;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "cfgfile",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "C",                          /* short option */
    "cfgfile",                    /* long option */
    "Specify the configuration file",     /* short description */
    "Specify the configuration file"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsTypeInt,             /* type */
    "help",                       /* name */
    0,                            /* minnum */
    0,                            /* maxnum */
    "h",                          /* short option */
    "help",                       /* long option */
    "Show this help screen. For help on commands, run aqhbci-tool <COMMAND> --help.",      /* short description */
    "Show this help screen. For help on commands, run aqhbci-tool <COMMAND> --help."       /* long description */
  }
  };

#ifdef HAVE_I18N
  setlocale(LC_ALL,"");
  if (bindtextdomain(PACKAGE,  LOCALEDIR)==0)
    fprintf(stderr, "Error binding locale\n");
#endif

  GWEN_Logger_Open("aqhbci-tool", "aqhbci-tool", 0,
                   GWEN_LoggerTypeConsole,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel("aqhbci-tool", GWEN_LoggerLevelWarning);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelWarning);

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
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutTypeTXT)) {
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
                             I18N("  addmedium:\n"
                                  "    Makes a crypttoken (medium) known to "
                                  "AqHBCI\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  listmedia:\n"
                                  "    Shows the list of currently known "
                                  "crypttoken (media)\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  adduser:\n"
                                  "    Adds a user "
                                  "(-> setup HBCI for a bank)\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  getkeys:\n"
                                  "    Requests the server's key\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  createkeys:\n"
                                  "    Create user keys.\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  resetkeys:\n"
                                  "    Destroy keys (use with care!!)\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  sendkeys:\n"
                                  "    Send the user keys to the bank.\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  getaccounts:\n"
                                  "    Requests account list for a "
                                  "user\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  getsysid:\n"
                                  "    Requests a system id for the given "
                                  "user\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  iniletter:\n"
                                  "    Print the INI letter for a given "
                                  "user\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  activate:\n"
                                  "    Activates the AqHBCI backend\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  deactivate:\n"
                                  "    Deactivates the AqHBCI backend\n\n"));

    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
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
    return 1;
  }

  ab=CBanking_new("aqhbci-tool", GWEN_DB_GetCharValue(db, "cfgfile", 0, 0));
  CBanking_SetCharSet(ab, "ISO-8859-15");

  if (strcasecmp(cmd, "mkpinlist")==0) {
    rv=mkPinList(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "adduser")==0) {
    rv=addUser(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "addmedium")==0) {
    rv=addMedium(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "listmedia")==0) {
    rv=listMedia(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "getaccounts")==0) {
    rv=getAccounts(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "getsysid")==0) {
    rv=getSysId(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "getkeys")==0) {
    rv=getKeys(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "createkeys")==0) {
    rv=createKeys(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "resetkeys")==0) {
    rv=resetKeys(ab, db, argc, argv);
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
  else if (strcasecmp(cmd, "changepin")==0) {
    rv=changePin(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "test1")==0) {
    rv=test1(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "test2")==0) {
    rv=test2(ab, db, argc, argv);
  }

  else if (strcasecmp(cmd, "activate")==0) {
    int res;

    rv=0;
    res=AB_Banking_Init(ab);
    if (res) {
      DBG_ERROR(0, "Error on init (%d)", res);
      rv=2;
    }
    else {
      res=AB_Banking_ActivateProvider(ab, "aqhbci");
      if (res) {
        DBG_ERROR(0, "Error activating HBCI backend (%d)", res);
        rv=3;
      }
      else {
        res=AB_Banking_Fini(ab);
        if (res) {
          DBG_ERROR(0, "Error on fini (%d)", res);
          rv=5;
        }
        else {
          fprintf(stderr, "Backend AqHBCI activated.\n");
        }
      }
    }
  }
  else if (strcasecmp(cmd, "deactivate")==0) {
    int res;

    rv=0;
    res=AB_Banking_Init(ab);
    if (res) {
      DBG_ERROR(0, "Error on init (%d)", res);
      rv=2;
    }
    else {
      res=AB_Banking_DeactivateProvider(ab, "aqhbci");
      if (res) {
        DBG_ERROR(0, "Error deactivating HBCI backend (%d)", res);
        rv=3;
      }
      else {
        res=AB_Banking_Fini(ab);
        if (res) {
          DBG_ERROR(0, "Error on fini (%d)", res);
          rv=5;
        }
        else {
          fprintf(stderr, "Backend AqHBCI deactivated.\n");
        }
      }
    }
  }
  else {
    fprintf(stderr, "ERROR: Unknown command \"%s\".\n", cmd);
    rv=1;
  }

  return rv;
}



