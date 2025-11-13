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

#include "globals.h"
#include <aqbanking/banking_be.h>

int listConfDir(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
      GWEN_ArgsType_Int,             /* type */
      "help",                        /* name */
      0,                             /* minnum */
      0,                             /* maxnum */
      "h",                           /* short option */
      "help",                        /* long option */
      "Show this help screen",       /* short description */
      "Show this help screen"        /* long description */
    }
  };

  GWEN_BUFFER *buf;
  int rv;

  rv=AB_Cmd_Handle_Args(argc, argv, args, dbArgs);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    return 1;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    return 0;
  }

  buf=GWEN_Buffer_new(0, 1024, 0, 1);
  rv=AB_Banking_GetUserDataDir(ab, buf);
  if (rv<0) {
    fprintf(stderr,
            "Error: Could not determine AqBanking user data dir (rc=%d)\n",
            rv);
    GWEN_Buffer_free(buf);
    return 2;
  }

  printf("%s\n", GWEN_Buffer_GetStart(buf));
  GWEN_Buffer_free(buf);
  return 0;
}
