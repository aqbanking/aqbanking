/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "banking.h"
#include <aqbanking/banking_be.h>
#include <assert.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>



AB_Banking::AB_Banking(const char *appname, const char *fname) {
  assert(appname);
  _banking=AB_Banking_new(appname, fname, 0);
}



AB_Banking::~AB_Banking(){
  DBG_NOTICE(AQBANKING_LOGDOMAIN, "~AB_Banking: Freeing AB_Banking");
  AB_Banking_free(_banking);
}



int AB_Banking::init(){
  return AB_Banking_Init(_banking);
}



int AB_Banking::fini(){
  return AB_Banking_Fini(_banking);
}



int AB_Banking::onlineInit(){
  return AB_Banking_OnlineInit(_banking);
}



int AB_Banking::onlineFini(){
  return AB_Banking_OnlineFini(_banking);
}



const char *AB_Banking::getAppName(){
  return AB_Banking_GetAppName(_banking);
}



std::list<AB_ACCOUNT*> AB_Banking::getAccounts(){
  AB_ACCOUNT_LIST2 *ll;
  std::list<AB_ACCOUNT*> rl;

  ll=AB_Banking_GetAccounts(_banking);
  if (ll) {
    AB_ACCOUNT *a;
    AB_ACCOUNT_LIST2_ITERATOR *it;

    it=AB_Account_List2_First(ll);
    assert(it);
    a=AB_Account_List2Iterator_Data(it);
    assert(a);
    while(a) {
      rl.push_back(a);
      a=AB_Account_List2Iterator_Next(it);
    }
    AB_Account_List2Iterator_free(it);
    AB_Account_List2_free(ll);
  }
  return rl;
}



AB_ACCOUNT *AB_Banking::getAccount(uint32_t uniqueId){
  return AB_Banking_GetAccount(_banking, uniqueId);
}



std::list<AB_USER*> AB_Banking::getUsers() {
  AB_USER_LIST2 *ll;
  std::list<AB_USER*> rl;

  ll=AB_Banking_GetUsers(_banking);
  if (ll) {
    AB_USER *a;
    AB_USER_LIST2_ITERATOR *it;

    it=AB_User_List2_First(ll);
    assert(it);
    a=AB_User_List2Iterator_Data(it);
    assert(a);
    while(a) {
      rl.push_back(a);
      a=AB_User_List2Iterator_Next(it);
    }
    AB_User_List2Iterator_free(it);
    AB_User_List2_free(ll);
  }
  return rl;
}



GWEN_DB_NODE *AB_Banking::getAppData(){
  return AB_Banking_GetAppData(_banking);
}



GWEN_DB_NODE *AB_Banking::getSharedData(const char *name) {
  return AB_Banking_GetSharedData(_banking, name);
}



int AB_Banking::getUserDataDir(GWEN_BUFFER *buf) const{
  return AB_Banking_GetUserDataDir(_banking, buf);
}



int AB_Banking::getAppUserDataDir(GWEN_BUFFER *buf) const{
  return AB_Banking_GetAppUserDataDir(_banking, buf);
}



std::list<GWEN_PLUGIN_DESCRIPTION*> AB_Banking::getProviderDescrs(){
  GWEN_PLUGIN_DESCRIPTION_LIST2 *ll;
  std::list<GWEN_PLUGIN_DESCRIPTION*> rl;

  ll=AB_Banking_GetProviderDescrs(_banking);
  if (ll) {
    GWEN_PLUGIN_DESCRIPTION *d;
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *it;

    it=GWEN_PluginDescription_List2_First(ll);
    assert(it);
    d=GWEN_PluginDescription_List2Iterator_Data(it);
    assert(d);
    while(d) {
      rl.push_back(d);
      d=GWEN_PluginDescription_List2Iterator_Next(it);
    }
    GWEN_PluginDescription_List2Iterator_free(it);
    GWEN_PluginDescription_List2_free(ll);
  }
  return rl;
}



void AB_Banking::clearPluginDescrs(std::list<GWEN_PLUGIN_DESCRIPTION*> &l){
  std::list<GWEN_PLUGIN_DESCRIPTION*>::iterator it;

  for (it=l.begin(); it!=l.end(); it++)
    GWEN_PluginDescription_free(*it);
}



std::list<GWEN_PLUGIN_DESCRIPTION*>
AB_Banking::getWizardDescrs(){
  GWEN_PLUGIN_DESCRIPTION_LIST2 *ll;
  std::list<GWEN_PLUGIN_DESCRIPTION*> rl;

  ll=AB_Banking_GetWizardDescrs(_banking);
  if (ll) {
    GWEN_PLUGIN_DESCRIPTION *d;
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *it;

    it=GWEN_PluginDescription_List2_First(ll);
    assert(it);
    d=GWEN_PluginDescription_List2Iterator_Data(it);
    assert(d);
    while(d) {
      rl.push_back(d);
      d=GWEN_PluginDescription_List2Iterator_Next(it);
    }
    GWEN_PluginDescription_List2Iterator_free(it);
    GWEN_PluginDescription_List2_free(ll);
  }
  return rl;
}



std::list<std::string> AB_Banking::getActiveProviders(){
  const GWEN_STRINGLIST *sl;
  std::list<std::string> l;

  sl=AB_Banking_GetActiveProviders(_banking);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(sl);
    assert(se);
    while(se) {
      const char *p;

      p=GWEN_StringListEntry_Data(se);
      assert(p);
      l.push_back(p);
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }
  return l;
}



std::string AB_Banking::findWizard(const char *frontends){
  GWEN_BUFFER *buf;
  int rv;
  std::string result;

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=AB_Banking_FindWizard(_banking, 0, frontends, buf);
  if (!rv)
    result=GWEN_Buffer_GetStart(buf);
  GWEN_Buffer_free(buf);
  return result;
}



AB_BANKING *AB_Banking::getCInterface(){
  return _banking;
}


AB_PROVIDER *AB_Banking::getProvider(const char *name){
  return AB_Banking_GetProvider(_banking, name);
}



bool AB_Banking::importContext(AB_IMEXPORTER_CONTEXT *ctx,
                            uint32_t flags) {
  return false;
}



int AB_Banking::executeJobs(AB_JOB_LIST2 *jl, AB_IMEXPORTER_CONTEXT *ctx, uint32_t guiid) {
  return AB_Banking_ExecuteJobs(_banking, jl, ctx, guiid);
}

















