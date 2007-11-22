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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int addMedium(AB_BANKING *ab,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv) {
  GWEN_DB_NODE *db;
  AB_PROVIDER *pro;
  int rv;
  AH_MEDIUM *medium=0;
  const char *mediumName;
  const char *mediumType;
  GWEN_CRYPTTOKEN_DEVICE dev;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "mediumType",                 /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "t",                            /* short option */
    "mediumtype",                 /* long option */
    "Specify the medium type (file, card or pintan)",      /* short description */
    "Specify the medium type (file, card or pintan)"       /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "mediumName",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "m",                          /* short option */
    "mediumname",                 /* long option */
    "Specify the medium name",    /* short description */
    "Specify the medium name"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsType_Int,            /* type */
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

  pro=AB_Banking_GetProvider(ab, "aqhbci");
  assert(pro);

  mediumName=GWEN_DB_GetCharValue(db, "mediumName", 0, 0);
  if (mediumName)
    mediumType=GWEN_DB_GetCharValue(db, "mediumType", 0, "file");
  else
    mediumType=GWEN_DB_GetCharValue(db, "mediumType", 0, "card");
  if (strcasecmp(mediumType, "pintan")==0)
    dev=GWEN_CryptToken_Device_None;
  else {
    dev=GWEN_CryptToken_Device_fromString(mediumType);
    if (dev==GWEN_CryptToken_Device_Unknown) {
      DBG_ERROR(0, "Unknown device type name \"%s\"", mediumType);
      return 1;
    }
  }

  if (strcasecmp(mediumType, "pintan")!=0) {
    GWEN_BUFFER *bufTypeName;
    GWEN_BUFFER *bufSubTypeName;
    GWEN_BUFFER *bufName;

    bufTypeName=GWEN_Buffer_new(0, 128, 0, 1);
    bufSubTypeName=GWEN_Buffer_new(0, 128, 0, 1);
    bufName=GWEN_Buffer_new(0, 128, 0, 1);
    if (mediumName)
      GWEN_Buffer_AppendString(bufName, mediumName);

    rv=AH_Provider_CheckMedium(pro, dev,
                               bufTypeName, bufSubTypeName, bufName);
    if (rv) {
      DBG_ERROR(0, "Medium not supported");
      AB_Banking_Fini(ab);
      return 3;
    }

    if (AH_Provider_FindMedium(pro,
                               GWEN_Buffer_GetStart(bufTypeName),
                               GWEN_Buffer_GetStart(bufName))) {
      DBG_ERROR(0, "Medium is already listed");
      AB_Banking_Fini(ab);
      return 3;
    }

    medium=AH_Provider_MediumFactory(pro,
                                     GWEN_Buffer_GetStart(bufTypeName),
                                     GWEN_Buffer_GetStart(bufSubTypeName),
                                     GWEN_Buffer_GetStart(bufName));
  }
  else {
    GWEN_BUFFER *bufName;

    bufName=GWEN_Buffer_new(0, 128, 0, 1);
    if (mediumName)
      GWEN_Buffer_AppendString(bufName, mediumName);
    else {
      GWEN_TIME *ti;

      GWEN_Buffer_AppendString(bufName, "PINTAN-");
      ti=GWEN_CurrentTime();
      assert(ti);
      GWEN_Time_toString(ti, "YYYYMMDD-hhmmss", bufName);
      GWEN_Time_free(ti);
    }
    medium=AH_Provider_MediumFactory(pro,
                                     "pintan", 0,
                                     GWEN_Buffer_GetStart(bufName));
    GWEN_Buffer_free(bufName);
  }

  if (!medium) {
    DBG_ERROR(0, "Could not create medium");
    AB_Banking_Fini(ab);
    return 3;
  }

  rv=AH_Medium_Mount(medium);
  if (rv) {
    DBG_ERROR(0, "Could not mount medium (%d)", rv);
    AB_Banking_Fini(ab);
    return 3;
  }

  rv=AH_Medium_Unmount(medium, 1);
  if (rv) {
    DBG_ERROR(0, "Could not unmount medium (%d)", rv);
    AB_Banking_Fini(ab);
    return 3;
  }

  AH_Provider_AddMedium(pro, medium);
  fprintf(stdout, "Medium added.\n");


  rv=AB_Banking_OnlineFini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}




