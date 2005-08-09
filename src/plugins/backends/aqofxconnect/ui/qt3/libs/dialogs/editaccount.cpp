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


#include "editaccount.h"

#include <aqbanking/country.h>

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qfiledialog.h>

#include <gwenhywfar/debug.h>
#include <aqofxconnect/bank.h>
#include <aqofxconnect/provider.h>

#ifdef WIN32
# define strcasecmp stricmp
#endif


EditAccount::EditAccount(QBanking *app,
                         AB_ACCOUNT *a,
                         bool isNew,
                         QWidget* parent, const char* name,
                         bool modal, WFlags fl)
:EditAccountUi(parent, name, modal, fl)
,_app(app), _account(a), _isNew(isNew){
  bankCodeEdit->setEnabled(isNew);
  accountIdEdit->setEnabled(isNew);
  countryCombo->setEnabled(isNew);

  QObject::connect((QObject*)bankCodeEdit, SIGNAL(lostFocus()),
                   this, SLOT(slotBankCodeLostFocus()));
  QObject::connect((QObject*)whatsThisButton, SIGNAL(clicked()),
                   this, SLOT(slotWhatsThis()));
}



EditAccount::~EditAccount(){
}



bool EditAccount::init() {
  accountToGui(_account);
  return true;
}



void EditAccount::countriesToCombo(QComboBox *qc, const char *c) {
  AB_COUNTRY_CONSTLIST2 *countries;
  QString selected;

  countries=AB_Banking_ListCountriesByName(_app->getCInterface(), "*");
  qc->clear();
  qc->insertItem(tr("-- select country --"));
  if (countries) {
    AB_COUNTRY_CONSTLIST2_ITERATOR *cit;
    QStringList sl;
    int idx=0;
    int i=0;

    DBG_ERROR(0, "Have %d entries",
              AB_Country_ConstList2_GetSize(countries));
    cit=AB_Country_ConstList2_First(countries);
    if (cit) {
      const AB_COUNTRY *cnt;

      cnt=AB_Country_ConstList2Iterator_Data(cit);
      assert(cnt);
      while(cnt) {
	const char *s;
        QString qs;

	s=AB_Country_GetLocalName(cnt);
	if (s==0)
	  qs=tr("unknown");
	else
	  qs=QString::fromUtf8(s);
	s=AB_Country_GetCode(cnt);
	if (c && *c && s && *s && strcasecmp(s, c)==0)
	  selected=qs;
	sl.append(qs);
	cnt=AB_Country_ConstList2Iterator_Next(cit);
      } // while */
      AB_Country_ConstList2Iterator_free(cit);
    } // if cit

    sl.sort();

    for (QStringList::Iterator it=sl.begin(); it!= sl.end(); ++it) {
      i++;
      qc->insertItem(*it);
      if (selected.lower()==(*it).lower())
        idx=i;
    } // for
    if (idx)
      qc->setCurrentItem(idx);

  } // if _countries
}



void EditAccount::usersToCombo(QComboBox *qc,
                               const char *country,
			       const char *bankCode,
			       const char *userId){
  AO_USER_LIST *users;
  QString selected;
  AO_BANK *b;
  AB_PROVIDER *pro;

  qc->clear();
  if (!country || !(*country) || !bankCode || !(*bankCode))
    return;
  pro=AB_Banking_GetProvider(_app->getCInterface(), "aqofxconnect");
  assert(pro);
  b=AO_Provider_FindMyBank(pro, country, bankCode);
  if (!b)
    return;

  users=AO_Bank_GetUsers(b);
  if (users) {
    AO_USER *u;
    QStringList sl;
    int idx=0;
    int i=0;

    qc->insertItem(tr("-- select user --"));
    DBG_ERROR(0, "Have %d entries",
              AO_User_List_GetCount(users));
    u=AO_User_List_First(users);
    while(u) {
      const char *s;
      QString qs;

      s=AO_User_GetUserId(u);
      if (s==0)
	qs=tr("unknown");
      else
	qs=QString::fromUtf8(s);
      if (userId && *userId && s && *s && strcasecmp(s, userId)==0)
	selected=qs;
      sl.append(qs);
      u=AO_User_List_Next(u);
    } // while */

    sl.sort();

    for (QStringList::Iterator it=sl.begin(); it!= sl.end(); ++it) {
      i++;
      qc->insertItem(*it);
      if (selected.lower()==(*it).lower())
        idx=i;
    } // for
    if (idx)
      qc->setCurrentItem(idx);

  } // if _users

}



void EditAccount::accountToGui(AB_ACCOUNT *a) {
  const char *s;
  const char *country;
  const char *bankId;
  const char *userId;

  country=AB_Account_GetCountry(a);
  if (!country || !*country)
    country="us";
  countriesToCombo(countryCombo, country);

  bankId=AB_Account_GetBankCode(a);
  if (bankId)
    bankCodeEdit->setText(QString::fromUtf8(bankId));
  s=AB_Account_GetBankName(a);
  if (s)
    bankNameEdit->setText(QString::fromUtf8(s));
  s=AB_Account_GetAccountNumber(a);
  if (s)
    accountIdEdit->setText(QString::fromUtf8(s));
  s=AB_Account_GetAccountName(a);
  if (s)
    accountNameEdit->setText(QString::fromUtf8(s));
  userId=AO_Account_GetUserId(a);
  usersToCombo(userCombo, country, bankId, userId);

}



void EditAccount::guiToAccount(AB_ACCOUNT *a) {
  std::string s;

  s=QBanking::QStringToUtf8String(bankCodeEdit->text());
  AB_Account_SetBankCode(a, s.c_str());

  s=QBanking::QStringToUtf8String(bankNameEdit->text());
  if (s.empty()) AB_Account_SetBankName(a, 0);
  else AB_Account_SetBankName(a, s.c_str());

  s=QBanking::QStringToUtf8String(accountIdEdit->text());
  AB_Account_SetAccountNumber(a, s.c_str());

  s=QBanking::QStringToUtf8String(accountNameEdit->text());
  if (s.empty()) AB_Account_SetAccountName(a, 0);
  else AB_Account_SetAccountName(a, s.c_str());

}


void EditAccount::accept(){
  if (bankCodeEdit->text().isEmpty() ||
      accountIdEdit->text().isEmpty() ||
      userCombo->currentItem()==0) {
    QMessageBox::critical(0,
                          tr("Insufficient Input"),
			  tr("<qt>"
			     "<p>"
			     "Your input is incomplete."
			     "</p>"
			     "<p>"
			     "Please fill out all required fields or "
			     "abort the dialog."
			     "</p>"
			     "</qt>"
			    ),
                          tr("Dismiss"),0,0,0);
    return;
  }

  guiToAccount(_account);

  QDialog::accept();
}



void EditAccount::slotBankCodeLostFocus() {
  std::string s;

  s=QBanking::QStringToUtf8String(bankCodeEdit->text());
  if (!s.empty()) {
    AB_BANKINFO *bi;

    bi=AB_Banking_GetBankInfo(_app->getCInterface(),
                              "de", 0, s.c_str());
    if (bi) {
      const char *p;

      p=AB_BankInfo_GetBankName(bi);
      if (p)
        bankNameEdit->setText(QString::fromUtf8(p));
      AB_BankInfo_free(bi);
    }
  }
}



void EditAccount::slotWhatsThis(){
  QWhatsThis::enterWhatsThisMode();

}








