/***************************************************************************
 begin       : Fri Nov 14 2025
 copyright   : (C) 2025 by Ralf Habacker
 email       : ralf.habacker@freenet.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <aqbanking/i18n_l.h>
#include <aqbanking/version.h>
#include <cli/helper.h>

int AB_Cmd_Handle_Args(int argc,
                       char **argv,
                       const GWEN_ARGS *args,
                       GWEN_DB_NODE *db)
{
  int rv;
  rv=GWEN_Args_Check(argc, argv, 1,
                       0,
                       args,
                       db);

  if (rv==GWEN_ARGS_RESULT_ERROR) {
    fprintf(stderr, "ERROR: Could not parse arguments\n");
    return rv;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf = GWEN_Buffer_new(0, 1024, 0, 1);

    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutType_Txt)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      GWEN_Buffer_free(ubuf);
      return GWEN_ARGS_RESULT_ERROR;
    }

    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
  }
  return rv;
}

int AB_App_Handle_Args(int argc,
                      char **argv,
                      const GWEN_ARGS *args,
                      GWEN_DB_NODE *db)
{
  int rv;
  rv=GWEN_Args_Check(argc, argv, 1,
                     GWEN_ARGS_MODE_ALLOW_FREEPARAM |
                     GWEN_ARGS_MODE_STOP_AT_FREEPARAM,
                     args,
                     db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    fprintf(stderr, "ERROR: Could not parse arguments main\n");
    GWEN_DB_Group_free(db);
      return rv;
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
      return GWEN_ARGS_RESULT_ERROR;
    }

    GWEN_Buffer_AppendString(ubuf, "\n");

    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
  }
  return rv;
}
