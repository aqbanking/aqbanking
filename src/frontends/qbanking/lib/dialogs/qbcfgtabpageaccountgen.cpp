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


#include "qbcfgtabpageaccountgen.h"
#include "qbcfgtabpageaccountgen.ui.h"

#include <qbanking/qbanking.h>
#include <qbanking/qbuserlist.h>

#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlayout.h>

#include <gwenhywfar/debug.h>



QBCfgTabPageAccountGeneral::QBCfgTabPageAccountGeneral(QBanking *qb,
                                                       AB_ACCOUNT *a,
                                                       QWidget *parent,
                                                       const char *name,
                                                       WFlags f)
:QBCfgTabPageAccount(qb, tr("General"), a, parent, name, f){
  int i;

  setHelpSubject("QBCfgTabPageAccountGeneral");
  setDescription(tr(
                    "<p>This page contains some general settings like "
                    "account name, account number, bank code, "
                    "connected users etc.</p>"
                    "<p>To each account one or more <i>users</i> are "
                    "assigned. "
                    "These users provide the means by which we "
                    "<b>identify</b> ourselves to the bank (by means of "
                    "<i>user id</i>, security media etc).</p>"));

  _realPage=new QBCfgTabPageAccountGeneralUi(this);
  _realPage->userList1->setSelectionMode(QListView::Single);
  _realPage->userList1->setAllColumnsShowFocus(true);
  for (i=0; i<_realPage->userList1->columns(); i++)
    _realPage->userList1->setColumnWidthMode(i, QListView::Manual);
  _realPage->userList1->setColumnWidth(3, 0);

  _realPage->userList2->setSelectionMode(QListView::Single);
  _realPage->userList2->setAllColumnsShowFocus(true);
  for (i=0; i<_realPage->userList2->columns(); i++)
    _realPage->userList2->setColumnWidthMode(i, QListView::Manual);
  _realPage->userList2->setColumnWidth(3, 0);
  _realPage->userList2->setSorting(-1);

  addWidget(_realPage);
  _realPage->show();
  connect(_realPage->bankIdButton, SIGNAL(clicked()),
          SLOT(slotBankIdButtonClicked()));
  connect(_realPage->rightButton, SIGNAL(clicked()),
          SLOT(slotRightButtonClicked()));
  connect(_realPage->leftButton, SIGNAL(clicked()),
          SLOT(slotLeftButtonClicked()));

  fillCountryCombo(_realPage->countryCombo);
}



QBCfgTabPageAccountGeneral::~QBCfgTabPageAccountGeneral() {
}



bool QBCfgTabPageAccountGeneral::toGui() {
  AB_ACCOUNT *a;
  const char *s;
  AB_USER_LIST2 *ulAll;
  AB_USER_LIST2 *ulSel;

  a=getAccount();
  assert(a);

  s=AB_Account_GetAccountNumber(a);
  if (s)
    _realPage->accountIdEdit->setText(QString::fromUtf8(s));
  s=AB_Account_GetAccountName(a);
  if (s)
    _realPage->accountNameEdit->setText(QString::fromUtf8(s));

  s=AB_Account_GetOwnerName(a);
  if (s)
    _realPage->ownerNameEdit->setText(QString::fromUtf8(s));

  s=AB_Account_GetBankCode(getAccount());
  if (s)
    _realPage->bankIdEdit->setText(QString::fromUtf8(s));

  s=AB_Account_GetBankName(getAccount());
  if (s)
    _realPage->bankNameEdit->setText(QString::fromUtf8(s));

  selectCountryInCombo(_realPage->countryCombo,
                       AB_Account_GetCountry(getAccount()));

  ulAll=AB_Account_GetUsers(a);
  ulSel=AB_Account_GetSelectedUsers(a);
  _addUsersToLists(ulAll, ulSel);
  AB_User_List2_free(ulAll);
  AB_User_List2_free(ulSel);

  return true;
}



bool QBCfgTabPageAccountGeneral::fromGui() {
  AB_ACCOUNT *a;
  std::string s;
  const char *cs;
  const AB_COUNTRY *ci;
  AB_USER_LIST2 *ul;

  a=getAccount();
  assert(a);

  s=QBanking::QStringToUtf8String(_realPage->accountIdEdit->text());
  if (s.empty())
    AB_Account_SetAccountNumber(a, 0);
  else
    AB_Account_SetAccountNumber(a, s.c_str());

  s=QBanking::QStringToUtf8String(_realPage->accountNameEdit->text());
  if (s.empty())
    AB_Account_SetAccountName(a, 0);
  else
    AB_Account_SetAccountName(a, s.c_str());

  s=QBanking::QStringToUtf8String(_realPage->ownerNameEdit->text());
  if (s.empty())
    AB_Account_SetOwnerName(a, 0);
  else
    AB_Account_SetOwnerName(a, s.c_str());

  s=QBanking::QStringToUtf8String(_realPage->bankIdEdit->text());
  if (s.empty())
    AB_Account_SetBankCode(a, 0);
  else
    AB_Account_SetBankCode(a, s.c_str());

  s=QBanking::QStringToUtf8String(_realPage->bankNameEdit->text());
  if (s.empty())
    AB_Account_SetBankName(a, 0);
  else
    AB_Account_SetBankName(a, s.c_str());

  s=QBanking::QStringToUtf8String(_realPage->countryCombo->currentText());
  assert(!s.empty());

  ci=AB_Banking_FindCountryByLocalName(getBanking()->getCInterface(),
                                       s.c_str());
  assert(ci);
  cs=AB_Country_GetCode(ci);
  assert(cs);
  AB_Account_SetCountry(a, cs);

  ul=_realPage->userList2->getSortedUsersList2();
  if (ul) {
    AB_Account_SetSelectedUsers(a, ul);
    AB_User_List2_free(ul);
  }

  return true;
}



bool QBCfgTabPageAccountGeneral::checkGui() {
  if (_realPage->accountIdEdit->text().isEmpty() &&
      _realPage->accountNameEdit->text().isEmpty()) {
    QMessageBox::critical(0,
                          tr("Input Error"),
                          tr("<qt>"
                             "You must at least provide some kind of account "
                             "idenitification (account id, customer id or "
                             "account name)"
                             "</qt>"),
                          tr("Dismiss"));
    return false;
  }
  if (_realPage->ownerNameEdit->text().isEmpty()) {
    QMessageBox::critical(0,
                          tr("Input Error"),
                          tr("<qt>"
                             "Owner name is missing."
                             "</qt>"),
                          tr("Dismiss"));
    return false;
  }

  if (_realPage->bankIdEdit->text().isEmpty() &&
      _realPage->bankNameEdit->text().isEmpty()) {
    QMessageBox::critical(0,
                          tr("Input Error"),
                          tr("<qt>"
                             "You must at least provide some kind of bank "
                             "idenitification (bank id or name)."
                             "</qt>"),
                          tr("Dismiss"));
    return false;
  }

  if (_realPage->countryCombo->currentItem()==0) {
    QMessageBox::critical(0,
                          tr("Input Error"),
                          tr("<qt>"
                             "Please select a country."
                             "</qt>"),
                          tr("Dismiss"));
    return false;
  }

  if (!_realPage->userList1->getSortedUsers().empty() &&
      _realPage->userList2->getSortedUsers().empty()) {
    QMessageBox::critical(0,
                          tr("Input Error"),
                          tr("<qt>"
                             "Please assign users."
                             "</qt>"),
                          tr("Dismiss"));
    return false;
  }

  return true;
}



void QBCfgTabPageAccountGeneral::slotBankIdButtonClicked() {
  if (_realPage->countryCombo->currentItem()==0) {
    QMessageBox::critical(0,
                          tr("Country Needed"),
                          tr("<qt>"
                             "Please select a country first."
                             "</qt>"),
                          tr("Dismiss"));
  }
  else {
    std::string s;
    const char *cs;
    const AB_COUNTRY *ci;
    AB_BANKINFO *bi;

    s=QBanking::QStringToUtf8String(_realPage->countryCombo->currentText());
    assert(!s.empty());

    ci=AB_Banking_FindCountryByLocalName(getBanking()->getCInterface(),
                                         s.c_str());
    assert(ci);
    cs=AB_Country_GetCode(ci);
    assert(cs);


    bi=getBanking()->selectBank(this,
                                tr("Select the Bank For This Account"),
                                QString::fromUtf8(cs),
                                _realPage->bankIdEdit->text());
    if (bi) {
      const char *t;

      t=AB_BankInfo_GetBankId(bi);
      if (t)
        _realPage->bankIdEdit->setText(QString::fromUtf8(t));
      t=AB_BankInfo_GetBankName(bi);
      if (t)
        _realPage->bankNameEdit->setText(QString::fromUtf8(t));

      AB_BankInfo_free(bi);
    }
  }
}



bool QBCfgTabPageAccountGeneral::_listHasUser(AB_USER_LIST2 *ul, AB_USER *u) {
  AB_USER_LIST2_ITERATOR *it;

  assert(ul);
  it=AB_User_List2_First(ul);
  if (it) {
    AB_USER *tu;

    tu=AB_User_List2Iterator_Data(it);
    while(tu) {
      if (tu==u)
        return true;
      tu=AB_User_List2Iterator_Next(it);
    }

    AB_User_List2Iterator_free(it);
  }

  return false;
}



void QBCfgTabPageAccountGeneral::_addUsersToLists(AB_USER_LIST2 *ulAll,
                                                  AB_USER_LIST2 *ulSel) {
  _realPage->userList1->clear();
  _realPage->userList2->clear();
  if (ulAll) {
    AB_USER_LIST2_ITERATOR *it;

    it=AB_User_List2_First(ulAll);
    if (it) {
      AB_USER *tu;

      tu=AB_User_List2Iterator_Data(it);
      while(tu) {
        if (ulSel && _listHasUser(ulSel, tu))
          _realPage->userList2->addUser(tu);
        else
          _realPage->userList1->addUser(tu);
        tu=AB_User_List2Iterator_Next(it);
      }

      AB_User_List2Iterator_free(it);
    }
  }
}



void QBCfgTabPageAccountGeneral::slotLeftButtonClicked() {
  std::list<AB_USER*> ul;
  AB_USER *u;

  ul=_realPage->userList2->getSelectedUsers();
  if (ul.empty()) {
    QMessageBox::critical(this,
                          tr("Selection Error"),
                          tr("No user selected.\n"),
                          QMessageBox::Retry,QMessageBox::NoButton);
  }
  u=ul.front();
  _realPage->userList2->removeUser(u);
  _realPage->userList1->addUser(u);
}



void QBCfgTabPageAccountGeneral::slotRightButtonClicked() {
  std::list<AB_USER*> ul;
  AB_USER *u;

  ul=_realPage->userList1->getSelectedUsers();
  if (ul.empty()) {
    QMessageBox::critical(this,
                          tr("Selection Error"),
                          tr("No user selected.\n"),
                          QMessageBox::Retry,QMessageBox::NoButton);
  }
  u=ul.front();
  _realPage->userList1->removeUser(u);
  _realPage->userList2->addUser(u);
}










