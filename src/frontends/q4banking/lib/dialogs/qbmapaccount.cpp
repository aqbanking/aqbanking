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
#include "qbmapaccount.h"
#include "qbaccountlist.h"

// QT includes
#include <qlabel.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <q3groupbox.h>
#include <qlineedit.h>




QBMapAccount::QBMapAccount(QBanking *kb,
                           const char *bankCode,
                           const char *accountId,
                           QWidget* parent,
                           const char* name,
                           bool modal,
                           Qt::WFlags fl)
:QDialog(parent, name, modal, fl)
,Ui_QBMapAccountUi()
,_banking(kb)
,_account(0) {
  setupUi(this);

  accountList->setSelectionMode(Q3ListView::Single);
  accountList->setAllColumnsShowFocus(true);

  if (bankCode)
    bankCodeEdit->setText(QString::fromUtf8(bankCode));
  else
    bankCodeEdit->setEnabled(false);
  if (accountId)
    accountIdEdit->setText(QString::fromUtf8(accountId));
  else
    accountIdEdit->setEnabled(false);

  QObject::connect(accountList, SIGNAL(selectionChanged()),
                   this, SLOT(slotSelectionChanged()));
  QObject::connect(helpButton, SIGNAL(clicked()),
                   this, SLOT(slotHelpClicked()));

  accountList->addAccounts(_banking->getAccounts());
}


QBMapAccount::~QBMapAccount(){
}



AB_ACCOUNT *QBMapAccount::getAccount(){
  return _account;
}



void QBMapAccount::accept(){
  if (_account)
    QDialog::accept();
}



void QBMapAccount::slotSelectionChanged(){
  std::list<AB_ACCOUNT*> al;
  AB_ACCOUNT *a;

  al=accountList->getSelectedAccounts();
  if (al.empty()) {
    assignButton->setEnabled(false);
    _account=0;
    return;
  }
  a=al.front();
  if (AB_Account_GetUniqueId(a)!=0) {
    _account=a;
    assignButton->setEnabled(true);
  }
  else
    assignButton->setEnabled(false);
}



void QBMapAccount::slotHelpClicked() {
  _banking->invokeHelp("QBMapAccount", "none");
}







#include "qbmapaccount.moc"



