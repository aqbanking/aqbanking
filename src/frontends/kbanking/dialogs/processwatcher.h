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

#ifndef AQBANKING_KDE_PROCWATCHER_H
#define AQBANKING_KDE_PROCWATCHER_H


#include "processwatcher.ui.h"

#include <qstring.h>


class QProcess;


class ProcessWatcher : public ProcessWatcherUi{
  Q_OBJECT
public:
  ProcessWatcher(QProcess* process,
                 const QString &text=QString::null,
                 QWidget* parent=0,
                 const char* name=0,
                 bool modal=FALSE,
                 WFlags fl=0 );
  ~ProcessWatcher();

  void accept();

  int getStatus();

public slots:
  void slotTerminate();
  void slotKill();
  void slotProcessFinished();

private:
  QProcess *_process;
  int _result;
  bool _closeEnabled;
};




#endif // AQBANKING_KDE_PROCWATCHER_H

