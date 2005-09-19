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

  QObject::connect((QObject*)useFloppyCheck, SIGNAL(toggled(bool)),
                   this, SLOT(slotFloppyToggled(bool)));
  QObject::connect((QObject*)mountCheck, SIGNAL(toggled(bool)),
                   this, SLOT(slotMountToggled(bool)));
  QObject::connect((QObject*)bankCodeEdit, SIGNAL(lostFocus()),
                   this, SLOT(slotBankCodeLostFocus()));
  QObject::connect((QObject*)folderEdit, SIGNAL(lostFocus()),
                   this, SLOT(slotFolderLostFocus()));
  QObject::connect((QObject*)whatsThisButton, SIGNAL(clicked()),
                   this, SLOT(slotWhatsThis()));
  QObject::connect((QObject*)folderButton, SIGNAL(clicked()),
                   this, SLOT(slotFolder()));

}



EditAccount::~EditAccount(){
}



bool EditAccount::init() {
  accountToGui(_account);
  return true;
}



void EditAccount::accountToGui(AB_ACCOUNT *a) {
  const char *s;
  int i;

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

  i=AD_Account_GetMaxPurposeLines(a);
  DBG_ERROR(0, "Purpose lines: %d", i);
  if (i==0)
    i=4;
  maxPurposeSpin->setValue(i);
  debitNoteCheck->setChecked(AD_Account_GetDebitAllowed(a));

  useFloppyCheck->setChecked(AD_Account_GetUseDisc(a));
  mountCheck->setChecked(AD_Account_GetMountAllowed(a));
  printAllCheck->setChecked(AD_Account_GetPrintAllTransactions(a));
  s=AD_Account_GetFolder(a);
  if (s)
    folderEdit->setText(QString::fromUtf8(s));
  s=AD_Account_GetMountCommand(a);
  if (s)
    mountCmdEdit->setText(QString::fromUtf8(s));
  s=AD_Account_GetUnmountCommand(a);
  if (s)
    unmountCmdEdit->setText(QString::fromUtf8(s));

  if (AD_Account_GetUseDisc(a)==0 ||
      AD_Account_GetMountAllowed(a)==0) {
    mountCmdEdit->setEnabled(false);
    unmountCmdEdit->setEnabled(false);
  }
  else {
    mountCmdEdit->setEnabled(false);
    unmountCmdEdit->setEnabled(false);
  }

}



void EditAccount::guiToAccount(AB_ACCOUNT *a) {
  std::string s;
  int i;

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

  i=maxPurposeSpin->value();
  AD_Account_SetMaxPurposeLines(a, i);

  AD_Account_SetDebitAllowed(a, debitNoteCheck->isChecked());
  AD_Account_SetPrintAllTransactions(a, printAllCheck->isChecked());
  AD_Account_SetUseDisc(a, useFloppyCheck->isChecked());
  AD_Account_SetMountAllowed(a, mountCheck->isChecked());

  s=QBanking::QStringToUtf8String(folderEdit->text());
  if (s.empty()) AD_Account_SetFolder(a, 0);
  else AD_Account_SetFolder(a, s.c_str());

  s=QBanking::QStringToUtf8String(mountCmdEdit->text());
  if (s.empty()) AD_Account_SetMountCommand(a, 0);
  else AD_Account_SetMountCommand(a, s.c_str());

  s=QBanking::QStringToUtf8String(unmountCmdEdit->text());
  if (s.empty()) AD_Account_SetUnmountCommand(a, 0);
  else AD_Account_SetUnmountCommand(a, s.c_str());
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



void EditAccount::slotFloppyToggled(bool on){
  bool mo;

  mo=mountCheck->isChecked();
  mountCheck->setEnabled(on);
  mountCmdEdit->setEnabled(on && mo);
  unmountCmdEdit->setEnabled(on && mo);
}



void EditAccount::slotMountToggled(bool on){
  bool mo;

  mo=useFloppyCheck->isChecked();
  mountCmdEdit->setEnabled(on && mo);
  unmountCmdEdit->setEnabled(on && mo);
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



void EditAccount::slotFolderLostFocus() {
  QString s;

  s=folderEdit->text();
  if (!s.isEmpty()) {
    if (mountCmdEdit->text().isEmpty()) {
      QString qs;
      qs="mount ";
      qs+=s;
      mountCmdEdit->setText(qs);
    }
    if (unmountCmdEdit->text().isEmpty()) {
      QString qs;
      qs="umount ";
      qs+=s;
      unmountCmdEdit->setText(qs);
    }
  }
}



void EditAccount::slotWhatsThis(){
  QWhatsThis::enterWhatsThisMode();

}



void EditAccount::slotFolder(){
  QString filename=
    QFileDialog::getExistingDirectory(folderEdit->text(),
                                      this,
                                      "slotFolder file dialog");
  if (!filename.isEmpty()) {
    folderEdit->setText(filename);
    slotFolderLostFocus();
  }
}








