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



/* ========================================================================================================================
 *                                                readContext
 * ========================================================================================================================
 */

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



/* ========================================================================================================================
 *                                                writeContext
 * ========================================================================================================================
 */

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



/* ========================================================================================================================
 *                                                mkSepaTransfer
 * ========================================================================================================================
 */
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

  s=GWEN_DB_GetCharValue(db, "remoteName", 0, 0);
  if (s && *s)
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
  if (cmd==AB_Transaction_CommandSepaCreateStandingOrder) {
     s=GWEN_DB_GetCharValue(db, "firstExecutionDate", 0, 0);
     if (!(s && *s)) {
       DBG_ERROR(0, "Missing first execution date");
       return NULL;
     }
  }

  if (cmd==AB_Transaction_CommandSepaModifyStandingOrder ||
      cmd==AB_Transaction_CommandSepaDeleteStandingOrder) {
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

  if (cmd==AB_Transaction_CommandSepaCreateStandingOrder ||
      cmd==AB_Transaction_CommandSepaModifyStandingOrder ||
      cmd==AB_Transaction_CommandSepaDeleteStandingOrder) {
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



/* ========================================================================================================================
 *                                                mkSepaDebitNote
 * ========================================================================================================================
 */

AB_TRANSACTION *mkSepaDebitNote(GWEN_DB_NODE *db, int cmd) {
  AB_TRANSACTION *t;
  const char *s;

  t=mkSepaTransfer(db, cmd);
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



/* ========================================================================================================================
 *                                                getSelectedAccounts
 * ========================================================================================================================
 */

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



/* ========================================================================================================================
 *                                                getSingleSelectedAccount
 * ========================================================================================================================
 */


AB_ACCOUNT_SPEC *getSingleSelectedAccount(AB_BANKING *ab, GWEN_DB_NODE *db) {
  int rv;
  AB_ACCOUNT_SPEC_LIST *al=NULL;
  AB_ACCOUNT_SPEC *as;

  rv=getSelectedAccounts(ab, db, &al);
  if (rv<0) {
    if (rv==GWEN_ERROR_NOT_FOUND)
      fprintf(stderr, "No matching accounts\n");
    else
      fprintf(stderr, "Error getting selected accounts (%d)\n", rv);
    return NULL;
  }

  if (AB_AccountSpec_List_GetCount(al)>1) {
    fprintf(stderr, "Ambiguous account specification (%d accounts matching)\n", AB_AccountSpec_List_GetCount(al));
    AB_AccountSpec_List_free(al);
    return NULL;
  }

  as=AB_AccountSpec_List_First(al);
  assert(as);
  AB_AccountSpec_List_Del(as);
  AB_AccountSpec_List_free(al);
  return as;
}



/* ========================================================================================================================
 *                                                replaceVars
 * ========================================================================================================================
 */

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



/* ========================================================================================================================
 *                                                checkTransactionIbans
 * ========================================================================================================================
 */

int checkTransactionIbans(const AB_TRANSACTION *t) {
  const char *rIBAN;
  const char *rBIC;
  const char *lIBAN;
  const char *lBIC;
  int rv;

  assert(t);

  /* some checks */
  rIBAN=AB_Transaction_GetRemoteIban(t);
  rBIC=AB_Transaction_GetRemoteBic(t);
  lIBAN=AB_Transaction_GetLocalIban(t);
  lBIC=AB_Transaction_GetLocalBic(t);

  if (!rIBAN || !(*rIBAN)) {
    fprintf(stderr, "Missing remote IBAN\n");
    return 1;
  }
  rv=AB_Banking_CheckIban(rIBAN);
  if (rv != 0) {
    fprintf(stderr, "Invalid remote IBAN (%s)\n", rIBAN);
    return 3;
  }

  if (!lBIC || !(*lBIC)) {
    fprintf(stderr, "Missing local BIC\n");
    return 1;
  }
  if (!lIBAN || !(*lIBAN)) {
    fprintf(stderr, "Missing local IBAN\n");
    return 1;
  }
  rv=AB_Banking_CheckIban(lIBAN);
  if (rv != 0) {
    fprintf(stderr, "Invalid local IBAN (%s)\n", lIBAN);
    return 3;
  }

  if (strncmp(lIBAN, rIBAN, 2) && (!rBIC || !*rBIC)) {
    DBG_ERROR(0, "Remote BIC id required for international transaction");
    return 1;
  }

  return 0;
}



/* ========================================================================================================================
 *                                                checkTransactionLimits
 * ========================================================================================================================
 */

int checkTransactionLimits(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim, uint32_t flags) {
  if (lim==NULL) {
    fprintf(stderr, "ERROR: Job not supported with this account.\n");
    return 3;
  }

  if (flags & AQBANKING_TOOL_LIMITFLAGS_PURPOSE)
    if (AB_Banking_CheckTransactionAgainstLimits_Purpose(t, lim)) {
      fprintf(stderr, "ERROR: Purpose violates job limits.\n");
      return 3;
    }

  if (flags & AQBANKING_TOOL_LIMITFLAGS_NAMES)
    if (AB_Banking_CheckTransactionAgainstLimits_Names(t, lim)) {
      fprintf(stderr, "ERROR: Names violate job limits.\n");
      return 3;
    }

  if (flags & AQBANKING_TOOL_LIMITFLAGS_SEQUENCE)
    if (AB_Banking_CheckTransactionAgainstLimits_Sequence(t, lim)) {
      fprintf(stderr, "ERROR: Sequence violate job limits.\n");
      return 3;
    }

  if (flags & AQBANKING_TOOL_LIMITFLAGS_DATE)
    if (AB_Banking_CheckTransactionAgainstLimits_Date(t, lim)) {
      fprintf(stderr, "ERROR: Date violate job limits.\n");
      return 3;
    }

  if (flags & AQBANKING_TOOL_LIMITFLAGS_SEPA)
    if (AB_Banking_CheckTransactionForSepaConformity(t, 0)) {
      fprintf(stderr, "ERROR: Transaction fails SEPA conformity check.\n");
      return 3;
    }

  return 0;
}



/* ========================================================================================================================
 *                                                addTransactionToContextFile
 * ========================================================================================================================
 */

int addTransactionToContextFile(const AB_TRANSACTION *t, const char *ctxFile) {
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx=NULL;

  /* load ctx file */
  rv=readContext(ctxFile, &ctx, 0);
  if (rv<0) {
    DBG_ERROR(0, "Error reading context (%d)", rv);
    return 4;
  }

  /* add transaction to */
  AB_ImExporterContext_AddTransaction(ctx, AB_Transaction_dup(t));

  /* write result back */
  rv=writeContext(ctxFile, ctx);
  AB_ImExporterContext_free(ctx);
  if (rv<0) {
    DBG_ERROR(0, "Error writing context file (%d)", rv);
    return 4;
  }

  /* write result */
  rv=writeContext(ctxFile, ctx);
  AB_ImExporterContext_free(ctx);
  if (rv<0) {
    DBG_ERROR(0, "Error writing context file (%d)", rv);
    AB_ImExporterContext_free(ctx);
    return 4;
  }
  AB_ImExporterContext_free(ctx);

  return 0;
}



/* ========================================================================================================================
 *                                                execBankingJobs
 * ========================================================================================================================
 */

int execBankingJobs(AB_BANKING *ab, AB_TRANSACTION_LIST2 *tList, const char *ctxFile) {
  int rv;
  int rvExec=0;
  AB_IMEXPORTER_CONTEXT *ctx=NULL;

  /* execute job */
  ctx=AB_ImExporterContext_new();
  rv=AB_Banking_SendCommands(ab, tList, ctx);
  if (rv) {
    fprintf(stderr, "Error on executeQueue (%d)\n", rv);
    rvExec=3;
  }

  /* write result */
  rv=writeContext(ctxFile, ctx);
  AB_ImExporterContext_free(ctx);
  if (rv<0) {
    DBG_ERROR(0, "Error writing context file (%d)", rv);
    if (rvExec==0)
      return 4;
  }

  return rvExec;
}



/* ========================================================================================================================
 *                                                execSingleBankingJob
 * ========================================================================================================================
 */

int execSingleBankingJob(AB_BANKING *ab, AB_TRANSACTION *t, const char *ctxFile) {
  AB_TRANSACTION_LIST2 *jobList;
  int rv;

  jobList=AB_Transaction_List2_new();
  AB_Transaction_List2_PushBack(jobList, t);
  rv=execBankingJobs(ab, jobList, ctxFile);
  AB_Transaction_List2_free(jobList);

  return rv;
}



/* ========================================================================================================================
 *                                                createAndCheckRequest
 * ========================================================================================================================
 */

AB_TRANSACTION *createAndCheckRequest(AB_BANKING *ab, AB_ACCOUNT_SPEC *as, AB_TRANSACTION_COMMAND cmd) {
  if (AB_AccountSpec_GetTransactionLimitsForCommand(as, cmd)) {
    AB_TRANSACTION *j;

    j=AB_Transaction_new();
    AB_Transaction_SetUniqueAccountId(j, AB_AccountSpec_GetUniqueId(as));
    AB_Transaction_SetCommand(j, cmd);
    return j;
  }
  else {
    return NULL;
  }
}



/* ========================================================================================================================
 *                                                createAndAddRequest
 * ========================================================================================================================
 */

int createAndAddRequest(AB_BANKING *ab,
			AB_TRANSACTION_LIST2 *tList,
			AB_ACCOUNT_SPEC *as,
			AB_TRANSACTION_COMMAND cmd,
			const GWEN_DATE *fromDate,
			const GWEN_DATE *toDate,
			int ignoreUnsupported) {
  uint32_t aid;
  AB_TRANSACTION *j;

  assert(as);
  aid=AB_AccountSpec_GetUniqueId(as);

  j=createAndCheckRequest(ab, as, cmd);
  if (j) {
    if (cmd==AB_Transaction_CommandGetTransactions) {
      if (fromDate)
	AB_Transaction_SetFirstDate(j, fromDate);
      if (toDate)
	AB_Transaction_SetLastDate(j, toDate);
    }
    AB_Transaction_List2_PushBack(tList, j);
    return 0;
  }
  else {
    if (ignoreUnsupported) {
      fprintf(stderr, "Warning: Ignoring request \"%s\" for %lu, not supported.\n",
	      AB_Transaction_Command_toString(cmd),
	      (unsigned long int) aid);
      return 0;
    }
    else {
      fprintf(stderr, "Error: Request \"%s\" for %lu not supported.\n",
	      AB_Transaction_Command_toString(cmd),
	      (unsigned long int) aid);
      return GWEN_ERROR_GENERIC;
    }
  }
}



/* ========================================================================================================================
 *                                                createAndAddRequests
 * ========================================================================================================================
 */

int createAndAddRequests(AB_BANKING *ab,
			 AB_TRANSACTION_LIST2 *tList,
			 AB_ACCOUNT_SPEC *as,
			 const GWEN_DATE *fromDate,
			 const GWEN_DATE *toDate,
			 uint32_t requestFlags) {
  int ignoreUnsupported=requestFlags & AQBANKING_TOOL_REQUEST_IGNORE_UNSUP;
  int rv;

  assert(ab);
  assert(tList);
  assert(as);

  /* create and add requests */
  if (requestFlags & AQBANKING_TOOL_REQUEST_BALANCE) {
    rv=createAndAddRequest(ab, tList, as, AB_Transaction_CommandGetBalance, fromDate, toDate, ignoreUnsupported);
    if (rv)
      return rv;
  }

  if (requestFlags & AQBANKING_TOOL_REQUEST_STATEMENTS) {
    rv=createAndAddRequest(ab, tList, as, AB_Transaction_CommandGetTransactions, fromDate, toDate, ignoreUnsupported);
    if (rv)
      return rv;
  }

  if (requestFlags & AQBANKING_TOOL_REQUEST_SEPASTO) {
    rv=createAndAddRequest(ab, tList, as, AB_Transaction_CommandSepaGetStandingOrders, fromDate, toDate, ignoreUnsupported);
    if (rv)
      return rv;
  }

  if (requestFlags & AQBANKING_TOOL_REQUEST_ESTATEMENTS) {
    rv=createAndAddRequest(ab, tList, as, AB_Transaction_CommandGetEStatements, fromDate, toDate, ignoreUnsupported);
    if (rv)
      return rv;
  }

  return 0;
}




