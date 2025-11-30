/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "globals_l.h"
#include "aqhbci/banking/user.h"
#include "aqhbci/banking/provider_l.h"
#include "aqhbci/banking/provider_tan.h"

#include <gwenhywfar/text.h>


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>




static GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv);
static int _readFile(const char *fname, GWEN_BUFFER *dbuf);
static const AH_TAN_METHOD *_getSelectedTanMethod(AB_USER *u, int tanMethodId);





int AH_Control_Test1(AB_PROVIDER *pro,
                     GWEN_DB_NODE *dbArgs,
                     int argc,
                     char **argv)
{
  int rv;
  GWEN_DB_NODE *db;
  uint32_t uid;
  AB_USER *u=NULL;
  uint32_t tanMethodId;
  const char *challengFile;
  GWEN_BUFFER *challengeBuf=NULL;

  db=_readCommandLine(dbArgs, argc, argv);
  if (db==NULL) {
    fprintf(stderr, "ERROR: Could not parse arguments\n");
    return 1;
  }

  challengFile=GWEN_DB_GetCharValue(db, "challengeFile", 0, NULL);
  if (!(challengFile && *challengFile)) {
    fprintf(stderr, "ERROR: Missing challenge file\n");
    return 1;
  }

  uid=(uint32_t) GWEN_DB_GetIntValue(db, "userId", 0, 0);
  if (uid==0) {
    fprintf(stderr, "ERROR: Invalid or missing unique user id\n");
    return 1;
  }

  tanMethodId=(uint32_t) GWEN_DB_GetIntValue(db, "tanMethodId", 0, 0);
  if (tanMethodId==0) {
    fprintf(stderr, "ERROR: Invalid or missing tan method if\n");
    return 1;
  }

  if (challengFile) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=_readFile(challengFile, dbuf);
    if (rv<0) {
      fprintf(stderr, "ERROR: Could not read file \"%s\": %d\n", challengFile, rv);
      GWEN_Buffer_free(dbuf);
      return 2;
    }

    challengeBuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_Text_ToHexBuffer(GWEN_Buffer_GetStart(dbuf),
                             GWEN_Buffer_GetUsedBytes(dbuf),
                             challengeBuf,
                             0, 0, 0);
    if (rv<0) {
      fprintf(stderr, "ERROR: Could not hex encode file \"%s\": %d\n", challengFile, rv);
      GWEN_Buffer_free(challengeBuf);
      GWEN_Buffer_free(dbuf);
      return 2;
    }
    GWEN_Buffer_free(dbuf);
  }


  rv=AB_Provider_HasUser(pro, uid);
  if (rv<0) {
    fprintf(stderr, "ERROR: User with id %lu not found\n", (unsigned long int) uid);
    GWEN_Buffer_free(challengeBuf);
    return 2;
  }
  rv=AB_Provider_GetUser(pro, uid, 1, 1, &u);
  if (rv<0) {
    fprintf(stderr, "ERROR: User with id %lu not found\n", (unsigned long int) uid);
    GWEN_Buffer_free(challengeBuf);
    return 2;
  }
  else {
    const AH_TAN_METHOD *tanMethod;
    char tanBuffer[16];

    tanMethod=_getSelectedTanMethod(u, tanMethodId);
    if (tanMethod==NULL) {
      fprintf(stderr, "ERROR: TAN method with id %lu not found\n", (unsigned long int) tanMethodId);
      GWEN_Buffer_free(challengeBuf);
      return 2;
    }

    rv=AH_Provider_InputTanWithChallenge(pro,
                                         u,
                                         tanMethod,
                                         "Could be a real challenge string but it isn't",
                                         GWEN_Buffer_GetStart(challengeBuf),
                                         tanBuffer,
                                         1,
                                         sizeof(tanBuffer)-1);
    if (rv<0) {
      fprintf(stderr, "ERROR: Error in TAN input (%d)\n", rv);
      GWEN_Buffer_free(challengeBuf);
      return 2;
    }
  }


  AB_User_free(u);

  return 0;
}




GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "userId",                     /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "u",                          /* short option */
      "user",                       /* long option */
      "Specify the unique user id",    /* short description */
      "Specify the unique user id"     /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "tanMethodId",                /* name */
      1,                            /* minnum */
      1,                            /* maxnum */
      "m",                          /* short option */
      NULL,                         /* long option */
      "Specify the TAN method id",  /* short description */
      "Specify the TAN method id"     /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "challengeFile",              /* name */
      1,                            /* minnum */
      1,                            /* maxnum */
      "f",                          /* short option */
      NULL,                         /* long option */
      "Specify the challenge file to load", /* short */
      "Specify the challenge file to load" /* long */
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
  rv=AB_Cmd_Handle_Args(argc, argv, args, db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    return NULL;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    return NULL;
  }

  return db;
}




int _readFile(const char *fname, GWEN_BUFFER *dbuf)
{
  FILE *f;

  f=fopen(fname, "rb");
  if (f) {
    while (!feof(f)) {
      uint32_t l;
      ssize_t s;
      char *p;

      GWEN_Buffer_AllocRoom(dbuf, 1024);
      l=GWEN_Buffer_GetMaxUnsegmentedWrite(dbuf);
      p=GWEN_Buffer_GetPosPointer(dbuf);
      s=fread(p, 1, l, f);
      if (s==0)
        break;
      if (s==(ssize_t)-1) {
        DBG_ERROR(0,
                  "fread(%s): %s",
                  fname, strerror(errno));
        fclose(f);
        return GWEN_ERROR_IO;
      }

      GWEN_Buffer_IncrementPos(dbuf, s);
      GWEN_Buffer_AdjustUsedBytes(dbuf);
    }

    fclose(f);
    return 0;
  }
  else {
    DBG_ERROR(0,
              "fopen(%s): %s",
              fname, strerror(errno));
    return GWEN_ERROR_IO;
  }
}



const AH_TAN_METHOD *_getSelectedTanMethod(AB_USER *u, int tanMethodId)
{
  const AH_TAN_METHOD_LIST *tanMethodList;

  tanMethodList=AH_User_GetTanMethodDescriptions(u);
  if (tanMethodList) {
    const AH_TAN_METHOD *tanMethod;

    tanMethod=AH_TanMethod_List_First(tanMethodList);
    while (tanMethod) {
      int combinedVersion;

      combinedVersion=AH_TanMethod_GetFunction(tanMethod)+(AH_TanMethod_GetGvVersion(tanMethod)*1000);
      if (combinedVersion==tanMethodId)
        return tanMethod;
      tanMethod=AH_TanMethod_List_Next(tanMethod);
    }
  }

  return NULL;
}



