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


#include "banking_p.h"
#include <aqbanking/banking_be.h>
#include <assert.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>


namespace AB {


  GWEN_INHERIT(AB_BANKING, Banking);




  int Banking_Linker::MessageBox(AB_BANKING *ab,
                                 GWEN_TYPE_UINT32 flags,
                                 const char *title,
                                 const char *text,
                                 const char *b1,
                                 const char *b2,
                                 const char *b3){
    Banking *kb;

    assert(ab);
    kb=GWEN_INHERIT_GETDATA(AB_BANKING, Banking, ab);
    assert(kb);

    return kb->messageBox(flags, title, text, b1, b2, b3);
  }


  
  int Banking_Linker::InputBox(AB_BANKING *ab,
                               GWEN_TYPE_UINT32 flags,
                               const char *title,
                               const char *text,
                               char *buffer,
                               int minLen,
                               int maxLen){
    Banking *kb;

    assert(ab);
    kb=GWEN_INHERIT_GETDATA(AB_BANKING, Banking, ab);
    assert(kb);
  
    return kb->inputBox(flags, title, text, buffer, minLen, maxLen);
  }
  
  
  
  GWEN_TYPE_UINT32 Banking_Linker::ShowBox(AB_BANKING *ab,
                                           GWEN_TYPE_UINT32 flags,
                                           const char *title,
                                           const char *text){
    Banking *kb;
  
    assert(ab);
    kb=GWEN_INHERIT_GETDATA(AB_BANKING, Banking, ab);
    assert(kb);
  
    return kb->showBox(flags, title, text);
  }
  
  
  
  void Banking_Linker::HideBox(AB_BANKING *ab, GWEN_TYPE_UINT32 id){
    Banking *kb;
  
    assert(ab);
    kb=GWEN_INHERIT_GETDATA(AB_BANKING, Banking, ab);
    assert(kb);
  
    return kb->hideBox(id);
  }
  
  
  
  GWEN_TYPE_UINT32 Banking_Linker::ProgressStart(AB_BANKING *ab,
                                                 const char *title,
                                                 const char *text,
                                                 GWEN_TYPE_UINT32 total){
    Banking *kb;
  
    assert(ab);
    kb=GWEN_INHERIT_GETDATA(AB_BANKING, Banking, ab);
    assert(kb);
  
    return kb->progressStart(title, text, total);
  }
  
  
  
  int Banking_Linker::ProgressAdvance(AB_BANKING *ab,
                                      GWEN_TYPE_UINT32 id,
                                      GWEN_TYPE_UINT32 progress){
    Banking *kb;
  
    assert(ab);
    kb=GWEN_INHERIT_GETDATA(AB_BANKING, Banking, ab);
    assert(kb);
  
    return kb->progressAdvance(id, progress);
  }
  
  
  
  int Banking_Linker::ProgressLog(AB_BANKING *ab,
                                  GWEN_TYPE_UINT32 id,
                                  AB_BANKING_LOGLEVEL level,
                                  const char *text){
    Banking *kb;
  
    assert(ab);
    kb=GWEN_INHERIT_GETDATA(AB_BANKING, Banking, ab);
    assert(kb);
  
    return kb->progressLog(id, level, text);
  }
  
  
  
  int Banking_Linker::ProgressEnd(AB_BANKING *ab, GWEN_TYPE_UINT32 id){
    Banking *kb;
  
    assert(ab);
    kb=GWEN_INHERIT_GETDATA(AB_BANKING, Banking, ab);
    assert(kb);
  
    return kb->progressEnd(id);
  }
  
  
  void Banking_Linker::freeData(void *bp, void *p) {
    Banking *kb;

    DBG_NOTICE(AQBANKING_LOGDOMAIN, "Banking_Linker: Freeing Banking");
    kb=(Banking*)p;
    if (kb->_banking) {
      kb->_banking=0;
    }
    delete kb;
  }
  
  
  
  
  
  
  Banking::Banking(const char *appname,
                   const char *fname) {
    assert(appname);
    _banking=AB_Banking_new(appname, fname);
    GWEN_INHERIT_SETDATA(AB_BANKING, Banking,
                         _banking, this,
                         Banking_Linker::freeData);
    AB_Banking_SetMessageBoxFn(_banking, Banking_Linker::MessageBox);
    AB_Banking_SetInputBoxFn(_banking, Banking_Linker::InputBox);
    AB_Banking_SetShowBoxFn(_banking, Banking_Linker::ShowBox);
    AB_Banking_SetHideBoxFn(_banking, Banking_Linker::HideBox);
    AB_Banking_SetProgressStartFn(_banking, Banking_Linker::ProgressStart);
    AB_Banking_SetProgressAdvanceFn(_banking, Banking_Linker::ProgressAdvance);
    AB_Banking_SetProgressLogFn(_banking, Banking_Linker::ProgressLog);
    AB_Banking_SetProgressEndFn(_banking, Banking_Linker::ProgressEnd);
  }

  
  
  Banking::~Banking(){
    DBG_NOTICE(AQBANKING_LOGDOMAIN, "~Banking: Freeing Banking");
    if (_banking) {
      GWEN_INHERIT_UNLINK(AB_BANKING, Banking, _banking)
      AB_Banking_free(_banking);
    }
  }
  
  
  
  int Banking::messageBox(GWEN_TYPE_UINT32 flags,
                          const char *title,
                          const char *text,
                          const char *b1,
                          const char *b2,
                          const char *b3){
    return AB_ERROR_NOT_SUPPORTED;
  }
  
  
  
  int Banking::inputBox(GWEN_TYPE_UINT32 flags,
                        const char *title,
                        const char *text,
                        char *buffer,
                        int minLen,
                        int maxLen){
    return AB_ERROR_NOT_SUPPORTED;
  }
  
  
  
  GWEN_TYPE_UINT32 Banking::showBox(GWEN_TYPE_UINT32 flags,
                                    const char *title,
                                    const char *text){
    return 0;
  }
  
  
  
  void Banking::hideBox(GWEN_TYPE_UINT32 id){
  }
  
  
  
  GWEN_TYPE_UINT32 Banking::progressStart(const char *title,
                                          const char *text,
                                          GWEN_TYPE_UINT32 total){
    return 0;
  }
  
  
  
  int Banking::progressAdvance(GWEN_TYPE_UINT32 id,
                               GWEN_TYPE_UINT32 progress){
    return AB_ERROR_NOT_SUPPORTED;
  }
  
  
  
  int Banking::progressLog(GWEN_TYPE_UINT32 id,
                           AB_BANKING_LOGLEVEL level,
                           const char *text){
    return AB_ERROR_NOT_SUPPORTED;
  }
  
  
  
  int Banking::progressEnd(GWEN_TYPE_UINT32 id){
    return AB_ERROR_NOT_SUPPORTED;
  }
  
  
  
  int Banking::init(){
    return AB_Banking_Init(_banking);
  }
  
  
  
  int Banking::fini(){
    return AB_Banking_Fini(_banking);
  }
  
  
  
  const char *Banking::getAppName(){
    return AB_Banking_GetAppName(_banking);
  }
  
  
  
  std::list<AB_ACCOUNT*> Banking::getAccounts(){
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
  
  
  
  AB_ACCOUNT *Banking::getAccount(GWEN_TYPE_UINT32 uniqueId){
    return AB_Banking_GetAccount(_banking, uniqueId);
  }
  
  
  
  GWEN_DB_NODE *Banking::getAppData(){
    return AB_Banking_GetAppData(_banking);
  }


  int Banking::getUserDataDir(GWEN_BUFFER *buf) const{
    return AB_Banking_GetUserDataDir(_banking, buf);
  }



  int Banking::getAppUserDataDir(GWEN_BUFFER *buf) const{
    return AB_Banking_GetAppUserDataDir(_banking, buf);
  }

  
  
  std::list<GWEN_PLUGIN_DESCRIPTION*> Banking::getProviderDescrs(){
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
  
  
  
  void Banking::clearPluginDescrs(std::list<GWEN_PLUGIN_DESCRIPTION*> &l){
    std::list<GWEN_PLUGIN_DESCRIPTION*>::iterator it;
  
    for (it=l.begin(); it!=l.end(); it++)
      GWEN_PluginDescription_free(*it);
  }
  
  
  
  std::list<GWEN_PLUGIN_DESCRIPTION*>
  Banking::getWizardDescrs(const char *pn){
    GWEN_PLUGIN_DESCRIPTION_LIST2 *ll;
    std::list<GWEN_PLUGIN_DESCRIPTION*> rl;
  
    ll=AB_Banking_GetWizardDescrs(_banking, pn);
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
  
  
  
  int Banking::enqueueJob(AB_JOB *j){
    return AB_Banking_EnqueueJob(_banking, j);
  }
  
  
  
  int Banking::dequeueJob(AB_JOB *j){
    return AB_Banking_DequeueJob(_banking, j);
  }
  
  
  
  int Banking::executeQueue(){
    return AB_Banking_ExecuteQueue(_banking);
  }
  
  
  
  std::list<AB_JOB*> Banking::getEnqueuedJobs(){
    AB_JOB_LIST2 *ll;
    std::list<AB_JOB*> rl;
  
    ll=AB_Banking_GetEnqueuedJobs(_banking);
    if (ll) {
      AB_JOB *j;
      AB_JOB_LIST2_ITERATOR *it;
  
      it=AB_Job_List2_First(ll);
      assert(it);
      j=AB_Job_List2Iterator_Data(it);
      assert(j);
      while(j) {
        rl.push_back(j);
        j=AB_Job_List2Iterator_Next(it);
      }
      AB_Job_List2Iterator_free(it);
      AB_Job_List2_free(ll);
    }
    return rl;
  }



  int Banking::activateProvider(const char *pname){
    return AB_Banking_ActivateProvider(_banking, pname);
  }



  int Banking::deactivateProvider(const char *pname){
    return AB_Banking_DeactivateProvider(_banking, pname);
  }


  int Banking::suspendProvider(const char *pname){
    return AB_Banking_SuspendProvider(_banking, pname);
  }



  int Banking::resumeProvider(const char *pname){
    return AB_Banking_ResumeProvider(_banking, pname);
  }



  std::list<std::string> Banking::getActiveProviders(){
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



  std::string Banking::findWizard(const char *backend,
                                  const char *frontends){
    GWEN_BUFFER *buf;
    int rv;
    std::string result;

    buf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=AB_Banking_FindWizard(_banking, backend, frontends, buf);
    if (!rv)
      result=GWEN_Buffer_GetStart(buf);
    GWEN_Buffer_free(buf);
    return result;
  }



  AB_BANKING *Banking::getCInterface(){
    return _banking;
  }


  AB_PROVIDER *Banking::getProvider(const char *name){
    return AB_Banking_GetProvider(_banking, name);
  }


} /* namespace */



















