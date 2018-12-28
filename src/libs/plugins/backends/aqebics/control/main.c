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

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>

#include <gwenhywfar/logger.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/cgui.h>

#include <aqbanking/banking.h>
#include <aqbanking/abgui.h>


#include "globals.h"


int EBC_Control(AB_PROVIDER *pro, int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *cmd;
  int rv;

  db=GWEN_DB_Group_new("arguments");

  if (argc<1){
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    GWEN_Buffer_AppendString(ubuf,
                             I18N("Usage: "));
    GWEN_Buffer_AppendString(ubuf, argv[0]);
    GWEN_Buffer_AppendString(ubuf, I18N(" COMMAND [LOCAL OPTIONS]\n"));

    GWEN_Buffer_AppendString(ubuf,
                             I18N("\nCommands:\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  mkpinlist:\n"
                                  "    This command creates an empty PIN "
                                  "file\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  adduser:\n"
                                  "    Adds a user "
				  "(-> setup EBICS for a bank)\n\n"));
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

    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }

  cmd=argv[0];
  if (!(cmd && *cmd)) {
    fprintf(stderr, "ERROR: Command needed.\n");
    GWEN_DB_Group_free(db);
    return 1;
  }

  if (strcasecmp(cmd, "adduser")==0) {
    rv=addUser(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "addaccount")==0) {
    rv=addAccount(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "createkeys")==0) {
    rv=createKeys(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "createtempkey")==0) {
    rv=createTempKey(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "sendkeys")==0) {
    rv=sendKeys(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "sendsignkey")==0) {
    rv=sendSignKey(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "getkeys")==0) {
    rv=getKeys(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "getaccounts")==0) {
    rv=getAccounts(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "iniletter")==0) {
    rv=iniLetter(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "hialetter")==0) {
    rv=hiaLetter(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "hpd")==0) {
    rv=sendHPD(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "hkd")==0) {
    rv=sendHKD(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "download")==0) {
    rv=download(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "upload")==0) {
    rv=upload(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "mkpinlist")==0) {
    rv=mkPinList(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "resetuser")==0) {
    rv=resetUser(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "adduserflags")==0) {
    rv=addUserFlags(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "subuserflags")==0) {
    rv=subUserFlags(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "getcert")==0) {
    rv=getCert(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "listusers")==0) {
    rv=listUsers(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "listaccounts")==0) {
    rv=listAccounts(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "setEbicsVersion")==0) {
    rv=setEbicsVersion(pro, db, argc, argv);
  }
  else {
    fprintf(stderr, "ERROR: Unknown command \"%s\".\n", cmd);
    rv=1;
  }

  return rv;
}



