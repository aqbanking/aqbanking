/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: qbprogress.cpp 935 2006-02-14 02:11:55Z aquamaniac $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "qguiprogresswidget.h"
#include "qguiprogress.h"

#include <qlabel.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qdatetime.h>
#include <qtextbrowser.h>
#include <qprogressbar.h>
#include <qgroupbox.h>

#include <gwenhywfar/debug.h>



QGuiProgressWidget::QGuiProgressWidget(QGuiProgress *pr,
				       const QString &title,
				       const QString &text,
				       QWidget* parent,
				       const char* name,
				       WFlags fl)
:QGuiProgressWidgetUI(parent, name, fl)
,_aborted(false)
,_shouldStay(false)
,_doShowText(false)
,_startTime(0)
,_lastTime(0)
,_currentSubProgress(NULL){
  uint32_t flags;

  assert(pr);
  flags=pr->getFlags();

  if (!title.isEmpty())
    setCaption(title);
  if (!text.isEmpty())
    textWidget->setText(text);

  logWidget->setMinimumHeight(350);
  logWidget->hide();
  currentGroupBox->hide();

  if (!(flags & GWEN_GUI_PROGRESS_SHOW_ABORT))
    abortButton->hide();

  if (!(flags & GWEN_GUI_PROGRESS_SHOW_PROGRESS))
    progressBar->hide();

  progressBar->setTotalSteps(pr->getTotal());
  progressBar->setProgress(0);

  _startTime=time(0);

  addProgress(pr);

  abortButton->setEnabled(true);
  closeButton->setEnabled(false);

  QObject::connect(abortButton, SIGNAL(clicked()),
                   this, SLOT(abort()));

  QObject::connect(closeButton, SIGNAL(clicked()),
                   this, SLOT(close()));

}



QGuiProgressWidget::~QGuiProgressWidget(){
  std::list<QGuiProgress*>::iterator it;

  for (it=_progressPtrList.begin();
       it!=_progressPtrList.end();
       it++)
    (*it)->setWidget(NULL);
}



bool QGuiProgressWidget::hasProgresses() const {
  return !_progressPtrList.empty();
}



bool QGuiProgressWidget::_handleTime(){
  time_t currTime;

  if (!_progressPtrList.empty()) {
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
      timeLabel->setText(label);

      return true;
    }
  }
  return false;
}



int QGuiProgressWidget::log(GWEN_LOGGER_LEVEL level,
			    const QString& text){
  QString tmp;

  tmp+=_logtext;
  tmp+="<tr><td>" + 
      QTime::currentTime().toString() +
      "</td><td>";
  if (level<=GWEN_LoggerLevel_Error) {
    tmp+=QString("<font color=\"red\">%1</font>").arg(text);
  }
  else if (level==GWEN_LoggerLevel_Warning) {
    tmp+=QString("<font color=\"blue\">%1</font>").arg(text);
  }
  else if (level==GWEN_LoggerLevel_Info) {
    tmp+=QString("<font color=\"green\">%1</font>").arg(text);
  }
  else if (level>=GWEN_LoggerLevel_Debug) {
    if (_aborted)
      return GWEN_ERROR_USER_ABORTED;
    return 0;
  }
  else
    tmp+=text;

  if (level<=GWEN_LoggerLevel_Notice) {
    if (!_doShowText) {
      _doShowText=true;
      logWidget->show();
    }
  }

  if (level<=GWEN_LoggerLevel_Warning) {
    if (!_shouldStay) {
      _shouldStay=true;
#if (QT_VERSION >= 0x040000)
      setWindowFlags(Qt::WDestructiveClose);
#else
      setWFlags(Qt::WDestructiveClose);
#endif
    }
  }

  tmp+=QString("</td></tr>");
  _logtext=tmp;
  tmp="<qt><table>"+_logtext+"</table></qt>";
  logWidget->setText(tmp);
  logWidget->scrollToBottom();

  qApp->processEvents();
  if (_aborted)
    return GWEN_ERROR_USER_ABORTED;
  return 0;
}



void QGuiProgressWidget::_selectSublevel() {
  std::list<QGuiProgress*>::reverse_iterator it;

  for (it=_progressPtrList.rbegin();
       it!=_progressPtrList.rend();
       it++) {
    if ((*it)!=_progressPtrList.front() &&
	(*it)->isVisible()) {
      /* found new candidate */
      if (_currentSubProgress!=(*it)) {
	/* show content in window */
	_currentSubProgress=(*it);
	currentGroupBox->setEnabled(true);
	currentLabel->setText(_currentSubProgress->getTitle());
	currentProgressBar->setTotalSteps(_currentSubProgress->getTotal());
	currentProgressBar->setProgress(_currentSubProgress->getCurrent());
	currentLabel->setEnabled(true);
	currentProgressBar->setEnabled(true);
	if (!currentGroupBox->isShown())
	  currentGroupBox->show();
	qApp->processEvents();
      }
      return;
    } /* if visible */
  } /* for */
  if (currentGroupBox->isShown()) {
    currentGroupBox->hide();
    qApp->processEvents();
  }
  /*
  currentLabel->setText(tr("No action"));
  currentProgressBar->setTotalSteps(0);
  currentProgressBar->setProgress(0);
  currentGroupBox->setEnabled(false);*/
  _currentSubProgress=NULL;
}



void QGuiProgressWidget::addProgress(QGuiProgress *pro) {
  uint32_t flags;

  flags=pro->getFlags();

  if (flags & GWEN_GUI_PROGRESS_KEEP_OPEN) {
    if (!_shouldStay) {
      _shouldStay=true;
#if (QT_VERSION >= 0x040000)
      setWindowFlags(Qt::WDestructiveClose);
#else
      setWFlags(Qt::WDestructiveClose);
#endif
    }
  }

  if (flags & GWEN_GUI_PROGRESS_ALWAYS_SHOW_LOG) {
    logWidget->show();
    _doShowText=true;
  }

  if (!isShown() && pro->isVisible())
    show();

  _progressPtrList.push_back(pro);
  pro->setWidget(this);
  _selectSublevel();
}



void QGuiProgressWidget::delProgress(QGuiProgress *pro) {
  std::list<QGuiProgress*>::iterator it;

  for (it=_progressPtrList.begin();
       it!=_progressPtrList.end();
       it++) {
    if (*it==pro) {
      /* this progress is in the list, so remove this one and all
       * behind it, starting from the last one */
      while(_progressPtrList.size()) {
	QGuiProgress *tp;

	tp=_progressPtrList.back();
	assert(tp);
	_progressPtrList.pop_back();
	tp->setWidget(NULL);
	if (tp==_currentSubProgress)
	  _currentSubProgress=NULL;
	if (tp==pro)
	  break;
      }
      break;
    }
  }

  if (_progressPtrList.empty()) {
    /* last progress closed, may close widget */
    abortButton->setEnabled(false);
    closeButton->setEnabled(true);
    closeButton->setFocus();
    if (_shouldStay) {
      QString qs;

      qs=tr("Finished. You may close this window.");
      log(GWEN_LoggerLevel_Notice, qs);
    }
    raise();
  }
  _selectSublevel();
  qApp->processEvents();
}



void QGuiProgressWidget::closeEvent(QCloseEvent *e){
  if (_progressPtrList.empty())
    e->accept();
}



void QGuiProgressWidget::abort() {
  _aborted=true;
  abortButton->setEnabled(false);
  log(GWEN_LoggerLevel_Warning, tr("User aborted"));
}



int QGuiProgressWidget::checkAbort() {
  /* find out when we checked the last time */
  if (_handleTime()) {
    /* try to make sure the current progress widget is valid */
    _selectSublevel();

    if (!isShown()) {
      std::list<QGuiProgress*>::iterator it;

      for (it=_progressPtrList.begin();
	   it!=_progressPtrList.end();
	   it++) {
	if ((*it)->isVisible())
          show();
      }
    }

    /* update current sub progress */
    if (_currentSubProgress) {
      uint64_t currentPos;

      currentPos=_currentSubProgress->getCurrent();
      if (currentPos!=_currentSubProgress->getLastPos()) {
	/* update current sub widget */
	currentProgressBar->setProgress(currentPos);

	// TODO: update widget
	_currentSubProgress->setLastPos(currentPos);
      }
    }

    /* update main (=first) progress */
    if (!_progressPtrList.empty()) {
      QGuiProgress *pro;
      uint64_t currentPos;

      pro=_progressPtrList.front();
      assert(pro);
      currentPos=pro->getCurrent();
      if (currentPos!=pro->getLastPos()) {
	progressBar->setProgress(currentPos);
        pro->setLastPos(currentPos);
      }
    }

    /* gui update */
    qApp->processEvents();
  }
  return _aborted?GWEN_ERROR_USER_ABORTED:0;
}




#include "qguiprogresswidget.moc"





