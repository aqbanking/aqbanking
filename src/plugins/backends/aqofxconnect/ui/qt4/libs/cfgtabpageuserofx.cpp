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


#include "cfgtabpageuserofx.h"

#include <aqofxconnect/user.h>
#include <aqofxconnect/provider.h>

#include <q4banking/qbanking.h>
#include <q4banking/qbcfgtab.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/url.h>

#include <qmessagebox.h>
#include <qtimer.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>




CfgTabPageUserOfx::CfgTabPageUserOfx(QBanking *qb,
                                     AB_USER *u,
                                     QWidget *parent,
                                     const char *name, Qt::WFlags f)
:QBCfgTabPageUser(qb, "OFX", u, parent, name, f) {
  _realPage.setupUi(this);

  setHelpSubject("CfgTabPageUserOfx");
  setDescription(tr("<p>This page contains "
                    "OFX DirectConnect-specific settings.</p>"));

  connect(_realPage.fidButton, SIGNAL(clicked()),
          this, SLOT(slotPickFid()));
  connect(_realPage.testUrlButton, SIGNAL(clicked()),
          this, SLOT(slotServerTest()));
  connect(_realPage.urlEdit, SIGNAL(textChanged(const QString&)),
          this, SLOT(slotServerChanged(const QString&)));
  connect(_realPage.accountListCheck, SIGNAL(toggled(bool)),
          this, SLOT(slotAccountCheckToggled(bool)));
  connect(_realPage.getAccountsButton, SIGNAL(clicked()),
          this, SLOT(slotGetAccounts()));

  QTimer::singleShot(0, this, SLOT(adjustSize()));
}



CfgTabPageUserOfx::~CfgTabPageUserOfx() {
}



bool CfgTabPageUserOfx::fromGui() {
  AB_USER *u;
  std::string s;
  const char *t;
  GWEN_URL *url;
  uint32_t f=0;

  u=getUser();
  assert(u);
  s=QBanking::QStringToUtf8String(_realPage.fidEdit->text());
  assert(!s.empty());
  AO_User_SetFid(u, s.c_str());

  s=QBanking::QStringToUtf8String(_realPage.orgEdit->text());
  assert(!s.empty());
  AO_User_SetOrg(u, s.c_str());

  s=QBanking::QStringToUtf8String(_realPage.brokerEdit->text());
  if (s.empty())
    AO_User_SetBrokerId(u, 0);
  else
    AO_User_SetBrokerId(u, s.c_str());

  s=QBanking::QStringToUtf8String(_realPage.appIdEdit->text());
  if (s.empty())
    AO_User_SetAppId(u, 0);
  else
    AO_User_SetAppId(u, s.c_str());

  s=QBanking::QStringToUtf8String(_realPage.appVerEdit->text());
  if (s.empty())
    AO_User_SetAppVer(u, 0);
  else
    AO_User_SetAppVer(u, s.c_str());

  s=QBanking::QStringToUtf8String(_realPage.headerVerEdit->text());
  if (s.empty())
    AO_User_SetHeaderVer(u, 0);
  else
    AO_User_SetHeaderVer(u, s.c_str());

  s=QBanking::QStringToUtf8String(_realPage.clientUidEdit->text());
  if (s.empty())
    AO_User_SetClientUid(u, 0);
  else
    AO_User_SetClientUid(u, s.c_str());

  s=QBanking::QStringToUtf8String(_realPage.urlEdit->text());
  url=GWEN_Url_fromString(s.c_str());
  t=GWEN_Url_GetProtocol(url);
  if (!t || !(*t))
    t="https";
  if (strcasecmp(t, "https")==0)
    AO_User_SetServerType(u, AO_User_ServerTypeHTTPS);
  else
    AO_User_SetServerType(u, AO_User_ServerTypeHTTP);
  AO_User_SetServerAddr(u, s.c_str());
  GWEN_Url_free(url);

  if (_realPage.accountListCheck->isChecked())
    f|=AO_USER_FLAGS_ACCOUNT_LIST;
  if (_realPage.statementCheck->isChecked())
    f|=AO_USER_FLAGS_STATEMENTS;
  if (_realPage.investmentCheck->isChecked())
    f|=AO_USER_FLAGS_INVESTMENT;
  if (_realPage.billPayCheck->isChecked())
    f|=AO_USER_FLAGS_BILLPAY;
  if (_realPage.emptyBankIdCheck->isChecked())
    f|=AO_USER_FLAGS_EMPTY_BANKID;
  if (_realPage.emptyFidCheck->isChecked())
    f|=AO_USER_FLAGS_EMPTY_FID;
  if (_realPage.forceSsl3Check->isChecked())
    f|=AO_USER_FLAGS_FORCE_SSL3;
  if (_realPage.sendShortDateCheck->isChecked())
    f|=AO_USER_FLAGS_SEND_SHORT_DATE;
  AO_User_SetFlags(u, f);

  return true;
}



bool CfgTabPageUserOfx::toGui() {
  AB_USER *u;
  const char *s;
  uint32_t f;

  u=getUser();
  assert(u);
  s=AO_User_GetFid(u);
  if (s)
    _realPage.fidEdit->setText(QString::fromUtf8(s));

  s=AO_User_GetOrg(u);
  if (s)
    _realPage.orgEdit->setText(QString::fromUtf8(s));

  s=AO_User_GetBrokerId(u);
  if (s)
    _realPage.brokerEdit->setText(QString::fromUtf8(s));
  s=AO_User_GetServerAddr(u);
  if (s)
    _realPage.urlEdit->setText(QString::fromUtf8(s));

  s=AO_User_GetAppId(u);
  if (s)
    _realPage.appIdEdit->setText(QString::fromUtf8(s));
  s=AO_User_GetAppVer(u);
  if (s)
    _realPage.appVerEdit->setText(QString::fromUtf8(s));
  s=AO_User_GetHeaderVer(u);
  if (s)
    _realPage.headerVerEdit->setText(QString::fromUtf8(s));
  s=AO_User_GetClientUid(u);
  if (s)
    _realPage.clientUidEdit->setText(QString::fromUtf8(s));

  f=AO_User_GetFlags(u);
  _realPage.accountListCheck->setChecked(f & AO_USER_FLAGS_ACCOUNT_LIST);
  slotAccountCheckToggled(f & AO_USER_FLAGS_ACCOUNT_LIST);
  _realPage.statementCheck->setChecked(f & AO_USER_FLAGS_STATEMENTS);
  _realPage.investmentCheck->setChecked(f & AO_USER_FLAGS_INVESTMENT);
  _realPage.billPayCheck->setChecked(f & AO_USER_FLAGS_BILLPAY);
  _realPage.emptyBankIdCheck->setChecked(f & AO_USER_FLAGS_EMPTY_BANKID);
  _realPage.emptyFidCheck->setChecked(f & AO_USER_FLAGS_EMPTY_FID);
  _realPage.forceSsl3Check->setChecked(f & AO_USER_FLAGS_FORCE_SSL3);
  _realPage.sendShortDateCheck->setChecked(f & AO_USER_FLAGS_SEND_SHORT_DATE);

  return true;
}



bool CfgTabPageUserOfx::checkGui() {
  std::string s;
  GWEN_URL *url;

  s=QBanking::QStringToUtf8String(_realPage.fidEdit->text());
  if (s.empty()) {
    _realPage.fidEdit->setFocus();
    return false;
  }

  s=QBanking::QStringToUtf8String(_realPage.orgEdit->text());
  if (s.empty()) {
    _realPage.orgEdit->setFocus();
    return false;
  }

  s=QBanking::QStringToUtf8String(_realPage.urlEdit->text());
  if (s.empty()) {
    QMessageBox::critical(this,
                          tr("Input Error"),
                          tr("You need to specify the server address."),
                          tr("Dismiss"));
    _realPage.urlEdit->setFocus();
    return false;
  }

  url=GWEN_Url_fromString(s.c_str());
  if (!url) {
    QMessageBox::critical(this,
                          tr("Input Error"),
                          tr("<qt>"
                             "<p>"
                             "Invalid server address."
                             "</p>"
                             "<p>"
                             "Please enter something along the line "
                             "<i>https:://www.server.com/here/there</i>"
                             "</p>"
                             "</qt>"),
                          tr("Dismiss"));
    _realPage.urlEdit->setFocus();
    return false;
  }
  GWEN_Url_free(url);

  return true;
}



void CfgTabPageUserOfx::slotPickFid() {
}



void CfgTabPageUserOfx::slotServerTest() {
}



void CfgTabPageUserOfx::slotServerChanged(const QString &qs) {
}



void CfgTabPageUserOfx::slotAccountCheckToggled(bool on) {
  _realPage.getAccountsButton->setEnabled(on);
}



void CfgTabPageUserOfx::slotGetAccounts() {
  AB_USER *u;
  AB_PROVIDER *pro;
  int rv;

  if (!getCfgTab()->checkGui())
    return;

  u=getUser();
  assert(u);
  pro=AB_User_GetProvider(u);
  assert(pro);
  rv=AO_Provider_RequestAccounts(pro, u, 0);
  if (rv) {
    DBG_ERROR(0, "Error requesting account list");
  }
  getCfgTab()->updateViews();

}


#include "cfgtabpageuserofx.moc"






