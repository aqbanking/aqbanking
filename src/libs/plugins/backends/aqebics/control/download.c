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


#include "globals.h"

#include <gwenhywfar/text.h>
#include <gwenhywfar/gui.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int download(AB_PROVIDER *pro,
	     GWEN_DB_NODE *dbArgs,
	     int argc,
	     char **argv) {
  GWEN_DB_NODE *db;
  uint32_t uid;
  AB_USER *u=NULL;
  int rv;
  const char *requestType;
  const char *fromTime;
  const char *toTime;
  int receipt;
  int verbosity;
  const GWEN_ARGS args[]={
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
    "requestType",                /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "r",                          /* short option */
    "request",                  /* long option */
    "Specify the request type",      /* short description */
    "Specify the request type"       /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "fromTime",
    0,         
    1,         
    0,
    "fromtime", 
    "Specify the start date",
    "Specify the start date"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "toTime",
    0,         
    1,         
    0,
    "totime",
    "Specify the end date",
    "Specify the end date"
  },
  {
    0,
    GWEN_ArgsType_Int,
    "receipt",
    0,         
    1,         
    0,
    "receipt",
    "Acknowledge receiption",
    "Acknowledge receiption"
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

  verbosity=GWEN_DB_GetIntValue(dbArgs, "verbosity", 0, 0);

  requestType=GWEN_DB_GetCharValue(db, "requestType", 0, NULL);
  fromTime=GWEN_DB_GetCharValue(db, "fromTime", 0, NULL);
  toTime=GWEN_DB_GetCharValue(db, "toTime", 0, NULL);
  receipt=GWEN_DB_GetIntValue(db, "receipt", 0, 0);

  /* doit */
  uid=(uint32_t) GWEN_DB_GetIntValue(db, "userId", 0, 0);
  if (uid==0) {
    fprintf(stderr, "ERROR: Invalid or missing unique user id\n");
    return 1;
  }

  rv=AB_Provider_GetUser(pro, uid, 1, 1, &u);
  if (rv<0) {
    fprintf(stderr, "ERROR: User with id %lu not found\n", (unsigned long int) uid);
    return 2;
  }
  else {
    GWEN_DATE *daFrom=NULL;
    GWEN_DATE *daTo=NULL;
    GWEN_BUFFER *destBuffer;
    uint32_t guiid;

    if (fromTime) {
      daFrom=GWEN_Date_fromStringWithTemplate(fromTime, "YYYYMMDD");
      if (daFrom==NULL) {
	fprintf(stderr, "ERROR: Invalid fromDate (use \"YYYYMMDD\")\n");
	return 1;
      }
    }

    if (toTime) {
      daTo=GWEN_Date_fromStringWithTemplate(toTime, "YYYYMMDD");
      if (daTo==NULL) {
	fprintf(stderr, "ERROR: Invalid toDate (use \"YYYYMMDD\")\n");
	return 1;
      }
    }

    guiid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS |
				 GWEN_GUI_PROGRESS_SHOW_PROGRESS |
				 GWEN_GUI_PROGRESS_SHOW_LOG |
				 GWEN_GUI_PROGRESS_ALWAYS_SHOW_LOG |
				 GWEN_GUI_PROGRESS_KEEP_OPEN |
				 GWEN_GUI_PROGRESS_SHOW_ABORT,
				 I18N("Executing Request"),
				 I18N("Now the request is send "
				      "to the credit institute."),
				 GWEN_GUI_PROGRESS_NONE,
				 0);

    destBuffer=GWEN_Buffer_new(0, 1024, 0, 1);
    GWEN_Buffer_SetHardLimit(destBuffer, EBICS_BUFFER_MAX_HARD_LIMIT);

    rv=EBC_Provider_Download(pro, u,
			     requestType,
			     destBuffer,
			     receipt,
			     daFrom,
                             daTo,
                             1);
    if (rv==GWEN_ERROR_NO_DATA) {
      GWEN_Gui_ProgressLog(guiid, GWEN_LoggerLevel_Warning, I18N("No download data"));
    }
    GWEN_Gui_ProgressEnd(guiid);
    if (rv) {
      DBG_ERROR(0, "Error sending download request (%d)", rv);
      return 4;
    }
    else {
      fprintf(stderr, "Download request sent.\n");
    }
    if (GWEN_Buffer_GetUsedBytes(destBuffer)) {
      rv=writeFile(stdout, GWEN_Buffer_GetStart(destBuffer), GWEN_Buffer_GetUsedBytes(destBuffer));
      if (rv<0) {
        fprintf(stderr, "ERROR: Unable to write result to stdout (%s)\n", strerror(errno));
        return 6;
      }
      else {
	if (verbosity>0)
	  fprintf(stderr, "INFO: Wrote %d bytes\n", GWEN_Buffer_GetUsedBytes(destBuffer));
      }
    }
    else {
      fprintf(stderr, "WARNING: Empty download data\n");
    }

    GWEN_Buffer_free(destBuffer);
    GWEN_Date_free(daTo);
    GWEN_Date_free(daFrom);
  }

  fprintf(stderr, "Download request ok.\n");

  return 0;
}




