/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: listtrans.c 764 2006-01-13 14:00:00Z cstim $
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef WITH_AQFINANCE

#include "globals.h"
#include <gwenhywfar/text.h>
#include <gwenhywfar/io_file.h>
#include <gwenhywfar/iomanager.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>



static
int dblistTrans(AB_BANKING *ab,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv) {
  GWEN_DB_NODE *db;
  int rv;
  const char *outFile;
  const char *exporterName;
  const char *profileName;
  const char *profileFile;
  const char *queryString;
  AB_IMEXPORTER *exporter;
  GWEN_DB_NODE *dbProfiles;
  GWEN_DB_NODE *dbProfile;
  AB_IMEXPORTER_CONTEXT *ctx=0;
  int fd;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "queryString",               /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "q",                            /* short option */
    "query",                    /* long option */
    "Specify the transactions to export",   /* short description */
    "Specify the transactions to export"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "outFile",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "o",                          /* short option */
    "outfile",                    /* long option */
    "Specify the file to store the data in",   /* short description */
    "Specify the file to store the data in"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "exporterName",               /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "exporter",                    /* long option */
    "Specify the exporter to use",   /* short description */
    "Specify the exporter to use"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
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
    GWEN_ArgsType_Char,            /* type */
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

  exporterName=GWEN_DB_GetCharValue(db, "exporterName", 0, "csv");
  profileName=GWEN_DB_GetCharValue(db, "profileName", 0, "default");
  profileFile=GWEN_DB_GetCharValue(db, "profileFile", 0, 0);
  queryString=GWEN_DB_GetCharValue(db, "queryString", 0, 0);

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  /* get export module */
  exporter=AB_Banking_GetImExporter(ab, exporterName);
  if (!exporter) {
    DBG_ERROR(0, "Export module \"%s\" not found", exporterName);
    return 3;
  }

  /* get profiles */
  if (profileFile) {
    dbProfiles=GWEN_DB_Group_new("profiles");
    if (GWEN_DB_ReadFile(dbProfiles, profileFile,
                         GWEN_DB_FLAGS_DEFAULT |
			 GWEN_PATH_FLAGS_CREATE_GROUP,
			 0,
			 2000)) {
      DBG_ERROR(0, "Error reading profiles from \"%s\"",
                profileFile);
      return 3;
    }
  }
  else {
    dbProfiles=AB_Banking_GetImExporterProfiles(ab, exporterName);
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
    DBG_ERROR(0, "Profile \"%s\" for exporter \"%s\" not found",
              profileName, exporterName);
    return 3;
  }

  ctx=AB_ImExporterContext_new();
  rv=AFM_import_export_statements(ab, ctx, AE_Book_TableType_BankStatement, queryString);
  if (rv<0) {
    DBG_ERROR(0, "Error exporting statements (%d)", rv);
    return 3;
  }

  /* export new context */
  outFile=GWEN_DB_GetCharValue(db, "outFile", 0, 0);
  if (outFile==0)
    fd=fileno(stdout);
  else
    fd=open(outFile, O_RDWR | O_CREAT | O_TRUNC,
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
    AB_ImExporterContext_free(ctx);
    GWEN_DB_Group_free(dbProfiles);
    return 4;
  }
  else {
    GWEN_IO_LAYER *io;

    io=GWEN_Io_LayerFile_new(-1, fd);
    GWEN_Io_Layer_AddFlags(io, GWEN_IO_LAYER_FLAGS_DONTCLOSE);
    rv=GWEN_Io_Manager_RegisterLayer(io);
    if (rv<0) {
      DBG_ERROR(0, "Error registering io layer (%d)", rv);
      GWEN_Io_Layer_free(io);
      AB_ImExporterContext_free(ctx);
      GWEN_DB_Group_free(dbProfiles);
      return 4;
    }

    rv=AB_ImExporter_Export(exporter,
			    ctx,
                            io,
			    dbProfile,
			    0);
    if (rv<0) {
      DBG_ERROR(0, "Error exporting data (%d)", rv);
      GWEN_Io_Layer_free(io);
      AB_ImExporterContext_free(ctx);
      GWEN_DB_Group_free(dbProfiles);
      return 4;
    }

    rv=GWEN_Io_Layer_DisconnectRecursively(io, NULL, 0, 0, 2000);
    if (rv<0) {
      DBG_ERROR(0, "Error flushing (%d)", rv);
      GWEN_Io_Layer_free(io);
      AB_ImExporterContext_free(ctx);
      GWEN_DB_Group_free(dbProfiles);
      return 4;
    }
    GWEN_Io_Layer_free(io);
    if (outFile) {
      if (close(fd)) {
	DBG_ERROR(0, "close(): %s", strerror(errno));
	AB_ImExporterContext_free(ctx);
	GWEN_DB_Group_free(dbProfiles);
	return 4;
      }
    }
  }
  AB_ImExporterContext_free(ctx);
  GWEN_DB_Group_free(dbProfiles);

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}




#endif

