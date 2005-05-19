/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
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
#include <gwenhywfar/text.h>
#include <gwenhywfar/bufferedio.h>
#include <gwenhywfar/bio_file.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>



int qexec(AB_BANKING *ab,
          GWEN_DB_NODE *dbArgs,
          int argc,
          char **argv) {
  GWEN_DB_NODE *db;
  int rv;
  const char *ctxFile;
  AB_IMEXPORTER_CONTEXT *ctx;
  GWEN_DB_NODE *dbCtx;
  int fd;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "ctxFile",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "c",                          /* short option */
    "ctxfile",                    /* long option */
    "Specify the file to store the context in",   /* short description */
    "Specify the file to store the context in"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsTypeInt,             /* type */
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
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutTypeTXT)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return 1;
    }
    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }

  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  rv=AB_Banking_ExecuteQueue(ab);
  if (rv) {
    DBG_ERROR(0, "Error executing queue: %d", rv);
    return 3;
  }

  ctx=AB_ImExporterContext_new();
  rv=AB_Banking_GatherResponses(ab, ctx);
  if (rv) {
    DBG_ERROR(0, "Error gathering responses: %d", rv);
    AB_ImExporterContext_free(ctx);
    return 4;
  }

  dbCtx=GWEN_DB_Group_new("context");
  rv=AB_ImExporterContext_toDb(ctx, dbCtx);
  if (rv) {
    DBG_ERROR(0, "Error storing context: %d", rv);
    AB_ImExporterContext_free(ctx);
    return 4;
  }
  AB_ImExporterContext_free(ctx);
  if (ctxFile==0)
    fd=fileno(stdout);
  else
    fd=open(ctxFile, O_RDWR | O_CREAT | O_TRUNC,
            S_IRUSR | S_IWUSR
#ifdef S_IRGRP
            | S_IRGRP
#endif
#ifdef S_IWGRP
            | S_IWGRP
#endif
           );
  if (fd<0) {
    DBG_ERROR(0, "Error selecting output file: %s",
              strerror(errno));
    return 4;
  }
  else {
    GWEN_BUFFEREDIO *bio;
    GWEN_ERRORCODE err;

    bio=GWEN_BufferedIO_File_new(fd);
    if (!ctxFile)
      GWEN_BufferedIO_SubFlags(bio, GWEN_BUFFEREDIO_FLAGS_CLOSE);
    GWEN_BufferedIO_SetWriteBuffer(bio, 0, 1024);
    if (GWEN_DB_WriteToStream(dbCtx, bio, GWEN_DB_FLAGS_DEFAULT)) {
      DBG_ERROR(0, "Error writing context");
      GWEN_DB_Group_free(dbCtx);
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      return 4;
    }
    err=GWEN_BufferedIO_Close(bio);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      GWEN_DB_Group_free(dbCtx);
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      return 4;
    }
    GWEN_BufferedIO_free(bio);
  }
  GWEN_DB_Group_free(dbCtx);

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}






