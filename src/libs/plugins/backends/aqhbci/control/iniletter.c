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

#include <gwenhywfar/text.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int AH_Control_IniLetter(AB_PROVIDER *pro,
                         GWEN_DB_NODE *dbArgs,
                         int argc,
                         char **argv)
{
  GWEN_DB_NODE *db;
  AB_USER *u=0;
  uint32_t uid;
  int rv;
  int bankKey;
  int html;
  int variant;
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
      0, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "bankKey",                    /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "B",                          /* short option */
      "bankkey",                    /* long option */
      "Show iniletter of bank keys",/* short description */
      "Show iniletter of bank keys" /* long description */
    },
    {
      0, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "html",                       /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      NULL,                          /* short option */
      "html",                    /* long option */
      "HTML output",/* short description */
      "HTML output" /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "variant",                    /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                          /* short option */
      "variant",                    /* long option */
      "Choose the variant of the iniletter (0, 1, 2)",/* short description */
      "Choose the variant of the iniletter (0 for auto, 1 for RDH1, 2 for RDH2 and above)"
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
    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }

  /* get parameters */
  bankKey=GWEN_DB_VariableExists(db, "bankKey");
  html=GWEN_DB_VariableExists(db, "html");
  variant=GWEN_DB_GetIntValue(db, "variant", 0, 0);


  /* do it */
  uid=(uint32_t) GWEN_DB_GetIntValue(db, "userId", 0, 0);
  if (uid==0) {
    fprintf(stderr, "ERROR: Invalid or missing unique user id\n");
    return 1;
  }

  rv=AB_Provider_HasUser(pro, uid);
  if (rv<0) {
    fprintf(stderr, "ERROR: User with id %lu not found\n", (unsigned long int) uid);
    return 2;
  }
  rv=AB_Provider_GetUser(pro, uid, 1, 1, &u);
  if (rv<0) {
    fprintf(stderr, "ERROR: User with id %lu not found\n", (unsigned long int) uid);
    return 2;
  }
  else {
    GWEN_BUFFER *lbuf;

    lbuf=GWEN_Buffer_new(0, 1024, 0, 1);
    if (html)
      rv=AH_Provider_GetIniLetterHtml(pro, u, bankKey, variant, lbuf, 0);
    else
      rv=AH_Provider_GetIniLetterTxt(pro, u, bankKey, variant, lbuf, 0);
    if (rv) {
      DBG_ERROR(0, "Could not create ini letter (%d)", rv);
      AB_User_free(u);
      return 3;
    }

    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(lbuf));
    GWEN_Buffer_free(lbuf);
  }
  AB_User_free(u);

  return 0;
}




