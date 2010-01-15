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
#include "qbcfgtabpageaccountgen.h"

// Gwenhywfar includes
#include <gwenhywfar/debug.h>

// QT includes
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qcheckbox.h>




QBCfgTabPageAccountGeneral::QBCfgTabPageAccountGeneral(QBanking *qb,
                                                       AB_ACCOUNT *a,
                                                       QWidget *parent,
                                                       const char *name,
                                                       Qt::WFlags f)
:QBCfgTabPageAccount(qb, tr("General"), a, parent, name, f){
  int i;

  _realPage.setupUi(this);

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

  _realPage.userList1->setSelectionMode(Q3ListView::Single);
  _realPage.userList1->setAllColumnsShowFocus(true);
  for (i=0; i<_realPage.userList1->columns(); i++)
    _realPage.userList1->setColumnWidthMode(i, Q3ListView::Manual);
  _realPage.userList1->setColumnWidth(3, 0);

  _realPage.userList2->setSelectionMode(Q3ListView::Single);
  _realPage.userList2->setAllColumnsShowFocus(true);
  for (i=0; i<_realPage.userList2->columns(); i++)
    _realPage.userList2->setColumnWidthMode(i, Q3ListView::Manual);
  _realPage.userList2->setColumnWidth(3, 0);
  _realPage.userList2->setSorting(-1);

  connect(_realPage.bankIdButton, SIGNAL(clicked()),
          SLOT(slotBankIdButtonClicked()));
  connect(_realPage.rightButton, SIGNAL(clicked()),
          SLOT(slotRightButtonClicked()));
  connect(_realPage.leftButton, SIGNAL(clicked()),
          SLOT(slotLeftButtonClicked()));
  connect(_realPage.allUsersCheck, SIGNAL(toggled(bool)),
          SLOT(slotAllUsersToggled(bool)));

  fillCountryCombo(_realPage.countryCombo);
}



QBCfgTabPageAccountGeneral::~QBCfgTabPageAccountGeneral() {
}



bool QBCfgTabPageAccountGeneral::toGui() {
  AB_ACCOUNT *a;
  const char *s;
  AB_USER_LIST2 *ulAll;
  AB_USER_LIST2 *ulSel;
  AB_ACCOUNT_TYPE at;
  int idx;

  a=getAccount();
  assert(a);

  s=AB_Account_GetAccountNumber(a);
  if (s)
    _realPage.accountIdEdit->setText(QString::fromUtf8(s));
  s=AB_Account_GetAccountName(a);
  if (s)
    _realPage.accountNameEdit->setText(QString::fromUtf8(s));

  s=AB_Account_GetOwnerName(a);
  if (s)
    _realPage.ownerNameEdit->setText(QString::fromUtf8(s));

  s=AB_Account_GetBankCode(getAccount());
  if (s)
    _realPage.bankIdEdit->setText(QString::fromUtf8(s));

  s=AB_Account_GetBankName(getAccount());
  if (s)
    _realPage.bankNameEdit->setText(QString::fromUtf8(s));

  s=AB_Account_GetIBAN(getAccount());
  if (s)
    _realPage.ibanEdit->setText(QString::fromUtf8(s));
  s=AB_Account_GetBIC(getAccount());
  if (s)
    _realPage.bicEdit->setText(QString::fromUtf8(s));

  selectCountryInCombo(_realPage.countryCombo,
                       AB_Account_GetCountry(getAccount()));

  idx=0;
  at=AB_Account_GetAccountType(a);
  switch(at) {
  case AB_AccountType_Unknown:     idx=1; break; /* unknown->bank */
  case AB_AccountType_Bank:        idx=1; break;
  case AB_AccountType_CreditCard:  idx=2; break;
  case AB_AccountType_Checking:    idx=3; break;
  case AB_AccountType_Savings:     idx=4; break;
  case AB_AccountType_Investment:  idx=5; break;
  case AB_AccountType_Cash:        idx=6; break;
  case AB_AccountType_MoneyMarket: idx=7; break;
  }
  _realPage.accountTypeCombo->setCurrentItem(idx);


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
  AB_ACCOUNT_TYPE at;

  a=getAccount();
  assert(a);

  s=QBanking::QStringToUtf8String(QBanking::sanitizedNumber(_realPage.accountIdEdit->text()));
  if (s.empty())
    AB_Account_SetAccountNumber(a, 0);
  else
    AB_Account_SetAccountNumber(a, s.c_str());

  s=QBanking::QStringToUtf8String(_realPage.accountNameEdit->text());
  if (s.empty())
    AB_Account_SetAccountName(a, 0);
  else
    AB_Account_SetAccountName(a, s.c_str());

  s=QBanking::QStringToUtf8String(_realPage.ownerNameEdit->text());
  if (s.empty())
    AB_Account_SetOwnerName(a, 0);
  else
    AB_Account_SetOwnerName(a, s.c_str());

  s=QBanking::QStringToUtf8String(QBanking::sanitizedNumber(_realPage.bankIdEdit->text()));
  if (s.empty())
    AB_Account_SetBankCode(a, 0);
  else
    AB_Account_SetBankCode(a, s.c_str());

  s=QBanking::QStringToUtf8String(_realPage.bankNameEdit->text());
  if (s.empty())
    AB_Account_SetBankName(a, 0);
  else
    AB_Account_SetBankName(a, s.c_str());

  s=QBanking::QStringToUtf8String(QBanking::sanitizedAlphaNum(_realPage.ibanEdit->text()));
  if (s.empty())
    AB_Account_SetIBAN(a, 0);
  else
    AB_Account_SetIBAN(a, s.c_str());

  s=QBanking::QStringToUtf8String(QBanking::sanitizedAlphaNum(_realPage.bicEdit->text()));
  if (s.empty())
    AB_Account_SetBIC(a, 0);
  else
    AB_Account_SetBIC(a, s.c_str());

  s=QBanking::QStringToUtf8String(_realPage.countryCombo->currentText());
  assert(!s.empty());

  ci=AB_Banking_FindCountryByLocalName(getBanking()->getCInterface(),
                                       s.c_str());
  assert(ci);
  cs=AB_Country_GetCode(ci);
  assert(cs);
  AB_Account_SetCountry(a, cs);

  ul=_realPage.userList2->getSortedUsersList2();
  if (ul) {
    AB_Account_SetSelectedUsers(a, ul);
    //if (AB_Account_GetFirstUser(a)==0)
      AB_Account_SetUsers(a, ul);
    AB_User_List2_free(ul);
  }

  switch(_realPage.accountTypeCombo->currentItem()) {
  case 0:  at=AB_AccountType_Unknown; break;
  case 1:  at=AB_AccountType_Bank; break;
  case 2:  at=AB_AccountType_CreditCard; break;
  case 3:  at=AB_AccountType_Checking; break;
  case 4:  at=AB_AccountType_Savings; break;
  case 5:  at=AB_AccountType_Investment; break;
  case 6:  at=AB_AccountType_Cash; break;
  case 7:  at=AB_AccountType_MoneyMarket; break;
  default: at=AB_AccountType_Unknown; break;
  }
  AB_Account_SetAccountType(a, at);

  return true;
}



bool QBCfgTabPageAccountGeneral::checkGui() {
  std::string s;

  if (_realPage.accountIdEdit->text().isEmpty() &&
      _realPage.accountNameEdit->text().isEmpty()) {
    QMessageBox::critical(this,
                          tr("Input Error"),
                          tr("<qt>"
                             "You must at least provide some kind of account "
                             "identification (account id, customer id or "
                             "account name)"
                             "</qt>"),
                          tr("Dismiss"));
    return false;
  }
  if (_realPage.ownerNameEdit->text().isEmpty()) {
    QMessageBox::critical(this,
                          tr("Input Error"),
                          tr("<qt>"
                             "Owner name is missing."
                             "</qt>"),
                          tr("Dismiss"));
    return false;
  }

  s=QBanking::QStringToUtf8String(QBanking::sanitizedAlphaNum(_realPage.ibanEdit->text()));
  if (!s.empty()) {
    if (AB_Banking_CheckIban(s.c_str())) {
      QMessageBox::critical(this,
			    tr("Input Error"),
			    tr("<qt>"
			       "The IBAN you entered is invalid. "
			       "Please correct."
			       "</qt>"),
			    tr("Dismiss"));
      return false;
    }
  }

  if (_realPage.bankIdEdit->text().isEmpty() &&
      _realPage.bankNameEdit->text().isEmpty()) {
    QMessageBox::critical(this,
                          tr("Input Error"),
                          tr("<qt>"
                             "You must at least provide some kind of bank "
                             "identification (bank id or name)."
                             "</qt>"),
                          tr("Dismiss"));
    return false;
  }

  if (_realPage.countryCombo->currentItem()==0) {
    QMessageBox::critical(this,
                          tr("Input Error"),
                          tr("<qt>"
                             "Please select a country."
                             "</qt>"),
                          tr("Dismiss"));
    return false;
  }

  if (!_realPage.userList1->getSortedUsers().empty() &&
      _realPage.userList2->getSortedUsers().empty()) {
    QMessageBox::critical(this,
                          tr("Input Error"),
                          tr("<qt>"
                             "Please assign users."
                             "</qt>"),
                          tr("Dismiss"));
    return false;
  }

  return true;
}



void QBCfgTabPageAccountGeneral::updateView() {
  toGui();
}



void QBCfgTabPageAccountGeneral::slotBankIdButtonClicked() {
  if (_realPage.countryCombo->currentItem()==0) {
    QMessageBox::critical(this,
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

    s=QBanking::QStringToUtf8String(_realPage.countryCombo->currentText());
    assert(!s.empty());

    ci=AB_Banking_FindCountryByLocalName(getBanking()->getCInterface(),
                                         s.c_str());
    assert(ci);
    cs=AB_Country_GetCode(ci);
    assert(cs);


    bi=getBanking()->selectBank(this,
                                tr("Select the Bank for this Account"),
                                QString::fromUtf8(cs),
                                _realPage.bankIdEdit->text());
    if (bi) {
      const char *t;

      t=AB_BankInfo_GetBankId(bi);
      if (t)
        _realPage.bankIdEdit->setText(QString::fromUtf8(t));
      t=AB_BankInfo_GetBankName(bi);
      if (t)
        _realPage.bankNameEdit->setText(QString::fromUtf8(t));
      t=AB_BankInfo_GetBic(bi);
      if (t)
        _realPage.bicEdit->setText(QString::fromUtf8(t));

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
  _realPage.userList1->clear();
  _realPage.userList2->clear();
  if (ulAll) {
    AB_USER_LIST2_ITERATOR *it;

    it=AB_User_List2_First(ulAll);
    if (it) {
      AB_USER *tu;

      tu=AB_User_List2Iterator_Data(it);
      while(tu) {
        if (ulSel && _listHasUser(ulSel, tu))
          _realPage.userList2->addUser(tu);
        else
          _realPage.userList1->addUser(tu);
        tu=AB_User_List2Iterator_Next(it);
      }

      AB_User_List2Iterator_free(it);
    }
  }
  else if (ulSel){
    AB_USER_LIST2_ITERATOR *it;

    it=AB_User_List2_First(ulSel);
    if (it) {
      AB_USER *tu;

      tu=AB_User_List2Iterator_Data(it);
      while(tu) {
        _realPage.userList2->addUser(tu);
        tu=AB_User_List2Iterator_Next(it);
      }

      AB_User_List2Iterator_free(it);
    }
  }
}



void QBCfgTabPageAccountGeneral::slotLeftButtonClicked() {
  std::list<AB_USER*> ul;
  AB_USER *u;

  ul=_realPage.userList2->getSelectedUsers();
  if (ul.empty()) {
    QMessageBox::critical(this,
                          tr("Selection Error"),
                          tr("No user selected."),
                          QMessageBox::Retry,QMessageBox::NoButton);
    return;
  }
  u=ul.front();
  _realPage.userList2->removeUser(u);
  _realPage.userList1->addUser(u);
}



void QBCfgTabPageAccountGeneral::slotRightButtonClicked() {
  std::list<AB_USER*> ul;
  AB_USER *u;

  ul=_realPage.userList1->getSelectedUsers();
  if (ul.empty()) {
    QMessageBox::critical(this,
                          tr("Selection Error"),
                          tr("No user selected."),
                          QMessageBox::Retry,QMessageBox::NoButton);
    return;
  }
  u=ul.front();
  _realPage.userList1->removeUser(u);
  _realPage.userList2->addUser(u);
}



void QBCfgTabPageAccountGeneral::slotAllUsersToggled(bool on) {
  AB_ACCOUNT *a;
  AB_PROVIDER *pro;
  AB_USER_LIST2 *ulAll;
  AB_USER_LIST2 *ulSel;

  a=getAccount();
  assert(a);
  pro=AB_Account_GetProvider(a);
  assert(pro);

  if (on) {
    ulAll=AB_Banking_FindUsers(getBanking()->getCInterface(),
                               AB_Provider_GetName(pro),
                               "*", "*", "*", "*");
    ulSel=AB_Account_GetSelectedUsers(a);
    _addUsersToLists(ulAll, ulSel);
  }
  else {
    ulAll=AB_Account_GetUsers(a);
    ulSel=AB_Account_GetSelectedUsers(a);
    _addUsersToLists(ulAll, ulSel);
  }
  AB_User_List2_free(ulAll);
  AB_User_List2_free(ulSel);
}





#include "qbcfgtabpageaccountgen.moc"



