/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2005-2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"
#include <gwenhywfar/text.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>



static
int listProfiles(AB_BANKING *ab,
                 GWEN_DB_NODE *dbArgs,
                 int argc,
                 char **argv) {
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbProfiles;
  int rv;
  const char *importerName;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "importerName",               /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "importer",                    /* long option */
    "Specify the importer to use",   /* short description */
    "Specify the importer to use"      /* long description */
  },
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

  importerName=GWEN_DB_GetCharValue(db, "importerName", 0, "csv");

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  rv=AB_Banking_OnlineInit(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }


  dbProfiles=AB_Banking_GetImExporterProfiles(ab, importerName);
  if (dbProfiles) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetFirstGroup(dbProfiles);
    while(dbT) {
      const char *sName;
      const char *sVersion;
      const char *sShortDescr;
      int isGlobal;

      sName=GWEN_DB_GetCharValue(dbT, "name", 0, "(none)");
      sVersion=GWEN_DB_GetCharValue(dbT, "version", 0, "(none)");
      sShortDescr=GWEN_DB_GetCharValue(dbT, "shortDescr", 0, "");
      isGlobal=GWEN_DB_GetIntValue(dbT, "isGlobal", 0, 0);

      fprintf(stdout, "%s\t%s\t%s\t%s\n",
	      sName?sName:"(none)",
	      sVersion?sVersion:"(none)",
	      sShortDescr?sShortDescr:"",
	      isGlobal?"global":"local");

      dbT=GWEN_DB_GetNextGroup(dbT);
    }

    GWEN_DB_Group_free(dbProfiles);
  }
  else {
    fprintf(stderr, "No profiles for this im-/exporter");
  }

  /* that's is */
  rv=AB_Banking_OnlineFini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    AB_Banking_Fini(ab);
    return 5;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}






