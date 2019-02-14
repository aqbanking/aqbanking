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

#include <gwenhywfar/text.h>

#include <aqhbci/user.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int AH_Control_SetSepaProfile(AB_PROVIDER *pro,
                              GWEN_DB_NODE *dbArgs,
                              int argc,
                              char **argv)
{
  GWEN_DB_NODE *db;
  uint32_t uid;
  AB_USER *u=NULL;
  int rv;
  const char *tProfile;
  const char *dProfile;
  GWEN_DB_NODE *profile;
  const char *s;
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
      GWEN_ArgsType_Char,           /* type */
      "transferProfile",            /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "t",                          /* short option */
      "transfers",                  /* long option */
      "Specify the SEPA profile for transfers",    /* short description */
      "Specify the SEPA profile for transfers (\"\" for default)" /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "debitNoteProfile",           /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "d",                          /* short option */
      "debitnotes",                 /* long option */
      "Specify the SEPA profile for debit notes",    /* short description */
      "Specify the SEPA profile for debit notes (\"\" for default)" /* long description */
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

  /* get and check params */
  uid=(uint32_t) GWEN_DB_GetIntValue(db, "userId", 0, 0);
  if (uid==0) {
    fprintf(stderr, "ERROR: Invalid or missing unique user id\n");
    return 1;
  }

  tProfile=GWEN_DB_GetCharValue(db, "transferProfile", 0, NULL);
  dProfile=GWEN_DB_GetCharValue(db, "debitNoteProfile", 0, NULL);
  if (!tProfile && !dProfile) {
    DBG_ERROR(0, "No action specified");
    return 1;
  }


  /* doit */
  if (tProfile && *tProfile) {
    /* check whether given profile is supported by AqBankings SEPA im-/exporter */
    profile=AB_Banking_GetImExporterProfile(AB_Provider_GetBanking(pro), "sepa", tProfile);
    if (!profile) {
      DBG_ERROR(0, "Profile \"%s\" not found", tProfile);
      return 1;
    }
    s=GWEN_DB_GetCharValue(profile, "type", 0, "");
    if (GWEN_Text_ComparePattern(s, "001.*", 1)==-1) {
      DBG_ERROR(0, "Profile \"%s\" is of type \"%s\" but should match \"001.*\"", tProfile, s);
      return 1;
    }
  }

  if (dProfile && *dProfile) {
    /* check whether given profile is supported by AqBankings SEPA im-/exporter */
    profile=AB_Banking_GetImExporterProfile(AB_Provider_GetBanking(pro), "sepa", dProfile);
    if (!profile) {
      DBG_ERROR(0, "Profile \"%s\" not found", dProfile);
      return 1;
    }
    s=GWEN_DB_GetCharValue(profile, "type", 0, "");
    if (GWEN_Text_ComparePattern(s, "008.*", 1)==-1) {
      DBG_ERROR(0, "Profile \"%s\" is of type \"%s\" but should match \"008.*\"", dProfile, s);
      return 1;
    }
  }

  rv=AB_Provider_GetUser(pro, uid, 1, 0, &u); /* don't lock to allow for AH_Provider_EndExclUseUser */
  if (rv<0) {
    fprintf(stderr, "ERROR: User with id %lu not found\n", (unsigned long int) uid);
    return 2;
  }
  else {
    /* modify user */
    if (tProfile) {
      if (*tProfile) {
        fprintf(stderr, "Setting SEPA profile for transfers to \"%s\"\n", tProfile);
      }
      else {
        fprintf(stderr, "Resetting default SEPA profile for transfers\n");
        tProfile=NULL;
      }
      AH_User_SetSepaTransferProfile(u, tProfile);
    }

    if (dProfile) {
      if (*dProfile) {
        fprintf(stderr, "Setting SEPA profile for debit notes to \"%s\"\n", dProfile);
      }
      else {
        fprintf(stderr, "Resetting default SEPA profile for debit notes\n");
        dProfile=NULL;
      }
      AH_User_SetSepaDebitNoteProfile(u, dProfile);
    }

    /* unlock user */
    rv=AB_Provider_EndExclUseUser(pro, u, 0);
    if (rv<0) {
      fprintf(stderr, "ERROR: Could not unlock user (%d)\n", rv);
      AB_Provider_EndExclUseUser(pro, u, 1); /* abort */
      AB_User_free(u);
      return 4;
    }
  }
  AB_User_free(u);

  return 0;
}




