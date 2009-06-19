/***************************************************************************
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
#include <gwenhywfar/io_file.h>
#include <gwenhywfar/iomanager.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>



int readContext(const char *ctxFile,
		AB_IMEXPORTER_CONTEXT **pCtx,
		int mustExist) {
  AB_IMEXPORTER_CONTEXT *ctx;
  int fd;

  if (ctxFile==0)
    fd=fileno(stdin);
  else
    fd=open(ctxFile, O_RDONLY);
  if (fd<0) {
    if (!mustExist) {
      ctx=AB_ImExporterContext_new();
      *pCtx=ctx;
      return 0;
    }
    DBG_ERROR(0, "open(%s): %s", ctxFile, strerror(errno));
    return GWEN_ERROR_IO;
  }
  else {
    GWEN_DB_NODE *dbCtx;
    int rv;

    dbCtx=GWEN_DB_Group_new("context");
    rv=GWEN_DB_ReadFromFd(dbCtx, fd,
			  GWEN_DB_FLAGS_DEFAULT |
			  GWEN_PATH_FLAGS_CREATE_GROUP,
			  0,
			  2000);
    if (ctxFile)
      close(fd);
    if (rv<0) {
      DBG_ERROR(0, "Error reading context file (%d)", rv);
      GWEN_DB_Group_free(dbCtx);
      return rv;
    }

    ctx=AB_ImExporterContext_fromDb(dbCtx);
    if (!ctx) {
      DBG_ERROR(0, "No context in input data");
      GWEN_DB_Group_free(dbCtx);
      return GWEN_ERROR_BAD_DATA;
    }
    GWEN_DB_Group_free(dbCtx);
    *pCtx=ctx;
  }

  return 0;
}



int writeContext(const char *ctxFile, const AB_IMEXPORTER_CONTEXT *ctx) {
  GWEN_DB_NODE *dbCtx;
  int fd;
  int rv;

  dbCtx=GWEN_DB_Group_new("context");
  rv=AB_ImExporterContext_toDb(ctx, dbCtx);
  if (rv<0) {
    DBG_ERROR(0, "Error writing context to db");
    return rv;
  }
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
    DBG_ERROR(0, "Error selecting output file: %s",
	      strerror(errno));
    return GWEN_ERROR_IO;
  }
  else {
    GWEN_DB_NODE *dbCtx;
    int rv;

    dbCtx=GWEN_DB_Group_new("context");
    rv=AB_ImExporterContext_toDb(ctx, dbCtx);
    if (rv<0) {
      DBG_ERROR(0, "Error writing context to db (%d)", rv);
      GWEN_DB_Group_free(dbCtx);
      return rv;
    }

    rv=GWEN_DB_WriteToFd(dbCtx, fd,
			 GWEN_DB_FLAGS_DEFAULT,
			 0,
			 2000);
    if (rv<0) {
      DBG_ERROR(0, "Error writing context (%d)", rv);
      GWEN_DB_Group_free(dbCtx);
      if (ctxFile)
	close(fd);
      return rv;
    }

    if (ctxFile) {
      if (close(fd)) {
	DBG_ERROR(0, "Error writing context (%d)", rv);
	GWEN_DB_Group_free(dbCtx);
	return GWEN_ERROR_IO;
      }
    }

    GWEN_DB_Group_free(dbCtx);
  }

  return 0;
}



AB_TRANSACTION *mkTransfer(AB_ACCOUNT *a, GWEN_DB_NODE *db) {
  AB_TRANSACTION *t;
  const char *s;
  int i;

  assert(a);
  assert(db);
  t=AB_Transaction_new();

  AB_Transaction_FillLocalFromAccount(t, a);

  /* remote account */
  s=GWEN_DB_GetCharValue(db, "remoteBankId", 0, 0);
  if (s && *s)
    AB_Transaction_SetRemoteBankCode(t, s);
  else {
    DBG_ERROR(0, "No remote bank id given");
    AB_Transaction_free(t);
    return 0;
  }
  s=GWEN_DB_GetCharValue(db, "remoteAccountId", 0, 0);
  if (s && *s)
    AB_Transaction_SetRemoteAccountNumber(t, s);

  s=GWEN_DB_GetCharValue(db, "remoteIban", 0, 0);
  if (s && *s)
    AB_Transaction_SetRemoteIban(t, s);

  s=GWEN_DB_GetCharValue(db, "remoteBic", 0, 0);
  if (s && *s)
    AB_Transaction_SetRemoteBic(t, s);

  for (i=0; i<10; i++) {
    s=GWEN_DB_GetCharValue(db, "remoteName", i, 0);
    if (!s)
      break;
    if (*s)
      AB_Transaction_AddRemoteName(t, s, 0);
  }
  if (i<1) {
    DBG_ERROR(0, "No remote name given");
    AB_Transaction_free(t);
    return 0;
  }

  /* transfer data */
  for (i=0; i<20; i++) {
    s=GWEN_DB_GetCharValue(db, "purpose", i, 0);
    if (!s)
      break;
    if (*s)
      AB_Transaction_AddPurpose(t, s, 0);
  }
  if (i<1) {
    DBG_ERROR(0, "No purpose given");
    AB_Transaction_free(t);
    return 0;
  }

  i=GWEN_DB_GetIntValue(db, "textkey", 0, -1);
  if (i>0)
    AB_Transaction_SetTextKey(t, i);

  s=GWEN_DB_GetCharValue(db, "value", 0, 0);
  if (s && *s) {
    AB_VALUE *v;

    v=AB_Value_fromString(s);
    assert(v);
    if (AB_Value_IsNegative(v) || AB_Value_IsZero(v)) {
      DBG_ERROR(0, "Only positive non-zero amount allowed");
      AB_Transaction_free(t);
      return 0;
    }
    AB_Transaction_SetValue(t, v);
    AB_Value_free(v);
  }
  else {
    DBG_ERROR(0, "No value given");
    AB_Transaction_free(t);
    return 0;
  }

  return t;
}



