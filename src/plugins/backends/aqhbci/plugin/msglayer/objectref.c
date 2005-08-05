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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "objectref_p.h"
#include "aqhbci_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>


GWEN_LIST_FUNCTIONS(AH_OBJECTREF, AH_ObjectRef);


AH_OBJECTREF *AH_ObjectRef_new(const char *t,
                               int country,
                               const char *bankId,
                               const char *accountId,
                               const char *userId,
                               const char *customerId){
  AH_OBJECTREF *o;

  assert(t);
  GWEN_NEW_OBJECT(AH_OBJECTREF, o);
  GWEN_LIST_INIT(AH_OBJECTREF, o);
  o->type=strdup(t);
  o->country=country;
  if (bankId)
    o->bankId=strdup(bankId);
  if (accountId)
    o->accountId=strdup(accountId);
  if (userId)
    o->userId=strdup(userId);
  if (customerId)
    o->customerId=strdup(customerId);

  return o;
}



void AH_ObjectRef_free(AH_OBJECTREF *o){
  if (o) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Destroying AH_OBJECTREF");
    GWEN_LIST_FINI(AH_OBJECTREF, o);
    free(o->customerId);
    free(o->userId);
    free(o->accountId);
    free(o->bankId);
    free(o->type);

    GWEN_FREE_OBJECT(o);
  }
}



const char *AH_ObjectRef_GetType(const AH_OBJECTREF *o){
  assert(o);
  return o->type;
}



int AH_ObjectRef_GetCountry(const AH_OBJECTREF *o){
  assert(o);
  return o->country;
}



const char *AH_ObjectRef_GetBankId(const AH_OBJECTREF *o){
  assert(o);
  return o->bankId;
}



const char *AH_ObjectRef_GetAccountId(const AH_OBJECTREF *o){
  assert(o);
  return o->accountId;
}



const char *AH_ObjectRef_GetUserId(const AH_OBJECTREF *o){
  assert(o);
  return o->userId;
}



const char *AH_ObjectRef_GetCustomerId(const AH_OBJECTREF *o){
  assert(o);
  return o->customerId;
}





