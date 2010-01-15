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


#include "qbcfgtab.h"
#include "qbcfgtabpage.h"

#include <qtabwidget.h>
#include <q3textbrowser.h>
#include <qpushbutton.h>

#include <gwenhywfar/debug.h>




QBCfgTab::QBCfgTab(QBanking *qb, QWidget *parent, const char *name, Qt::WFlags f)
:QDialog(parent, name, f)
,Ui_QBCfgTabUi()
,_qbanking(qb)
,_allowApply(true)
 {
  setupUi(this);
  connect(buttonHelp, SIGNAL(clicked()),
          SLOT(slotHelp()));

  connect(buttonApply, SIGNAL(clicked()),
	  SLOT(slotApply()));

}



QBCfgTab::~QBCfgTab() {
}



QBanking *QBCfgTab::getBanking() {
  return _qbanking;
}



void QBCfgTab::setDescription(const QString &s) {
  _description=s;
}



const QString &QBCfgTab::getDescription() {
  return _description;
}



void QBCfgTab::addPage(QBCfgTabPage *p) {
  const QString &title=p->getTitle();
  const QString &descr=p->getDescription();

  p->_setCfgTab(this);
  tabWidget->addTab(p, title);
  if (!title.isEmpty() && !descr.isEmpty()) {
    _fullDescription+="<h2>";
    _fullDescription+=title+"</h2>"+descr;
  }
}



QBCfgTabPage *QBCfgTab::getPage(int idx) {
  QWidget *w;

  w=tabWidget->page(idx);
  if (w)
    return dynamic_cast<QBCfgTabPage*>(w);

  return 0;
}



QBCfgTabPage *QBCfgTab::getCurrentPage() {
  QWidget *w;

  w=tabWidget->currentPage();
  if (w)
    return dynamic_cast<QBCfgTabPage*>(w);

  return 0;
}



void QBCfgTab::setCurrentPage(int idx) {
  tabWidget->setCurrentPage(idx);
}



bool QBCfgTab::fromGui() {
  int i;

  for (i=0; i<tabWidget->count(); i++) {
    QBCfgTabPage *p;

    p=getPage(i);
    if (p && !p->fromGui())
      return false;
  } // for

  return true;
}



bool QBCfgTab::toGui() {
  int i;

  for (i=0; i<tabWidget->count(); i++) {
    QBCfgTabPage *p;

    p=getPage(i);
    if (p && !p->toGui())
      return false;
  } // for

  return true;
}



bool QBCfgTab::checkGui() {
  int i;

  for (i=0; i<tabWidget->count(); i++) {
    QBCfgTabPage *p;

    p=getPage(i);
    if (p && !p->checkGui()) {
      setCurrentPage(i);
      return false;
    }
  }

  return true;
}



void QBCfgTab::updateViews() {
  int i;

  for (i=0; i<tabWidget->count(); i++) {
    QBCfgTabPage *p;

    p=getPage(i);
    if (p)
      p->updateView();
  }
}



void QBCfgTab::accept() {
  if (checkGui())
    return QDialog::accept();
}



int QBCfgTab::exec() {
  QString s;

  s="<qt>";
  s+=_description;
  s+=_fullDescription;
  s+="</qt>";
  introBrowser->setText(s);
  return QDialog::exec();
}



void QBCfgTab::setHelpContext(const QString &s) {
  _helpContext=s;
}



const QString &QBCfgTab::getHelpContext() {
  return _helpContext;
}



void QBCfgTab::slotHelp() {
  QBCfgTabPage *p;

  p=getCurrentPage();
  if (p)
    _qbanking->invokeHelp(_helpContext, p->getHelpSubject());
  else
    _qbanking->invokeHelp(_helpContext, "none");
}



void QBCfgTab::languageChange() {
  QDialog::languageChange();
}



void QBCfgTab::slotApply() {
  if (_allowApply) {
    if (checkGui()) {
      fromGui();
      toGui();
    }
  }
}



void QBCfgTab::setAllowApply(bool b) {
  _allowApply=b;
  if (!b) {
    if (buttonApply->isVisible())
      buttonApply->hide();
  }
  else {
    if (!buttonApply->isVisible())
      buttonApply->show();
  }
}



#include "qbcfgtab.moc"







