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


#include "cfgtabpageaccountyn.h"
#include "cfgtabpageaccountyn.ui.h"

#include <aqyellownet/account.h>

#include <qbanking/qbanking.h>

#include <qmessagebox.h>
#include <qtimer.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qfiledialog.h>




CfgTabPageAccountYn::CfgTabPageAccountYn(QBanking *qb,
                                         AB_ACCOUNT *a,
                                         QWidget *parent,
                                         const char *name,
                                         WFlags f)
:QBCfgTabPageAccount(qb, "Yellownet", a, parent, name, f) {

  _realPage=new CfgTabPageAccountYnUi(this);

  setHelpSubject("CfgTabPageAccountYn");
  setDescription(tr("<p>This page contains "
                    "Yellownet-specific settings.</p>"));

  addWidget(_realPage);
  _realPage->show();

  QTimer::singleShot(0, this, SLOT(adjustSize()));
}



CfgTabPageAccountYn::~CfgTabPageAccountYn() {
}



bool CfgTabPageAccountYn::fromGui() {
  AB_ACCOUNT *a;
  std::string s;
  int i;

  a=getAccount();
  assert(a);

  i=_realPage->maxPurposeSpin->value();
  AY_Account_SetMaxPurposeLines(a, i);

  AY_Account_SetDebitAllowed(a, _realPage->debitNoteCheck->isChecked());

  return true;
}



bool CfgTabPageAccountYn::toGui() {
  AB_ACCOUNT *a;
  int i;

  a=getAccount();
  assert(a);

  i=AY_Account_GetMaxPurposeLines(a);
  if (i==0)
    i=4;
  _realPage->maxPurposeSpin->setValue(i);
  _realPage->debitNoteCheck->setChecked(AY_Account_GetDebitAllowed(a));

  return true;
}



bool CfgTabPageAccountYn::checkGui() {
  return true;
}






