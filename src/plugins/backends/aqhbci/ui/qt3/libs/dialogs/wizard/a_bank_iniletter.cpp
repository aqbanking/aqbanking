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


#include "a_bank_iniletter.h"
#include "wizard.h"
#include "iniletter.h"

#include <qbanking/qbanking.h>


ActionBankIniLetter::ActionBankIniLetter(Wizard *w)
:WizardAction(w, "BankIniLetter",
              QWidget::tr("Verify Server Key")) {

  _iniLetterDialog=new IniLetter(false, this, "BankIniLetter");
  addWidget(_iniLetterDialog);
}



ActionBankIniLetter::~ActionBankIniLetter() {
}



bool ActionBankIniLetter::apply() {
}



void ActionBankIniLetter::enter() {
  WizardInfo *wInfo;
  AH_BANK *b;
  AH_MEDIUM *m;
  int idx;
  int rv;
  const char *s;
  QString bankId;

  _iniLetterDialog->reset();

  wInfo=getWizard()->getWizardInfo();
  assert(wInfo);
  b=wInfo->getBank();
  assert(b);
  m=wInfo->getMedium();
  assert(m);

  if (!AH_Medium_IsMounted(m)) {
    rv=AH_Medium_Mount(m);
    if (rv) {
      DBG_ERROR(0, "Error mounting (%d)", rv);
      QMessageBox::critical(this,
                            tr("Mount Medium"),
                            tr("Could not mount medium"),
                            QMessageBox::Ok,QMessageBox::NoButton);
    }
    return;
  }

  idx=AH_User_GetContextIdx(u);
  rv=AH_Medium_SelectContext(m, idx);
  if (rv) {
    DBG_ERROR(0, "Could not select context %d (%d)", idx, rv);
    QMessageBox::critical(this,
                          tr("Mount Medium"),
                          tr("Could not select context"),
                          QMessageBox::Ok,QMessageBox::NoButton);
    return ;
  }

  _key=AH_Medium_GetPubSignKey(m);
  if (!_key) {
    DBG_WARN(0, "No sign key");
    _key=AH_Medium_GetPubCryptKey(m);
  }
  assert(_key);

  s=AH_Bank_GetBankId(b);
  if (s)
    bankId=QString::fromUtf8(s);

  _iniLetterDialog->init(bankId, _key);

}



bool ActionBankIniLetter::undo() {
  _iniLetterDialog->reset();

  if (_key) {
    GWEN_CryptKey_free(_key);
    _key=0;
  }

  return true;
}







