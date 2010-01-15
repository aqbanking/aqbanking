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

#include "wizard.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <q3listview.h>
#include <qdatetime.h>
#include <q3textbrowser.h>
#include <qapplication.h>

#include <gwenhywfar/debug.h>



Wizard::Wizard(QBanking *qb,
               WizardInfo *wInfo,
               const QString &title,
               QWidget* parent,
               const char* name, bool modal)
:Q3Wizard(parent, name)
,Ui_WizardUi()
,_app(qb)
,_wInfo(wInfo)
,_lastActionWidget(0) {
  setupUi(this);

  setModal(modal);
  if (!title.isEmpty())
    setCaption(title);
}



Wizard::~Wizard() {
}



QBanking *Wizard::getBanking() {
  return _app;
}



void Wizard::addAction(WizardAction *a) {
  addPage(a, a->getDescription());
  _lastActionWidget=a;
}



WizardInfo *Wizard::getWizardInfo() {
  return _wInfo;
}



void Wizard::log(GWEN_LOGGER_LEVEL level, const QString &text) {
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
    return;
  }
  else
    tmp+=text;

  tmp+=QString("</td></tr>");
  _logtext=tmp;
  tmp="<qt><table>"+_logtext+"</table></qt>";
  /*
  logBrowser->setText(tmp);
  logBrowser->scrollToBottom();

  qApp->processEvents();
  */
}



void Wizard::setDescription(const QString &s) {
  textLabel->setText(s);
}



QWidget *Wizard::getWidgetAsParent() {
  return (QWidget*)this;
}



int Wizard::exec() {
  return Q3Wizard::exec();
}




void Wizard::back() {
  QWidget *w;
  WizardAction *p;

  w=currentPage();
  if (w!=startPage) {
    p=dynamic_cast<WizardAction*>(w);
    assert(p);
    p->leave(true);
  }

  Q3Wizard::back();
  w=currentPage();
  if (w!=startPage) {
    p=dynamic_cast<WizardAction*>(w);
    assert(p);
    p->undo();
  }
}



void Wizard::next() {
  QWidget *w;
  WizardAction *p;

  w=currentPage();
  if (w!=startPage) {
    p=dynamic_cast<WizardAction*>(w);
    assert(p);
    if (!(p->apply())) {
      return;
    }
    p->leave(false);
  }

  Q3Wizard::next();
  w=currentPage();
  p=dynamic_cast<WizardAction*>(w);
  assert(p);
  DBG_INFO(0, "Entering \"%s\"",
           QBanking::QStringToUtf8String(p->getName()).c_str());
  p->enter();
  if (w==_lastActionWidget)
    setFinishEnabled(w, TRUE);
  else
    setFinishEnabled(w, FALSE);

}



void Wizard::setNextEnabled(WizardAction *a, bool b) {
  DBG_INFO(0, "SetNextEnabled for page \"%s\": %s",
           QBanking::QStringToUtf8String(a->getName()).c_str(),
           (b==true)?"Enabled":"Disabled");
  Q3Wizard::setNextEnabled(a, b);
}


void Wizard::setBackEnabled(WizardAction *a, bool b) {
  DBG_INFO(0, "SetBackEnabled for page \"%s\": %s",
           QBanking::QStringToUtf8String(a->getName()).c_str(),
           (b==true)?"Enabled":"Disabled");
  Q3Wizard::setBackEnabled(a, b);
}





#include "wizard.moc"





