/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: getsysid.c 1288 2007-08-11 16:53:57Z martin $
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
/* The code below uses the private symbol AH_Provider_SendDtazv from libaqhbci */
#include "hbci_l.h"
#include "provider_l.h"

#include <gwenhywfar/text.h>
#include <gwenhywfar/fslock.h>
#include <gwenhywfar/gwentime.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#ifdef OS_WIN32
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif


static int _incrementUniqueId(AB_ACCOUNT *a, const char *path) {
  GWEN_BUFFER *pbuf;
  const char *s;
  FILE *f;
  int cnt=0;
  int rv;
  GWEN_FSLOCK *fl;
  GWEN_FSLOCK_RESULT res;
  GWEN_TIME *ti;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (path) {
    GWEN_Buffer_AppendString(pbuf, path);
    GWEN_Buffer_AppendString(pbuf, DIRSEP);
  }

  ti=GWEN_CurrentTime();
  assert(ti);

  s=AB_Account_GetBankCode(a);
  assert(s);
  GWEN_Buffer_AppendString(pbuf, s);
  GWEN_Buffer_AppendString(pbuf, "-");
  s=AB_Account_GetAccountNumber(a);
  assert(s);
  GWEN_Buffer_AppendString(pbuf, s);
  GWEN_Buffer_AppendString(pbuf, "-");

  rv=GWEN_Time_toString(ti, "YYYYMMDD", pbuf);
  assert(rv>=0);
  GWEN_Time_free(ti);

  GWEN_Buffer_AppendString(pbuf, ".cnt");

  fl=GWEN_FSLock_new(GWEN_Buffer_GetStart(pbuf), GWEN_FSLock_TypeFile);
  assert(fl);

  /* lock file */
  res=GWEN_FSLock_Lock(fl, 10000, 0);
  if (res!=GWEN_FSLock_ResultOk) {
    fprintf(stderr, "ERROR: Could not lock file [%s]",
	    GWEN_Buffer_GetStart(pbuf));
    GWEN_FSLock_free(fl);
    GWEN_Buffer_free(pbuf);
    return -1;
  }

  /* read value, if possible */
  f=fopen(GWEN_Buffer_GetStart(pbuf), "r");
  if (f) {
    if (1!=fscanf(f, "%d", &cnt)) {
      fprintf(stderr,
	      "ERROR: Bad value in file [%s], assuming 0\n",
	      GWEN_Buffer_GetStart(pbuf));
      cnt=0;
    }
    fclose(f);
  }

  /* incremente value */
  cnt++;

  /* write value */
  f=fopen(GWEN_Buffer_GetStart(pbuf), "w+");
  if (f) {
    fprintf(f, "%d\n", cnt);
    if (fclose(f)) {
      fprintf(stderr,
	      "ERROR: Could not close file [%s]: %s",
	      GWEN_Buffer_GetStart(pbuf),
	      strerror(errno));
      GWEN_FSLock_Unlock(fl);
      GWEN_FSLock_free(fl);
      GWEN_Buffer_free(pbuf);
      return -1;
    }
  }
  else {
    fprintf(stderr,
	    "ERROR: Could not create file [%s]: %s",
	    GWEN_Buffer_GetStart(pbuf),
            strerror(errno));
    GWEN_FSLock_Unlock(fl);
    GWEN_FSLock_free(fl);
    GWEN_Buffer_free(pbuf);
    return -1;
  }

  /* unlock file */
  GWEN_FSLock_Unlock(fl);
  GWEN_FSLock_free(fl);
  GWEN_Buffer_free(pbuf);

  return 0;
}



static int _readFile(const char *fname, GWEN_BUFFER *fbuf) {
  FILE *f;
  char buffer[512];

  if (!fname)
    f=stdin;
  else
    f=fopen(fname, "rb");
  if (!f) {
    fprintf(stderr,
	    "Could not open file [%s]: %s\n",
	    fname,
	    strerror(errno));
    return -1;
  }

  while(!feof(f)) {
    size_t s;

    s=fread(buffer, 1, sizeof(buffer), f);
    if (s==0) {
      if (ferror(f)) {
	fprintf(stderr,
		"Could not read from file [%s]: %s\n",
		fname,
		strerror(errno));
	if (fname)
	  fclose(f);
	return -1;
      }
      else
        break;
    }
    else {
      GWEN_Buffer_AppendBytes(fbuf, buffer, s);
    }
  }

  if (fname)
    fclose(f);
  return 0;
}



static
int sendDtazv(AB_BANKING *ab,
	      GWEN_DB_NODE *dbArgs,
	      int argc,
	      char **argv) {
  GWEN_DB_NODE *db;
  AB_PROVIDER *pro;
  AB_ACCOUNT_LIST2 *al;
  AB_ACCOUNT *a=0;
  int rv;
  const char *bankId;
  const char *accountId;
  const char *subAccountId;
  const char *inFile;
  const char *ctxFile;
  const char *cpath;
  GWEN_BUFFER *dtazv;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "bankId",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "b",                          /* short option */
    "bank",                       /* long option */
    "Specify the bank code",      /* short description */
    "Specify the bank code"       /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "accountId",                  /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "a",                          /* short option */
    "account",                    /* long option */
    "Specify the account number", /* short description */
    "Specify the account number"  /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "subAccountId",                /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "aa",                          /* short option */
    "subaccount",                   /* long option */
    "Specify the sub account id (Unterkontomerkmal)",    /* short description */
    "Specify the sub account id (Unterkontomerkmal)"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "infile",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "i",                          /* short option */
    "infile",                     /* long option */
    "Specify the input file",     /* short description */
    "Specify the input file"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "cpath",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "p",                          /* short option */
    "cdir",                     /* long option */
    "Specify the folder for counter files",     /* short description */
    "Specify the folder for counter files"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
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
		     0,
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

  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, "*");
  accountId=GWEN_DB_GetCharValue(db, "accountId", 0, "*");
  subAccountId=GWEN_DB_GetCharValue(db, "subAccountId", 0, "*");
  inFile=GWEN_DB_GetCharValue(db, "infile", 0, NULL);
  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, NULL);
  cpath=GWEN_DB_GetCharValue(db, "cpath", 0, NULL);


  dtazv=GWEN_Buffer_new(0, 1024, 0, 1);
  /* Read DTAZV file */
  if (_readFile(inFile, dtazv)) {
    GWEN_Buffer_free(dtazv);
    return 3;
  }

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    GWEN_Buffer_free(dtazv);
    return 2;
  }

  rv=AB_Banking_OnlineInit(ab);
  if (rv) {
    DBG_ERROR(0, "Error on onlineInit (%d)", rv);
    GWEN_Buffer_free(dtazv);
    return 2;
  }

  pro=AB_Banking_GetProvider(ab, AH_PROVIDER_NAME);
  assert(pro);

  al=AB_Banking_FindAccounts(ab, AH_PROVIDER_NAME, "de",
                             bankId, accountId, subAccountId);
  if (al) {
    if (AB_Account_List2_GetSize(al)!=1) {
      DBG_ERROR(0, "Ambiguous account specification");
      GWEN_Buffer_free(dtazv);
      return 3;
    }
    else {
      a=AB_Account_List2_GetFront(al);
      assert(a);
    }
    AB_Account_List2_free(al);
  }
  if (!a) {
    DBG_ERROR(0, "No matching account");
    GWEN_Buffer_free(dtazv);
    return 3;
  }
  else {
    AB_IMEXPORTER_CONTEXT *ctx;
    int rv2=0;

    _incrementUniqueId(a, cpath);

    ctx=AB_ImExporterContext_new();
    rv=AH_Provider_SendDtazv(pro, a, ctx,
			     (const uint8_t*)GWEN_Buffer_GetStart(dtazv),
			     GWEN_Buffer_GetUsedBytes(dtazv),
			     1, 1, 0);
    /* write ctx file */
    if (ctxFile) {
      GWEN_DB_NODE *dbCtx;

      dbCtx=GWEN_DB_Group_new("context");
      rv2=AB_ImExporterContext_toDb(ctx, dbCtx);
      if (rv2) {
	DBG_ERROR(0, "Error writing context to DB (%d)", rv2);
      }
      else {
	rv2=GWEN_DB_WriteFile(dbCtx, ctxFile,
			      GWEN_DB_FLAGS_DEFAULT);
	if (rv2) {
	  DBG_ERROR(0, "Error writing context to file [%s] (%d)",
		    ctxFile, rv2);
	}
      }
      GWEN_DB_Group_free(dbCtx);
    }

    AB_ImExporterContext_free(ctx);
    if (rv) {
      DBG_ERROR(0, "Error sending DTAZV (%d)", rv);
      AB_Banking_Fini(ab);
      GWEN_Buffer_free(dtazv);
      return 4;
    }
    if (rv2) {
      AB_Banking_Fini(ab);
      GWEN_Buffer_free(dtazv);
      return 5;
    }
  }

  GWEN_Buffer_free(dtazv);

  rv=AB_Banking_OnlineFini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 6;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 6;
  }

  return 0;
}





