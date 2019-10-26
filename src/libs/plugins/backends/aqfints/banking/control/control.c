/***************************************************************************
 begin       : Sat Oct 26 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "control.h"

#include "logfile.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>




static void showUsage(const char *prgName)
{
  GWEN_BUFFER *ubuf;

  ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
  GWEN_Buffer_AppendString(ubuf,
                           I18N("Usage: "));
  GWEN_Buffer_AppendString(ubuf, prgName);
  GWEN_Buffer_AppendString(ubuf, I18N(" COMMAND [LOCAL OPTIONS]\n"));
  GWEN_Buffer_AppendString(ubuf,
                           I18N("\nCommands:\n\n"));
  GWEN_Buffer_AppendString(ubuf,
                           I18N("  logfile:\n"
                                "    Analyze log files\n\n"));
  fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
  GWEN_Buffer_free(ubuf);
}



int AF_Control(AB_PROVIDER *pro, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  const char *cmd;
  int rv;

  db=GWEN_DB_Group_new("arguments");
  if (argc<1) {
    showUsage(argv[0]);
    GWEN_DB_Group_free(db);
    return 0;
  } /* if too few args */

  cmd=argv[0];
  if (!(cmd && *cmd)) {
    fprintf(stderr, "ERROR: Command needed.\n");
    GWEN_DB_Group_free(db);
    return 1;
  }

  if (strcasecmp(cmd, "help")==0) {
    showUsage(argv[0]);
    rv=0;
  }
  else if (strcasecmp(cmd, "logfile")==0) {
    rv=AF_Control_LogFile(pro, db, argc, argv);
  }
  else {
    fprintf(stderr, "ERROR: Unknown command \"%s\".\n", cmd);
    rv=1;
  }

  GWEN_DB_Group_free(db);
  return rv;
}



