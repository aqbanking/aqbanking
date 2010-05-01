/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2005-2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"
#include <gwenhywfar/text.h>
#include <gwenhywfar/syncio_file.h>

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
  GWEN_SYNCIO *sio;
  GWEN_DB_NODE *dbCtx;
  int rv;

  if (ctxFile==NULL) {
    sio=GWEN_SyncIo_File_fromStdin();
    GWEN_SyncIo_AddFlags(sio,
			 GWEN_SYNCIO_FLAGS_DONTCLOSE |
			 GWEN_SYNCIO_FILE_FLAGS_READ);
  }
  else {
    sio=GWEN_SyncIo_File_new(ctxFile, GWEN_SyncIo_File_CreationMode_OpenExisting);
    GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
    rv=GWEN_SyncIo_Connect(sio);
    if (rv<0) {
      if (!mustExist) {
	ctx=AB_ImExporterContext_new();
	*pCtx=ctx;
	GWEN_SyncIo_free(sio);
	return 0;
      }
      GWEN_SyncIo_free(sio);
      return 4;
    }
  }

  /* actually read */
  dbCtx=GWEN_DB_Group_new("context");
  rv=GWEN_DB_ReadFromIo(dbCtx, sio,
			GWEN_DB_FLAGS_DEFAULT |
			GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv<0) {
    DBG_ERROR(0, "Error reading context file (%d)", rv);
    GWEN_DB_Group_free(dbCtx);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }
  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);

  ctx=AB_ImExporterContext_fromDb(dbCtx);
  if (!ctx) {
    DBG_ERROR(0, "No context in input data");
    GWEN_DB_Group_free(dbCtx);
    return GWEN_ERROR_BAD_DATA;
  }
  GWEN_DB_Group_free(dbCtx);
  *pCtx=ctx;

  return 0;
}



int writeContext(const char *ctxFile, const AB_IMEXPORTER_CONTEXT *ctx) {
  GWEN_DB_NODE *dbCtx;
  GWEN_SYNCIO *sio;
  int rv;

  dbCtx=GWEN_DB_Group_new("context");
  rv=AB_ImExporterContext_toDb(ctx, dbCtx);
  if (rv<0) {
    DBG_ERROR(0, "Error writing context to db");
    return rv;
  }
  if (ctxFile==NULL) {
    sio=GWEN_SyncIo_File_fromStdout();
    GWEN_SyncIo_AddFlags(sio,
			 GWEN_SYNCIO_FLAGS_DONTCLOSE |
			 GWEN_SYNCIO_FILE_FLAGS_WRITE);
  }
  else {
    sio=GWEN_SyncIo_File_new(ctxFile, GWEN_SyncIo_File_CreationMode_CreateAlways);
    GWEN_SyncIo_AddFlags(sio,
			 GWEN_SYNCIO_FILE_FLAGS_READ |
			 GWEN_SYNCIO_FILE_FLAGS_WRITE |
			 GWEN_SYNCIO_FILE_FLAGS_UREAD |
			 GWEN_SYNCIO_FILE_FLAGS_UWRITE |
			 GWEN_SYNCIO_FILE_FLAGS_GREAD |
			 GWEN_SYNCIO_FILE_FLAGS_GWRITE);
    rv=GWEN_SyncIo_Connect(sio);
    if (rv<0) {
      DBG_ERROR(0, "Error selecting output file: %s",
		strerror(errno));
      GWEN_SyncIo_free(sio);
      return 4;
    }
  }


  dbCtx=GWEN_DB_Group_new("context");
  rv=AB_ImExporterContext_toDb(ctx, dbCtx);
  if (rv<0) {
    DBG_ERROR(0, "Error writing context to db (%d)", rv);
    GWEN_DB_Group_free(dbCtx);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  rv=GWEN_DB_WriteToIo(dbCtx, sio, GWEN_DB_FLAGS_DEFAULT);
  if (rv<0) {
    DBG_ERROR(0, "Error writing context (%d)", rv);
    GWEN_DB_Group_free(dbCtx);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  GWEN_DB_Group_free(dbCtx);

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

