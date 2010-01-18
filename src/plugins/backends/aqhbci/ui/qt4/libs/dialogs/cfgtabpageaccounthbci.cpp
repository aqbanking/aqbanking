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


#include "cfgtabpageaccounthbci.h"

#include <aqhbci/account.h>


#include <q4banking/qbanking.h>

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <q3listview.h>
#include <qtimer.h>
#include <q3groupbox.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <q3filedialog.h>




CfgTabPageAccountHbci::CfgTabPageAccountHbci(QBanking *qb,
                                             AB_ACCOUNT *a,
                                             QWidget *parent,
                                             const char *name,
                                             Qt::WFlags f)
:QBCfgTabPageAccount(qb, "HBCI", a, parent, name, f) {
  _realPage.setupUi(this);
  setHelpSubject("CfgTabPageAccountHbci");
  setDescription(tr("<p>This page contains HBCI specific "
                    "account settings.</p>"));

  if (parent)
    parent->adjustSize();
  else
    adjustSize();
}



CfgTabPageAccountHbci::~CfgTabPageAccountHbci() {
}



bool CfgTabPageAccountHbci::fromGui() {
  AB_ACCOUNT *a;
  std::string s;

  a=getAccount();
  assert(a);

  if (_realPage.preferSingleTransferCheck->isChecked())
    AH_Account_AddFlags(a, AH_BANK_FLAGS_PREFER_SINGLE_TRANSFER);
  else
    AH_Account_SubFlags(a, AH_BANK_FLAGS_PREFER_SINGLE_TRANSFER);

  if (_realPage.preferSingleDebitNoteCheck->isChecked())
    AH_Account_AddFlags(a, AH_BANK_FLAGS_PREFER_SINGLE_DEBITNOTE);
  else
    AH_Account_SubFlags(a, AH_BANK_FLAGS_PREFER_SINGLE_DEBITNOTE);

  return true;
}



bool CfgTabPageAccountHbci::toGui() {
  AB_ACCOUNT *a;
  uint32_t aFlags;
  int i;

  a=getAccount();
  assert(a);

  aFlags=AH_Account_GetFlags(a);
  i=aFlags & AH_BANK_FLAGS_PREFER_SINGLE_TRANSFER;
  _realPage.preferSingleTransferCheck->setChecked(i);

  i=aFlags & AH_BANK_FLAGS_PREFER_SINGLE_DEBITNOTE;
  _realPage.preferSingleDebitNoteCheck->setChecked(i);

  return true;
}



bool CfgTabPageAccountHbci::checkGui() {
  return true;
}


// These are currently unimplemented!
void CfgTabPageAccountHbci::slotFloppyToggled(bool on) {
  assert(0);
}



void CfgTabPageAccountHbci::slotMountToggled(bool on){
  assert(0);
}



void CfgTabPageAccountHbci::slotFolderLostFocus() {
  assert(0);
}



void CfgTabPageAccountHbci::slotFolder(){
  assert(0);
}


#include "cfgtabpageaccounthbci.moc"





