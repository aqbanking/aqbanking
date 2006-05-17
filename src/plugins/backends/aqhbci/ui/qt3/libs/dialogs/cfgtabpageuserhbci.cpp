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


#include "cfgtabpageuserhbci.h"
#include "userwizard.h"

#include <qbanking/qbanking.h>

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qlistview.h>
#include <qtimer.h>
#include <qgroupbox.h>
#include <qcheckbox.h>

#include <aqhbci/user.h>
#include <aqhbci/provider.h>

#include <aqbanking/banking.h>

#include <gwenhywfar/debug.h>

#ifdef WIN32
# define strcasecmp stricmp
#endif



CfgTabPageUserHbci::CfgTabPageUserHbci(QBanking *qb,
                                       AB_USER *u,
                                       QWidget *parent,
                                       const char *name, WFlags f)
:QBCfgTabPageUser(qb, "HBCI", u, parent, name, f)
,_provider(0)
,_withHttp(true){
  AB_PROVIDER *pro;

  setHelpSubject("CfgTabPageUserHbci");
  setDescription(tr("<p>This page contains HBCI specific "
                    "user settings.</p>"));

  pro=AB_User_GetProvider(u);
  _provider=pro;

  _realPage=new CfgTabPageUserHbciUi(this);

  addWidget(_realPage);
  _realPage->show();

  connect(_realPage->getServerKeysButton,
          SIGNAL(clicked()),
          this,
          SLOT(slotGetServerKeys()));
  connect(_realPage->getSysIdButton,
          SIGNAL(clicked()),
          this,
          SLOT(slotGetSysId()));
  connect(_realPage->getAccountsButton,
          SIGNAL(clicked()),
          this,
          SLOT(slotGetAccounts()));

  connect(_realPage->finishUserButton,
          SIGNAL(clicked()),
          this,
          SLOT(slotFinishUser()));

  connect(_realPage->userStatusCombo,
          SIGNAL(activated(int)),
          this,
          SLOT(slotStatusChanged(int)));

  QTimer::singleShot(0, this, SLOT(adjustSize()));
}



CfgTabPageUserHbci::~CfgTabPageUserHbci() {
}



bool CfgTabPageUserHbci::toGui() {
  const char *s;
  const GWEN_URL *url;
  AH_MEDIUM *m;
  AH_USER_STATUS ust;
  AB_USER *u;
  QString qs;
  int i;

  u=getUser();
  assert(u);
  _realPage->userStatusCombo->insertItem(tr("New"));
  _realPage->userStatusCombo->insertItem(tr("Enabled"));
  _realPage->userStatusCombo->insertItem(tr("Pending"));
  _realPage->userStatusCombo->insertItem(tr("Disabled"));
  _realPage->userStatusCombo->insertItem(tr("Unknown"));
  ust=AH_User_GetStatus(getUser());
  switch(ust) {
  case AH_UserStatusNew:      i=0; break;
  case AH_UserStatusEnabled:  i=1; break;
  case AH_UserStatusPending:  i=2; break;
  case AH_UserStatusDisabled: i=3; break;
  case AH_UserStatusUnknown:
  default:                    i=4; break;
  }
  _realPage->userStatusCombo->setCurrentItem(i);
  slotStatusChanged(i);

  url=AH_User_GetServerUrl(u);
  if (url) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 256, 0, 1);
    if (GWEN_Url_toString(url, ubuf)==0) {
      _realPage->serverEdit
	->setText(QString::fromUtf8(GWEN_Buffer_GetStart(ubuf)));
    }
    GWEN_Buffer_free(ubuf);
  }

  m=AH_User_GetMedium(u);
  if (m) {
    s=AH_Medium_GetDescriptiveName(m);
    if (s)
      _realPage->descriptiveEdit->setText(QString::fromUtf8(s));
  }

  _realPage->getServerKeysButton->setEnabled(false);
  _realPage->getSysIdButton->setEnabled(false);

  if (AH_User_GetCryptMode(u)==AH_CryptMode_Pintan) {
    _withHttp=true;
    _realPage->httpVersionCombo->insertItem(tr("1.0"));
    _realPage->httpVersionCombo->insertItem(tr("1.1"));
    qs = QString::number(AH_User_GetHttpVMajor(u))
      + "." + QString::number(AH_User_GetHttpVMinor(u));
    _setComboTextIfPossible(_realPage->httpVersionCombo, qs);

    s=AH_User_GetHttpUserAgent(u);
    if (s)
      _realPage->userAgentEdit->setText(QString::fromUtf8(s));

    _realPage->getSysIdButton->setEnabled(true);
  }
  else {
    _withHttp=false;
    _realPage->httpBox->hide();

    if (AH_User_GetCryptMode(u)==AH_CryptMode_Rdh) {
      _realPage->getServerKeysButton->setEnabled(true);
      _realPage->getSysIdButton->setEnabled(true);
    }
  }

  _realPage->bankSignCheck->setChecked(!(AH_User_GetFlags(u) &
                                         AH_USER_FLAGS_BANK_DOESNT_SIGN));
  _realPage->bankCounterCheck->setChecked(AH_User_GetFlags(u) &
                                          AH_USER_FLAGS_BANK_USES_SIGNSEQ);

  return true;
}



bool CfgTabPageUserHbci::fromGui() {
  AH_USER_STATUS ust;
  GWEN_URL *url;
  AH_MEDIUM *m;
  AB_USER *u;

  u=getUser();
  assert(u);
  switch(_realPage->userStatusCombo->currentItem()) {
  case 0:  ust=AH_UserStatusNew; break;
  case 1:  ust=AH_UserStatusEnabled; break;
  case 2:  ust=AH_UserStatusPending; break;
  case 3:  ust=AH_UserStatusDisabled; break;
  default: ust=AH_UserStatusUnknown; break;
  }
  AH_User_SetStatus(u, ust);

  /* url */
  QString qs=_realPage->serverEdit->text();
  url=GWEN_Url_fromString(qs.utf8());
  assert(url);

  if (AH_User_GetCryptMode(u)==AH_CryptMode_Pintan) {
    GWEN_Url_SetProtocol(url, "https");
    GWEN_Url_SetPort(url, 443);
  }
  else {
    GWEN_Url_SetProtocol(url, "hbci");
    GWEN_Url_SetPort(url, 3000);
  }
  AH_User_SetServerUrl(u, url);
  GWEN_Url_free(url);

  m=AH_User_GetMedium(u);
  assert(m);

  AH_Medium_SetDescriptiveName(m, _realPage->descriptiveEdit->text().utf8());

  if (_withHttp) {
    std::string s;

    s=QBanking::QStringToUtf8String(_realPage->httpVersionCombo->currentText());
    if (strcasecmp(s.c_str(), "1.0")==0) {
      AH_User_SetHttpVMajor(u, 1);
      AH_User_SetHttpVMinor(u, 0);
    }
    else if (strcasecmp(s.c_str(), "1.1")==0) {
      AH_User_SetHttpVMajor(u, 1);
      AH_User_SetHttpVMinor(u, 1);
    }

    s=QBanking::QStringToUtf8String(_realPage->userAgentEdit->text());
    if (s.empty())
      AH_User_SetHttpUserAgent(u, 0);
    else
      AH_User_SetHttpUserAgent(u, s.c_str());

  }

  if (_realPage->bankSignCheck->isChecked())
    AH_User_SubFlags(u, AH_USER_FLAGS_BANK_DOESNT_SIGN);
  else
    AH_User_AddFlags(u, AH_USER_FLAGS_BANK_DOESNT_SIGN);

  if (_realPage->bankCounterCheck->isChecked())
    AH_User_AddFlags(u, AH_USER_FLAGS_BANK_USES_SIGNSEQ);
  else
    AH_User_SubFlags(u, AH_USER_FLAGS_BANK_USES_SIGNSEQ);

  return true;
}



bool CfgTabPageUserHbci::checkGui() {
  return true;
}




void CfgTabPageUserHbci::slotStatusChanged(int i) {
  _realPage->finishUserButton
    ->setEnabled((i==2) &&
                 (AH_User_GetCryptMode(getUser())==AH_CryptMode_Rdh));
}



void CfgTabPageUserHbci::_setComboTextIfPossible(QComboBox *qb,
                                                 const QString &s){
  int i;

  for (i=0; i<qb->count(); i++) {
    if (qb->text(i)==s) {
      qb->setCurrentItem(i);
      break;
    }
  }
}



void CfgTabPageUserHbci::slotGetServerKeys() {
  AB_USER *u;
  QBanking *qb;
  AB_PROVIDER *pro;
  int rv;
  GWEN_TYPE_UINT32 pid;
  AB_IMEXPORTER_CONTEXT *ctx;

  qb=getBanking();
  assert(qb);
  pro=_provider;
  assert(pro);
  u=getUser();
  assert(u);

  DBG_ERROR(0, "Retrieving server keys");
  pid=qb->progressStart(tr("Getting Server Keys"),
                        tr("<qt>"
                           "Retrieving the public keys of the server."
                           "</qt>"),
                        1);
  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetServerKeys(pro, u, ctx, 1);
  AB_ImExporterContext_free(ctx);
  if (rv) {
    DBG_ERROR(0, "Error getting server keys");
    qb->progressEnd(pid);
    return;
  }

  pid=qb->progressStart(tr("Retrieving System Id"),
                        tr("<qt>"
                           "Retrieving a system id from the "
                           "bank server."
                           "</qt>"),
                        1);
  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetSysId(pro, u, ctx, 1);
  AB_ImExporterContext_free(ctx);
  if (rv) {
    DBG_ERROR(0, "Error getting sysid (%d)", rv);
    qb->progressEnd(pid);
    return;
  }
  getBanking()->progressLog(0,
                            AB_Banking_LogLevelNotice,
                            tr("Keys saved."));
  qb->progressEnd(pid);
}



void CfgTabPageUserHbci::slotGetSysId() {
  AB_USER *u;
  QBanking *qb;
  AB_PROVIDER *pro;
  int rv;
  GWEN_TYPE_UINT32 pid;
  AB_IMEXPORTER_CONTEXT *ctx;

  qb=getBanking();
  assert(qb);
  pro=_provider;
  assert(pro);
  u=getUser();
  assert(u);

  DBG_ERROR(0, "Retrieving system id");
  pid=qb->progressStart(tr("Retrieving System Id"),
                        tr("<qt>"
                           "Retrieving a system id from the "
                           "bank server."
                           "</qt>"),
                        1);
  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetSysId(pro, u, ctx, 1);
  AB_ImExporterContext_free(ctx);
  if (rv) {
    DBG_ERROR(0, "Error getting sysid (%d)", rv);
    qb->progressEnd(pid);
    return;
  }
  qb->progressEnd(pid);
}



void CfgTabPageUserHbci::slotGetAccounts() {
  AB_USER *u;
  QBanking *qb;
  AB_PROVIDER *pro;
  int rv;
  GWEN_TYPE_UINT32 pid;
  AB_IMEXPORTER_CONTEXT *ctx;

  qb=getBanking();
  assert(qb);
  pro=_provider;
  assert(pro);
  u=getUser();
  assert(u);

  DBG_INFO(0, "Retrieving accounts");
  pid=qb->progressStart(tr("Getting List of Accounts"),
                        tr("<qt>"
                           "Retrieving the list of our accounts from the "
                           "bank server."
                           "</qt>"),
                        1);
  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetAccounts(pro, u, ctx, 1);
  AB_ImExporterContext_free(ctx);
  if (rv) {
    if (rv==AB_ERROR_NO_DATA) {
      QMessageBox::information(this,
                               tr("No Account List"),
                               tr("<qt>"
                                  "<p>"
                                  "Your bank does not send a list of "
                                  "accounts."
                                  "</p>"
                                  "<p>"
                                  "You will have to setup the accounts for "
                                  "this user manually."
                                  "</p>"
                                  "</qt>"),
                               QMessageBox::Ok,QMessageBox::NoButton);
    }
    else {
      DBG_ERROR(0, "Error getting accounts");
      qb->progressEnd(pid);
      return;
    }
  }
  qb->progressEnd(pid);
}



void CfgTabPageUserHbci::slotFinishUser() {
  UserWizard::finishUser(getBanking(),
                         _provider,
                         getUser(),
                         this);
  toGui();
}







