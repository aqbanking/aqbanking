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



int import(AB_BANKING *ab,
	   GWEN_DB_NODE *dbArgs,
	   int argc,
	   char **argv) {
  GWEN_DB_NODE *db;
  int rv;
  const char *ctxFile;
  const char *inFile;
  const char *importerName;
  const char *profileName;
  const char *profileFile;
  AB_IMEXPORTER *importer;
  GWEN_DB_NODE *dbProfiles;
  GWEN_DB_NODE *dbProfile;
  GWEN_DB_NODE *dbCtx;
  AB_IMEXPORTER_CONTEXT *ctx=0;
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
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "inFile",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "f",                          /* short option */
    "infile",                    /* long option */
    "Specify the file to read the data from",   /* short description */
    "Specify the file to read the data from"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "importerName",               /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "importer",                    /* long option */
    "Specify the importer to use",   /* short description */
    "Specify the importer to use"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "profileName",                /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "profile",                    /* long option */
    "Specify the export profile to use",   /* short description */
    "Specify the export profile to use"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "profileFile",                /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "profile-file",               /* long option */
    "Specify the file to load the export profile from",/* short description */
    "Specify the file to load the export profile from" /* long description */
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

  importerName=GWEN_DB_GetCharValue(db, "importerName", 0, "csv");
  profileName=GWEN_DB_GetCharValue(db, "profileName", 0, "default");
  profileFile=GWEN_DB_GetCharValue(db, "profileFile", 0, 0);

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(AQT_LOGDOMAIN, "Error on init (%d)", rv);
    return 2;
  }

  /* get export module */
  importer=AB_Banking_GetImExporter(ab, importerName);
  if (!importer) {
    DBG_ERROR(AQT_LOGDOMAIN, "Import module \"%s\" not found", importerName);
    return 3;
  }

  /* get profiles */
  if (profileFile) {
    dbProfiles=GWEN_DB_Group_new("profiles");
    if (GWEN_DB_ReadFile(dbProfiles, profileFile,
                         GWEN_DB_FLAGS_DEFAULT |
                         GWEN_PATH_FLAGS_CREATE_GROUP)) {
      DBG_ERROR(AQT_LOGDOMAIN, "Error reading profiles from \"%s\"",
                profileFile);
      return 3;
    }
  }
  else {
    dbProfiles=AB_Banking_GetImExporterProfiles(ab, importerName);
  }

  /* select profile */
  dbProfile=GWEN_DB_GetFirstGroup(dbProfiles);
  while(dbProfile) {
    const char *name;

    name=GWEN_DB_GetCharValue(dbProfile, "name", 0, 0);
    assert(name);
    if (strcasecmp(name, profileName)==0)
      break;
    dbProfile=GWEN_DB_GetNextGroup(dbProfile);
  }
  if (!dbProfile) {
    DBG_ERROR(AQT_LOGDOMAIN,
	      "Profile \"%s\" for importer \"%s\" not found",
              profileName, importerName);
    return 3;
  }

  /* import new context */
  ctx=AB_ImExporterContext_new();

  inFile=GWEN_DB_GetCharValue(db, "inFile", 0, 0);
  if (inFile==0)
    fd=fileno(stdin);
  else
    fd=open(inFile, O_RDONLY);
  if (fd<0) {
    DBG_ERROR(AQT_LOGDOMAIN, "Error selecting input file: %s",
	      strerror(errno));
    AB_ImExporterContext_free(ctx);
    GWEN_DB_Group_free(dbProfiles);
    return 4;
  }
  else {
    GWEN_BUFFEREDIO *bio;
    GWEN_ERRORCODE err;

    bio=GWEN_BufferedIO_File_new(fd);
    if (!inFile)
      GWEN_BufferedIO_SubFlags(bio, GWEN_BUFFEREDIO_FLAGS_CLOSE);
    GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);
    rv=AB_ImExporter_Import(importer,
			    ctx,
			    bio,
			    dbProfile);
    if (rv) {
      DBG_ERROR(AQT_LOGDOMAIN, "Error importing data (%d)", rv);
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      AB_ImExporterContext_free(ctx);
      GWEN_DB_Group_free(dbProfiles);
      return 4;
    }
    err=GWEN_BufferedIO_Close(bio);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(AQT_LOGDOMAIN, err);
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      AB_ImExporterContext_free(ctx);
      GWEN_DB_Group_free(dbProfiles);
      return 4;
    }
    GWEN_BufferedIO_free(bio);
  }
  GWEN_DB_Group_free(dbProfiles);


  /* write context */
  dbCtx=GWEN_DB_Group_new("context");
  if (AB_ImExporterContext_toDb(ctx, dbCtx)) {
    DBG_ERROR(AQT_LOGDOMAIN, "Error writing context to db");
    return 4;
  }
  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);
  if (ctxFile==0)
    fd=fileno(stdout);
  else
    fd=open(ctxFile, O_RDWR|O_CREAT|O_TRUNC,
	    S_IRUSR | S_IWUSR
#ifdef S_IRGRP
	    | S_IRGRP
#endif
#ifdef S_IWGRP
	    | S_IWGRP
#endif
	   );
  if (fd<0) {
    DBG_ERROR(AQT_LOGDOMAIN, "Error selecting output file: %s",
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
      DBG_ERROR(AQT_LOGDOMAIN, "Error writing context");
      GWEN_DB_Group_free(dbCtx);
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      return 4;
    }
    err=GWEN_BufferedIO_Close(bio);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(AQT_LOGDOMAIN, err);
      GWEN_DB_Group_free(dbCtx);
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      return 4;
    }
    GWEN_BufferedIO_free(bio);
  }

  /* that's is */
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}






