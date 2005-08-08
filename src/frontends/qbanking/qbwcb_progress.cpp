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
#include "qbwcb_progress.h"
#include "qbprogress.h"

#include <gwenhywfar/debug.h>



QBProgressCallback::QBProgressCallback(const char *id,
                                       QBProgress::ProgressType pt)
:QBWaitCallback(id)
,_progressWidget(0)
,_progressType(pt)
,_lastProgress(0)
,_started(false) {
}



QBProgressCallback::~QBProgressCallback(){
  if (_progressWidget) {
    if (_progressWidget->shouldStay()) {
      _progressWidget->advance(_lastProgress);
      _progressWidget->end();
    }
    else {
      delete _progressWidget;
    }
  }
}



QBWaitCallback *QBProgressCallback::instantiate(){
  return new QBProgressCallback(getId(), _progressType);
}



bool QBProgressCallback::_checkStart(bool force) {
  time_t currTime;
  double d;

  currTime=time(0);
  d=difftime(currTime, lastEntered());
  if (!_started &&
      (force ||
       _progressType==QBProgress::ProgressTypeNormal ||
       (_progressType==QBProgress::ProgressTypeFast && d>=5) ||
       (_progressType==QBProgress::ProgressTypeSimple && d>=2))
     ) {
    DBG_ERROR(0, "Starting with %d",
              (int) getProgressTotal());
    _progressWidget=new QBProgress(0,
                                   _progressType,
                                   QWidget::tr("Waiting..."),
                                   getText(),
                                   getUnits(),
                                   0,
                                   "ProgressWidget",
                                   Qt::WType_Dialog | Qt::WShowModal);
    _progressWidget->setProgressText(getText());
    _progressWidget->setProgressUnits(getUnits());
    _progressWidget->start(getProgressTotal());
    _progressWidget->show();
    _started=true;
  }
  return _started;
}


GWEN_WAITCALLBACK_RESULT
QBProgressCallback::checkAbort(unsigned int level){
  int rv;

  if (_checkStart(false)) {
    if (level!=0) {
      rv=_progressWidget->advance(AB_BANKING_PROGRESS_NONE);
    }
    else {
      if (_lastProgress==getProgressPos())
        rv=_progressWidget->advance(AB_BANKING_PROGRESS_NONE);
      else {
        _lastProgress=getProgressPos();
        rv=_progressWidget->advance(_lastProgress);
      }
    }

    if (rv==AB_ERROR_USER_ABORT) {
      DBG_WARN(0, "Aborted");
      return GWEN_WaitCallbackResult_Abort;
    }
  }
  return GWEN_WaitCallbackResult_Continue;
}



void QBProgressCallback::log(unsigned int level,
                             GWEN_LOGGER_LEVEL loglevel,
                             const char *s){
  AB_BANKING_LOGLEVEL nl;

  if (_checkStart(loglevel<=GWEN_LoggerLevelWarning)) {
    switch(loglevel) {
    case GWEN_LoggerLevelError:     nl=AB_Banking_LogLevelError; break;
    case GWEN_LoggerLevelWarning:   nl=AB_Banking_LogLevelWarn; break;
    case GWEN_LoggerLevelNotice:    nl=AB_Banking_LogLevelNotice; break;
    default:
      DBG_NOTICE(AQBANKING_LOGDOMAIN, "Loglevel %d -> info", loglevel);
      nl=AB_Banking_LogLevelInfo;
      break;
    }

    _progressWidget->log(nl, s);
  }
}





