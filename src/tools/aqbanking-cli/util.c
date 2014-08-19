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
  }
  else
    rv=0;

  GWEN_DB_Group_free(dbCtx);
  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);

  return rv;
}



AB_TRANSACTION *mkTransfer(AB_ACCOUNT *a, GWEN_DB_NODE *db, AB_JOB_TYPE *jobType) {
  AB_BANKING *ab;
  AB_TRANSACTION *t;
  const char *s;
  int i;
  GWEN_TIME *d;
  AB_TRANSACTION_PERIOD period=AB_Transaction_PeriodUnknown;

  *jobType=AB_Job_TypeTransfer; // single transfer
  assert(a);
  assert(db);

  ab=AB_Account_GetBanking(a);
  assert(ab);

  t=AB_Transaction_new();

  AB_Banking_FillGapsInTransaction(ab, a, t);

  s=GWEN_DB_GetCharValue(db, "name", 0, 0);
  if (s && *s)
    AB_Transaction_SetLocalName(t, s);

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

  // dated transfer
  s=GWEN_DB_GetCharValue(db, "executionDate", 0, 0);
  if (s && *s) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(dbuf, s);
    GWEN_Buffer_AppendString(dbuf, "-00:00");
    d=GWEN_Time_fromUtcString(GWEN_Buffer_GetStart(dbuf),
                                     "YYYYMMDD-hh:mm");
    GWEN_Buffer_free(dbuf);
    if (d==0) {
      DBG_ERROR(0, "Invalid execution date value \"%s\"", s);
      AB_Transaction_free(t);
      return 0;
    }
    AB_Transaction_SetDate(t, d);
    GWEN_Time_free(d);
    *jobType=AB_Job_TypeCreateDatedTransfer;
    return t;
  }

  // standing orders
  s=GWEN_DB_GetCharValue(db, "firstExecutionDate", 0, 0);
  if (s && *s) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(dbuf, s);
    GWEN_Buffer_AppendString(dbuf, "-00:00");
    d=GWEN_Time_fromUtcString(GWEN_Buffer_GetStart(dbuf),
                                     "YYYYMMDD-hh:mm");
    GWEN_Buffer_free(dbuf);
    if (d==0) {
      DBG_ERROR(0, "Invalid first execution date value \"%s\"", s);
      AB_Transaction_free(t);
      return 0;
    }
    AB_Transaction_SetFirstExecutionDate(t, d);
    GWEN_Time_free(d);
  } else
    return t; // single transfer

  *jobType=AB_Job_TypeCreateStandingOrder;
  s=GWEN_DB_GetCharValue(db, "lastExecutionDate", 0, 0);
  if (s && *s) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(dbuf, s);
    GWEN_Buffer_AppendString(dbuf, "-00:00");
    d=GWEN_Time_fromUtcString(GWEN_Buffer_GetStart(dbuf),
                                     "YYYYMMDD-hh:mm");
    GWEN_Buffer_free(dbuf);
    if (d==0) {
      DBG_ERROR(0, "Invalid last execution date value \"%s\"", s);
      AB_Transaction_free(t);
      return 0;
    }
    AB_Transaction_SetLastExecutionDate(t, d);
    GWEN_Time_free(d);
  }

  s=GWEN_DB_GetCharValue(db, "executionPeriod", 0, 0);
  if (s && *s) {
    period=AB_Transaction_Period_fromString(s);
    if (period==AB_Transaction_PeriodUnknown) {
      DBG_ERROR(0, "Invalid execution period value \"%s\"", s);
      AB_Transaction_free(t);
      return NULL;
    }
    AB_Transaction_SetPeriod(t, period);
  }

  i=GWEN_DB_GetIntValue(db, "executionCycle", 0, -1);
  if (i <= 0) {
    DBG_ERROR(0, "Invalid execution cycle value \"%d\"", i);
    AB_Transaction_free(t);
    return 0;
  }
  AB_Transaction_SetCycle(t, i);

  i=GWEN_DB_GetIntValue(db, "executionDay", 0, -1);
  if (i <= 0 || (period == AB_Transaction_PeriodWeekly && i > 7) ||
      (period == AB_Transaction_PeriodMonthly && i > 30 &&
       (i < 97 || i > 99))) {
    DBG_ERROR(0, "Invalid execution day value \"%d\"", i);
    AB_Transaction_free(t);
    return 0;
  }
  AB_Transaction_SetExecutionDay(t, i);

  s=GWEN_DB_GetCharValue(db, "fiId", 0, 0);
  if (s && *s) {
    AB_Transaction_SetFiId(t, s);
    *jobType=AB_Job_TypeDeleteStandingOrder;
  }

  return t;
}



AB_TRANSACTION *mkSepaTransfer(AB_ACCOUNT *a, GWEN_DB_NODE *db, int expTransferType) {
  AB_BANKING *ab;
  AB_TRANSACTION *t;
  const char *s;
  int i;
  GWEN_TIME *d;

  assert(a);
  assert(db);

  ab=AB_Account_GetBanking(a);
  assert(ab);

  t=AB_Transaction_new();

  AB_Banking_FillGapsInTransaction(ab, a, t);

  s=GWEN_DB_GetCharValue(db, "name", 0, 0);
  if (s && *s)
    AB_Transaction_SetLocalName(t, s);

  /* remote account */
  s=GWEN_DB_GetCharValue(db, "remoteBankId", 0, 0);
  if (s && *s)
    AB_Transaction_SetRemoteBankCode(t, s);
  s=GWEN_DB_GetCharValue(db, "remoteAccountId", 0, 0);
  if (s && *s)
    AB_Transaction_SetRemoteAccountNumber(t, s);

  s=GWEN_DB_GetCharValue(db, "remoteIban", 0, 0);
  if (s && *s)
    AB_Transaction_SetRemoteIban(t, s);
  else {
    DBG_ERROR(0, "No remote IBAN given");
    AB_Transaction_free(t);
    return NULL;
  }

  s=GWEN_DB_GetCharValue(db, "remoteBic", 0, 0);
  if (s && *s)
    AB_Transaction_SetRemoteBic(t, s);
  else if (strncmp(AB_Transaction_GetLocalIban(t),
                   AB_Transaction_GetRemoteIban(t), 2)) {
    DBG_ERROR(0, "Remote BIC id required for international transaction");
    AB_Transaction_free(t);
    return NULL;
  }

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
    return NULL;
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
    return NULL;
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
      return NULL;
    }
    AB_Transaction_SetValue(t, v);
    AB_Value_free(v);
  }
  else {
    DBG_ERROR(0, "No value given");
    AB_Transaction_free(t);
    return NULL;
  }

  /* dated transfer, SEPA debit notes */
  s=GWEN_DB_GetCharValue(db, "executionDate", 0, 0);
  if (s && *s) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(dbuf, s);
    GWEN_Buffer_AppendString(dbuf, "-00:00");
    d=GWEN_Time_fromUtcString(GWEN_Buffer_GetStart(dbuf),
                                     "YYYYMMDD-hh:mm");
    GWEN_Buffer_free(dbuf);
    if (d==0) {
      DBG_ERROR(0, "Invalid execution date value \"%s\"", s);
      AB_Transaction_free(t);
      return NULL;
    }
    AB_Transaction_SetDate(t, d);
    GWEN_Time_free(d);
  }

  /* standing orders */
  s=GWEN_DB_GetCharValue(db, "firstExecutionDate", 0, 0);
  if (s && *s) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(dbuf, s);
    GWEN_Buffer_AppendString(dbuf, "-00:00");
    d=GWEN_Time_fromUtcString(GWEN_Buffer_GetStart(dbuf),
                                     "YYYYMMDD-hh:mm");
    GWEN_Buffer_free(dbuf);
    if (d==0) {
      DBG_ERROR(0, "Invalid first execution date value \"%s\"", s);
      AB_Transaction_free(t);
      return NULL;
    }
    AB_Transaction_SetFirstExecutionDate(t, d);
    GWEN_Time_free(d);
  }

  s=GWEN_DB_GetCharValue(db, "lastExecutionDate", 0, 0);
  if (s && *s) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(dbuf, s);
    GWEN_Buffer_AppendString(dbuf, "-00:00");
    d=GWEN_Time_fromUtcString(GWEN_Buffer_GetStart(dbuf),
                                     "YYYYMMDD-hh:mm");
    GWEN_Buffer_free(dbuf);
    if (d==0) {
      DBG_ERROR(0, "Invalid last execution date value \"%s\"", s);
      AB_Transaction_free(t);
      return NULL;
    }
    AB_Transaction_SetLastExecutionDate(t, d);
    GWEN_Time_free(d);
  }

  if (expTransferType==AB_Job_TypeSepaCreateStandingOrder ||
      expTransferType==AB_Job_TypeSepaModifyStandingOrder) {
    const char *s;
    AB_TRANSACTION_PERIOD period=AB_Transaction_PeriodUnknown;

    /* only needed for standing orders */
    s=GWEN_DB_GetCharValue(db, "executionPeriod", 0, 0);
    if (s && *s) {
      period=AB_Transaction_Period_fromString(s);
      if (period==AB_Transaction_PeriodUnknown) {
        DBG_ERROR(0, "Invalid execution period value \"%s\"", s);
        AB_Transaction_free(t);
        return NULL;
      }
    }
    else {
      DBG_ERROR(0, "Missing execution period value");
      return NULL;
    }
    AB_Transaction_SetPeriod(t, period);

    i=GWEN_DB_GetIntValue(db, "executionCycle", 0, -1);
    if (i <= 0) {
      DBG_ERROR(0, "Invalid execution cycle value \"%d\"", i);
      AB_Transaction_free(t);
      return NULL;
    }
    AB_Transaction_SetCycle(t, i);

    i=GWEN_DB_GetIntValue(db, "executionDay", 0, -1);
    if (i <= 0 || (period == AB_Transaction_PeriodWeekly && i > 7) ||
        (period == AB_Transaction_PeriodMonthly && i > 30 &&
         (i < 97 || i > 99))) {
      DBG_ERROR(0, "Invalid execution day value \"%d\"", i);
      AB_Transaction_free(t);
      return NULL;
    }
    AB_Transaction_SetExecutionDay(t, i);
  }

  return t;
}



AB_TRANSACTION *mkSepaDebitNote(AB_ACCOUNT *a, GWEN_DB_NODE *db) {
  AB_TRANSACTION *t;
  const char *s;

  t=mkSepaTransfer(a, db, AB_Job_TypeSepaDebitNote);
  if (t==NULL) {
    DBG_INFO(0, "here");
    return NULL;
  }

  /* read some additional fields */
  s=GWEN_DB_GetCharValue(db, "creditorSchemeId", 0, 0);
  if (!(s && *s)) {
    DBG_ERROR(0, "Missing creditor scheme id");
    AB_Transaction_free(t);
    return NULL;
  }
  AB_Transaction_SetCreditorSchemeId(t, s);

  s=GWEN_DB_GetCharValue(db, "mandateId", 0, 0);
  if (!(s && *s)) {
    DBG_ERROR(0, "Missing mandate id");
    AB_Transaction_free(t);
    return NULL;
  }
  AB_Transaction_SetMandateId(t, s);

  s=GWEN_DB_GetCharValue(db, "mandateDate", 0, 0);
  if (!(s && *s)) {
    DBG_ERROR(0, "Missing mandate date");
    AB_Transaction_free(t);
    return NULL;
  }
  else {
    GWEN_DATE *dt;

    dt=GWEN_Date_fromString(s);
    if (dt==NULL) {
      DBG_ERROR(0, "Bad date format for mandate date");
      AB_Transaction_free(t);
      return NULL;
    }
    AB_Transaction_SetMandateDate(t, dt);
    GWEN_Date_free(dt);
  }

  s=GWEN_DB_GetCharValue(db, "sequenceType", 0, "once");
  if (s && *s) {
    AB_TRANSACTION_SEQUENCETYPE st;

    st=AB_Transaction_SequenceType_fromString(s);
    if (st!=AB_Transaction_SequenceTypeUnknown)
      AB_Transaction_SetSequenceType(t, st);
    else {
      DBG_ERROR(0, "Unknown sequence type [%s]", s);
      AB_Transaction_free(t);
      return NULL;
    }
  }
  else
    AB_Transaction_SetSequenceType(t, AB_Transaction_SequenceTypeOnce);

  return t;
}






