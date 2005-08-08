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


#include "processwatcher.h"

#include <qprocess.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qmessagebox.h>



ProcessWatcher::ProcessWatcher(QProcess* process,
                               const QString &text,
                               QWidget* parent,
                               const char* name,
                               bool modal,
                               WFlags fl)
:ProcessWatcherUi(parent, name, modal, fl)
,_process(process)
,_result(-1)
,_closeEnabled(false){
  if (text==QString::null)
    textLabel->setText(tr("Process running..."));
  else
    textLabel->setText(text);

  QObject::connect((QObject*)process, SIGNAL(processExited()),
                   this, SLOT(slotProcessFinished()));
  QObject::connect((QObject*)terminateButton, SIGNAL(clicked()),
                   this, SLOT(slotTerminate()));
  QObject::connect((QObject*)killButton, SIGNAL(clicked()),
                   this, SLOT(slotKill()));
}



ProcessWatcher::~ProcessWatcher(){
}



void ProcessWatcher::slotTerminate(){
  _process->tryTerminate();
  terminateButton->setEnabled(false);
}



void ProcessWatcher::slotKill(){
  _process->kill();
  terminateButton->setEnabled(false);
  killButton->setEnabled(false);
  _closeEnabled=true;
}



void ProcessWatcher::slotProcessFinished() {
  int rv;

  _closeEnabled=true;
  rv=_process->exitStatus();
  _result=rv;
  if (rv) {
    QMessageBox::critical(0,
                          tr("Process Error"),
                          QString(tr("<qt>"
                                     "<p>"
                                     "Process exited with status %1"
                                     "</p>"
                                     "</qt>"))
                          .arg(rv),
                          tr("Dismiss"),0,0,0);
    QDialog::reject();
  }
  else
    QDialog::accept();
}



void ProcessWatcher::accept(){
  if (_closeEnabled)
    QDialog::accept();
}



int ProcessWatcher::getStatus(){
  return _process->exitStatus();
}








