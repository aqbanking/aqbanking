#include "transaction_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/db.h>
#include <assert.h>
#include <stdlib.h>


GWEN_INHERIT_FUNCTIONS(AB_TRANSACTION)
GWEN_LIST_FUNCTIONS(AB_TRANSACTION, AB_Transaction)
GWEN_LIST2_FUNCTIONS(AB_TRANSACTION, AB_Transaction)


AB_TRANSACTION *AB_Transaction_new() {
  AB_TRANSACTION *st;

  GWEN_NEW_OBJECT(AB_TRANSACTION, st)
  st->_usage=1;
  GWEN_INHERIT_INIT(AB_TRANSACTION, st)
  GWEN_LIST_INIT(AB_TRANSACTION, st)
  st->remoteName=GWEN_StringList_new();
  st->purpose=GWEN_StringList_new();
  return st;
}


void AB_Transaction_free(AB_TRANSACTION *st) {
  if (st) {
    assert(st->_usage);
    if (--(st->_usage)==0) {
  GWEN_INHERIT_FINI(AB_TRANSACTION, st)
  if (st->localBankCode)
    free(st->localBankCode);
  if (st->localAccountNumber)
    free(st->localAccountNumber);
  if (st->localSuffix)
    free(st->localSuffix);
  if (st->localName)
    free(st->localName);
  if (st->remoteBankCode)
    free(st->remoteBankCode);
  if (st->remoteAccountNumber)
    free(st->remoteAccountNumber);
  if (st->remoteSuffix)
    free(st->remoteSuffix);
  if (st->remoteName)
    GWEN_StringList_free(st->remoteName);
  if (st->uniqueId)
    free(st->uniqueId);
  if (st->valutaDate)
    GWEN_Time_free(st->valutaDate);
  if (st->date)
    GWEN_Time_free(st->date);
  if (st->value)
    AB_Value_free(st->value);
  if (st->transactionKey)
    free(st->transactionKey);
  if (st->customerReference)
    free(st->customerReference);
  if (st->bankReference)
    free(st->bankReference);
  if (st->transactionText)
    free(st->transactionText);
  if (st->primanota)
    free(st->primanota);
  if (st->purpose)
    GWEN_StringList_free(st->purpose);
  GWEN_LIST_FINI(AB_TRANSACTION, st)
  GWEN_FREE_OBJECT(st);
    }
  }

}


AB_TRANSACTION *AB_Transaction_dup(const AB_TRANSACTION *d) {
  AB_TRANSACTION *st;

  assert(d);
  st=AB_Transaction_new();
  st->localCountryCode=d->localCountryCode;
  if (d->localBankCode)
    st->localBankCode=strdup(d->localBankCode);
  if (d->localAccountNumber)
    st->localAccountNumber=strdup(d->localAccountNumber);
  if (d->localSuffix)
    st->localSuffix=strdup(d->localSuffix);
  if (d->localName)
    st->localName=strdup(d->localName);
  st->remoteCountryCode=d->remoteCountryCode;
  if (d->remoteBankCode)
    st->remoteBankCode=strdup(d->remoteBankCode);
  if (d->remoteAccountNumber)
    st->remoteAccountNumber=strdup(d->remoteAccountNumber);
  if (d->remoteSuffix)
    st->remoteSuffix=strdup(d->remoteSuffix);
  if (d->remoteName)
    st->remoteName=GWEN_StringList_dup(d->remoteName);
  if (d->uniqueId)
    st->uniqueId=strdup(d->uniqueId);
  if (d->valutaDate)
    st->valutaDate=GWEN_Time_dup(d->valutaDate);
  if (d->date)
    st->date=GWEN_Time_dup(d->date);
  if (d->value)
    st->value=AB_Value_dup(d->value);
  st->textKey=d->textKey;
  if (d->transactionKey)
    st->transactionKey=strdup(d->transactionKey);
  if (d->customerReference)
    st->customerReference=strdup(d->customerReference);
  if (d->bankReference)
    st->bankReference=strdup(d->bankReference);
  st->transactionCode=d->transactionCode;
  if (d->transactionText)
    st->transactionText=strdup(d->transactionText);
  if (d->primanota)
    st->primanota=strdup(d->primanota);
  if (d->purpose)
    st->purpose=GWEN_StringList_dup(d->purpose);
  return st;
}


int AB_Transaction_toDb(const AB_TRANSACTION *st, GWEN_DB_NODE *db) {
  assert(st);
  assert(db);
  if (GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "localCountryCode", st->localCountryCode))
    return -1;
  if (st->localBankCode)
    if (GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "localBankCode", st->localBankCode))
      return -1;
  if (st->localAccountNumber)
    if (GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "localAccountNumber", st->localAccountNumber))
      return -1;
  if (st->localSuffix)
    if (GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "localSuffix", st->localSuffix))
      return -1;
  if (st->localName)
    if (GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "localName", st->localName))
      return -1;
  if (GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "remoteCountryCode", st->remoteCountryCode))
    return -1;
  if (st->remoteBankCode)
    if (GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "remoteBankCode", st->remoteBankCode))
      return -1;
  if (st->remoteAccountNumber)
    if (GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "remoteAccountNumber", st->remoteAccountNumber))
      return -1;
  if (st->remoteSuffix)
    if (GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "remoteSuffix", st->remoteSuffix))
      return -1;
  if (st->remoteName)
    {
      GWEN_STRINGLISTENTRY *se;

      GWEN_DB_DeleteVar(db, "remoteName");
      se=GWEN_StringList_FirstEntry(st->remoteName);
      while(se) {
        const char *s;

        s=GWEN_StringListEntry_Data(se);
        assert(s);
        if (GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, "remoteName", s))
          return -1;
        se=GWEN_StringListEntry_Next(se);
      } /* while */
    }
  if (st->uniqueId)
    if (GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "uniqueId", st->uniqueId))
      return -1;
  if (st->valutaDate)
    if (GWEN_Time_toDb(st->valutaDate, GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "valutaDate")))
      return -1;
  if (st->date)
    if (GWEN_Time_toDb(st->date, GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "date")))
      return -1;
  if (st->value)
    if (AB_Value_toDb(st->value, GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "value")))
      return -1;
  if (GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "textKey", st->textKey))
    return -1;
  if (st->transactionKey)
    if (GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "transactionKey", st->transactionKey))
      return -1;
  if (st->customerReference)
    if (GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "customerReference", st->customerReference))
      return -1;
  if (st->bankReference)
    if (GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "bankReference", st->bankReference))
      return -1;
  if (GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "transactionCode", st->transactionCode))
    return -1;
  if (st->transactionText)
    if (GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "transactionText", st->transactionText))
      return -1;
  if (st->primanota)
    if (GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "primanota", st->primanota))
      return -1;
  if (st->purpose)
    {
      GWEN_STRINGLISTENTRY *se;

      GWEN_DB_DeleteVar(db, "purpose");
      se=GWEN_StringList_FirstEntry(st->purpose);
      while(se) {
        const char *s;

        s=GWEN_StringListEntry_Data(se);
        assert(s);
        if (GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, "purpose", s))
          return -1;
        se=GWEN_StringListEntry_Next(se);
      } /* while */
    }
  return 0;
}


AB_TRANSACTION *AB_Transaction_fromDb(GWEN_DB_NODE *db) {
AB_TRANSACTION *st;

  assert(db);
  st=AB_Transaction_new();
  AB_Transaction_SetLocalCountryCode(st, GWEN_DB_GetIntValue(db, "localCountryCode", 0, 280));
  if (1) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "localBankCode");
    if (dbT)  AB_Transaction_SetLocalBankCode(st, GWEN_DB_GetCharValue(db, "localBankCode", 0, 0));
  }
  if (1) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "localAccountNumber");
    if (dbT)  AB_Transaction_SetLocalAccountNumber(st, GWEN_DB_GetCharValue(db, "localAccountNumber", 0, 0));
  }
  if (1) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "localSuffix");
    if (dbT)  AB_Transaction_SetLocalSuffix(st, GWEN_DB_GetCharValue(db, "localSuffix", 0, 0));
  }
  if (1) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "localName");
    if (dbT)  AB_Transaction_SetLocalName(st, GWEN_DB_GetCharValue(db, "localName", 0, 0));
  }
  AB_Transaction_SetRemoteCountryCode(st, GWEN_DB_GetIntValue(db, "remoteCountryCode", 0, 280));
  if (1) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "remoteBankCode");
    if (dbT)  AB_Transaction_SetRemoteBankCode(st, GWEN_DB_GetCharValue(db, "remoteBankCode", 0, 0));
  }
  if (1) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "remoteAccountNumber");
    if (dbT)  AB_Transaction_SetRemoteAccountNumber(st, GWEN_DB_GetCharValue(db, "remoteAccountNumber", 0, 0));
  }
  if (1) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "remoteSuffix");
    if (dbT)  AB_Transaction_SetRemoteSuffix(st, GWEN_DB_GetCharValue(db, "remoteSuffix", 0, 0));
  }
  if (1) {
    int i;

    for (i=0; ; i++) {
      const char *s;

      s=GWEN_DB_GetCharValue(db, "remoteName", i, 0);
      if (!s)
        break;
      AB_Transaction_AddRemoteName(st, s, 0);
    } /* for */
  }
  if (1) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "uniqueId");
    if (dbT)  AB_Transaction_SetUniqueId(st, GWEN_DB_GetCharValue(db, "uniqueId", 0, 0));
  }
  if (1) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "valutaDate");
    if (dbT)  AB_Transaction_SetValutaDate(st, GWEN_Time_fromDb(dbT));
  }
  if (1) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "date");
    if (dbT)  AB_Transaction_SetDate(st, GWEN_Time_fromDb(dbT));
  }
  if (1) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "value");
    if (dbT)  AB_Transaction_SetValue(st, AB_Value_fromDb(dbT));
  }
  AB_Transaction_SetTextKey(st, GWEN_DB_GetIntValue(db, "textKey", 0, 0));
  if (1) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "transactionKey");
    if (dbT)  AB_Transaction_SetTransactionKey(st, GWEN_DB_GetCharValue(db, "transactionKey", 0, 0));
  }
  if (1) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "customerReference");
    if (dbT)  AB_Transaction_SetCustomerReference(st, GWEN_DB_GetCharValue(db, "customerReference", 0, 0));
  }
  if (1) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "bankReference");
    if (dbT)  AB_Transaction_SetBankReference(st, GWEN_DB_GetCharValue(db, "bankReference", 0, 0));
  }
  AB_Transaction_SetTransactionCode(st, GWEN_DB_GetIntValue(db, "transactionCode", 0, 0));
  if (1) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "transactionText");
    if (dbT)  AB_Transaction_SetTransactionText(st, GWEN_DB_GetCharValue(db, "transactionText", 0, 0));
  }
  if (1) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "primanota");
    if (dbT)  AB_Transaction_SetPrimanota(st, GWEN_DB_GetCharValue(db, "primanota", 0, 0));
  }
  if (1) {
    int i;

    for (i=0; ; i++) {
      const char *s;

      s=GWEN_DB_GetCharValue(db, "purpose", i, 0);
      if (!s)
        break;
      AB_Transaction_AddPurpose(st, s, 0);
    } /* for */
  }
  st->_modified=0;
  return st;
}


int AB_Transaction_GetLocalCountryCode(const AB_TRANSACTION *st) {
  assert(st);
  return st->localCountryCode;
}


void AB_Transaction_SetLocalCountryCode(AB_TRANSACTION *st, int d) {
  assert(st);
  st->localCountryCode=d;
  st->_modified=1;
}


const char *AB_Transaction_GetLocalBankCode(const AB_TRANSACTION *st) {
  assert(st);
  return st->localBankCode;
}


void AB_Transaction_SetLocalBankCode(AB_TRANSACTION *st, const char *d) {
  assert(st);
  if (st->localBankCode)
    free(st->localBankCode);
  if (d)
    st->localBankCode=strdup(d);
  else
    st->localBankCode=0;
  st->_modified=1;
}


const char *AB_Transaction_GetLocalAccountNumber(const AB_TRANSACTION *st) {
  assert(st);
  return st->localAccountNumber;
}


void AB_Transaction_SetLocalAccountNumber(AB_TRANSACTION *st, const char *d) {
  assert(st);
  if (st->localAccountNumber)
    free(st->localAccountNumber);
  if (d)
    st->localAccountNumber=strdup(d);
  else
    st->localAccountNumber=0;
  st->_modified=1;
}


const char *AB_Transaction_GetLocalSuffix(const AB_TRANSACTION *st) {
  assert(st);
  return st->localSuffix;
}


void AB_Transaction_SetLocalSuffix(AB_TRANSACTION *st, const char *d) {
  assert(st);
  if (st->localSuffix)
    free(st->localSuffix);
  if (d)
    st->localSuffix=strdup(d);
  else
    st->localSuffix=0;
  st->_modified=1;
}


const char *AB_Transaction_GetLocalName(const AB_TRANSACTION *st) {
  assert(st);
  return st->localName;
}


void AB_Transaction_SetLocalName(AB_TRANSACTION *st, const char *d) {
  assert(st);
  if (st->localName)
    free(st->localName);
  if (d)
    st->localName=strdup(d);
  else
    st->localName=0;
  st->_modified=1;
}


int AB_Transaction_GetRemoteCountryCode(const AB_TRANSACTION *st) {
  assert(st);
  return st->remoteCountryCode;
}


void AB_Transaction_SetRemoteCountryCode(AB_TRANSACTION *st, int d) {
  assert(st);
  st->remoteCountryCode=d;
  st->_modified=1;
}


const char *AB_Transaction_GetRemoteBankCode(const AB_TRANSACTION *st) {
  assert(st);
  return st->remoteBankCode;
}


void AB_Transaction_SetRemoteBankCode(AB_TRANSACTION *st, const char *d) {
  assert(st);
  if (st->remoteBankCode)
    free(st->remoteBankCode);
  if (d)
    st->remoteBankCode=strdup(d);
  else
    st->remoteBankCode=0;
  st->_modified=1;
}


const char *AB_Transaction_GetRemoteAccountNumber(const AB_TRANSACTION *st) {
  assert(st);
  return st->remoteAccountNumber;
}


void AB_Transaction_SetRemoteAccountNumber(AB_TRANSACTION *st, const char *d) {
  assert(st);
  if (st->remoteAccountNumber)
    free(st->remoteAccountNumber);
  if (d)
    st->remoteAccountNumber=strdup(d);
  else
    st->remoteAccountNumber=0;
  st->_modified=1;
}


const char *AB_Transaction_GetRemoteSuffix(const AB_TRANSACTION *st) {
  assert(st);
  return st->remoteSuffix;
}


void AB_Transaction_SetRemoteSuffix(AB_TRANSACTION *st, const char *d) {
  assert(st);
  if (st->remoteSuffix)
    free(st->remoteSuffix);
  if (d)
    st->remoteSuffix=strdup(d);
  else
    st->remoteSuffix=0;
  st->_modified=1;
}


const GWEN_STRINGLIST *AB_Transaction_GetRemoteName(const AB_TRANSACTION *st) {
  assert(st);
  return st->remoteName;
}


void AB_Transaction_SetRemoteName(AB_TRANSACTION *st, const GWEN_STRINGLIST *d) {
  assert(st);
  if (st->remoteName)
    GWEN_StringList_free(st->remoteName);
  if (d)
    st->remoteName=GWEN_StringList_dup(d);
  else
    st->remoteName=0;
  st->_modified=1;
}


void AB_Transaction_AddRemoteName(AB_TRANSACTION *st, const char *d, int chk){
  assert(st);
  assert(d);
  if (GWEN_StringList_AppendString(st->remoteName, d, 0, chk))
    st->_modified=1;
}


void AB_Transaction_RemoveRemoteName(AB_TRANSACTION *st, const char *d) {
  if (GWEN_StringList_RemoveString(st->remoteName, d))
    st->_modified=1;
}


void AB_Transaction_ClearRemoteName(AB_TRANSACTION *st) {
  if (GWEN_StringList_Count(st->remoteName)) {
    GWEN_StringList_Clear(st->remoteName);
    st->_modified=1;
  }
}


int AB_Transaction_HasRemoteName(AB_TRANSACTION *st, const char *d) {
  return GWEN_StringList_HasString(st->remoteName, d);
}


const char *AB_Transaction_GetUniqueId(const AB_TRANSACTION *st) {
  assert(st);
  return st->uniqueId;
}


void AB_Transaction_SetUniqueId(AB_TRANSACTION *st, const char *d) {
  assert(st);
  if (st->uniqueId)
    free(st->uniqueId);
  if (d)
    st->uniqueId=strdup(d);
  else
    st->uniqueId=0;
  st->_modified=1;
}


const GWEN_TIME *AB_Transaction_GetValutaDate(const AB_TRANSACTION *st) {
  assert(st);
  return st->valutaDate;
}


void AB_Transaction_SetValutaDate(AB_TRANSACTION *st, const GWEN_TIME *d) {
  assert(st);
  if (st->valutaDate)
    GWEN_Time_free(st->valutaDate);
  if (d)
    st->valutaDate=GWEN_Time_dup(d);
  else
    st->valutaDate=0;
  st->_modified=1;
}


const GWEN_TIME *AB_Transaction_GetDate(const AB_TRANSACTION *st) {
  assert(st);
  return st->date;
}


void AB_Transaction_SetDate(AB_TRANSACTION *st, const GWEN_TIME *d) {
  assert(st);
  if (st->date)
    GWEN_Time_free(st->date);
  if (d)
    st->date=GWEN_Time_dup(d);
  else
    st->date=0;
  st->_modified=1;
}


const AB_VALUE *AB_Transaction_GetValue(const AB_TRANSACTION *st) {
  assert(st);
  return st->value;
}


void AB_Transaction_SetValue(AB_TRANSACTION *st, const AB_VALUE *d) {
  assert(st);
  if (st->value)
    AB_Value_free(st->value);
  if (d)
    st->value=AB_Value_dup(d);
  else
    st->value=0;
  st->_modified=1;
}


int AB_Transaction_GetTextKey(const AB_TRANSACTION *st) {
  assert(st);
  return st->textKey;
}


void AB_Transaction_SetTextKey(AB_TRANSACTION *st, int d) {
  assert(st);
  st->textKey=d;
  st->_modified=1;
}


const char *AB_Transaction_GetTransactionKey(const AB_TRANSACTION *st) {
  assert(st);
  return st->transactionKey;
}


void AB_Transaction_SetTransactionKey(AB_TRANSACTION *st, const char *d) {
  assert(st);
  if (st->transactionKey)
    free(st->transactionKey);
  if (d)
    st->transactionKey=strdup(d);
  else
    st->transactionKey=0;
  st->_modified=1;
}


const char *AB_Transaction_GetCustomerReference(const AB_TRANSACTION *st) {
  assert(st);
  return st->customerReference;
}


void AB_Transaction_SetCustomerReference(AB_TRANSACTION *st, const char *d) {
  assert(st);
  if (st->customerReference)
    free(st->customerReference);
  if (d)
    st->customerReference=strdup(d);
  else
    st->customerReference=0;
  st->_modified=1;
}


const char *AB_Transaction_GetBankReference(const AB_TRANSACTION *st) {
  assert(st);
  return st->bankReference;
}


void AB_Transaction_SetBankReference(AB_TRANSACTION *st, const char *d) {
  assert(st);
  if (st->bankReference)
    free(st->bankReference);
  if (d)
    st->bankReference=strdup(d);
  else
    st->bankReference=0;
  st->_modified=1;
}


int AB_Transaction_GetTransactionCode(const AB_TRANSACTION *st) {
  assert(st);
  return st->transactionCode;
}


void AB_Transaction_SetTransactionCode(AB_TRANSACTION *st, int d) {
  assert(st);
  st->transactionCode=d;
  st->_modified=1;
}


const char *AB_Transaction_GetTransactionText(const AB_TRANSACTION *st) {
  assert(st);
  return st->transactionText;
}


void AB_Transaction_SetTransactionText(AB_TRANSACTION *st, const char *d) {
  assert(st);
  if (st->transactionText)
    free(st->transactionText);
  if (d)
    st->transactionText=strdup(d);
  else
    st->transactionText=0;
  st->_modified=1;
}


const char *AB_Transaction_GetPrimanota(const AB_TRANSACTION *st) {
  assert(st);
  return st->primanota;
}


void AB_Transaction_SetPrimanota(AB_TRANSACTION *st, const char *d) {
  assert(st);
  if (st->primanota)
    free(st->primanota);
  if (d)
    st->primanota=strdup(d);
  else
    st->primanota=0;
  st->_modified=1;
}


const GWEN_STRINGLIST *AB_Transaction_GetPurpose(const AB_TRANSACTION *st) {
  assert(st);
  return st->purpose;
}


void AB_Transaction_SetPurpose(AB_TRANSACTION *st, const GWEN_STRINGLIST *d) {
  assert(st);
  if (st->purpose)
    GWEN_StringList_free(st->purpose);
  if (d)
    st->purpose=GWEN_StringList_dup(d);
  else
    st->purpose=0;
  st->_modified=1;
}


void AB_Transaction_AddPurpose(AB_TRANSACTION *st, const char *d, int chk){
  assert(st);
  assert(d);
  if (GWEN_StringList_AppendString(st->purpose, d, 0, chk))
    st->_modified=1;
}


void AB_Transaction_RemovePurpose(AB_TRANSACTION *st, const char *d) {
  if (GWEN_StringList_RemoveString(st->purpose, d))
    st->_modified=1;
}


void AB_Transaction_ClearPurpose(AB_TRANSACTION *st) {
  if (GWEN_StringList_Count(st->purpose)) {
    GWEN_StringList_Clear(st->purpose);
    st->_modified=1;
  }
}


int AB_Transaction_HasPurpose(AB_TRANSACTION *st, const char *d) {
  return GWEN_StringList_HasString(st->purpose, d);
}


int AB_Transaction_IsModified(const AB_TRANSACTION *st) {
  assert(st);
  return st->_modified;
}


void AB_Transaction_SetModified(AB_TRANSACTION *st, int i) {
  assert(st);
  st->_modified=i;
}


void AB_Transaction_Attach(AB_TRANSACTION *st) {
  assert(st);
  st->_usage++;
}
AB_TRANSACTION *AB_Transaction_List2__freeAll_cb(AB_TRANSACTION *st, void *user_data) {
  AB_Transaction_free(st);
return 0;
}


void AB_Transaction_List2_freeAll(AB_TRANSACTION_LIST2 *stl) {
  if (stl) {
    AB_Transaction_List2_ForEach(stl, AB_Transaction_List2__freeAll_cb, 0);
    AB_Transaction_List2_free(stl); 
  }
}



