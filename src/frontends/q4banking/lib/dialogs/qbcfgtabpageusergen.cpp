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
#include "qbcfgtabpageusergen.h"

// Gwenhywfar includes
#include <gwenhywfar/debug.h>

// QT includes
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qlocale.h>



QBCfgTabPageUserGeneral::QBCfgTabPageUserGeneral(QBanking *qb,
                                                 AB_USER *u,
                                                 QWidget *parent,
                                                 const char *name,
                                                 Qt::WFlags f)
  :QBCfgTabPageUser(qb, tr("General"), u, parent, name, f){
  AB_COUNTRY_CONSTLIST2 *cl;
  _realPage.setupUi(this);

  connect(_realPage.bankIdButton, SIGNAL(clicked()),
          SLOT(slotBankIdButtonClicked()));

  setHelpSubject("QBCfgTabPageUserGeneral");
  setDescription(tr("<p>This page contains some general settings.</p>"));

  _realPage.countryCombo->clear();
  _realPage.countryCombo->insertItem(tr("- select country -"));
  cl=AB_Banking_ListCountriesByName(qb->getCInterface(), "*");
  if (cl) {
    AB_COUNTRY_CONSTLIST2_ITERATOR *it;

    it=AB_Country_ConstList2_First(cl);
    if (it) {
      const AB_COUNTRY *c;
      GWEN_STRINGLIST *sl;
      GWEN_STRINGLISTENTRY *se;
      const char *s;

      sl=GWEN_StringList_new();
      c=AB_Country_ConstList2Iterator_Data(it);
      while(c) {
        s=AB_Country_GetLocalName(c);
        assert(s);
        GWEN_StringList_AppendString(sl, s, 0, 1);
        c=AB_Country_ConstList2Iterator_Next(it);
      }
      AB_Country_ConstList2Iterator_free(it);
      GWEN_StringList_Sort(sl, 0, GWEN_StringList_SortModeNoCase);
      se=GWEN_StringList_FirstEntry(sl);
      while(se) {
	s=GWEN_StringListEntry_Data(se);
        assert(s);
        _realPage.countryCombo->insertItem(QString::fromUtf8(s));
        se=GWEN_StringListEntry_Next(se);
      }
      GWEN_StringList_free(sl);
    }

    AB_Country_ConstList2_free(cl);
  }

  if (parent)
    parent->adjustSize();
  else
    adjustSize();
}



QBCfgTabPageUserGeneral::~QBCfgTabPageUserGeneral() {
}



void QBCfgTabPageUserGeneral::updateView() {
  toGui();
}



bool QBCfgTabPageUserGeneral::toGui() {
  const char *s;
  const AB_COUNTRY *ci;
  QString qs;
  std::string stds;

  qs=getUserIdLabel();
  if (!qs.isEmpty())
    _realPage.userIdLabel->setText(qs);

  qs=getCustomerIdLabel();
  if (!qs.isEmpty())
    _realPage.customerIdLabel->setText(qs);

  s=AB_User_GetUserId(getUser());
  if (s)
    _realPage.userIdEdit->setText(QString::fromUtf8(s));
  s=AB_User_GetCustomerId(getUser());
  if (s)
    _realPage.customerIdEdit->setText(QString::fromUtf8(s));
  s=AB_User_GetUserName(getUser());
  if (s)
    _realPage.userNameEdit->setText(QString::fromUtf8(s));


  s=AB_User_GetBankCode(getUser());
  if (s)
    _realPage.bankIdEdit->setText(QString::fromUtf8(s));

  s=AB_User_GetCountry(getUser());
  if (!s){
    /* derive default country name from the current locale */
    QLocale locale=QLocale::system();
    QString lname=locale.name();
    int pos;

    pos=lname.find('_');
    if (pos>=0) {
      QString l=lname.mid(uint(pos+1));
      stds=QBanking::QStringToUtf8String(l);
      if (!stds.empty())
	s=stds.c_str();
    }
  }
  if (!s)
    s="de";

  ci=AB_Banking_FindCountryByCode(getBanking()->getCInterface(), s);
  if (ci) {
    s=AB_Country_GetLocalName(ci);
    assert(s);
    _realPage.countryCombo->setCurrentText(QString::fromUtf8(s));
  }

  return true;
}



bool QBCfgTabPageUserGeneral::fromGui() {
  std::string s;
  const char *cs;
  const AB_COUNTRY *ci;

  s=QBanking::QStringToUtf8String(_realPage.userIdEdit->text());
  if (s.empty())
    AB_User_SetUserId(getUser(), 0);
  else
    AB_User_SetUserId(getUser(), s.c_str());

  s=QBanking::QStringToUtf8String(_realPage.customerIdEdit->text());
  if (s.empty())
    AB_User_SetCustomerId(getUser(), 0);
  else
    AB_User_SetCustomerId(getUser(), s.c_str());

  s=QBanking::QStringToUtf8String(_realPage.userNameEdit->text());
  if (s.empty())
    AB_User_SetUserName(getUser(), 0);
  else
    AB_User_SetUserName(getUser(), s.c_str());

  s=QBanking::QStringToUtf8String(_realPage.bankIdEdit->text());
  if (s.empty())
    AB_User_SetBankCode(getUser(), 0);
  else
    AB_User_SetBankCode(getUser(), s.c_str());

  s=QBanking::QStringToUtf8String(_realPage.countryCombo->currentText());
  assert(!s.empty());

  ci=AB_Banking_FindCountryByLocalName(getBanking()->getCInterface(),
                                       s.c_str());
  assert(ci);
  cs=AB_Country_GetCode(ci);
  assert(cs);
  AB_User_SetCountry(getUser(), cs);

  return true;
}



bool QBCfgTabPageUserGeneral::checkGui() {
  if (_realPage.userIdEdit->text().isEmpty() &&
      _realPage.customerIdEdit->text().isEmpty() &&
      _realPage.userNameEdit->text().isEmpty()) {
    QMessageBox::critical(this,
                          tr("Input Error"),
                          tr("<qt>"
                             "You must at least provide some kind of user "
                             "identification (user id, customer id or "
                             "user name)"
                             "</qt>"),
                          tr("Dismiss"));
    return false;
  }
  if (_realPage.bankIdEdit->text().isEmpty()) {
    QMessageBox::critical(this,
                          tr("Input Error"),
                          tr("<qt>"
                             "Bank id is missing."
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

  return true;
}



void QBCfgTabPageUserGeneral::slotBankIdButtonClicked() {
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
                                tr("Select the Bank for this User"),
                                QString::fromUtf8(cs),
                                _realPage.bankIdEdit->text());
    if (bi) {
      const char *t;

      t=AB_BankInfo_GetBankId(bi);
      if (t)
	_realPage.bankIdEdit->setText(QString::fromUtf8(t));
      AB_BankInfo_free(bi);
    }
  }
}



#include "qbcfgtabpageusergen.moc"



