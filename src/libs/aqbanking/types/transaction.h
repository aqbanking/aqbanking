/* This is a generated file. Please do not edit. */
#ifndef TRANSACTION_H
#define TRANSACTION_H

/** @page P_AB_TRANSACTION_PUBLIC AB_Transaction (public)
This page describes the properties of AB_TRANSACTION
<h3>Local Account Info</h3>
<p>
This group contains information about the local account.
</p>
@anchor AB_TRANSACTION_LocalCountryCode
<h4>LocalCountryCode</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetLocalCountryCode, 
get it with @ref AB_Transaction_GetLocalCountryCode
</p>

@anchor AB_TRANSACTION_LocalBankCode
<h4>LocalBankCode</h4>
<p>
This is the code of the local bank (i.e.
<b>
your
</b>
bank).
</p>
<p>
Set this property with @ref AB_Transaction_SetLocalBankCode, 
get it with @ref AB_Transaction_GetLocalBankCode
</p>

@anchor AB_TRANSACTION_LocalAccountNumber
<h4>LocalAccountNumber</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetLocalAccountNumber, 
get it with @ref AB_Transaction_GetLocalAccountNumber
</p>

@anchor AB_TRANSACTION_LocalSuffix
<h4>LocalSuffix</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetLocalSuffix, 
get it with @ref AB_Transaction_GetLocalSuffix
</p>

@anchor AB_TRANSACTION_LocalName
<h4>LocalName</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetLocalName, 
get it with @ref AB_Transaction_GetLocalName
</p>

<h3>Remote Account Info</h3>
<p>
This group contains information about the remote account.
</p>
@anchor AB_TRANSACTION_RemoteCountryCode
<h4>RemoteCountryCode</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetRemoteCountryCode, 
get it with @ref AB_Transaction_GetRemoteCountryCode
</p>

@anchor AB_TRANSACTION_RemoteBankCode
<h4>RemoteBankCode</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetRemoteBankCode, 
get it with @ref AB_Transaction_GetRemoteBankCode
</p>

@anchor AB_TRANSACTION_RemoteAccountNumber
<h4>RemoteAccountNumber</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetRemoteAccountNumber, 
get it with @ref AB_Transaction_GetRemoteAccountNumber
</p>

@anchor AB_TRANSACTION_RemoteSuffix
<h4>RemoteSuffix</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetRemoteSuffix, 
get it with @ref AB_Transaction_GetRemoteSuffix
</p>

@anchor AB_TRANSACTION_RemoteName
<h4>RemoteName</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetRemoteName, 
get it with @ref AB_Transaction_GetRemoteName
</p>

@anchor AB_TRANSACTION_UniqueId
<h3>UniqueId</h3>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetUniqueId, 
get it with @ref AB_Transaction_GetUniqueId
</p>

<h3>Dates</h3>
<p>
</p>
@anchor AB_TRANSACTION_ValutaDate
<h4>ValutaDate</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetValutaDate, 
get it with @ref AB_Transaction_GetValutaDate
</p>

@anchor AB_TRANSACTION_Date
<h4>Date</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetDate, 
get it with @ref AB_Transaction_GetDate
</p>

<h3>Value</h3>
<p>
</p>
@anchor AB_TRANSACTION_Value
<h4>Value</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetValue, 
get it with @ref AB_Transaction_GetValue
</p>

<h3>Info Which Is Not Supported by All Backends</h3>
<p>
<p>
This group contains information which differ between backends.
</p>
<p>
Some of this information might not even be
<b>
supported
</b>
by every backends.
</p>
</p>
@anchor AB_TRANSACTION_TextKey
<h4>TextKey</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetTextKey, 
get it with @ref AB_Transaction_GetTextKey
</p>

@anchor AB_TRANSACTION_TransactionKey
<h4>TransactionKey</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetTransactionKey, 
get it with @ref AB_Transaction_GetTransactionKey
</p>

@anchor AB_TRANSACTION_CustomerReference
<h4>CustomerReference</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetCustomerReference, 
get it with @ref AB_Transaction_GetCustomerReference
</p>

@anchor AB_TRANSACTION_BankReference
<h4>BankReference</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetBankReference, 
get it with @ref AB_Transaction_GetBankReference
</p>

@anchor AB_TRANSACTION_TransactionCode
<h4>TransactionCode</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetTransactionCode, 
get it with @ref AB_Transaction_GetTransactionCode
</p>

@anchor AB_TRANSACTION_TransactionText
<h4>TransactionText</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetTransactionText, 
get it with @ref AB_Transaction_GetTransactionText
</p>

@anchor AB_TRANSACTION_Primanota
<h4>Primanota</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetPrimanota, 
get it with @ref AB_Transaction_GetPrimanota
</p>

@anchor AB_TRANSACTION_Purpose
<h4>Purpose</h4>
<p>
</p>
<p>
Set this property with @ref AB_Transaction_SetPurpose, 
get it with @ref AB_Transaction_GetPurpose
</p>

*/
#include <gwenhywfar/db.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/list2.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/stringlist.h>
#include <aqbanking/value.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AB_TRANSACTION AB_TRANSACTION;

GWEN_INHERIT_FUNCTION_LIB_DEFS(AB_TRANSACTION, AQBANKING_API)
GWEN_LIST2_FUNCTION_LIB_DEFS(AB_TRANSACTION, AB_Transaction, AQBANKING_API)

void AB_Transaction_List2_freeAll(AB_TRANSACTION_LIST2 *stl);

AB_TRANSACTION *AB_Transaction_new();
void AB_Transaction_free(AB_TRANSACTION *st);
void AB_Transaction_Attach(AB_TRANSACTION *st);
AB_TRANSACTION *AB_Transaction_dup(const AB_TRANSACTION*st);AB_TRANSACTION *AB_Transaction_fromDb(GWEN_DB_NODE *db);int AB_Transaction_toDb(const AB_TRANSACTION*st, GWEN_DB_NODE *db);int AB_Transaction_IsModified(const AB_TRANSACTION *st);
void AB_Transaction_SetModified(AB_TRANSACTION *st, int i);

/** @name Local Account Info
 *
This group contains information about the local account.
*/
/*@{*/

/**
* Returns the property @ref AB_TRANSACTION_LocalCountryCode
*/
int AB_Transaction_GetLocalCountryCode(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_LocalCountryCode
*/
void AB_Transaction_SetLocalCountryCode(AB_TRANSACTION *el, int d);

/**
* Returns the property @ref AB_TRANSACTION_LocalBankCode
*/
const char *AB_Transaction_GetLocalBankCode(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_LocalBankCode
*/
void AB_Transaction_SetLocalBankCode(AB_TRANSACTION *el, const char *d);

/**
* Returns the property @ref AB_TRANSACTION_LocalAccountNumber
*/
const char *AB_Transaction_GetLocalAccountNumber(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_LocalAccountNumber
*/
void AB_Transaction_SetLocalAccountNumber(AB_TRANSACTION *el, const char *d);

/**
* Returns the property @ref AB_TRANSACTION_LocalSuffix
*/
const char *AB_Transaction_GetLocalSuffix(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_LocalSuffix
*/
void AB_Transaction_SetLocalSuffix(AB_TRANSACTION *el, const char *d);

/**
* Returns the property @ref AB_TRANSACTION_LocalName
*/
const char *AB_Transaction_GetLocalName(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_LocalName
*/
void AB_Transaction_SetLocalName(AB_TRANSACTION *el, const char *d);

/*@}*/

/** @name Remote Account Info
 *
This group contains information about the remote account.
*/
/*@{*/

/**
* Returns the property @ref AB_TRANSACTION_RemoteCountryCode
*/
int AB_Transaction_GetRemoteCountryCode(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_RemoteCountryCode
*/
void AB_Transaction_SetRemoteCountryCode(AB_TRANSACTION *el, int d);

/**
* Returns the property @ref AB_TRANSACTION_RemoteBankCode
*/
const char *AB_Transaction_GetRemoteBankCode(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_RemoteBankCode
*/
void AB_Transaction_SetRemoteBankCode(AB_TRANSACTION *el, const char *d);

/**
* Returns the property @ref AB_TRANSACTION_RemoteAccountNumber
*/
const char *AB_Transaction_GetRemoteAccountNumber(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_RemoteAccountNumber
*/
void AB_Transaction_SetRemoteAccountNumber(AB_TRANSACTION *el, const char *d);

/**
* Returns the property @ref AB_TRANSACTION_RemoteSuffix
*/
const char *AB_Transaction_GetRemoteSuffix(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_RemoteSuffix
*/
void AB_Transaction_SetRemoteSuffix(AB_TRANSACTION *el, const char *d);

/**
* Returns the property @ref AB_TRANSACTION_RemoteName
*/
const GWEN_STRINGLIST *AB_Transaction_GetRemoteName(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_RemoteName
*/
void AB_Transaction_SetRemoteName(AB_TRANSACTION *el, const GWEN_STRINGLIST *d);
void AB_Transaction_AddRemoteName(AB_TRANSACTION *st, const char *d, int chk);
void AB_Transaction_RemoveRemoteName(AB_TRANSACTION *st, const char *d);
void AB_Transaction_ClearRemoteName(AB_TRANSACTION *st);
int AB_Transaction_HasRemoteName(AB_TRANSACTION *st, const char *d);

/*@}*/

/**
* Returns the property @ref AB_TRANSACTION_UniqueId
*/
const char *AB_Transaction_GetUniqueId(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_UniqueId
*/
void AB_Transaction_SetUniqueId(AB_TRANSACTION *el, const char *d);

/** @name Dates
*/
/*@{*/
/**
* Returns the property @ref AB_TRANSACTION_ValutaDate
*/
const GWEN_TIME *AB_Transaction_GetValutaDate(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_ValutaDate
*/
void AB_Transaction_SetValutaDate(AB_TRANSACTION *el, const GWEN_TIME *d);

/**
* Returns the property @ref AB_TRANSACTION_Date
*/
const GWEN_TIME *AB_Transaction_GetDate(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_Date
*/
void AB_Transaction_SetDate(AB_TRANSACTION *el, const GWEN_TIME *d);

/*@}*/

/** @name Value
*/
/*@{*/
/**
* Returns the property @ref AB_TRANSACTION_Value
*/
const AB_VALUE *AB_Transaction_GetValue(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_Value
*/
void AB_Transaction_SetValue(AB_TRANSACTION *el, const AB_VALUE *d);

/*@}*/

/** @name Info Which Is Not Supported by All Backends
 *
<p>
This group contains information which differ between backends.
</p>
<p>
Some of this information might not even be
<b>
supported
</b>
by every backends.
</p>
*/
/*@{*/

/**
* Returns the property @ref AB_TRANSACTION_TextKey
*/
int AB_Transaction_GetTextKey(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_TextKey
*/
void AB_Transaction_SetTextKey(AB_TRANSACTION *el, int d);

/**
* Returns the property @ref AB_TRANSACTION_TransactionKey
*/
const char *AB_Transaction_GetTransactionKey(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_TransactionKey
*/
void AB_Transaction_SetTransactionKey(AB_TRANSACTION *el, const char *d);

/**
* Returns the property @ref AB_TRANSACTION_CustomerReference
*/
const char *AB_Transaction_GetCustomerReference(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_CustomerReference
*/
void AB_Transaction_SetCustomerReference(AB_TRANSACTION *el, const char *d);

/**
* Returns the property @ref AB_TRANSACTION_BankReference
*/
const char *AB_Transaction_GetBankReference(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_BankReference
*/
void AB_Transaction_SetBankReference(AB_TRANSACTION *el, const char *d);

/**
* Returns the property @ref AB_TRANSACTION_TransactionCode
*/
int AB_Transaction_GetTransactionCode(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_TransactionCode
*/
void AB_Transaction_SetTransactionCode(AB_TRANSACTION *el, int d);

/**
* Returns the property @ref AB_TRANSACTION_TransactionText
*/
const char *AB_Transaction_GetTransactionText(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_TransactionText
*/
void AB_Transaction_SetTransactionText(AB_TRANSACTION *el, const char *d);

/**
* Returns the property @ref AB_TRANSACTION_Primanota
*/
const char *AB_Transaction_GetPrimanota(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_Primanota
*/
void AB_Transaction_SetPrimanota(AB_TRANSACTION *el, const char *d);

/**
* Returns the property @ref AB_TRANSACTION_Purpose
*/
const GWEN_STRINGLIST *AB_Transaction_GetPurpose(const AB_TRANSACTION *el);
/**
* Set the property @ref AB_TRANSACTION_Purpose
*/
void AB_Transaction_SetPurpose(AB_TRANSACTION *el, const GWEN_STRINGLIST *d);
void AB_Transaction_AddPurpose(AB_TRANSACTION *st, const char *d, int chk);
void AB_Transaction_RemovePurpose(AB_TRANSACTION *st, const char *d);
void AB_Transaction_ClearPurpose(AB_TRANSACTION *st);
int AB_Transaction_HasPurpose(AB_TRANSACTION *st, const char *d);

/*@}*/


#ifdef __cplusplus
} /* __cplusplus */
#endif


#endif /* TRANSACTION_H */
