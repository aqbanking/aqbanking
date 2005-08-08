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


#include "mapaccount.h"
#include "kbanking.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlineedit.h>




MapAccount::MapAccount(KBanking *kb,
                       const char *bankCode,
                       const char *accountId,
                       QWidget* parent,
                       const char* name,
                       bool modal,
                       WFlags fl)
:MapAccountUi(parent, name, modal, fl)
,_banking(kb)
,_account(0) {

  // Manually create and add layout here because the .ui-generated
  // QGroupBox doesn't have one.
  accountBox->setColumnLayout(0, Qt::Vertical );
  QBoxLayout *accountBoxLayout = new QHBoxLayout(accountBox->layout() );
  accountBoxLayout->setAlignment(Qt::AlignTop);
  _accountList=new AccountListView((QWidget*)accountBox, "AccountList");
  accountBoxLayout->addWidget(_accountList);
  _accountList->setSelectionMode(QListView::Single);
  _accountList->setAllColumnsShowFocus(true);

  if (bankCode)
    bankCodeEdit->setText(bankCode);
  else
    bankCodeEdit->setEnabled(false);
  if (accountId)
    accountIdEdit->setText(accountId);
  else
    accountIdEdit->setEnabled(false);

  QObject::connect((QObject*)_accountList, SIGNAL(selectionChanged()),
                   this, SLOT(slotSelectionChanged()));

  _accountList->addAccounts(_banking->getAccounts());
}


MapAccount::~MapAccount(){
}



AB_ACCOUNT *MapAccount::getAccount(){
  return _account;
}



void MapAccount::accept(){
  if (_account)
    QDialog::accept();
}



void MapAccount::slotSelectionChanged(){
  std::list<AB_ACCOUNT*> al;
  AB_ACCOUNT *a;

  al=_accountList->getSelectedAccounts();
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










