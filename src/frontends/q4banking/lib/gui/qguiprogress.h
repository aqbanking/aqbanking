/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: qbprogress.h 809 2006-01-20 14:15:15Z cstim $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef QGUI_PROGRESS_H
#define QGUI_PROGRESS_H


#include <gwenhywfar/types.h>

#include <time.h>

#include <qstring.h>


class QGuiProgressWidget;


class QGuiProgress {
protected:
  uint32_t _id;
  uint32_t _flags;
  bool _finished;
  bool _isVisible;

  uint64_t _total;
  uint64_t _current;

  time_t _startTime;
  time_t _lastTime;

  QGuiProgressWidget *_widget;
  uint64_t _lastPos;

  QString _title;

public:
  QGuiProgress(uint32_t id, const char *title,
	       uint32_t flags, uint64_t _total);
  virtual ~QGuiProgress();


  uint32_t getId() const { return _id; };
  uint32_t getFlags() const { return _flags; };

  uint64_t getTotal() const { return _total;};
  uint64_t getCurrent() const { return _current;};
  void setCurrent(uint64_t i) { _current=i;};

  const QString &getTitle() const { return _title; };

  bool finished() const { return _finished;};
  bool isVisible() const { return _isVisible;};
  void setVisible(bool b) { _isVisible=b;};

  time_t getStartTime() const { return _startTime;};

  void setWidget(QGuiProgressWidget *w) { _widget=w;};
  QGuiProgressWidget *getWidget() const { return _widget;};

  uint64_t getLastPos() const { return _lastPos;};
  void setLastPos(uint64_t i) { _lastPos=i;};
};






#endif

