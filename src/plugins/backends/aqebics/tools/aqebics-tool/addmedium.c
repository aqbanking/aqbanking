/***************************************************************************
 $RCSfile: user.c,v $
                             -------------------
    cvs         : $Id: user.c,v 1.4 2006/01/17 22:58:29 aquamaniac Exp $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"


int addMedium(AB_BANKING *ab,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv) {
  GWEN_DB_NODE *db;
  AB_PROVIDER *pro;
  int rv;
  const char *mediumName;
  const char *mediumType;
  GWEN_CRYPT_TOKEN_DEVICE dev;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
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
    GWEN_ArgsTypeChar,            /* type */
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

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  pro=AB_Banking_GetProvider(ab, "aqebics");
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

  rv=EBC_Provider_AddMedium(pro, dev, mediumName);
  if (rv) {
    DBG_ERROR(0, "Could not add medium \"%s\"", mediumName);
    return 3;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    DBG_ERROR(0, "Error on fini (%d)", rv);
    return 5;
  }

  return 0;
}



