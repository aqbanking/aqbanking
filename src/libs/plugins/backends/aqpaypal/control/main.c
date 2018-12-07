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




int APY_Control(AB_PROVIDER *pro, int argc, char **argv) {
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
    GWEN_Buffer_AppendString(ubuf, I18N("\nCommands:\n\n"));
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

  cmd=argv[0];
  if (!(cmd && *cmd)) {
    fprintf(stderr, "ERROR: Command needed.\n");
    GWEN_DB_Group_free(db);
    return 1;
  }

  if (strcasecmp(cmd, "listusers")==0) {
    rv=APY_Control_ListUsers(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "listaccounts")==0) {
    rv=APY_Control_ListAccounts(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "adduser")==0) {
    rv=APY_Control_AddUser(pro, db, argc, argv);
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



