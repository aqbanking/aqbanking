/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"



static int updateConf(AB_BANKING *ab,
		      GWEN_DB_NODE *dbArgs,
		      int argc,
		      char **argv) {
  GWEN_DB_NODE *db;
  int rv;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsType_Int,             /* type */
    "help",                       /* name */
    0,                            /* minnum */
    0,                            /* maxnum */
    "h",                          /* short option */
    "help",                       /* long option */
    "Show this help screen",      /* short description */
    "Show this help screen"       /* long description */
  }
  };

  db=GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_DEFAULT, "local");
  rv=GWEN_Args_Check(argc, argv, 1,
                     0 /*GWEN_ARGS_MODE_ALLOW_FREEPARAM*/,
                     args,
                     db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    fprintf(stderr, "ERROR: Could not parse arguments\n");
    return 1;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutType_Txt)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return 1;
    }
    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }

  rv=AB_Banking_HasConf4(ab, 0);
  if (!rv) {
    fprintf(stderr, "Config for AqBanking 4 found, no update needed.\n");
    return 0;
  }
  fprintf(stderr, "Config for AqBanking 4 not found, update needed (%d)\n", rv);

  rv=AB_Banking_HasConf3(ab, 0);
  if (!rv) {
    fprintf(stderr, "Config for AqBanking 3 found, importing\n");
    rv=AB_Banking_ImportConf3(ab, 0);
    if (rv<0) {
      fprintf(stderr, "Error importing configuration (%d)\n", rv);
      return 2;
    }
    return 0;
  }
  fprintf(stderr, "Config for AqBanking 3 not found (%d)\n", rv);

  rv=AB_Banking_HasConf2(ab, 0);
  if (!rv) {
    fprintf(stderr, "Config for AqBanking 2 found, importing\n");
    rv=AB_Banking_ImportConf2(ab, 0);
    if (rv<0) {
      fprintf(stderr, "Error importing configuration (%d)\n", rv);
      return 2;
    }
    return 0;
  }
  fprintf(stderr,
	  "Config for AqBanking 2 not found (%d), "
	  "no usable configuration found to update.\n",
	  rv);

  return 2;
}




