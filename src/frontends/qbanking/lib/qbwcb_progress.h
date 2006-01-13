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


#ifndef QBANKING_WCB_PROGRESS_H
#define QBANKING_WCB_PROGRESS_H

#include "qbwaitcallback.h"
#include "qbprogress.h"

#include <time.h>


class QBProgressCallback: public QBWaitCallback {
public:
  QBProgressCallback(const char *id, QBProgress::ProgressType pt);
  virtual ~QBProgressCallback();
  virtual QBWaitCallback *instantiate();
  virtual GWEN_WAITCALLBACK_RESULT checkAbort(unsigned int level);
  virtual void log(unsigned int level,
                   GWEN_LOGGER_LEVEL loglevel,
                   const char *s);

private:
  QBProgress *_progressWidget;
  QBProgress::ProgressType _progressType;
  GWEN_TYPE_UINT64 _lastProgress;
  bool _started;
  time_t _startTime;

  bool _checkStart(bool force);

};



#endif

