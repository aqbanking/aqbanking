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


#include "cfgtabpageaccountdtaus.h"
#include "cfgtabpageaccountdtaus.ui.h"

#include <aqdtaus/account.h>


#include <qbanking/qbanking.h>

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qlistview.h>
#include <qtimer.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qfiledialog.h>




CfgTabPageAccountDtaus::CfgTabPageAccountDtaus(QBanking *qb,
                                               AB_ACCOUNT *a,
                                               QWidget *parent,
                                               const char *name,
                                               WFlags f)
:QBCfgTabPageAccount(qb, "DTAUS", a, parent, name, f) {

  _realPage=new CfgTabPageAccountDtausUi(this);

  setHelpSubject("CfgTabPageAccountDtaus");
  setDescription(tr("<p>This page contains DTAUS-specific settings.</p>"));

  addWidget(_realPage);
  _realPage->show();

  QObject::connect(_realPage->useFloppyCheck, SIGNAL(toggled(bool)),
                   this, SLOT(slotFloppyToggled(bool)));
  QObject::connect(_realPage->mountCheck, SIGNAL(toggled(bool)),
                   this, SLOT(slotMountToggled(bool)));
  QObject::connect(_realPage->folderEdit, SIGNAL(lostFocus()),
                   this, SLOT(slotFolderLostFocus()));
  QObject::connect(_realPage->folderButton, SIGNAL(clicked()),
                   this, SLOT(slotFolder()));

  QTimer::singleShot(0, this, SLOT(adjustSize()));
}



CfgTabPageAccountDtaus::~CfgTabPageAccountDtaus() {
}



bool CfgTabPageAccountDtaus::fromGui() {
  AB_ACCOUNT *a;
  std::string s;
  int i;

  a=getAccount();
  assert(a);

  i=_realPage->maxPurposeSpin->value();
  AD_Account_SetMaxPurposeLines(a, i);

  AD_Account_SetDebitAllowed(a,
                             _realPage->debitNoteCheck->isChecked());
  AD_Account_SetPrintAllTransactions(a,
                                     _realPage->printAllCheck->isChecked());
  AD_Account_SetUseDisc(a, _realPage->useFloppyCheck->isChecked());
  AD_Account_SetMountAllowed(a, _realPage->mountCheck->isChecked());

  s=QBanking::QStringToUtf8String(_realPage->folderEdit->text());
  if (s.empty()) AD_Account_SetFolder(a, 0);
  else AD_Account_SetFolder(a, s.c_str());

  s=QBanking::QStringToUtf8String(_realPage->mountCmdEdit->text());
  if (s.empty()) AD_Account_SetMountCommand(a, 0);
  else AD_Account_SetMountCommand(a, s.c_str());

  s=QBanking::QStringToUtf8String(_realPage->unmountCmdEdit->text());
  if (s.empty()) AD_Account_SetUnmountCommand(a, 0);
  else AD_Account_SetUnmountCommand(a, s.c_str());

  return true;
}



bool CfgTabPageAccountDtaus::toGui() {
  AB_ACCOUNT *a;
  const char *s;
  int i;

  a=getAccount();
  assert(a);

  i=AD_Account_GetMaxPurposeLines(a);
  if (i==0)
    i=4;
  _realPage->maxPurposeSpin->setValue(i);
  _realPage->debitNoteCheck->setChecked(AD_Account_GetDebitAllowed(a));

  _realPage->useFloppyCheck->setChecked(AD_Account_GetUseDisc(a));
  _realPage->mountCheck->setChecked(AD_Account_GetMountAllowed(a));
  _realPage->printAllCheck->setChecked(AD_Account_GetPrintAllTransactions(a));
  s=AD_Account_GetFolder(a);
  if (s)
    _realPage->folderEdit->setText(QString::fromUtf8(s));
  s=AD_Account_GetMountCommand(a);
  if (s)
    _realPage->mountCmdEdit->setText(QString::fromUtf8(s));
  s=AD_Account_GetUnmountCommand(a);
  if (s)
    _realPage->unmountCmdEdit->setText(QString::fromUtf8(s));

  if (AD_Account_GetUseDisc(a)==0 ||
      AD_Account_GetMountAllowed(a)==0) {
    _realPage->mountCmdEdit->setEnabled(false);
    _realPage->unmountCmdEdit->setEnabled(false);
  }
  else {
    _realPage->mountCmdEdit->setEnabled(false);
    _realPage->unmountCmdEdit->setEnabled(false);
  }

  return true;
}



bool CfgTabPageAccountDtaus::checkGui() {
  if (_realPage->mountCheck->isChecked() &&
      (_realPage->mountCmdEdit->text().isEmpty() ||
       _realPage->unmountCmdEdit->text().isEmpty())) {
    QMessageBox::critical(this,
                          tr("Input Error"),
                          tr("<qt>"
                             "Please fill in the <i>mount</i> and "
                             "<i>unmount</i> commands."
                             "</qt>"),
                          QMessageBox::Ok,QMessageBox::NoButton);
    return false;
  }

  return true;
}



void CfgTabPageAccountDtaus::slotFloppyToggled(bool on){
  bool mo;

  mo=_realPage->mountCheck->isChecked();
  _realPage->mountCheck->setEnabled(on);
  _realPage->mountCmdEdit->setEnabled(on && mo);
  _realPage->unmountCmdEdit->setEnabled(on && mo);
}



void CfgTabPageAccountDtaus::slotMountToggled(bool on){
  bool mo;

  mo=_realPage->useFloppyCheck->isChecked();
  _realPage->mountCmdEdit->setEnabled(on && mo);
  _realPage->unmountCmdEdit->setEnabled(on && mo);
}



void CfgTabPageAccountDtaus::slotFolderLostFocus() {
  QString s;

  s=_realPage->folderEdit->text();
  if (!s.isEmpty()) {
    if (_realPage->mountCmdEdit->text().isEmpty()) {
      QString qs;
      qs="mount ";
      qs+=s;
      _realPage->mountCmdEdit->setText(qs);
    }
    if (_realPage->unmountCmdEdit->text().isEmpty()) {
      QString qs;
      qs="umount ";
      qs+=s;
      _realPage->unmountCmdEdit->setText(qs);
    }
  }
}



void CfgTabPageAccountDtaus::slotFolder(){
  QString filename=
    QFileDialog::getExistingDirectory(_realPage->folderEdit->text(),
                                      this,
                                      "slotFolder file dialog");
  if (!filename.isEmpty()) {
    _realPage->folderEdit->setText(filename);
    slotFolderLostFocus();
  }
}


void CfgTabPageAccountDtaus::accept()
{ assert(0); }
