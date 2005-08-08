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

#define GWEN_EXTEND_WAITCALLBACK
#include "waitcallback.h"



typedef struct KBANKING_WAITCALLBACK KBANKING_WAITCALLBACK;
GWEN_INHERIT(GWEN_WAITCALLBACK, KBANKING_WAITCALLBACK)





GWEN_WAITCALLBACK*
  WaitCallback::_instantiate(GWEN_WAITCALLBACK *ctx){
    WaitCallback *wcb;
    WaitCallback *newwcb;

    wcb=(WaitCallback*)GWEN_INHERIT_GETDATA(GWEN_WAITCALLBACK,
                                                  KBANKING_WAITCALLBACK,
                                                  ctx);
    assert(wcb);
    newwcb=wcb->instantiate();
    assert(newwcb);
    return newwcb->_ctx;
  }

GWEN_WAITCALLBACK_RESULT
  WaitCallback::_checkAbort(GWEN_WAITCALLBACK *ctx,
                            unsigned int level){
    WaitCallback *wcb;

    wcb=(WaitCallback*)GWEN_INHERIT_GETDATA(GWEN_WAITCALLBACK,
                                            KBANKING_WAITCALLBACK,
                                            ctx);
    assert(wcb);
    return wcb->checkAbort(level);
  }



void WaitCallback::_log(GWEN_WAITCALLBACK *ctx,
                        unsigned int level,
                        GWEN_LOGGER_LEVEL logLevel,
                        const char *s){
  WaitCallback *wcb;

  wcb=(WaitCallback*)GWEN_INHERIT_GETDATA(GWEN_WAITCALLBACK,
                                          KBANKING_WAITCALLBACK,
                                          ctx);
  assert(wcb);
  wcb->log(level, logLevel, s);
}


void WaitCallback::_freeData(void *bp, void *p) {
  assert(p);
  delete (WaitCallback*)p;
}





WaitCallback::WaitCallback(const char *id){
  KBANKING_WAITCALLBACK *pp;

  _ctx=GWEN_WaitCallback_new(id);
  pp=(KBANKING_WAITCALLBACK*)this;
  GWEN_INHERIT_SETDATA(GWEN_WAITCALLBACK, KBANKING_WAITCALLBACK,
                       _ctx, pp, _freeData);
  GWEN_WaitCallback_SetCheckAbortFn(_ctx, _checkAbort);
  GWEN_WaitCallback_SetInstantiateFn(_ctx, _instantiate);
  GWEN_WaitCallback_SetLogFn(_ctx, _log);
}



WaitCallback::~WaitCallback(){
}



WaitCallback *WaitCallback::instantiate(){
  return 0;
}



GWEN_WAITCALLBACK_RESULT WaitCallback::checkAbort(unsigned int level){
  return GWEN_WaitCallbackResult_Continue;
}



void WaitCallback::log(unsigned int level,
                       GWEN_LOGGER_LEVEL loglevel,
                       const char *s){
}





int WaitCallback::getDistance(){
  return GWEN_WaitCallback_GetDistance(_ctx);
}



int WaitCallback::registerCallback(){
  return GWEN_WaitCallback_Register(_ctx);
}


int WaitCallback::unregisterCallback(){
  return GWEN_WaitCallback_Unregister(_ctx);
}


const char *WaitCallback::getId(){
  return GWEN_WaitCallback_GetId(_ctx);
}



GWEN_TYPE_UINT64 WaitCallback::getProgressPos(){
  return GWEN_WaitCallback_GetProgressPos(_ctx);
}



GWEN_TYPE_UINT64 WaitCallback::getProgressTotal(){
  return GWEN_WaitCallback_GetProgressTotal(_ctx);
}



time_t WaitCallback::lastCalled(){
  return GWEN_WaitCallback_LastCalled(_ctx);
}



time_t WaitCallback::lastEntered(){
  return GWEN_WaitCallback_LastEntered(_ctx);
}



void WaitCallback::setDistance(int d){
  GWEN_WaitCallback_SetDistance(_ctx, d);
}



int WaitCallback::nestingLevel(){
  return GWEN_WaitCallback_GetNestingLevel(_ctx);
}









