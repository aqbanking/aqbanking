/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Apr 05 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQBANKING_TRANSACTION_H
#define AQBANKING_TRANSACTION_H


#include <gwenhywfar/db.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/misc2.h>


typedef struct AB_TRANSACTION AB_TRANSACTION;

GWEN_LIST_FUNCTION_DEFS(AB_TRANSACTION, AB_Transaction);
GWEN_LIST2_FUNCTION_DEFS(AB_TRANSACTION, AB_Transaction);

#include <aqbanking/value.h>



AB_TRANSACTION * AB_Transaction_new();
void AB_Transaction_free(AB_TRANSACTION *t);

/** @name Reference To Local Account
 *
 */
/*@{*/
int AB_Transaction_GetLocalCountryCode(const AB_TRANSACTION *t);
void AB_Transaction_SetLocalCountryCode(AB_TRANSACTION *t, int i);
const char *AB_Transaction_GetLocalBankCode(const AB_TRANSACTION *t);
void AB_Transaction_SetLocalBankCode(AB_TRANSACTION *t, const char *code);
const char *AB_Transaction_GetLocalAccountId(const AB_TRANSACTION *t);
void AB_Transaction_SetLocalAccountId(AB_TRANSACTION *t, const char *id);
const char *AB_Transaction_GetLocalSuffix(const AB_TRANSACTION *t);
void AB_Transaction_SetLocalSuffix(AB_TRANSACTION *t, const char *id);
const char *AB_Transaction_GetLocalOwnerName(const AB_TRANSACTION *t);
void AB_Transaction_SetLocalOwnerName(AB_TRANSACTION *t, const char *id);
/*@}*/

/** @name Reference To Remote Account
 *
 */
/*@{*/
int AB_Transaction_GetRemoteCountryCode(const AB_TRANSACTION *t);
void AB_Transaction_SetRemoteCountryCode(AB_TRANSACTION *t, int i);
const char *AB_Transaction_GetRemoteBankCode(const AB_TRANSACTION *t);
void AB_Transaction_SetRemoteBankCode(AB_TRANSACTION *t, const char *s);
const char *AB_Transaction_GetRemoteAccountId(const AB_TRANSACTION *t);
void AB_Transaction_SetRemoteAccountId(AB_TRANSACTION *t, const char *s);
const char *AB_Transaction_GetRemoteSuffix(const AB_TRANSACTION *t);
void AB_Transaction_SetGetRemoteSuffix(AB_TRANSACTION *t, const char *id);
const GWEN_STRINGLIST *AB_Transaction_RemoteOwnerName(const AB_TRANSACTION *t);
void AB_Transaction_AddRemoteOwnerName(AB_TRANSACTION *t, const char *s);
/*@}*/


/** @name Dates
 *
 */
/*@{*/
const GWEN_TIME *AB_GetTransaction_GetValutaDate(const AB_TRANSACTION *t);
void AB_Transaction_SetValutaDate(AB_TRANSACTION *t, const GWEN_TIME *d);

const GWEN_TIME *AB_GetTransaction_GetDate(const AB_TRANSACTION *t);
void AB_Transaction_SetDate(AB_TRANSACTION *t, const GWEN_TIME *d);
/*@}*/

/** @name Value
 *
 */
/*@{*/
const AB_VALUE *AB_Transaction_GetValue(const AB_TRANSACTION *t);
void AB_Transaction_SetValue(AB_TRANSACTION *t, const AB_VALUE *v);
/*@}*/


/** @name Special Variables Not Supported by All Backends
 *
 */
/*@{*/
const char *AB_Transaction_GetTransactionKey(const AB_TRANSACTION *t);
void AB_Transaction_SetTransactionKey(AB_TRANSACTION *t, const char *s);
const char *AB_Transaction_GetCustomerReference(const AB_TRANSACTION *t);
void AB_Transaction_SetCustomerReference(AB_TRANSACTION *t, const char *s);
const char *AB_Transaction_GetBankReference(const AB_TRANSACTION *t);
void AB_Transaction_SetBankReference(AB_TRANSACTION *t, const char *s);
int AB_Transaction_GetTransactionCode(const AB_TRANSACTION *t);
void AB_Transaction_SetTransactionCode(AB_TRANSACTION *t, int c);
const char *AB_Transaction_GetTransactionText(const AB_TRANSACTION *t);
void AB_Transaction_SetTransactionText(AB_TRANSACTION *t, const char *s);
const char *AB_Transaction_GetPrimanota(const AB_TRANSACTION *t);
void AB_Transaction_SetPrimanota(AB_TRANSACTION *t, const char *s);
/*@}*/


/** @name Purpose
 *
 */
/*@{*/
const GWEN_STRINGLIST *AB_Transaction_GetPurpose(const AB_TRANSACTION *t);
void AB_Transaction_AddPurpose(AB_TRANSACTION *t, const char *s);
/*@}*/








#endif /* AQBANKING_TRANSACTION_H */


