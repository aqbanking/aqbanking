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



AB_TRANSACTION *mkTransfer(AB_ACCOUNT *a, GWEN_DB_NODE *db) {
  AB_TRANSACTION *t;
  const char *s;
  int i;

  assert(a);
  assert(db);
  t=AB_Transaction_new();

  AB_Transaction_FillLocalFromAccount(t, a);

  /* remote account */
  s=GWEN_DB_GetCharValue(db, "remoteBankId", 0, 0);
  if (s && *s)
    AB_Transaction_SetRemoteBankCode(t, s);
  else {
    DBG_ERROR(AQT_LOGDOMAIN, "No remote bank id given");
    AB_Transaction_free(t);
    return 0;
  }
  s=GWEN_DB_GetCharValue(db, "remoteAccountId", 0, 0);
  if (s && *s)
    AB_Transaction_SetRemoteAccountNumber(t, s);
  for (i=0; i<10; i++) {
    s=GWEN_DB_GetCharValue(db, "remoteName", i, 0);
    if (!s)
      break;
    if (*s)
      AB_Transaction_AddRemoteName(t, s, 0);
  }
  if (i<1) {
    DBG_ERROR(AQT_LOGDOMAIN, "No remote name given");
    AB_Transaction_free(t);
    return 0;
  }

  /* transfer data */
  for (i=0; i<20; i++) {
    s=GWEN_DB_GetCharValue(db, "purpose", i, 0);
    if (!s)
      break;
    if (*s)
      AB_Transaction_AddPurpose(t, s, 0);
  }
  if (i<1) {
    DBG_ERROR(AQT_LOGDOMAIN, "No purpose given");
    AB_Transaction_free(t);
    return 0;
  }

  i=GWEN_DB_GetIntValue(db, "textkey", 0, -1);
  if (i>0)
    AB_Transaction_SetTextKey(t, i);

  s=GWEN_DB_GetCharValue(db, "value", 0, 0);
  if (s && *s) {
    AB_VALUE *v;

    v=AB_Value_fromString(s);
    assert(v);
    if (AB_Value_IsNegative(v) || AB_Value_IsZero(v)) {
      DBG_ERROR(AQT_LOGDOMAIN, "Only positive non-zero amount allowed");
      AB_Transaction_free(t);
      return 0;
    }
    AB_Transaction_SetValue(t, v);
    AB_Value_free(v);
  }
  else {
    DBG_ERROR(AQT_LOGDOMAIN, "No value given");
    AB_Transaction_free(t);
    return 0;
  }

  return t;
}




int main(int argc, char **argv) {
  GWEN_DB_NODE *db;
  GWEN_LOGGER_LOGTYPE logType;
  GWEN_LOGGER_LEVEL logLevel;
  const char *s;
  const char *cmd;
  const char *pinFile;
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
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
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
    GWEN_ArgsTypeInt,             /* type */
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
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
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
    GWEN_ArgsTypeChar,            /* type */
    "logtype",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "logtype",                    /* long option */
    "Set the logtype",            /* short description */
    "Set the logtype (console, file)."
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "loglevel",                   /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "loglevel",                   /* long option */
    "Set the log level",          /* short description */
    "Set the log level (info, notice, warning, error)."
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "logfile",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "logfile",                   /* long option */
    "Set the log file",          /* short description */
    "Set the log file (if log type is \"file\")."
  },
  {
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsTypeInt,             /* type */
    "help",                       /* name */
    0,                            /* minnum */
    0,                            /* maxnum */
    "h",                          /* short option */
    "help",                       /* long option */
    "Show this help screen",      /* short description */
    "Show this help screen"       /* long description */
  }
  };

#ifdef HAVE_I18N
  setlocale(LC_ALL,"");
  if (bindtextdomain(PACKAGE,  LOCALEDIR)==0)
    fprintf(stderr, "Error binding locale\n");
#endif

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

    GWEN_Buffer_AppendString(ubuf, I18N("Usage: "));
    GWEN_Buffer_AppendString(ubuf, argv[0]);
    GWEN_Buffer_AppendString(ubuf, I18N(" [GLOBAL OPTIONS] "
                                       "COMMAND "
                                       "[LOCAL OPTIONS]\n"));

    GWEN_Buffer_AppendString(ubuf, "\n");
    GWEN_Buffer_AppendString(ubuf, I18N("Global Options\n"));
    GWEN_Buffer_AppendString(ubuf, I18N("==============\n\n"));
    GWEN_Buffer_AppendString(ubuf, I18N("Global options preceed a command.\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("Every command has the local option "
                                  "\"-h\".\n"
                                  "Please use that option on the commands"
                                  "you are interested in.\n"));
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutTypeTXT)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return 1;
    }
    GWEN_Buffer_AppendString(ubuf, "\n\n");
    GWEN_Buffer_AppendString(ubuf, I18N("Commands\n"));
    GWEN_Buffer_AppendString(ubuf, I18N("========\n\n"));
    GWEN_Buffer_AppendString(ubuf, " listaccs\n");
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  Prints a list of accounts\n"));
    GWEN_Buffer_AppendString(ubuf, "\n");
    GWEN_Buffer_AppendString(ubuf, " request\n");
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  Enqueues a request for transactions, "
                                  " balances, standing orders\n"
                                  "  etc.\n"));
    GWEN_Buffer_AppendString(ubuf, "\n");
    GWEN_Buffer_AppendString(ubuf, " transfer\n");
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  Enqueues a transfer request.\n"));
    GWEN_Buffer_AppendString(ubuf, "\n");
    GWEN_Buffer_AppendString(ubuf, " debitnote\n");
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  Enqueues a debit note request.\n"));
    GWEN_Buffer_AppendString(ubuf, "\n");
    GWEN_Buffer_AppendString(ubuf, " exec\n");
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  Executes all jobs in the queue.\n"));
    GWEN_Buffer_AppendString(ubuf, "\n");
    GWEN_Buffer_AppendString(ubuf, " listtrans\n");
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  Exports results from the queue's "
                                  "execution.\n"
                                  "  This command specifically exports "
                                  "transactions.\n"));
    GWEN_Buffer_AppendString(ubuf, "\n");
    GWEN_Buffer_AppendString(ubuf, " listbal\n");
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  Exports results from the queue's "
                                  "execution.\n"
                                  "  This command specifically exports "
                                  "balances.\n"));
    GWEN_Buffer_AppendString(ubuf, "\n");
    GWEN_Buffer_AppendString(ubuf, " chkacc\n");
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  Check a combination of bank id and "
                                  "account number\n"));
    GWEN_Buffer_AppendString(ubuf, "\n\n");
    GWEN_Buffer_AppendString(ubuf, I18N("Example\n"));
    GWEN_Buffer_AppendString(ubuf, I18N("=======\n\n"));
    GWEN_Buffer_AppendString(ubuf, argv[0]);
    GWEN_Buffer_AppendString(ubuf,
                             I18N(" listtrans -h\n"
                                  "This example prints the help screen "
                                  "for the command \"listtrans\"\n"));

    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }
  if (rv) {
    argc-=rv-1;
    argv+=rv-1;
  }

  /* setup logging */
  s=GWEN_DB_GetCharValue(db, "loglevel", 0, "warning");
  logLevel=GWEN_Logger_Name2Level(s);
  if (logLevel==GWEN_LoggerLevelUnknown) {
    fprintf(stderr, "ERROR: Unknown log level (%s)\n", s);
    return 1;
  }
  s=GWEN_DB_GetCharValue(db, "logtype", 0, "console");
  logType=GWEN_Logger_Name2Logtype(s);
  if (logType==GWEN_LoggerTypeUnknown) {
    fprintf(stderr, "ERROR: Unknown log type (%s)\n", s);
    return 1;
  }
  rv=GWEN_Logger_Open(AQBANKING_LOGDOMAIN,
                      "aqbanking-tool",
                      GWEN_DB_GetCharValue(db, "logfile", 0,
                                           "aqbanking-tool.log"),
                      logType,
                      GWEN_LoggerFacilityUser);
  if (rv) {
    fprintf(stderr, "ERROR: Could not setup logging (%d).\n", rv);
    return 2;
  }
  GWEN_Logger_SetLevel(AQT_LOGDOMAIN, logLevel);


  cmd=GWEN_DB_GetCharValue(db, "params", 0, 0);
  if (!cmd) {
    fprintf(stderr, "ERROR: Command needed.\n");
    return 1;
  }

  ab=CBanking_new("aqbanking-tool",
                  GWEN_DB_GetCharValue(db, "cfgfile", 0, 0));
  CBanking_SetCharSet(ab,
                      GWEN_DB_GetCharValue(db,
                                           "charset", 0,
                                           "ISO-8859-15"));
  CBanking_SetIsNonInteractive(ab,
                               GWEN_DB_GetIntValue(db,
                                                   "nonInteractive", 0, 0));

  pinFile=GWEN_DB_GetCharValue(db, "pinFile", 0, 0);
  if (pinFile) {
    GWEN_DB_NODE *dbPins;

    dbPins=GWEN_DB_Group_new("pins");
    if (GWEN_DB_ReadFile(dbPins, pinFile,
			 GWEN_DB_FLAGS_DEFAULT |
			 GWEN_PATH_FLAGS_CREATE_GROUP)) {
      DBG_ERROR(AQT_LOGDOMAIN, "Error reading pinfile \"%s\"", pinFile);
      return 2;
    }
    CBanking_SetPinDb(ab, dbPins);
  }

  if (strcasecmp(cmd, "listaccs")==0) {
    rv=listAccs(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "request")==0) {
    rv=request(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "exec")==0) {
    rv=qexec(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "listtrans")==0) {
    rv=listTrans(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "transfer")==0) {
    rv=transfer(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "debitnote")==0) {
    rv=debitNote(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "chkacc")==0) {
    rv=chkAcc(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "chkiban")==0) {
    rv=chkIban(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "listbal")==0) {
    rv=listBal(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "import")==0) {
    rv=import(ab, db, argc, argv);
  }
  else if (strcasecmp(cmd, "joblog")==0) {
    rv=jobLog(ab, db, argc, argv);
  }
  else {
    fprintf(stderr, "ERROR: Unknown command \"%s\".\n", cmd);
    rv=1;
  }

  AB_Banking_free(ab);
  return rv;
}



