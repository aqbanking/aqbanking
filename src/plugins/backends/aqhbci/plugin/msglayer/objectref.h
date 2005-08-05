/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_OBJECTREF_H
#define AH_OBJECTREF_H


#include <aqhbci/aqhbci.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct AH_OBJECTREF AH_OBJECTREF;
#ifdef __cplusplus
}
#endif

#include <gwenhywfar/misc.h>

#ifdef __cplusplus
extern "C" {
#endif

GWEN_LIST_FUNCTION_LIB_DEFS(AH_OBJECTREF, AH_ObjectRef, AQHBCI_API);

AQHBCI_API
AH_OBJECTREF *AH_ObjectRef_new(const char *t,
                               int country,
                               const char *bankId,
                               const char *accountId,
                               const char *userId,
                               const char *customerId);

AQHBCI_API
void AH_ObjectRef_free(AH_OBJECTREF *o);

AQHBCI_API
const char *AH_ObjectRef_GetType(const AH_OBJECTREF *o);

AQHBCI_API
int AH_ObjectRef_GetCountry(const AH_OBJECTREF *o);
AQHBCI_API
const char *AH_ObjectRef_GetBankId(const AH_OBJECTREF *o);
AQHBCI_API
const char *AH_ObjectRef_GetAccountId(const AH_OBJECTREF *o);
AQHBCI_API
const char *AH_ObjectRef_GetUserId(const AH_OBJECTREF *o);
AQHBCI_API
const char *AH_ObjectRef_GetCustomerId(const AH_OBJECTREF *o);

#ifdef __cplusplus
}
#endif




#endif /* AH_OBJECTREF_H */



