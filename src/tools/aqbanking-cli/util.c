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
#include <gwenhywfar/syncio_file.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>



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



AB_TRANSACTION *mkSepaTransfer(GWEN_DB_NODE *db, int cmd) {
  AB_TRANSACTION *t;
  const char *s;
  int i;
  GWEN_DATE *d;

  assert(db);

  t=AB_Transaction_new();

  AB_Transaction_SetType(t, AB_Transaction_TypeTransfer);

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

  s=GWEN_DB_GetCharValue(db, "remoteName", 0, 0);
  if (*s && *s)
    AB_Transaction_SetRemoteName(t, s);
  else {
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
      AB_Transaction_AddPurposeLine(t, s);
  }
  if (i<1) {
    DBG_ERROR(0, "No purpose given");
    AB_Transaction_free(t);
    return NULL;
  }

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

  s=GWEN_DB_GetCharValue(db, "endToEndReference", 0, 0);
  if (s && *s)
    AB_Transaction_SetEndToEndReference(t, s);

  /* dated transfer, SEPA debit notes */
  s=GWEN_DB_GetCharValue(db, "executionDate", 0, 0);
  if (s && *s) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(dbuf, s);
    GWEN_Buffer_AppendString(dbuf, "-00:00");
    d=GWEN_Date_fromStringWithTemplate(GWEN_Buffer_GetStart(dbuf), "YYYYMMDD");
    GWEN_Buffer_free(dbuf);
    if (d==0) {
      DBG_ERROR(0, "Invalid execution date value \"%s\"", s);
      AB_Transaction_free(t);
      return NULL;
    }
    AB_Transaction_SetDate(t, d);
    GWEN_Date_free(d);
  }

  /* standing orders */
  if (cmd==AB_Transaction_CommandCreateStandingOrder) {
     s=GWEN_DB_GetCharValue(db, "firstExecutionDate", 0, 0);
     if (!(s && *s)) {
       DBG_ERROR(0, "Missing first execution date");
       return NULL;
     }
  }

  if (cmd==AB_Transaction_CommandModifyStandingOrder ||
      cmd==AB_Transaction_CommandDeleteStandingOrder) {
     /*  not in the Specs, but the banks ask for it)    */
     s=GWEN_DB_GetCharValue(db, "nextExecutionDate", 0, 0);
     if (!(s && *s)) {
       DBG_ERROR(0, "Missing next execution date");
       return NULL;
     }
  }

  if (s && *s) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(dbuf, s);
    GWEN_Buffer_AppendString(dbuf, "-00:00");
    d=GWEN_Date_fromStringWithTemplate(GWEN_Buffer_GetStart(dbuf), "YYYYMMDD");
    GWEN_Buffer_free(dbuf);
    if (d==0) {
      DBG_ERROR(0, "Invalid first or next execution date value \"%s\"", s);
      AB_Transaction_free(t);
      return NULL;
    }
    AB_Transaction_SetFirstDate(t, d); /*next execution date, too */
    GWEN_Date_free(d);
  }

  s=GWEN_DB_GetCharValue(db, "lastExecutionDate", 0, 0);
  if (s && *s) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(dbuf, s);
    GWEN_Buffer_AppendString(dbuf, "-00:00");
    d=GWEN_Date_fromStringWithTemplate(GWEN_Buffer_GetStart(dbuf), "YYYYMMDD");
    GWEN_Buffer_free(dbuf);
    if (d==0) {
      DBG_ERROR(0, "Invalid last execution date value \"%s\"", s);
      AB_Transaction_free(t);
      return NULL;
    }
    AB_Transaction_SetLastDate(t, d);
    GWEN_Date_free(d);
  }

  if (cmd==AB_Transaction_CommandCreateStandingOrder ||
      cmd==AB_Transaction_CommandModifyStandingOrder ||
      cmd==AB_Transaction_CommandDeleteStandingOrder) {
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

    /* SetFiId */
    s=GWEN_DB_GetCharValue(db, "fiId", 0, 0);
    if (s && *s)
      AB_Transaction_SetFiId(t, s);
  }
  return t;
}



AB_TRANSACTION *mkSepaDebitNote(GWEN_DB_NODE *db) {
  AB_TRANSACTION *t;
  const char *s;

  t=mkSepaTransfer(db, AB_Transaction_CommandSepaDebitNote);
  if (t==NULL) {
    DBG_INFO(0, "here");
    return NULL;
  }

  AB_Transaction_SetType(t, AB_Transaction_TypeDebitNote);
  
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
    AB_TRANSACTION_SEQUENCE st;

    st=AB_Transaction_Sequence_fromString(s);
    if (st!=AB_Transaction_SequenceUnknown)
      AB_Transaction_SetSequence(t, st);
    else {
      DBG_ERROR(0, "Unknown sequence type [%s]", s);
      AB_Transaction_free(t);
      return NULL;
    }
  }
  else
    AB_Transaction_SetSequence(t, AB_Transaction_SequenceOnce);

  return t;
}



int getSelectedAccounts(AB_BANKING *ab, GWEN_DB_NODE *db, AB_ACCOUNT_SPEC_LIST **pAccountSpecList) {
  AB_ACCOUNT_SPEC_LIST *asl=NULL;
  uint32_t uniqueAccountId;
  int rv;

  asl=AB_AccountSpec_List_new();

  uniqueAccountId=(uint32_t) GWEN_DB_GetIntValue(db, "uniqueAccountId", 0, 0);
  if (uniqueAccountId) {
    AB_ACCOUNT_SPEC *as=NULL;

    /* specific unique id given, use that exclusively */
    rv=AB_Banking_GetAccountSpecByUniqueId(ab, uniqueAccountId, &as);
    if (rv<0) {
      DBG_ERROR(0, "Could not load account spec %lu (%d)", (unsigned long int) uniqueAccountId, rv);
      AB_AccountSpec_List_free(asl);
      return rv;
    }
    AB_AccountSpec_List_Add(as, asl);
  }
  else {
    /* no unique account id given, try match parameters */
    rv=AB_Banking_GetAccountSpecList(ab, &asl);
    if (rv<0) {
      DBG_ERROR(0, "Could not load account specs (%d)", rv);
      AB_AccountSpec_List_free(asl);
      return rv;
    }
    else {
      const char *backendName;
      const char *country;
      const char *bankId;
      const char *accountId;
      const char *subAccountId;
      const char *iban;
      const char *s;
      AB_ACCOUNT_TYPE aType=AB_AccountType_Unknown;
      AB_ACCOUNT_SPEC *as;

      backendName=GWEN_DB_GetCharValue(db, "backendName", 0, "*");
      country=GWEN_DB_GetCharValue(db, "country", 0, "*");
      bankId=GWEN_DB_GetCharValue(db, "bankId", 0, "*");
      country=GWEN_DB_GetCharValue(db, "country", 0, "*");
      accountId=GWEN_DB_GetCharValue(db, "accountId", 0, "*");
      subAccountId=GWEN_DB_GetCharValue(db, "subAccountId", 0, "*");
      iban=GWEN_DB_GetCharValue(db, "iban", 0, "*");
      s=GWEN_DB_GetCharValue(db, "accountType", 0, NULL);
      if (s && *s)
        aType=AB_AccountType_fromChar(s);
      if (aType==AB_AccountType_Invalid) {
        DBG_ERROR(0, "Invalid Could not load account specs (%d)", rv);
        AB_AccountSpec_List_free(asl);
        return GWEN_ERROR_INVALID;
      }

      as=AB_AccountSpec_List_First(asl);
      while(as) {
        AB_ACCOUNT_SPEC *asNext;

        asNext=AB_AccountSpec_List_Next(as);
        if (AB_AccountSpec_Matches(as, backendName,
                                   country, bankId, accountId, subAccountId,
                                   iban,
                                   "*", /* currency */
                                   aType)<1) {
          /* doesn't match, remove from list */
          AB_AccountSpec_List_Del(as);
          AB_AccountSpec_free(as);
        }
        as=asNext;
      }
    }
  }

  if (AB_AccountSpec_List_GetCount(asl))
    *pAccountSpecList=asl;
  else {
    AB_AccountSpec_List_free(asl);
    *pAccountSpecList=NULL;
    return GWEN_ERROR_NOT_FOUND;
  }
  return 0;
}



int replaceVars(const char *s, GWEN_DB_NODE *db, GWEN_BUFFER *dbuf) {
    const char *p;

  p=s;
  while(*p) {
    if (*p=='$') {
      p++;
      if (*p=='$')
	GWEN_Buffer_AppendByte(dbuf, '$');
      else if (*p=='(') {
	const char *pStart;

	p++;
	pStart=p;
	while(*p && *p!=')')
	  p++;
	if (*p!=')') {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Unterminated variable name in code");
	  return GWEN_ERROR_BAD_DATA;
	}
	else {
          int len;
	  char *name;
	  const char *valueString;
	  int valueInt;
	  char numbuf[32];
	  int rv;

	  len=p-pStart;
	  if (len<1) {
	    DBG_ERROR(GWEN_LOGDOMAIN, "Empty variable name in code");
	    return GWEN_ERROR_BAD_DATA;
	  }
	  name=(char*) malloc(len+1);
	  assert(name);
	  memmove(name, pStart, len);
          name[len]=0;

	  switch(GWEN_DB_GetVariableType(db, name)) {
	  case GWEN_DB_NodeType_ValueInt:
	    valueInt=GWEN_DB_GetIntValue(db, name, 0, 0);
	    rv=GWEN_Text_NumToString(valueInt, numbuf, sizeof(numbuf)-1, 0);
	    if (rv>=0)
	      GWEN_Buffer_AppendString(dbuf, numbuf);
	    break;
	  case GWEN_DB_NodeType_ValueChar:
	    valueString=GWEN_DB_GetCharValue(db, name, 0, NULL);
	    if (valueString)
	      GWEN_Buffer_AppendString(dbuf, valueString);
#if 0 /* just replace with empty value */
	    else {
	      GWEN_Buffer_AppendString(dbuf, " [__VALUE OF ");
	      GWEN_Buffer_AppendString(dbuf, name);
	      GWEN_Buffer_AppendString(dbuf, " WAS NOT SET__] ");
	    }
#endif
	    break;

	  default:
	    break;
	  }
	  free(name);
	}
      }
      else {
	DBG_ERROR(GWEN_LOGDOMAIN, "Bad variable string in code");
        return GWEN_ERROR_BAD_DATA;
      }
      p++;
    }
    else {
      if (*p=='#') {
	/* let # lines begin on a new line */
	GWEN_Buffer_AppendByte(dbuf, '\n');
	GWEN_Buffer_AppendByte(dbuf, *p);

	/* skip introducing cross and copy all stuff until the next cross
	 * upon which wa inject a newline (to make the preprocessor happy)
	 */
	p++;
	while(*p && *p!='#') {
	  GWEN_Buffer_AppendByte(dbuf, *p);
	  p++;
	}
	if (*p=='#') {
	  GWEN_Buffer_AppendByte(dbuf, '\n');
	  p++;
	}
      }
      else if (*p=='\\') {
	/* check for recognized control escapes */
	if (tolower(p[1])=='n') {
	  GWEN_Buffer_AppendByte(dbuf, '\n');
	  p+=2; /* skip introducing backslash and control character */
	}
	else if (tolower(p[1])=='t') {
	  GWEN_Buffer_AppendByte(dbuf, '\t');
	  p+=2; /* skip introducing backslash and control character */
	}
	else if (tolower(p[1])=='\\') {
	  GWEN_Buffer_AppendByte(dbuf, '\\');
	  p+=2; /* skip introducing backslash and control character */
	}
	else {
	  /* no known escape character, just add literally */
	  GWEN_Buffer_AppendByte(dbuf, *p);
	  p++;
	}
      }
      else {
	GWEN_Buffer_AppendByte(dbuf, *p);
	p++;
      }
    }
  }

  return 0;
}



