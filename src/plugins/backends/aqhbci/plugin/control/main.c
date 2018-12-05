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
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>

#include "globals_l.h"



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



int AH_Control(AB_PROVIDER *pro, int argc, char **argv) {
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
                             I18N("  sethbciversion:\n"
                                  "    Set the HBCI protocol version to be used\n\n"));

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
  } /* if too few args */

  cmd=argv[0];
  if (!(cmd && *cmd)) {
    fprintf(stderr, "ERROR: Command needed.\n");
    GWEN_DB_Group_free(db);
    return 1;
  }

  if (strcasecmp(cmd, "mkpinlist")==0) {
    rv=mkPinList(pro, db, argc, argv);
  }
#if 0
  else if (strcasecmp(cmd, "adduser")==0) {
    rv=addUser(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "deluser")==0) {
    rv=delUser(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "getaccounts")==0) {
    rv=getAccounts(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "addaccount")==0) {
    rv=addAccount(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "delaccount")==0) {
    rv=delAccount(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "getsysid")==0) {
    rv=getSysId(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "getcert")==0) {
    rv=getCert(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "getkeys")==0) {
    rv=getKeys(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "createkeys")==0) {
    rv=createKeys(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "sendkeys")==0) {
    rv=sendKeys(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "listusers")==0) {
    rv=listUsers(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "listaccounts")==0) {
    rv=listAccounts(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "iniletter")==0) {
    rv=iniLetter(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "getitanmodes")==0) {
    rv=getItanModes(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "listitanmodes")==0) {
    rv=listItanModes(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "setitanmode")==0) {
    rv=setItanMode(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "changepin")==0) {
    rv=changePin(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "sethbciversion")==0) {
    rv=setHbciVersion(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "adduserflags")==0) {
    rv=addsubUserFlags(pro, db, argc, argv, 1);
  }
  else if (strcasecmp(cmd, "subuserflags")==0) {
    rv=addsubUserFlags(pro, db, argc, argv, 0);
  }
  else if (strcasecmp(cmd, "addaccountflags")==0) {
    rv=addsubAccountFlags(pro, db, argc, argv, 1);
  }
  else if (strcasecmp(cmd, "subaccountflags")==0) {
    rv=addsubAccountFlags(pro, db, argc, argv, 0);
  }
  else if (strcasecmp(cmd, "setmaxtransfers")==0) {
    rv=setMaxTransfers(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "setTanMediumId")==0) {
    rv=setTanMediumId(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "logfile")==0) {
    rv=logFile(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "getaccsepa")==0) {
    rv=getAccSepa(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "setsepaprofile")==0) {
    rv=setSepaProfile(pro, db, argc, argv);
  }
#endif
  else if (strcasecmp(cmd, "versions")==0) {
    showVersions();
    rv=0;
  }
#if 0
  else if (strcasecmp(cmd, "test1")==0) {
    rv=test1(ab, db, argc, argv);
  }
#endif
  else {
    fprintf(stderr, "ERROR: Unknown command \"%s\".\n", cmd);
    rv=1;
  }

  GWEN_DB_Group_free(db);
  return rv;
}



