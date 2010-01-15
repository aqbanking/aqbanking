/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: banking.cpp 935 2006-02-14 02:11:55Z aquamaniac $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "qgui.h"
#include "qguiprogress.h"
#include "qguiprogresswidget.h"
#include "qguiinputbox.h"
#include "qguisimplebox.h"
//Added by qt3to4:
#include <Q3CString>

#include <gwenhywfar/debug.h>

#include <qmessagebox.h>
#include <qapplication.h>

#include <assert.h>


#define QGUI_PROGRESS_DELAY_TIME 3



QGui::QGui()
:_lastProgressId(0)
,_lastBoxId(0)
,_parentWidget(NULL)
{

}



QGui::~QGui() {
}



void QGui::pushParentWidget(QWidget *w) {
  if (_parentWidget)
    _pushedParents.push_back(_parentWidget);
  _parentWidget=w;
}



void QGui::popParentWidget() {
  if (!_pushedParents.empty()) {
    _parentWidget=_pushedParents.back();
    _pushedParents.pop_back();
  }
  else
    _parentWidget=NULL;
}



int QGui::messageBox(uint32_t flags,
		     const char *title,
		     const char *text,
		     const char *b1,
		     const char *b2,
		     const char *b3,
		     uint32_t guiid) {
  int rv;
  QString qtext;

  qtext=extractHtml(text);

  switch(flags & GWEN_GUI_MSG_FLAGS_TYPE_MASK) {
  case GWEN_GUI_MSG_FLAGS_TYPE_WARN:
    rv=QMessageBox::warning(_parentWidget, QString::fromUtf8(title),
			    qtext,
			    b1 ? QString::fromUtf8(b1) : QString::null,
			    b2 ? QString::fromUtf8(b2) : QString::null,
			    b3 ? QString::fromUtf8(b3) : QString::null);
    break;
  case GWEN_GUI_MSG_FLAGS_TYPE_ERROR:
    rv=QMessageBox::critical(_parentWidget, QString::fromUtf8(title),
			     qtext,
			     b1 ? QString::fromUtf8(b1) : QString::null,
			     b2 ? QString::fromUtf8(b2) : QString::null,
			     b3 ? QString::fromUtf8(b3) : QString::null);
    break;
  case GWEN_GUI_MSG_FLAGS_TYPE_INFO:
  default:
    rv=QMessageBox::information(_parentWidget, QString::fromUtf8(title),
				qtext,
				b1 ? QString::fromUtf8(b1) : QString::null,
				b2 ? QString::fromUtf8(b2) : QString::null,
				b3 ? QString::fromUtf8(b3) : QString::null);
    break;
  }
  rv++;
  return rv;
}



int QGui::inputBox(uint32_t flags,
		   const char *title,
		   const char *text,
		   char *buffer,
		   int minLen,
		   int maxLen,
		   uint32_t guiid) {
  QString qtext;

  qtext=extractHtml(text);

  QGuiInputBox ib(QString::fromUtf8(title), qtext,
		  flags, minLen, maxLen, 0, "InputBox", true);
  if (ib.exec()==QDialog::Accepted) {
    QString s;

    s=ib.getInput();
    int len=s.length();
    if (len && len<maxLen) {
      // FIXME: QString::latin1() is most probably wrong here!
      // This means that the entered string will be passed into
      // AQ_BANKING in latin1 encoding, not in utf8. This should
      // probably be replaced by s.utf8()! But we need to watch
      // out for potentially breaking some people's PINs. For
      // those who had Umlauts in their PIN there should at least
      // be a commandline-tool available that will accept PINs in
      // a configurable encoding for reading, and a different PIN
      // for writing. -- cstim, 2005-09-15
      memmove(buffer, s.latin1(), len);
      buffer[len]=0;
    }
    else {
      DBG_ERROR(0, "Bad pin length");
      return GWEN_ERROR_INVALID;
    }
    return 0;
  }
  else {
    DBG_WARN(0, "Aborted by user");
    return GWEN_ERROR_USER_ABORTED;
  }
}



uint32_t QGui::showBox(uint32_t flags,
		       const char *title,
		       const char *text,
		       uint32_t guiid) {
  uint32_t id;
  QGuiProgress *pro;
  QGuiSimpleBox *b;
  QWidget *w=NULL;
  QString qtext;

  qtext=extractHtml(text);

  pro=_findProgress(0);
  if (pro)
    w=pro->getWidget();
  id=++_lastBoxId;
  b=new QGuiSimpleBox(id, QString::fromUtf8(title),
		      qtext,
		      w, "SimpleBox",
		      Qt::WType_TopLevel | Qt::WType_Dialog | Qt::WShowModal);
  if (flags & GWEN_GUI_SHOWBOX_FLAGS_BEEP)
    QApplication::beep();

  b->show();
  b->raise();
  _simpleBoxWidgets.push_front(b);

  qApp->processEvents();
  return id;
}



void QGui::hideBox(uint32_t id) {
  if (_simpleBoxWidgets.size()==0) {
    DBG_WARN(0, "No simpleBox widgets");
    return;
  }
  if (id==0) {
    QGuiSimpleBox *b;

    b=_simpleBoxWidgets.front();
    b->close(true);
    _simpleBoxWidgets.pop_front();
  }
  else {
    std::list<QGuiSimpleBox*>::iterator it;

    for (it=_simpleBoxWidgets.begin(); it!=_simpleBoxWidgets.end(); it++) {
      if ((*it)->getId()==id) {
	QGuiSimpleBox *sb;

	sb=(*it);
	sb->close(true);
	_simpleBoxWidgets.erase(it);
	break;
      }
    }
  }
  qApp->processEvents();
}



uint32_t QGui::progressStart(uint32_t flags,
			     const char *title,
			     const char *text,
			     uint64_t total,
			     uint32_t guiid) {
  QGuiProgress *pro;
  QGuiProgressWidget *w;
  QString qtitle;
  QString qtext;

  qtitle=extractHtml(title);
  qtext=extractHtml(text);

  pro=new QGuiProgress(++_lastProgressId, title, flags, total);
  if (!(flags & GWEN_GUI_PROGRESS_DELAY))
    pro->setVisible(true);
  if (!_progressPtrList.empty() &&
      (flags & GWEN_GUI_PROGRESS_ALLOW_EMBED)) {
    /* search for progress widget in which this can be embedded */
    std::list<QGuiProgress*>::reverse_iterator it;

    for (it=_progressPtrList.rbegin();
	 it!=_progressPtrList.rend();
	 it++) {
      if ((*it)->getFlags() & GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS) {
	w=(*it)->getWidget();
	if (w) {
          _progressPtrList.push_back(pro);
	  _checkVisibilities();
	  w->addProgress(pro);
	  return pro->getId();
	}
	else {
          DBG_ERROR(0, "Progress has no widget");
	}
      }
      else {
	DBG_DEBUG(0, "Progress %d doesn't allow sublevels (%08x)",
		  (*it)->getId(), (*it)->getFlags());
      }
    }
  }

  /* fallthrough: new progress can't be embedded, create new widget */
  w=new QGuiProgressWidget(pro, qtitle, qtext,
                           _parentWidget,
			   0,
			   Qt::WType_TopLevel |
			   Qt::WType_Dialog |
			   Qt::WShowModal);
  if (!(flags & GWEN_GUI_PROGRESS_DELAY))
    w->show();
  _progressPtrList.push_back(pro);
  _checkVisibilities();
  return pro->getId();
}



int QGui::progressAdvance(uint32_t id, uint64_t progress) {
  QGuiProgress *pro;
  QGuiProgressWidget *w;

  pro=_findProgress(id);
  if (pro==NULL)
    return 0;

  w=pro->getWidget();
  if (w==NULL)
    /* no widget, progress vanished, assume user aborted it */
    return GWEN_ERROR_USER_ABORTED;

  /* adjust current progress */
  if (progress!=GWEN_GUI_PROGRESS_NONE) {
    if (progress==GWEN_GUI_PROGRESS_ONE)
      pro->setCurrent(pro->getCurrent()+1);
    else
      pro->setCurrent(progress);
  }

  /* check the timeouts of the currently active progresses */
  _checkVisibilities();
  /* check for user abort */
  return w->checkAbort();
}



int QGui::progressLog(uint32_t id,
		      GWEN_LOGGER_LEVEL level,
		      const char *text) {
  QGuiProgress *pro;
  QGuiProgressWidget *w;
  QString qtext;

  /* check the timeouts of the currently active progresses */
  _checkVisibilities();

  pro=_findProgress(id);
  if (pro==NULL)
    return 0;

  w=pro->getWidget();
  if (w==NULL)
    /* no widget, progress vanished, assume user aborted it */
    return GWEN_ERROR_USER_ABORTED;

  qtext=extractHtml(text);

  return w->log(level, qtext);
}



int QGui::progressEnd(uint32_t id) {
  QGuiProgress *pro;
  QGuiProgressWidget *w;
  QString qtext;
  bool wasAborted;

  /* check the timeouts of the currently active progresses */
  _checkVisibilities();

  pro=_findProgress(id);
  if (pro==NULL)
    return 0;

  w=pro->getWidget();
  if (w==NULL) {
    /* no widget, progress vanished, assume user aborted it */
    _delProgress(pro);
    return GWEN_ERROR_USER_ABORTED;
  }
  w->delProgress(pro);
  pro->setWidget(NULL);
  _delProgress(pro);
  wasAborted=w->aborted();
  if (!w->hasProgresses() && !w->shouldStay()) {
    //DBG_ERROR(0, "Destroying progress window");
    delete w;
  }

  if (wasAborted)
    return GWEN_ERROR_USER_ABORTED;

  return 0;
}




void QGui::_checkVisibilities() {
  std::list<QGuiProgress*>::iterator it;

  for (it=_progressPtrList.begin();
       it!=_progressPtrList.end();
       it++) {
    if (!(*it)->isVisible()) {
      time_t t1;

      // check timeout, set "visible" flag
      t1=time(0);
      if (difftime(t1, (*it)->getStartTime())>=QGUI_PROGRESS_DELAY_TIME) {
	(*it)->setVisible(true);
      }
    }
  }
}



QString QGui::extractHtml(const char *text) {
  const char *p=0;
  const char *p2=0;

  if (text==NULL)
    return QString("");

  /* find begin of HTML area */
  p=text;
  while ((p=strchr(p, '<'))) {
    const char *t;

    t=p;
    t++;
    if (toupper(*t)=='H') {
      t++;
      if (toupper(*t)=='T') {
        t++;
        if (toupper(*t)=='M') {
          t++;
          if (toupper(*t)=='L') {
	    t++;
	    if (toupper(*t)=='>') {
	      break;
	    }
	  }
        }
      }
    }
    p++;
  } /* while */

  /* find end of HTML area */
  if (p) {
    p+=6; /* skip "<html>" */
    p2=p;
    while ((p2=strchr(p2, '<'))) {
      const char *t;
  
      t=p2;
      t++;
      if (toupper(*t)=='/') {
	t++;
	if (toupper(*t)=='H') {
	  t++;
	  if (toupper(*t)=='T') {
	    t++;
	    if (toupper(*t)=='M') {
	      t++;
	      if (toupper(*t)=='L') {
		t++;
		if (toupper(*t)=='>') {
		  break;
		}
	      }
	    }
	  }
	}
      }
      p2++;
    } /* while */
  }

  if (p && p2)
    return QString("<qt>")+QString::fromUtf8(p, p2-p)+QString("</qt>");

  return QString::fromUtf8(text);
}



std::string QGui::qstringToUtf8String(const QString &qs) {
  if (qs.isEmpty())
    return "";
  else {
    Q3CString utfData=qs.utf8();
    return utfData.data();
  }
}



void QGui::_addProgress(QGuiProgress *pro) {
  _progressPtrList.push_back(pro);
}



void QGui::_delProgress(QGuiProgress *pro) {
  std::list<QGuiProgress*>::iterator it;

  for (it=_progressPtrList.begin();
       it!=_progressPtrList.end();
       it++) {
    if (*it==pro) {
      /* this progress is in the list, so end/delete this one and all
       * behind it, starting from the last one */
      while(_progressPtrList.size()) {
	QGuiProgress *tp;
	QGuiProgressWidget *w;

	tp=_progressPtrList.back();
	assert(tp);
	_progressPtrList.pop_back();
        w=tp->getWidget();
	if (w) {
	  w->delProgress(tp);
	  tp->setWidget(NULL);
	}
	if (tp==pro) {
	  delete tp;
	  break;
	}
	else {
	  DBG_ERROR(0, "Unfinished progress detected: %08x [%s]",
		    tp->getId(),
		    tp->getTitle().latin1());
	  delete tp;
	}
      }
      break;
    }
  }
}



QGuiProgress *QGui::_findProgress(uint32_t id) {
  if (id==0) {
    if (_progressPtrList.size())
      return _progressPtrList.back();
  }
  else {
    std::list<QGuiProgress*>::iterator it;

    for (it=_progressPtrList.begin();
	 it!=_progressPtrList.end();
	 it++) {
      if ((*it)->getId()==id)
	return *it;
    }
  }
  return NULL;
}




