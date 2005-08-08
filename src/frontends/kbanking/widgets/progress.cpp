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


#include <qlabel.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qdatetime.h>
#include <qtextbrowser.h>

#include "progress.h"



KBProgress::KBProgress(GWEN_TYPE_UINT32 id,
                       const QString& title,
                       const QString& text,
                       QWidget* parent, const char* name, WFlags fl)
:KBProgressUI(parent, name, fl), _id(id), _aborted(false), _closed(false) {
  if (!title.isEmpty())
    setCaption(title);
  if (!text.isEmpty())
    textWidget->setText(text);

  QObject::connect((QObject*)abortButton, SIGNAL(clicked()),
                   this, SLOT(abort()));

  QObject::connect((QObject*)closeButton, SIGNAL(clicked()),
                   this, SLOT(close()));
}



KBProgress::~KBProgress(){
}



GWEN_TYPE_UINT32 KBProgress::getId(){
  return _id;
}



int KBProgress::start(GWEN_TYPE_UINT32 total){
  _closed=false;
  abortButton->setEnabled(true);
  closeButton->setEnabled(false);
  _aborted=false;
  progressBar->setTotalSteps(total);
  progressBar->setProgress(0);
  qApp->processEvents();
  _startTime=time(0);
  _lastTime=0;
  return 0;
}



void KBProgress::_handleTime(){
  time_t currTime;

  if (!_closed) {
    currTime=time(0);
    if (_lastTime!=currTime) {
      unsigned int dt;
      int mins;
      int secs;
      
      _lastTime=currTime;
      dt=(unsigned int)difftime(currTime, _startTime);
      mins=dt/60;
      secs=dt%60;
      QString label = QString("%1:%2%3 min").arg(mins).	
	  arg(secs<10?"0":"").arg(secs);
      // Replacement for: 
      // snprintf(buf, sizeof(buf), "%d:%02d min", mins, secs);
      timeLabel->setText(label);
    }
  }
}



int KBProgress::advance(GWEN_TYPE_UINT32 progress){
  _handleTime();
  if (progress!=AB_BANKING_PROGRESS_NONE)
    progressBar->setProgress(progress);

  qApp->processEvents();
  if (_aborted)
    return AB_ERROR_USER_ABORT;
  return 0;
}



int KBProgress::log(AB_BANKING_LOGLEVEL level,
                    const QString& text){
  QTime d;
  QString tmp;

  _handleTime();
  tmp+=_logtext;
  tmp+="<tr><td>";
  d=QTime::currentTime();
  tmp+=d.toString();
  tmp+="</td><td>";
  if (level<=AB_Banking_LogLevelError) {
    tmp+="<font color=\"red\">";
    tmp+=text;
    tmp+="</font>";
  }
  else if (level==AB_Banking_LogLevelWarn) {
    tmp+="<font color=\"blue\">";
    tmp+=text;
    tmp+="</font>";
  }
  else if (level>=AB_Banking_LogLevelInfo) {
    tmp+="<font color=\"green\">";
    tmp+=text;
    tmp+="</font>";
  }
  else
    tmp+=text;
  tmp+="</td></tr>";
  _logtext=tmp;
  tmp="<qt><table>"+_logtext+"</table></qt>";
  logWidget->setText(tmp);
  logWidget->scrollToBottom();

  qApp->processEvents();
  if (_aborted)
    return AB_ERROR_USER_ABORT;
  return 0;
}



int KBProgress::end(){
  QString qs;

  abortButton->setEnabled(false);
  closeButton->setEnabled(true);
  closeButton->setFocus();
  qs=tr("Finished. You may close this window.");
  log(AB_Banking_LogLevelNotice, qs);
  raise();

  if (_aborted)
    return AB_ERROR_USER_ABORT;
  return 0;
}



void KBProgress::closeEvent(QCloseEvent *e){
  if (closeButton->isEnabled()) {
    _closed=true;
    e->accept();
  }
}



void KBProgress::abort() {
  _aborted=true;
  abortButton->setEnabled(false);
  //closeButton->setEnabled(true);
  closeButton->setFocus();
  log(AB_Banking_LogLevelWarn, "User aborted");
}



bool KBProgress::isClosed() {
  return _closed;
}









