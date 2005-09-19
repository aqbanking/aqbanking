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

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qfiledialog.h>

#include <chipcard2-client/cards/geldkarte.h>

#include <gwenhywfar/debug.h>

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
  fromCardButton->setEnabled(isNew);

  QObject::connect((QObject*)bankCodeEdit, SIGNAL(lostFocus()),
                   this, SLOT(slotBankCodeLostFocus()));
  QObject::connect((QObject*)whatsThisButton, SIGNAL(clicked()),
                   this, SLOT(slotWhatsThis()));
  QObject::connect((QObject*)fromCardButton, SIGNAL(clicked()),
                   this, SLOT(slotFromCard()));

}



EditAccount::~EditAccount(){
}



bool EditAccount::init() {
  accountToGui(_account);
  return true;
}



void EditAccount::accountToGui(AB_ACCOUNT *a) {
  const char *s;

  s=AB_Account_GetBankCode(a);
  if (s)
    bankCodeEdit->setText(QString::fromUtf8(s));
  s=AB_Account_GetBankName(a);
  if (s)
    bankNameEdit->setText(QString::fromUtf8(s));
  s=AB_Account_GetAccountNumber(a);
  if (s)
    accountIdEdit->setText(QString::fromUtf8(s));
  s=AB_Account_GetOwnerName(a);
  if (s)
    ownerNameEdit->setText(QString::fromUtf8(s));
  s=AB_Account_GetAccountName(a);
  if (s)
    accountNameEdit->setText(QString::fromUtf8(s));

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

  s=QBanking::QStringToUtf8String(ownerNameEdit->text());
  AB_Account_SetOwnerName(a, s.c_str());

}


void EditAccount::accept(){
  if (bankCodeEdit->text().isEmpty() ||
      accountIdEdit->text().isEmpty() ||
      ownerNameEdit->text().isEmpty()) {
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
                          QMessageBox::Ok,QMessageBox::NoButton);
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



void EditAccount::slotFromCard(){
  LC_CARD *card;
  AB_PROVIDER *pro;
  std::string cardId;
  const char *s;
  GWEN_TYPE_UINT32 bid;

  pro=_app->getProvider("aqgeldkarte");
  assert(pro);

  s=AG_Account_GetCardId(_account);
  if (s)
    cardId=std::string(s);
  // this is needed to make the module read *any* card
  AG_Account_SetCardId(_account, 0);
  bid=AB_Banking_ShowBox
    (AB_Provider_GetBanking(pro),
     0,
     QBanking::QStringToUtf8String(tr("Accessing Card")).c_str(),
     QBanking::QStringToUtf8String(tr("Reading card, "
                                      "please wait...")).c_str()
    );
  card=AG_Provider_MountCard(pro, _account);
  AB_Banking_HideBox(AB_Provider_GetBanking(pro), bid);
  if (card) {
    GWEN_DB_NODE *dbAccount;

    dbAccount=LC_GeldKarte_GetAccountDataAsDb(card);
    assert(dbAccount);
    s=GWEN_DB_GetCharValue(dbAccount, "bankCode", 0, 0);
    if (s) {
      bankCodeEdit->setText(QString::fromUtf8(s));
      // update bank name
      slotBankCodeLostFocus();
    }
    s=GWEN_DB_GetCharValue(dbAccount, "accountId", 0, 0);
    if (s)
      accountIdEdit->setText(QString::fromUtf8(s));

    LC_Card_Close(card);
    LC_Card_free(card);
  }
  else {
    if (!cardId.empty())
      AG_Account_SetCardId(_account, cardId.c_str());
  }
}








