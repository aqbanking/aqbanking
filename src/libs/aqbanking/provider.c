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
#include "banking_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>


GWEN_INHERIT_FUNCTIONS(AB_PROVIDER)
GWEN_LIST_FUNCTIONS(AB_PROVIDER, AB_Provider)


AB_PROVIDER *AB_Provider_new(AB_BANKING *ab, const char *name){
  AB_PROVIDER *pro;

  assert(ab);
  assert(name);
  GWEN_NEW_OBJECT(AB_PROVIDER, pro);
  pro->usage=1;
  GWEN_INHERIT_INIT(AB_PROVIDER, pro);
  GWEN_LIST_INIT(AB_PROVIDER, pro);
  pro->banking=ab;
  pro->name=strdup(name);

  return pro;
}



void AB_Provider_free(AB_PROVIDER *pro){
  if (pro) {
    assert(pro->usage);
    if (--(pro->usage)==0) {
      GWEN_INHERIT_FINI(AB_PROVIDER, pro);
      if (pro->libLoader) {
        GWEN_LibLoader_CloseLibrary(pro->libLoader);
        GWEN_LibLoader_free(pro->libLoader);
      }
      free(pro->name);
      GWEN_LIST_FINI(AB_PROVIDER, pro);
      GWEN_FREE_OBJECT(pro);
    }
  }
}



const char *AB_Provider_GetName(const AB_PROVIDER *pro){
  assert(pro);
  return pro->name;
}



AB_BANKING *AB_Provider_GetBanking(const AB_PROVIDER *pro){
  assert(pro);
  return pro->banking;
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



void AB_Provider_SetImportTransactionsFn(AB_PROVIDER *pro,
                                         AB_PROVIDER_IMPORTTRANSACTIONS_FN f){
  assert(pro);
  pro->importTransactionsFn=f;
}





int AB_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j){
  assert(pro);
  if (pro->updateJobFn) {
    return pro->updateJobFn(pro, j);
  }
  DBG_ERROR(0, "No updateJob function set");
  return AB_ERROR_NOFN;
}



int AB_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j){
  assert(pro);
  if (pro->addJobFn) {
    return pro->addJobFn(pro, j);
  }
  DBG_ERROR(0, "No addJob function set");
  return AB_ERROR_NOFN;
}



int AB_Provider_Execute(AB_PROVIDER *pro){
  assert(pro);
  if (pro->executeFn) {
    return pro->executeFn(pro);
  }
  DBG_ERROR(0, "No execute function set");
  return AB_ERROR_NOFN;
}



AB_ACCOUNT_LIST2 *AB_Provider_GetAccountList(AB_PROVIDER *pro){
  assert(pro);
  if (pro->getAccountListFn) {
    return pro->getAccountListFn(pro);
  }
  DBG_ERROR(0, "No getAccountList function set");
  return 0;
}



int AB_Provider_UpdateAccount(AB_PROVIDER *pro, AB_ACCOUNT *a){
  assert(pro);
  if (pro->updateAccountFn) {
    return pro->updateAccountFn(pro,a );
  }
  DBG_ERROR(0, "No updateAccount function set");
  return AB_ERROR_NOFN;
}



int AB_Provider_AddAccount(AB_PROVIDER *pro, AB_ACCOUNT *a){
  assert(pro);
  if (pro->addAccountFn) {
    return pro->addAccountFn(pro, a);
  }
  DBG_ERROR(0, "No addAccount function set");
  return AB_ERROR_NOFN;
}



int AB_Provider_ImportTransactions(AB_PROVIDER *pro,
                                   AB_TRANSACTION_LIST2 *tl,
                                   GWEN_BUFFEREDIO *bio){
  assert(pro);
  if (pro->importTransactionsFn) {
    return pro->importTransactionsFn(pro, tl, bio);
  }
  DBG_ERROR(0, "No importTransactions function set");
  return AB_ERROR_NOFN;
}






AB_PROVIDER *AB_Provider_LoadPluginFile(AB_BANKING *ab,
                                        const char *modname,
                                        const char *fname){
  GWEN_LIBLOADER *ll;
  AB_PROVIDER *pro;
  AB_PROVIDER_FACTORY_FN fn;
  void *p;
  GWEN_BUFFER *nbuf;
  const char *s;
  GWEN_ERRORCODE err;

  ll=GWEN_LibLoader_new();
  if (GWEN_LibLoader_OpenLibrary(ll, fname)) {
    DBG_ERROR(0,
              "Could not load provider plugin \"%s\" (%s)",
              modname, fname);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* create name of init function */
  nbuf=GWEN_Buffer_new(0, 128, 0, 1);
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(nbuf, tolower(*(s++)));
  GWEN_Buffer_AppendString(nbuf, "_factory");

  /* resolve name of factory function */
  err=GWEN_LibLoader_Resolve(ll, GWEN_Buffer_GetStart(nbuf), &p);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(nbuf);
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }
  GWEN_Buffer_free(nbuf);

  fn=(AB_PROVIDER_FACTORY_FN)p;
  assert(fn);
  pro=fn(ab);
  if (!pro) {
    DBG_ERROR(0, "Error in plugin: No provider created");
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* store libloader */
  pro->libLoader=ll;

  return pro;
}



AB_PROVIDER *AB_Provider_LoadPlugin(AB_BANKING *ab,
                                    const char *modname){
  GWEN_LIBLOADER *ll;
  AB_PROVIDER *pro;
  AB_PROVIDER_FACTORY_FN fn;
  void *p;
  const char *s;
  GWEN_ERRORCODE err;
  GWEN_BUFFER *mbuf;

  ll=GWEN_LibLoader_new();
  mbuf=GWEN_Buffer_new(0, 256, 0, 1);
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(mbuf, tolower(*(s++)));
  modname=GWEN_Buffer_GetStart(mbuf);
  if (GWEN_LibLoader_OpenLibraryWithPath(ll,
                                         AQBANKING_PLUGINS
                                         "/"
                                         AB_PROVIDER_FOLDER,
                                         modname)) {
    DBG_ERROR(0, "Could not load provider plugin \"%s\"", modname);
    GWEN_Buffer_free(mbuf);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* create name of init function */
  GWEN_Buffer_AppendString(mbuf, "_factory");

  /* resolve name of factory function */
  err=GWEN_LibLoader_Resolve(ll, GWEN_Buffer_GetStart(mbuf), &p);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(mbuf);
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }
  GWEN_Buffer_free(mbuf);

  fn=(AB_PROVIDER_FACTORY_FN)p;
  assert(fn);
  pro=fn(ab);
  if (!pro) {
    DBG_ERROR(0, "Error in plugin: No provider created");
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* store libloader */
  pro->libLoader=ll;

  return pro;
}







GWEN_INHERIT_FUNCTIONS(AB_PROVIDER_WIZARD)
GWEN_LIST_FUNCTIONS(AB_PROVIDER_WIZARD, AB_ProviderWizard)



AB_PROVIDER_WIZARD *AB_ProviderWizard_new(AB_PROVIDER *pro,
                                            const char *name){
  AB_PROVIDER_WIZARD *pw;

  assert(pro);
  assert(name);
  GWEN_NEW_OBJECT(AB_PROVIDER_WIZARD, pw);
  GWEN_INHERIT_INIT(AB_PROVIDER_WIZARD, pw);
  GWEN_LIST_INIT(AB_PROVIDER_WIZARD, pw);
  pw->provider=pro;
  pw->name=strdup(name);

  return pw;
}



void AB_ProviderWizard_free(AB_PROVIDER_WIZARD *pw){
  if (pw) {
    GWEN_INHERIT_FINI(AB_PROVIDER_WIZARD, pw);
    if (pw->libLoader) {
      GWEN_LibLoader_CloseLibrary(pw->libLoader);
      GWEN_LibLoader_free(pw->libLoader);
    }
    free(pw->name);
    GWEN_LIST_FINI(AB_PROVIDER_WIZARD, pw);
    GWEN_FREE_OBJECT(pw);
  }
}



const char *AB_ProviderWizard_GetName(const AB_PROVIDER_WIZARD *pw){
  assert(pw);
  return pw->name;
}



AB_PROVIDER *AB_ProviderWizard_GetProvider(const AB_PROVIDER_WIZARD *pw){
  assert(pw);
  return pw->provider;
}



GWEN_DB_NODE *AB_ProviderWizard_GetData(AB_PROVIDER_WIZARD *pw){
  assert(pw);
  return AB_Banking_GetWizardData(pw->provider->banking, pw);
}



int AB_ProviderWizard_Setup(AB_PROVIDER_WIZARD *pw){
  assert(pw);

  if (pw->setupFn) {
    return pw->setupFn(pw);
  }
  DBG_ERROR(0, "No setup function set");
  return AB_ERROR_NOFN;
}



void AB_ProviderWizard_SetSetupFn(AB_PROVIDER_WIZARD *pw,
                                   AB_PROVIDER_WIZARD_SETUP_FN f){
  assert(pw);
  pw->setupFn=f;
}




AB_PROVIDER_WIZARD *AB_ProviderWizard_LoadPluginFile(AB_PROVIDER *pro,
                                                       const char *modname,
                                                       const char *fname){
  GWEN_LIBLOADER *ll;
  AB_PROVIDER_WIZARD *pw;
  AB_PROVIDER_WIZARD_FACTORY_FN fn;
  void *p;
  GWEN_BUFFER *nbuf;
  const char *s;
  GWEN_ERRORCODE err;

  ll=GWEN_LibLoader_new();
  if (GWEN_LibLoader_OpenLibrary(ll, fname)) {
    DBG_ERROR(0,
              "Could not load wizard plugin \"%s\" (%s)",
              modname, fname);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* create name of init function */
  nbuf=GWEN_Buffer_new(0, 128, 0, 1);
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(nbuf, tolower(*(s++)));
  GWEN_Buffer_AppendString(nbuf, "_factory");

  /* resolve name of factory function */
  err=GWEN_LibLoader_Resolve(ll, GWEN_Buffer_GetStart(nbuf), &p);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(nbuf);
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }
  GWEN_Buffer_free(nbuf);

  fn=(AB_PROVIDER_WIZARD_FACTORY_FN)p;
  assert(fn);
  pw=fn(pro);
  if (!pw) {
    DBG_ERROR(0, "Error in plugin: No wizard created");
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* store libloader */
  pw->libLoader=ll;

  return pw;
}



AB_PROVIDER_WIZARD *AB_ProviderWizard_LoadPlugin(AB_PROVIDER *pro,
                                                   const char *modname){
  GWEN_LIBLOADER *ll;
  AB_PROVIDER_WIZARD *pw;
  AB_PROVIDER_WIZARD_FACTORY_FN fn;
  void *p;
  GWEN_BUFFER *nbuf;
  const char *s;
  GWEN_ERRORCODE err;
  GWEN_BUFFER *pbuf;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(pbuf,
                           AQBANKING_PLUGINS
                           "/"
                           AB_PROVIDER_WIZARD_FOLDER
                           "/");
  GWEN_Buffer_AppendString(pbuf, AB_Provider_GetName(pro));

  ll=GWEN_LibLoader_new();
  if (GWEN_LibLoader_OpenLibraryWithPath(ll,
                                         GWEN_Buffer_GetStart(pbuf),
                                         modname)) {
    DBG_ERROR(0, "Could not load wizard plugin \"%s\"", modname);
    GWEN_Buffer_free(pbuf);
    GWEN_LibLoader_free(ll);
    return 0;
  }
  GWEN_Buffer_free(pbuf);

  /* create name of init function */
  nbuf=GWEN_Buffer_new(0, 128, 0, 1);
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(nbuf, tolower(*(s++)));
  GWEN_Buffer_AppendString(nbuf, "_factory");

  /* resolve name of factory function */
  err=GWEN_LibLoader_Resolve(ll, GWEN_Buffer_GetStart(nbuf), &p);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(nbuf);
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }
  GWEN_Buffer_free(nbuf);

  fn=(AB_PROVIDER_WIZARD_FACTORY_FN)p;
  assert(fn);
  pw=fn(pro);
  if (!pw) {
    DBG_ERROR(0, "Error in plugin: No wizard created");
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* store libloader */
  pw->libLoader=ll;

  return pw;
}


































