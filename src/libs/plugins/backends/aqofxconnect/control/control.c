/***************************************************************************
 begin       : Thu Jan 16 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "control.h"
#include "adduser.h"
#include "listusers.h"
#include "getaccounts.h"

#include "aqbanking/i18n_l.h"



static void _showUsage();





int AO_Control(AB_PROVIDER *pro, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  const char *cmd;
  int rv;

  db=GWEN_DB_Group_new("arguments");
  if (argc<1) {
    _showUsage();
    return 0;
  }

  cmd=argv[0];
  if (!(cmd && *cmd)) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Command needed.\n");
    GWEN_DB_Group_free(db);
    return 1;
  }

  if (strcasecmp(cmd, "help")==0) {
    _showUsage();
    rv=0;
  }
  else if (strcasecmp(cmd, "adduser")==0) {
    rv=AO_Control_AddUser(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "getaccounts")==0) {
    rv=AO_Control_GetAccounts(pro, db, argc, argv);
  }
  else if (strcasecmp(cmd, "listusers")==0) {
    rv=AO_Control_ListUsers(pro, db, argc, argv);
  }
#if 0
  else if (strcasecmp(cmd, "listaccounts")==0) {
    rv=AO_Control_ListAccounts(pro, db, argc, argv);
  }
#endif
  else {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Unknown command \"%s\".\n", cmd);
    rv=1;
  }

  return rv;
}





void _showUsage()
{
  GWEN_BUFFER *ubuf;

  ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
  GWEN_Buffer_AppendString(ubuf,
                           I18N("Commands:\n\n"));
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
                                "    Add a user. "
                                "\n\n"));
  GWEN_Buffer_AppendString(ubuf,
                           I18N("  getaccounts:\n"
                                "    Retrieve list of accounts. "
                                "\n\n"));

  fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
  GWEN_Buffer_free(ubuf);
}



