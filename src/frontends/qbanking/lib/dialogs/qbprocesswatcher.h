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

#ifndef QBANKING_PROCWATCHER_H
#define QBANKING_PROCWATCHER_H


#include "qbprocesswatcher.ui.h"

#include <qstring.h>
#include <time.h>

class Q3Process;


class QBProcessWatcher : public QBProcessWatcherUi{
  Q_OBJECT
public:
  QBProcessWatcher(Q3Process* process,
                   const QString &text=QString::null,
                   QWidget* parent=0,
                   const char* name=0,
                   bool modal=FALSE,
                   Qt::WFlags fl=0 );
  ~QBProcessWatcher();

  void accept();

  int getStatus() const;

  int getDuration() const;

public slots:
  void slotTerminate();
  void slotKill();
  void slotProcessFinished();

  virtual void languageChange();

private:
  Q3Process *_process;
  int _result;
  bool _closeEnabled;
  time_t _startTime;
  int _duration;
};




#endif // AQBANKING_KDE_PROCWATCHER_H

