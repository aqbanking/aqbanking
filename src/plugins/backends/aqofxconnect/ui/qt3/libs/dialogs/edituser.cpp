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


#include "edituser.h"
#include <qbanking/qbselectbank.h>


extern "C" {
  // workaround
#include <aqbanking/country.h>
}

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


EditUser::EditUser(QBanking *app,
                   AO_USER *u,
                   bool isNew,
                   QWidget* parent, const char* name,
                   bool modal, WFlags fl)
:EditUserUi(parent, name, modal, fl)
,_app(app), _user(u), _isNew(isNew){
  QObject::connect((QObject*)bankCodeEdit, SIGNAL(lostFocus()),
                   this, SLOT(slotBankCodeLostFocus()));
  QObject::connect((QObject*)bankCodeButton, SIGNAL(clicked()),
                   this, SLOT(slotBankCodeClicked()));
  QObject::connect((QObject*)whatsThisButton, SIGNAL(clicked()),
                   this, SLOT(slotWhatsThis()));
}



EditUser::~EditUser(){
}



bool EditUser::init() {
  userToGui(_user);
  return true;
}



void EditUser::countriesToCombo(QComboBox *qc, const char *c) {
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



void EditUser::userToGui(AO_USER *u) {
  const char *s;
  const char *country=0;
  const char *bankId=0;
  AO_BANK *b;

  b=AO_User_GetBank(u);
  if (b) {
    QString qs;

    country=AO_Bank_GetCountry(b);
    bankId=AO_Bank_GetBankId(b);
    if (bankId)
      bankCodeEdit->setText(QString::fromUtf8(bankId));
    s=AO_Bank_GetBankName(b);
    if (s)
      bankNameEdit->setText(QString::fromUtf8(s));
    s=AO_Bank_GetOrg(b);
    if (s)
      orgEdit->setText(QString::fromUtf8(s));
    s=AO_Bank_GetFid(b);
    if (s)
      fidEdit->setText(QString::fromUtf8(s));

    qs="https://";
    s=AO_Bank_GetServerAddr(b);
    if (s)
      qs+=QString::fromUtf8(s);
    urlEdit->setText(qs);
  }

  if (!country || !*country)
    country="us";
  countriesToCombo(countryCombo, country);

  s=AO_User_GetUserId(u);
  if (s)
    userIdEdit->setText(QString::fromUtf8(s));
  s=AO_User_GetUserName(u);
  if (s)
    userNameEdit->setText(QString::fromUtf8(s));
}



void EditUser::guiToUser(AO_USER *u) {
  std::string s;
  AB_BANKING *ab;
  AO_BANK *b;
  AB_PROVIDER *pro;
  std::string country;
  std::string bankId;
  std::string userId;

  pro=AB_Banking_GetProvider(_app->getCInterface(), "aqofxconnect");
  assert(pro);
  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  if (countryCombo->currentItem()!=0) {
    const AB_COUNTRY *cnt;

    country=QBanking::QStringToUtf8String(countryCombo->currentText());
    cnt=AB_Banking_FindCountryByLocalName(ab, country.c_str());
    assert(cnt);
    country=AB_Country_GetCode(cnt);
  }
  bankId=QBanking::QStringToUtf8String(bankCodeEdit->text());
  userId=QBanking::QStringToUtf8String(userIdEdit->text());

  b=AO_User_GetBank(u);
  if (b)
    // unlink from bank
    AO_User_List_Del(u);

  b=AO_Provider_FindMyBank(pro, country.c_str(), bankId.c_str());
  if (!b) {
    // create bank
    b=AO_Bank_new(pro, country.c_str(), bankId.c_str());
    assert(b);
    AO_Provider_AddBank(pro, b);
  }

  s=QBanking::QStringToUtf8String(bankNameEdit->text());
  if (s.empty()) AO_Bank_SetBankName(b, 0);
  else AO_Bank_SetBankName(b, s.c_str());

  s=QBanking::QStringToUtf8String(fidEdit->text());
  if (s.empty()) AO_Bank_SetFid(b, 0);
  else AO_Bank_SetFid(b, s.c_str());

  s=QBanking::QStringToUtf8String(orgEdit->text());
  if (s.empty()) AO_Bank_SetOrg(b, 0);
  else AO_Bank_SetOrg(b, s.c_str());

  s=QBanking::QStringToUtf8String(urlEdit->text());
  if (s.empty()) AO_Bank_SetServerAddr(b, 0);
  else {
    QString entered = urlEdit->text();
    QString qs;

    if (entered.startsWith("http://")) {
      qs=entered.mid(7);
      AO_Bank_SetServerType(b, AO_Bank_ServerTypeHTTP);

    }
    else if (entered.startsWith("https://")) {
      qs=entered.mid(8);
      AO_Bank_SetServerType(b, AO_Bank_ServerTypeHTTPS);
    }
    else {
      qs=entered;
      AO_Bank_SetServerType(b, AO_Bank_ServerTypeHTTPS);
    }
    s=QBanking::QStringToUtf8String(qs);
    AO_Bank_SetServerAddr(b, s.c_str());
  }

  s=QBanking::QStringToUtf8String(userIdEdit->text());
  AO_User_SetUserId(u, s.c_str());

  s=QBanking::QStringToUtf8String(userNameEdit->text());
  if (s.empty()) AO_User_SetUserName(u, 0);
  else AO_User_SetUserName(u, s.c_str());

  // (re-)add user to his bank
  AO_Bank_AddUser(b, u);
  AO_User_SetBank(u, b);

}


void EditUser::accept(){
  if (bankCodeEdit->text().isEmpty() ||
      userIdEdit->text().isEmpty() ||
      countryCombo->currentItem()==0 ||
      urlEdit->text().isEmpty()) {
    QMessageBox::critical(this,
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
  if (fidEdit->text().isEmpty() ||
      orgEdit->text().isEmpty()) {
    DBG_WARN(0, "Either FID or ORG empty, might not work");
  }

  guiToUser(_user);

  QDialog::accept();
}



void EditUser::slotBankCodeLostFocus() {
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



void EditUser::slotWhatsThis(){
  QWhatsThis::enterWhatsThisMode();

}



void EditUser::slotBankCodeClicked() {
  AB_BANKINFO *bi;
  std::string country;

  if (countryCombo->currentItem()!=0) {
    const AB_COUNTRY *cnt;
    std::string s;

    country=QBanking::QStringToUtf8String(countryCombo->currentText());
    cnt=AB_Banking_FindCountryByLocalName(_app->getCInterface(),
					  country.c_str());
    assert(cnt);
    country=AB_Country_GetCode(cnt);
  }
  else
    return;

  bi=QBSelectBank::selectBank(_app, this,
                              tr("Select a Bank"),
                              QString::fromUtf8(country.c_str()),
                              bankCodeEdit->text(),
                              "", // no BIC
                              bankNameEdit->text(),
                              ""); // no location
  if (bi) {
    const char *s;
    AB_BANKINFO_SERVICE *sv;

    s=AB_BankInfo_GetBankId(bi);
    if (s)
      bankCodeEdit->setText(QString::fromUtf8(s));
    s=AB_BankInfo_GetBankName(bi);
    if (s)
      bankNameEdit->setText(QString::fromUtf8(s));

    sv=AB_BankInfoService_List_First(AB_BankInfo_GetServices(bi));
    while(sv) {
      const char *s;
      QString qs;

      s=AB_BankInfoService_GetType(sv);
      if (s && strcasecmp(s, "OFX")==0) {
	s=AB_BankInfoService_GetAux1(sv);
	if (s)
	  fidEdit->setText(QString::fromUtf8(s));
	s=AB_BankInfoService_GetAux2(sv);
	if (s)
	  orgEdit->setText(QString::fromUtf8(s));
	s=AB_BankInfoService_GetAddress(sv);
	if (s) {
	  qs=QString::fromUtf8(s);
	  if (!qs.startsWith("http://") &&
	      !qs.startsWith("https://"))
	    qs="https://"+qs;
	}
	else
	  qs="https://";
	urlEdit->setText(qs);
        break;
      }
      sv=AB_BankInfoService_List_Next(sv);
    } // while sv
    AB_BankInfo_free(bi);
  } // if bi
}





