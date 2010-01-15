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

// QBanking includes
#include "qbpickstartdate.h"

// Gwenhywfar includes
#include <gwenhywfar/debug.h>

// QT includes
#include <q3datetimeedit.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <q3buttongroup.h>





QBPickStartDate::QBPickStartDate(QBanking *qb,
                                 const QDate &firstPossible,
                                 const QDate &lastUpdate,
                                 int defaultChoice,
                                 QWidget* parent, const char* name,
                                 bool modal, Qt::WFlags fl)
:QDialog(parent, name, modal, fl)
,Ui_QBPickStartDateUi()
,_banking(qb)
,_firstPossible(firstPossible)
,_lastUpdate(lastUpdate){
  setupUi(this);

  QObject::connect(noDateButton, SIGNAL(toggled(bool)),
                   this, SLOT(slotNoDateToggled(bool)));
  QObject::connect(lastUpdateButton, SIGNAL(toggled(bool)),
                   this, SLOT(slotLastUpdateToggled(bool)));
  QObject::connect(firstDateButton, SIGNAL(toggled(bool)),
                   this, SLOT(slotFirstDateToggled(bool)));
  QObject::connect(pickDateButton, SIGNAL(toggled(bool)),
                   this, SLOT(slotPickDateToggled(bool)));
  QObject::connect(helpButton, SIGNAL(clicked()),
                   this, SLOT(slotHelpClicked()));

  if (_lastUpdate.isValid()) {
    lastUpdateLabel->setText(_lastUpdate.toString());
    lastUpdateButton->setEnabled(true);
    lastUpdateLabel->setEnabled(true);
  }
  else {
    lastUpdateButton->setEnabled(false);
    lastUpdateLabel->setEnabled(false);
    if (defaultChoice==2)
      defaultChoice=1;
  }

  if (_firstPossible.isValid()) {
    firstDateLabel->setText(_firstPossible.toString());
    firstDateButton->setEnabled(true);
    firstDateLabel->setEnabled(true);
    pickDateEdit->setRange(_firstPossible, QDate());
  }
  else {
    firstDateButton->setEnabled(false);
    firstDateLabel->setEnabled(false);
    if (defaultChoice==3)
      defaultChoice=1;
  }

  switch(defaultChoice) {
  case 2:  lastUpdateButton->setChecked(true); break;
  case 3:  firstDateButton->setChecked(true); break;
  default: noDateButton->setChecked(true); break;
  }

  pickDateEdit->setDate(QDate::currentDate());

  buttonGroup->setFocus();
}



QBPickStartDate::~QBPickStartDate(){
}



void QBPickStartDate::slotNoDateToggled(bool on){
}



void QBPickStartDate::slotLastUpdateToggled(bool on){
}



void QBPickStartDate::slotFirstDateToggled(bool on){
}



void QBPickStartDate::slotPickDateToggled(bool on){
  pickDateEdit->setEnabled(on);
}





QDate QBPickStartDate::getDate() {
  if (noDateButton->isChecked())
    return QDate();
  else if (firstDateButton->isChecked())
    return _firstPossible;
  else if (pickDateButton->isChecked())
    return pickDateEdit->date();
  else if (lastUpdateButton->isChecked())
    return _lastUpdate;
  else {
    DBG_ERROR(0, "Unknown date state");
    return QDate();
  }
}



void QBPickStartDate::slotHelpClicked() {
  _banking->invokeHelp("QBPickStartDate", "none");
}




#include "qbpickstartdate.moc"


