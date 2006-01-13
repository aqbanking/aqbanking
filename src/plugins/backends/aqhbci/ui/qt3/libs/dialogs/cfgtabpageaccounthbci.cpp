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
#include "cfgtabpageaccounthbci.ui.h"

#include <aqhbci/account.h>


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




CfgTabPageAccountHbci::CfgTabPageAccountHbci(QBanking *qb,
                                             AB_ACCOUNT *a,
                                             QWidget *parent,
                                             const char *name,
                                             WFlags f)
:QBCfgTabPageAccount(qb, "HBCI", a, parent, name, f) {

  _realPage=new CfgTabPageAccountHbciUi(this);
  setHelpSubject("CfgTabPageAccountHbci");
  setDescription(tr("<p>This page contains HBCI specific "
                    "account settings.</p>"));

  addWidget(_realPage);
  _realPage->show();

  QTimer::singleShot(0, this, SLOT(adjustSize()));
}



CfgTabPageAccountHbci::~CfgTabPageAccountHbci() {
}



bool CfgTabPageAccountHbci::fromGui() {
  AB_ACCOUNT *a;
  std::string s;

  a=getAccount();
  assert(a);

  if (_realPage->preferSingleTransferCheck->isChecked())
    AH_Account_AddFlags(a, AH_BANK_FLAGS_PREFER_SINGLE_TRANSFER);
  else
    AH_Account_SubFlags(a, AH_BANK_FLAGS_PREFER_SINGLE_TRANSFER);

  if (_realPage->preferSingleDebitNoteCheck->isChecked())
    AH_Account_AddFlags(a, AH_BANK_FLAGS_PREFER_SINGLE_DEBITNOTE);
  else
    AH_Account_SubFlags(a, AH_BANK_FLAGS_PREFER_SINGLE_DEBITNOTE);

  return true;
}



bool CfgTabPageAccountHbci::toGui() {
  AB_ACCOUNT *a;
  GWEN_TYPE_UINT32 aFlags;
  int i;

  a=getAccount();
  assert(a);

  aFlags=AH_Account_GetFlags(a);
  i=aFlags & AH_BANK_FLAGS_PREFER_SINGLE_TRANSFER;
  _realPage->preferSingleTransferCheck->setChecked(i);

  i=aFlags & AH_BANK_FLAGS_PREFER_SINGLE_DEBITNOTE;
  _realPage->preferSingleDebitNoteCheck->setChecked(i);

  return true;
}



bool CfgTabPageAccountHbci::checkGui() {
  return true;
}



