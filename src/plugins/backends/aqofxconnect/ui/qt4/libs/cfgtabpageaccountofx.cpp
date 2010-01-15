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


#include "cfgtabpageaccountofx.h"

#include <aqofxconnect/account.h>


#include <q4banking/qbanking.h>

#include <qmessagebox.h>
#include <qtimer.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <q3filedialog.h>




CfgTabPageAccountOfx::CfgTabPageAccountOfx(QBanking *qb,
                                           AB_ACCOUNT *a,
                                           QWidget *parent,
                                           const char *name,
                                           Qt::WFlags f)
:QBCfgTabPageAccount(qb, "OFX", a, parent, name, f) {

  _realPage.setupUi(this);

  setHelpSubject("CfgTabPageAccountOfx");
  setDescription(tr("<p>This page contains "
                    "OFX DirectConnect-specific settings.</p>"));

  QTimer::singleShot(0, this, SLOT(adjustSize()));
}



CfgTabPageAccountOfx::~CfgTabPageAccountOfx() {
}



bool CfgTabPageAccountOfx::fromGui() {
  AB_ACCOUNT *a;
  std::string s;
  int i;

  a=getAccount();
  assert(a);

  i=_realPage.maxPurposeSpin->value();
  AO_Account_SetMaxPurposeLines(a, i);

  AO_Account_SetDebitAllowed(a, _realPage.debitNoteCheck->isChecked());

  return true;
}



bool CfgTabPageAccountOfx::toGui() {
  AB_ACCOUNT *a;
  int i;

  a=getAccount();
  assert(a);

  i=AO_Account_GetMaxPurposeLines(a);
  if (i==0)
    i=4;
  _realPage.maxPurposeSpin->setValue(i);
  _realPage.debitNoteCheck->setChecked(AO_Account_GetDebitAllowed(a));

  return true;
}



bool CfgTabPageAccountOfx::checkGui() {
  return true;
}


#include "cfgtabpageaccountofx.moc"



