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
#include "qbwaitcallback.h"



typedef struct QBANKING_WAITCALLBACK QBANKING_WAITCALLBACK;
GWEN_INHERIT(GWEN_WAITCALLBACK, QBANKING_WAITCALLBACK)





GWEN_WAITCALLBACK*
  QBWaitCallback::_instantiate(GWEN_WAITCALLBACK *ctx){
    QBWaitCallback *wcb;
    QBWaitCallback *newwcb;

    wcb=(QBWaitCallback*)GWEN_INHERIT_GETDATA(GWEN_WAITCALLBACK,
                                                  QBANKING_WAITCALLBACK,
                                                  ctx);
    assert(wcb);
    newwcb=wcb->instantiate();
    assert(newwcb);
    return newwcb->_ctx;
  }

GWEN_WAITCALLBACK_RESULT
  QBWaitCallback::_checkAbort(GWEN_WAITCALLBACK *ctx,
                            unsigned int level){
    QBWaitCallback *wcb;

    wcb=(QBWaitCallback*)GWEN_INHERIT_GETDATA(GWEN_WAITCALLBACK,
                                            QBANKING_WAITCALLBACK,
                                            ctx);
    assert(wcb);
    return wcb->checkAbort(level);
  }



void QBWaitCallback::_log(GWEN_WAITCALLBACK *ctx,
                        unsigned int level,
                        GWEN_LOGGER_LEVEL logLevel,
                        const char *s){
  QBWaitCallback *wcb;

  wcb=(QBWaitCallback*)GWEN_INHERIT_GETDATA(GWEN_WAITCALLBACK,
                                          QBANKING_WAITCALLBACK,
                                          ctx);
  assert(wcb);
  wcb->log(level, logLevel, s);
}


void QBWaitCallback::_freeData(void *bp, void *p) {
  assert(p);
  delete (QBWaitCallback*)p;
}





QBWaitCallback::QBWaitCallback(const char *id){
  QBANKING_WAITCALLBACK *pp;

  _ctx=GWEN_WaitCallback_new(id);
  pp=(QBANKING_WAITCALLBACK*)this;
  GWEN_INHERIT_SETDATA(GWEN_WAITCALLBACK, QBANKING_WAITCALLBACK,
                       _ctx, pp, _freeData);
  GWEN_WaitCallback_SetCheckAbortFn(_ctx, _checkAbort);
  GWEN_WaitCallback_SetInstantiateFn(_ctx, _instantiate);
  GWEN_WaitCallback_SetLogFn(_ctx, _log);
}



QBWaitCallback::~QBWaitCallback(){
}



QBWaitCallback *QBWaitCallback::instantiate(){
  return 0;
}



GWEN_WAITCALLBACK_RESULT QBWaitCallback::checkAbort(unsigned int level){
  return GWEN_WaitCallbackResult_Continue;
}



void QBWaitCallback::log(unsigned int level,
                       GWEN_LOGGER_LEVEL loglevel,
                       const char *s){
}





int QBWaitCallback::getDistance() const {
  return GWEN_WaitCallback_GetDistance(_ctx);
}



int QBWaitCallback::registerCallback(){
  return GWEN_WaitCallback_Register(_ctx);
}


int QBWaitCallback::unregisterCallback(){
  return GWEN_WaitCallback_Unregister(_ctx);
}


const char *QBWaitCallback::getId() const {
  return GWEN_WaitCallback_GetId(_ctx);
}



const char *QBWaitCallback::getText() const {
  return GWEN_WaitCallback_GetText(_ctx);
}



const char *QBWaitCallback::getUnits() const {
  return GWEN_WaitCallback_GetUnits(_ctx);
}



GWEN_TYPE_UINT64 QBWaitCallback::getProgressPos() const{
  return GWEN_WaitCallback_GetProgressPos(_ctx);
}



GWEN_TYPE_UINT64 QBWaitCallback::getProgressTotal() const {
  return GWEN_WaitCallback_GetProgressTotal(_ctx);
}



time_t QBWaitCallback::lastCalled() const {
  return GWEN_WaitCallback_LastCalled(_ctx);
}



time_t QBWaitCallback::lastEntered() const {
  return GWEN_WaitCallback_LastEntered(_ctx);
}



void QBWaitCallback::setDistance(int d){
  GWEN_WaitCallback_SetDistance(_ctx, d);
}



int QBWaitCallback::nestingLevel() const {
  return GWEN_WaitCallback_GetNestingLevel(_ctx);
}









