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
#include <gwenhywfar/i18n.h>

#include <aqbanking/banking.h>


#include "globals_l.h"


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)



static void showUsage()
{
  GWEN_BUFFER *ubuf;

  ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
  GWEN_Buffer_AppendString(ubuf,
                           I18N("Commands:\n\n"));
  GWEN_Buffer_AppendString(ubuf,
                           I18N("  listusers:\n"
                                "    show Paypal users "
                                "\n\n"));
  GWEN_Buffer_AppendString(ubuf,
                           I18N("  listaccounts:\n"
                                "    show Paypal accounts "
                                "\n\n"));
  GWEN_Buffer_AppendString(ubuf,
                           I18N("  adduser:\n"
                                "    add a Paypal user and a corresponding account "
                                "\n\n"));
  GWEN_Buffer_AppendString(ubuf,
                           I18N("  setsecrets:\n"
                                "    set credentials for Paypal API "
                                "\n\n"));

  fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
  GWEN_Buffer_free(ubuf);
}



int APY_Control(AB_PROVIDER *pro, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  const char *cmd;
  int rv;

  db=GWEN_DB_Group_new("arguments");
  if (argc<1) {
    showUsage();
    return 0;
  }

  cmd=argv[0];
  if (!(cmd && *cmd)) {
    fprintf(stderr, "ERROR: Command needed.\n");
    GWEN_DB_Group_free(db);
    return 1;
  }

  if (strcasecmp(cmd, "help")==0) {
    showUsage();
    rv=0;
  }
  else if (strcasecmp(cmd, "listusers")==0) {
    rv=APY_Control_ListUsers(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "listaccounts")==0) {
    rv=APY_Control_ListAccounts(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "delaccount")==0) {
    rv=APY_Control_DelAccount(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "adduser")==0) {
    rv=APY_Control_AddUser(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "deluser")==0) {
    rv=APY_Control_DelUser(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "setsecrets")==0) {
    rv=APY_Control_SetSecrets(pro, db, argc, argv);
  }
  else {
    fprintf(stderr, "ERROR: Unknown command \"%s\".\n", cmd);
    rv=1;
  }

  return rv;
}



