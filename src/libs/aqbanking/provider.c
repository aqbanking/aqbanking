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

#include "provider_p.h"
#include "provider_be.h"
#include "banking_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>


GWEN_INHERIT_FUNCTIONS(AB_PROVIDER)
GWEN_LIST_FUNCTIONS(AB_PROVIDER, AB_Provider)


AB_PROVIDER *AB_Provider_new(AB_BANKING *ab, const char *name){
  AB_PROVIDER *pro;
  GWEN_BUFFER *nbuf;

  assert(ab);
  assert(name);

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (GWEN_Text_EscapeToBufferTolerant(name, nbuf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Bad backend name, aborting.");
    GWEN_Buffer_free(nbuf);
    abort();
  }
  else {
    char *s;

    s=GWEN_Buffer_GetStart(nbuf);
    while(*s) {
      *s=tolower(*s);
      s++;
    }
  }

  GWEN_NEW_OBJECT(AB_PROVIDER, pro);
  pro->usage=1;
  GWEN_INHERIT_INIT(AB_PROVIDER, pro);
  GWEN_LIST_INIT(AB_PROVIDER, pro);

  pro->banking=ab;
  pro->name=strdup(name);
  pro->escName=strdup(GWEN_Buffer_GetStart(nbuf));
  GWEN_Buffer_free(nbuf);

  return pro;
}



void AB_Provider_free(AB_PROVIDER *pro){
  if (pro) {
    assert(pro->usage);
    if (--(pro->usage)==0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Destroying AB_PROVIDER (%s)",
               pro->name);
      GWEN_INHERIT_FINI(AB_PROVIDER, pro);
      if (pro->libLoader) {
        GWEN_LibLoader_CloseLibrary(pro->libLoader);
        GWEN_LibLoader_free(pro->libLoader);
      }
      free(pro->name);
      free(pro->escName);
      GWEN_LIST_FINI(AB_PROVIDER, pro);
      GWEN_FREE_OBJECT(pro);
    }
  }
}



void AB_Provider_SetLibLoader(AB_PROVIDER *pro, GWEN_LIBLOADER *ll) {
  assert(pro);
  pro->libLoader=ll;
}



int AB_Provider_Init(AB_PROVIDER *pro){
  assert(pro);
  if (pro->isInit) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider already is initialized");
    return AB_ERROR_INVALID;
  }
  if (pro->initFn) {
    int rv;
    GWEN_DB_NODE *dbData;

    dbData=AB_Banking_GetProviderData(pro->banking, pro->escName);
    assert(dbData);

    rv=pro->initFn(pro, dbData);
    if (!rv)
      pro->isInit=1;
    return rv;
  }
  DBG_ERROR(AQBANKING_LOGDOMAIN, "No init function set");
  return AB_ERROR_NOFN;
}



int AB_Provider_Fini(AB_PROVIDER *pro){
  assert(pro);
  if (pro->isInit==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider is not initialized");
    return AB_ERROR_INVALID;
  }
  if (pro->finiFn) {
    GWEN_DB_NODE *dbData;

    dbData=AB_Banking_GetProviderData(pro->banking, pro->escName);
    assert(dbData);

    pro->isInit=0;
    return pro->finiFn(pro, dbData);
  }
  DBG_ERROR(AQBANKING_LOGDOMAIN, "No fini function set");
  return AB_ERROR_NOFN;
}



const char *AB_Provider_GetName(const AB_PROVIDER *pro){
  assert(pro);
  return pro->name;
}



const char *AB_Provider_GetEscapedName(const AB_PROVIDER *pro){
  assert(pro);
  return pro->escName;
}



AB_BANKING *AB_Provider_GetBanking(const AB_PROVIDER *pro){
  assert(pro);
  return pro->banking;
}




void AB_Provider_SetInitFn(AB_PROVIDER *pro, AB_PROVIDER_INIT_FN f){
  assert(pro);
  pro->initFn=f;
}



void AB_Provider_SetFiniFn(AB_PROVIDER *pro, AB_PROVIDER_FINI_FN f){
  assert(pro);
  pro->finiFn=f;
}




void AB_Provider_SetUpdateJobFn(AB_PROVIDER *pro, AB_PROVIDER_UPDATEJOB_FN f){
  assert(pro);
  pro->updateJobFn=f;
}



void AB_Provider_SetAddJobFn(AB_PROVIDER *pro, AB_PROVIDER_ADDJOB_FN f){
  assert(pro);
  pro->addJobFn=f;
}



void AB_Provider_SetExecuteFn(AB_PROVIDER *pro, AB_PROVIDER_EXECUTE_FN f){
  assert(pro);
  pro->executeFn=f;
}



void AB_Provider_SetGetAccountListFn(AB_PROVIDER *pro,
                                     AB_PROVIDER_GETACCOUNTLIST_FN f){
  assert(pro);
  pro->getAccountListFn=f;
}



void AB_Provider_SetUpdateAccountFn(AB_PROVIDER *pro,
                                    AB_PROVIDER_UPDATEACCOUNT_FN f){
  assert(pro);
  pro->updateAccountFn=f;
}



void AB_Provider_SetAddAccountFn(AB_PROVIDER *pro,
                                 AB_PROVIDER_ADDACCOUNT_FN f){
  assert(pro);
  pro->addAccountFn=f;
}




int AB_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j){
  assert(pro);
  if (pro->isInit==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider is not initialized");
    return AB_ERROR_INVALID;
  }
  if (pro->updateJobFn) {
    return pro->updateJobFn(pro, j);
  }
  DBG_ERROR(AQBANKING_LOGDOMAIN, "No updateJob function set");
  return AB_ERROR_NOFN;
}



int AB_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j){
  assert(pro);
  if (pro->isInit==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider is not initialized");
    return AB_ERROR_INVALID;
  }
  if (pro->addJobFn) {
    return pro->addJobFn(pro, j);
  }
  DBG_ERROR(AQBANKING_LOGDOMAIN, "No addJob function set");
  return AB_ERROR_NOFN;
}



int AB_Provider_Execute(AB_PROVIDER *pro){
  assert(pro);
  if (pro->isInit==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider is not initialized");
    return AB_ERROR_INVALID;
  }
  if (pro->executeFn) {
    return pro->executeFn(pro);
  }
  DBG_ERROR(AQBANKING_LOGDOMAIN, "No execute function set");
  return AB_ERROR_NOFN;
}



AB_ACCOUNT_LIST2 *AB_Provider_GetAccountList(AB_PROVIDER *pro){
  assert(pro);
  if (pro->isInit==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider is not initialized");
    return 0;
  }
  if (pro->getAccountListFn) {
    return pro->getAccountListFn(pro);
  }
  DBG_ERROR(AQBANKING_LOGDOMAIN, "No getAccountList function set");
  return 0;
}



int AB_Provider_UpdateAccount(AB_PROVIDER *pro, AB_ACCOUNT *a){
  assert(pro);
  if (pro->isInit==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider is not initialized");
    return AB_ERROR_INVALID;
  }
  if (pro->updateAccountFn) {
    return pro->updateAccountFn(pro,a );
  }
  DBG_ERROR(AQBANKING_LOGDOMAIN, "No updateAccount function set");
  return AB_ERROR_NOFN;
}



int AB_Provider_AddAccount(AB_PROVIDER *pro, AB_ACCOUNT *a){
  assert(pro);
  if (pro->isInit==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider is not initialized");
    return AB_ERROR_INVALID;
  }
  if (pro->addAccountFn) {
    return pro->addAccountFn(pro, a);
  }
  DBG_ERROR(AQBANKING_LOGDOMAIN, "No addAccount function set");
  return AB_ERROR_NOFN;
}



int AB_Provider_IsInit(const AB_PROVIDER *pro){
  assert(pro);
  return pro->isInit;
}



GWEN_TYPE_UINT32 AB_Provider_GetFlags(const AB_PROVIDER *pro){
  assert(pro);
  return pro->flags;
}



int AB_Provider_GetUserDataDir(const AB_PROVIDER *pro, GWEN_BUFFER *buf){
  assert(pro);
  assert(buf);
  assert(pro->banking);
  assert(pro->escName);

  return AB_Banking_GetProviderUserDataDir(pro->banking,
                                           pro->escName,
                                           buf);
}



GWEN_DB_NODE *AB_Provider_GetData(AB_PROVIDER *pro) {
  assert(pro);
  assert(pro->banking);
  assert(pro->escName);

  return AB_Banking_GetProviderData(pro->banking,
                                    pro->escName);
}










