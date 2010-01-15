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


#include "a_bankiniletter.h"
#include "wizard.h"
#include "iniletter.h"

#include <aqhbci/user.h>

#include <q4banking/qbanking.h>

#include <qpushbutton.h>
#include <q3textview.h>

#include <gwenhywfar/debug.h>



ActionBankIniLetter::ActionBankIniLetter(Wizard *w)
:WizardAction(w, "BankIniLetter", QWidget::tr("Verify Bank Key")) {
  _iniLetterDialog=new IniLetter(false,
				 w->getWizardInfo()->getProvider(),
				 this,
				 "IniLetterDialog");
  addWidget(_iniLetterDialog);
  _iniLetterDialog->show();

  connect(_iniLetterDialog->goodHashButton, SIGNAL(clicked()),
          this, SLOT(slotGoodHash()));
  connect(_iniLetterDialog->badHashButton, SIGNAL(clicked()),
          this, SLOT(slotBadHash()));
  connect(_iniLetterDialog->printButton, SIGNAL(clicked()),
          this, SLOT(slotPrint()));
}



ActionBankIniLetter::~ActionBankIniLetter() {
}



void ActionBankIniLetter::enter() {
  Wizard *w;
  WizardInfo *wi;
  AB_USER *u;

  setNextEnabled(false);
  w=getWizard();
  wi=w->getWizardInfo();
  u=wi->getUser();
  assert(u);

  if (!_iniLetterDialog->init(u)) {
    DBG_ERROR(0, "Could not init dialog");
    return;
  }
}



void ActionBankIniLetter::slotGoodHash() {
  setNextEnabled(true);
}



void ActionBankIniLetter::slotBadHash() {
}



void ActionBankIniLetter::slotPrint() {
  int rv;

  rv=getWizard()->getBanking()->print(tr("Bank's Ini Letter"),
                                      QString("BANK::INILETTER"),
                                      tr("This page contains the bank's "
                                         "iniletter."),
                                      _iniLetterDialog->iniBrowser->text());
  if (rv) {
    DBG_ERROR(0, "Could not print iniletter (%d)", rv);
  }
}










#include "a_bankiniletter.moc"


