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
#include <q3progressbar.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qdatetime.h>
#include <q3textbrowser.h>

#include "qbprogress.h"
//Added by qt3to4:
#include <QCloseEvent>

#include <gwenhywfar/debug.h>


int QBProgress::_openCount=0;



QBProgress::QBProgress(GWEN_TYPE_UINT32 id,
		       ProgressType pt,
                       const QString& title,
                       const QString& text,
                       const QString& units,
                       QWidget* parent, const char* name, Qt::WFlags fl)
:QBProgressUI(parent, name, fl)
,_id(id)
,_progressType(pt)
,_aborted(false)
,_closed(false)
,_doShowText(false)
,_shouldStay(false)
,_total(0)
,_lastProgress(0)
,_units(units) {

  _openCount++;
  if (_openCount>5) {
    DBG_ERROR(0,
	      "Too many progress widgets created, "
	      "please check your program");
    ::abort();
  }

  if (!title.isEmpty())
    setCaption(title);
  if (!text.isEmpty())
    textWidget->setText(text);

  if (pt==ProgressTypeNormal) {
    _doShowText=true;
    _shouldStay=true;
#if (QT_VERSION >= 0x040000)
    setWindowFlags
#else
    setWFlags
#endif
	(Qt::WDestructiveClose);
    logWidget->setMinimumHeight(350);
  }

  QObject::connect(abortButton, SIGNAL(clicked()),
                   this, SLOT(abort()));

  QObject::connect(closeButton, SIGNAL(clicked()),
                   this, SLOT(close()));

}



QBProgress::~QBProgress(){
  _openCount--;
}



void QBProgress::setProgressText(const QString &s) {
  textWidget->setText(s);
}



void QBProgress::setProgressUnits(const QString &s) {
  _units=s;
}



void QBProgress::show(){
  if (_total==AB_BANKING_PROGRESS_NONE)
    progressBar->hide();
  else
    progressUnitsLabel->hide();
  adjustSize();
  QBProgressUI::show();
  qApp->processEvents();
}



GWEN_TYPE_UINT32 QBProgress::getId(){
  return _id;
}



int QBProgress::start(GWEN_TYPE_UINT32 total){
  _total=total;
  _closed=false;
  abortButton->setEnabled(true);
  closeButton->setEnabled(false);
  _aborted=false;

  if (_progressType==ProgressTypeSimple ||
      _progressType==ProgressTypeFast) {
    _doShowText=false;
    logWidget->hide();
  }

  if (_total!=AB_BANKING_PROGRESS_NONE)
    progressBar->setTotalSteps(total);
  progressBar->setProgress(0);
  _lastProgress=0;

  _startTime=time(0);

  qApp->processEvents();
  _lastTime=0;

  return 0;
}



int QBProgress::setTotalPos(GWEN_TYPE_UINT32 total){
  if (total!=_total) {
    _total=total;
    if (_total!=AB_BANKING_PROGRESS_NONE)
      progressBar->setTotalSteps(total);
    progressBar->setProgress(_lastProgress);
    qApp->processEvents();
  }
  return 0;
}



bool QBProgress::_handleTime(){
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
	  arg(secs<10 ? QString("0") : QString::null).arg(secs);
      // Replacement for: 
      // snprintf(buf, sizeof(buf), "%d:%02d min", mins, secs);
      timeLabel->setText(label);

      return true;
    }
  }
  return false;
}



int QBProgress::advance(GWEN_TYPE_UINT32 progress){
  bool chg;

  chg=_handleTime();
  if (chg) {
    if (_total==AB_BANKING_PROGRESS_NONE) {
      if (progress==AB_BANKING_PROGRESS_ONE)
        progress=_lastProgress+1;
  
      if (progress!=AB_BANKING_PROGRESS_NONE) {
        if (progress!=_lastProgress) {
          QString qs;
  
          qs=QString::number(progress);
          if (!_units.isEmpty()) {
            qs+= " " + _units;
          }
          progressUnitsLabel->setText(qs);
          _lastProgress=progress;
        }
      }
    }
    else {
      if (progress==AB_BANKING_PROGRESS_NONE) {
      }
      else if (progress==AB_BANKING_PROGRESS_ONE) {
        progressBar->setProgress(progressBar->progress()+1);
      }
      else {
        progressBar->setProgress(progress);
      }
    }
    qApp->processEvents();
  }

  if (_aborted)
    return AB_ERROR_USER_ABORT;
  return 0;
}



int QBProgress::log(AB_BANKING_LOGLEVEL level,
                    const QString& text){
  QString tmp;

  _handleTime();
  tmp+=_logtext;
  tmp+="<tr><td>" + 
      QTime::currentTime().toString() +
      "</td><td>";
  if (level<=AB_Banking_LogLevelError) {
    tmp+=QString("<font color=\"red\">%1</font>").arg(text);
  }
  else if (level==AB_Banking_LogLevelWarn) {
    tmp+=QString("<font color=\"blue\">%1</font>").arg(text);
  }
  else if (level==AB_Banking_LogLevelInfo) {
    tmp+=QString("<font color=\"green\">%1</font>").arg(text);
  }
  else if (level>=AB_Banking_LogLevelDebug) {
    if (_aborted)
      return AB_ERROR_USER_ABORT;
    return 0;
  }
  else
    tmp+=text;

  if (level<=AB_Banking_LogLevelNotice) {
    if (!_doShowText) {
      _doShowText=true;
      logWidget->show();
    }
  }

  if (level<=AB_Banking_LogLevelWarn) {
    _shouldStay=true;
#if (QT_VERSION >= 0x040000)
    setWindowFlags
#else
    setWFlags
#endif
	(Qt::WDestructiveClose);
  }

  tmp+=QString("</td></tr>");
  _logtext=tmp;
  tmp="<qt><table>"+_logtext+"</table></qt>";
  logWidget->setText(tmp);
  logWidget->scrollToBottom();

  qApp->processEvents();
  if (_aborted)
    return AB_ERROR_USER_ABORT;
  return 0;
}



int QBProgress::end(){
  abortButton->setEnabled(false);
  closeButton->setEnabled(true);
  closeButton->setFocus();
  if (_shouldStay) {
    QString qs;

    qs=tr("Finished. You may close this window.");
    log(AB_Banking_LogLevelNotice, qs);
  }
  raise();

  if (_aborted)
    return AB_ERROR_USER_ABORT;
  return 0;
}



void QBProgress::closeEvent(QCloseEvent *e){
  if (closeButton->isEnabled()) {
    _closed=true;
    e->accept();
  }
}



void QBProgress::abort() {
  _aborted=true;
  abortButton->setEnabled(false);
  //closeButton->setEnabled(true);
  closeButton->setFocus();
  log(AB_Banking_LogLevelWarn, tr("User aborted"));
}



bool QBProgress::isClosed() {
  return _closed;
}



bool QBProgress::shouldStay() {
  return _shouldStay;
}









