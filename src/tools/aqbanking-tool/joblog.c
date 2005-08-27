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



static void _showJobs(AB_JOB_LIST2 *jl,
                      GWEN_TYPE_UINT32 jobId,
                      int logLevel) {
  AB_JOB_LIST2_ITERATOR *jit;

  jit=AB_Job_List2_First(jl);
  if (jit) {
    AB_JOB *j;

    j=AB_Job_List2Iterator_Data(jit);
    while(j) {
      if (jobId==0 || jobId==AB_Job_GetJobId(j)) {
        GWEN_STRINGLIST *sl;

        sl=AB_Job_GetLogs(j);
        if (sl) {
          GWEN_STRINGLISTENTRY *se;
          AB_ACCOUNT *a;
          const char *accountNumber;
          const char *bankCode;

          a=AB_Job_GetAccount(j);
          assert(a);
          bankCode=AB_Account_GetBankCode(a);
          accountNumber=AB_Account_GetAccountNumber(a);
          fprintf(stdout, "Logs for job \"%s\" for account %s/%s [id "
                  GWEN_TYPE_TMPL_UINT32
                  "/%04x]:\n",
                  AB_Job_Type2Char(AB_Job_GetType(j)),
                  bankCode, accountNumber,
                  AB_Job_GetJobId(j),
                  AB_Job_GetJobId(j));
          se=GWEN_StringList_FirstEntry(sl);
          while(se) {
            const char *s;

            s=GWEN_StringListEntry_Data(se);
            assert(s);
            if (strlen(s)<2) {
              DBG_ERROR(0, "Bad log entry (too short) in job "
                        GWEN_TYPE_TMPL_UINT32
                        "(%04x): [%s]",
                        AB_Job_GetJobId(j),
                        AB_Job_GetJobId(j),
                        s);
            }
            else {
              int ll=0;

              ll=(s[0]-'0')*10 + (s[1]-'0');
              if (ll<=logLevel) {
                GWEN_BUFFER *lbuf;

                lbuf=GWEN_Buffer_new(0, strlen(s)*2, 0, 1);
                GWEN_Text_UnescapeToBufferTolerant(s, lbuf);
                fprintf(stdout, "  %s\n", GWEN_Buffer_GetStart(lbuf));
                GWEN_Buffer_free(lbuf);
              }
            }
            se=GWEN_StringListEntry_Next(se);
          }
          GWEN_StringList_free(sl);
          fprintf(stdout, "\n");
        }
      }
      j=AB_Job_List2Iterator_Next(jit);
    }
    AB_Job_List2Iterator_free(jit);
  }

}



int jobLog(AB_BANKING *ab,
           GWEN_DB_NODE *dbArgs,
           int argc,
           char **argv) {
  AB_JOB_LIST2 *jl;
  GWEN_TYPE_UINT32 jobId;
  AB_BANKING_LOGLEVEL logLevel;
  GWEN_DB_NODE *db;
  int rv;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeInt,             /* type */
    "jobId",                      /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "j",                          /* short option */
    "jobid",                      /* long option */
    "Specify the job to show",     /* short description */
    "Specify the job to show"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeInt,             /* type */
    "logLevel",                   /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "l",                          /* short option */
    "loglevel",                   /* long option */
    "Specify the minimum loglevel to show",     /* short description */
    "Specify the minimum loglevel to show"      /* long description */
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

  jobId=GWEN_DB_GetIntValue(db, "jobId", 0, 0);
  logLevel=GWEN_DB_GetIntValue(db, "logLevel", 0, AB_Banking_LogLevelInfo);

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(AQT_LOGDOMAIN, "Error on init (%d)", rv);
    return 2;
  }


  jl=AB_Banking_GetFinishedJobs(ab);
  if (jl) {
    _showJobs(jl, jobId, logLevel);
    AB_Job_List2_FreeAll(jl);
  }

  jl=AB_Banking_GetArchivedJobs(ab);
  if (jl) {
    _showJobs(jl, jobId, logLevel);
    AB_Job_List2_FreeAll(jl);
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}

